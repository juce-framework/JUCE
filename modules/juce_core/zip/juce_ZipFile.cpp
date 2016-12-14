/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

class ZipFile::ZipEntryHolder
{
public:
    ZipEntryHolder (const char* const buffer, const int fileNameLen)
    {
        isCompressed            = ByteOrder::littleEndianShort (buffer + 10) != 0;
        entry.fileTime          = parseFileTime ((uint32) ByteOrder::littleEndianShort (buffer + 12),
                                                 (uint32) ByteOrder::littleEndianShort (buffer + 14));
        compressedSize          = (int64) (uint32) ByteOrder::littleEndianInt (buffer + 20);
        entry.uncompressedSize  = (int64) (uint32) ByteOrder::littleEndianInt (buffer + 24);
        streamOffset            = (int64) (uint32) ByteOrder::littleEndianInt (buffer + 42);
        entry.filename          = String::fromUTF8 (buffer + 46, fileNameLen);
    }

    struct FileNameComparator
    {
        static int compareElements (const ZipEntryHolder* e1, const ZipEntryHolder* e2) noexcept
        {
            return e1->entry.filename.compare (e2->entry.filename);
        }
    };

    ZipEntry entry;
    int64 streamOffset, compressedSize;
    bool isCompressed;

private:
    static Time parseFileTime (uint32 time, uint32 date) noexcept
    {
        const int year      = 1980 + (date >> 9);
        const int month     = ((date >> 5) & 15) - 1;
        const int day       = date & 31;
        const int hours     = time >> 11;
        const int minutes   = (time >> 5) & 63;
        const int seconds   = (int) ((time & 31) << 1);

        return Time (year, month, day, hours, minutes, seconds);
    }
};

//==============================================================================
namespace
{
    int findEndOfZipEntryTable (InputStream& input, int& numEntries)
    {
        BufferedInputStream in (input, 8192);

        in.setPosition (in.getTotalLength());
        int64 pos = in.getPosition();
        const int64 lowestPos = jmax ((int64) 0, pos - 1024);

        char buffer [32] = { 0 };

        while (pos > lowestPos)
        {
            in.setPosition (pos - 22);
            pos = in.getPosition();
            memcpy (buffer + 22, buffer, 4);

            if (in.read (buffer, 22) != 22)
                return 0;

            for (int i = 0; i < 22; ++i)
            {
                if (ByteOrder::littleEndianInt (buffer + i) == 0x06054b50)
                {
                    in.setPosition (pos + i);
                    in.read (buffer, 22);
                    numEntries = ByteOrder::littleEndianShort (buffer + 10);

                    return (int) ByteOrder::littleEndianInt (buffer + 16);
                }
            }
        }

        return 0;
    }
}

//==============================================================================
class ZipFile::ZipInputStream  : public InputStream
{
public:
    ZipInputStream (ZipFile& zf, ZipFile::ZipEntryHolder& zei)
        : file (zf),
          zipEntryHolder (zei),
          pos (0),
          headerSize (0),
          inputStream (zf.inputStream)
    {
        if (zf.inputSource != nullptr)
        {
            inputStream = streamToDelete = file.inputSource->createInputStream();
        }
        else
        {
           #if JUCE_DEBUG
            zf.streamCounter.numOpenStreams++;
           #endif
        }

        char buffer [30];

        if (inputStream != nullptr
             && inputStream->setPosition (zei.streamOffset)
             && inputStream->read (buffer, 30) == 30
             && ByteOrder::littleEndianInt (buffer) == 0x04034b50)
        {
            headerSize = 30 + ByteOrder::littleEndianShort (buffer + 26)
                            + ByteOrder::littleEndianShort (buffer + 28);
        }
    }

    ~ZipInputStream()
    {
       #if JUCE_DEBUG
        if (inputStream != nullptr && inputStream == file.inputStream)
            file.streamCounter.numOpenStreams--;
       #endif
    }

    int64 getTotalLength() override
    {
        return zipEntryHolder.compressedSize;
    }

    int read (void* buffer, int howMany) override
    {
        if (headerSize <= 0)
            return 0;

        howMany = (int) jmin ((int64) howMany, zipEntryHolder.compressedSize - pos);

        if (inputStream == nullptr)
            return 0;

        int num;

        if (inputStream == file.inputStream)
        {
            const ScopedLock sl (file.lock);
            inputStream->setPosition (pos + zipEntryHolder.streamOffset + headerSize);
            num = inputStream->read (buffer, howMany);
        }
        else
        {
            inputStream->setPosition (pos + zipEntryHolder.streamOffset + headerSize);
            num = inputStream->read (buffer, howMany);
        }

        pos += num;
        return num;
    }

    bool isExhausted() override
    {
        return headerSize <= 0 || pos >= zipEntryHolder.compressedSize;
    }

    int64 getPosition() override
    {
        return pos;
    }

    bool setPosition (int64 newPos) override
    {
        pos = jlimit ((int64) 0, zipEntryHolder.compressedSize, newPos);
        return true;
    }

private:
    ZipFile& file;
    ZipEntryHolder zipEntryHolder;
    int64 pos;
    int headerSize;
    InputStream* inputStream;
    ScopedPointer<InputStream> streamToDelete;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZipInputStream)
};


//==============================================================================
ZipFile::ZipFile (InputStream* const stream, const bool deleteStreamWhenDestroyed)
   : inputStream (stream)
{
    if (deleteStreamWhenDestroyed)
        streamToDelete = inputStream;

    init();
}

ZipFile::ZipFile (InputStream& stream)
   : inputStream (&stream)
{
    init();
}

ZipFile::ZipFile (const File& file)
    : inputStream (nullptr),
      inputSource (new FileInputSource (file))
{
    init();
}

ZipFile::ZipFile (InputSource* const source)
    : inputStream (nullptr),
      inputSource (source)
{
    init();
}

ZipFile::~ZipFile()
{
    entries.clear();
}

#if JUCE_DEBUG
ZipFile::OpenStreamCounter::~OpenStreamCounter()
{
    /* If you hit this assertion, it means you've created a stream to read one of the items in the
       zipfile, but you've forgotten to delete that stream object before deleting the file..
       Streams can't be kept open after the file is deleted because they need to share the input
       stream that is managed by the ZipFile object.
    */
    jassert (numOpenStreams == 0);
}
#endif

//==============================================================================
int ZipFile::getNumEntries() const noexcept
{
    return entries.size();
}

const ZipFile::ZipEntry* ZipFile::getEntry (const int index) const noexcept
{
    if (ZipEntryHolder* const zei = entries [index])
        return &(zei->entry);

    return nullptr;
}

int ZipFile::getIndexOfFileName (const String& fileName) const noexcept
{
    for (int i = 0; i < entries.size(); ++i)
        if (entries.getUnchecked (i)->entry.filename == fileName)
            return i;

    return -1;
}

const ZipFile::ZipEntry* ZipFile::getEntry (const String& fileName) const noexcept
{
    return getEntry (getIndexOfFileName (fileName));
}

InputStream* ZipFile::createStreamForEntry (const int index)
{
    InputStream* stream = nullptr;

    if (ZipEntryHolder* const zei = entries[index])
    {
        stream = new ZipInputStream (*this, *zei);

        if (zei->isCompressed)
        {
            stream = new GZIPDecompressorInputStream (stream, true,
                                                      GZIPDecompressorInputStream::deflateFormat,
                                                      zei->entry.uncompressedSize);

            // (much faster to unzip in big blocks using a buffer..)
            stream = new BufferedInputStream (stream, 32768, true);
        }
    }

    return stream;
}

InputStream* ZipFile::createStreamForEntry (const ZipEntry& entry)
{
    for (int i = 0; i < entries.size(); ++i)
        if (&entries.getUnchecked (i)->entry == &entry)
            return createStreamForEntry (i);

    return nullptr;
}

void ZipFile::sortEntriesByFilename()
{
    ZipEntryHolder::FileNameComparator sorter;
    entries.sort (sorter);
}

//==============================================================================
void ZipFile::init()
{
    ScopedPointer<InputStream> toDelete;
    InputStream* in = inputStream;

    if (inputSource != nullptr)
    {
        in = inputSource->createInputStream();
        toDelete = in;
    }

    if (in != nullptr)
    {
        int numEntries = 0;
        int pos = findEndOfZipEntryTable (*in, numEntries);

        if (pos >= 0 && pos < in->getTotalLength())
        {
            const int size = (int) (in->getTotalLength() - pos);

            in->setPosition (pos);
            MemoryBlock headerData;

            if (in->readIntoMemoryBlock (headerData, size) == (size_t) size)
            {
                pos = 0;

                for (int i = 0; i < numEntries; ++i)
                {
                    if (pos + 46 > size)
                        break;

                    const char* const buffer = static_cast<const char*> (headerData.getData()) + pos;

                    const int fileNameLen = ByteOrder::littleEndianShort (buffer + 28);

                    if (pos + 46 + fileNameLen > size)
                        break;

                    entries.add (new ZipEntryHolder (buffer, fileNameLen));

                    pos += 46 + fileNameLen
                            + ByteOrder::littleEndianShort (buffer + 30)
                            + ByteOrder::littleEndianShort (buffer + 32);
                }
            }
        }
    }
}

Result ZipFile::uncompressTo (const File& targetDirectory,
                              const bool shouldOverwriteFiles)
{
    for (int i = 0; i < entries.size(); ++i)
    {
        Result result (uncompressEntry (i, targetDirectory, shouldOverwriteFiles));
        if (result.failed())
            return result;
    }

    return Result::ok();
}

Result ZipFile::uncompressEntry (const int index,
                                 const File& targetDirectory,
                                 bool shouldOverwriteFiles)
{
    const ZipEntryHolder* zei = entries.getUnchecked (index);

   #if JUCE_WINDOWS
    const String entryPath (zei->entry.filename);
   #else
    const String entryPath (zei->entry.filename.replaceCharacter ('\\', '/'));
   #endif

    const File targetFile (targetDirectory.getChildFile (entryPath));

    if (entryPath.endsWithChar ('/') || entryPath.endsWithChar ('\\'))
        return targetFile.createDirectory(); // (entry is a directory, not a file)

    ScopedPointer<InputStream> in (createStreamForEntry (index));

    if (in == nullptr)
        return Result::fail ("Failed to open the zip file for reading");

    if (targetFile.exists())
    {
        if (! shouldOverwriteFiles)
            return Result::ok();

        if (! targetFile.deleteFile())
            return Result::fail ("Failed to write to target file: " + targetFile.getFullPathName());
    }

    if (! targetFile.getParentDirectory().createDirectory())
        return Result::fail ("Failed to create target folder: " + targetFile.getParentDirectory().getFullPathName());

    {
        FileOutputStream out (targetFile);

        if (out.failedToOpen())
            return Result::fail ("Failed to write to target file: " + targetFile.getFullPathName());

        out << *in;
    }

    targetFile.setCreationTime (zei->entry.fileTime);
    targetFile.setLastModificationTime (zei->entry.fileTime);
    targetFile.setLastAccessTime (zei->entry.fileTime);

    return Result::ok();
}


//==============================================================================
class ZipFile::Builder::Item
{
public:
    Item (const File& f, InputStream* s, int compression, const String& storedPath, Time time)
        : file (f), stream (s), storedPathname (storedPath), fileTime (time),
          compressedSize (0), uncompressedSize (0), headerStart (0),
          compressionLevel (compression), checksum (0)
    {
    }

    bool writeData (OutputStream& target, const int64 overallStartPosition)
    {
        MemoryOutputStream compressedData ((size_t) file.getSize());

        if (compressionLevel > 0)
        {
            GZIPCompressorOutputStream compressor (&compressedData, compressionLevel, false,
                                                   GZIPCompressorOutputStream::windowBitsRaw);
            if (! writeSource (compressor))
                return false;
        }
        else
        {
            if (! writeSource (compressedData))
                return false;
        }

        compressedSize = (int64) compressedData.getDataSize();
        headerStart = target.getPosition() - overallStartPosition;

        target.writeInt (0x04034b50);
        writeFlagsAndSizes (target);
        target << storedPathname
               << compressedData;

        return true;
    }

    bool writeDirectoryEntry (OutputStream& target)
    {
        target.writeInt (0x02014b50);
        target.writeShort (20); // version written
        writeFlagsAndSizes (target);
        target.writeShort (0); // comment length
        target.writeShort (0); // start disk num
        target.writeShort (0); // internal attributes
        target.writeInt (0); // external attributes
        target.writeInt ((int) (uint32) headerStart);
        target << storedPathname;

        return true;
    }

private:
    const File file;
    ScopedPointer<InputStream> stream;
    String storedPathname;
    Time fileTime;
    int64 compressedSize, uncompressedSize, headerStart;
    int compressionLevel;
    unsigned long checksum;

    static void writeTimeAndDate (OutputStream& target, Time t)
    {
        target.writeShort ((short) (t.getSeconds() + (t.getMinutes() << 5) + (t.getHours() << 11)));
        target.writeShort ((short) (t.getDayOfMonth() + ((t.getMonth() + 1) << 5) + ((t.getYear() - 1980) << 9)));
    }

    bool writeSource (OutputStream& target)
    {
        if (stream == nullptr)
        {
            stream = file.createInputStream();

            if (stream == nullptr)
                return false;
        }

        checksum = 0;
        uncompressedSize = 0;
        const int bufferSize = 4096;
        HeapBlock<unsigned char> buffer (bufferSize);

        while (! stream->isExhausted())
        {
            const int bytesRead = stream->read (buffer, bufferSize);

            if (bytesRead < 0)
                return false;

            checksum = zlibNamespace::crc32 (checksum, buffer, (unsigned int) bytesRead);
            target.write (buffer, (size_t) bytesRead);
            uncompressedSize += bytesRead;
        }

        stream = nullptr;
        return true;
    }

    void writeFlagsAndSizes (OutputStream& target) const
    {
        target.writeShort (10); // version needed
        target.writeShort ((short) (1 << 11)); // this flag indicates UTF-8 filename encoding
        target.writeShort (compressionLevel > 0 ? (short) 8 : (short) 0);
        writeTimeAndDate (target, fileTime);
        target.writeInt ((int) checksum);
        target.writeInt ((int) (uint32) compressedSize);
        target.writeInt ((int) (uint32) uncompressedSize);
        target.writeShort ((short) storedPathname.toUTF8().sizeInBytes() - 1);
        target.writeShort (0); // extra field length
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Item)
};

//==============================================================================
ZipFile::Builder::Builder() {}
ZipFile::Builder::~Builder() {}

void ZipFile::Builder::addFile (const File& file, const int compression, const String& path)
{
    items.add (new Item (file, nullptr, compression,
                         path.isEmpty() ? file.getFileName() : path,
                         file.getLastModificationTime()));
}

void ZipFile::Builder::addEntry (InputStream* stream, int compression, const String& path, Time time)
{
    jassert (stream != nullptr); // must not be null!
    jassert (path.isNotEmpty());
    items.add (new Item (File(), stream, compression, path, time));
}

bool ZipFile::Builder::writeToStream (OutputStream& target, double* const progress) const
{
    const int64 fileStart = target.getPosition();

    for (int i = 0; i < items.size(); ++i)
    {
        if (progress != nullptr)
            *progress = (i + 0.5) / items.size();

        if (! items.getUnchecked (i)->writeData (target, fileStart))
            return false;
    }

    const int64 directoryStart = target.getPosition();

    for (int i = 0; i < items.size(); ++i)
        if (! items.getUnchecked (i)->writeDirectoryEntry (target))
            return false;

    const int64 directoryEnd = target.getPosition();

    target.writeInt (0x06054b50);
    target.writeShort (0);
    target.writeShort (0);
    target.writeShort ((short) items.size());
    target.writeShort ((short) items.size());
    target.writeInt ((int) (directoryEnd - directoryStart));
    target.writeInt ((int) (directoryStart - fileStart));
    target.writeShort (0);

    if (progress != nullptr)
        *progress = 1.0;

    return true;
}
