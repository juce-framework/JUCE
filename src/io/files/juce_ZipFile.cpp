/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ZipFile.h"
#include "../streams/juce_GZIPDecompressorInputStream.h"
#include "../streams/juce_BufferedInputStream.h"
#include "../streams/juce_FileInputSource.h"
#include "../streams/juce_MemoryOutputStream.h"
#include "../streams/juce_GZIPCompressorOutputStream.h"
#include "juce_FileInputStream.h"
#include "juce_FileOutputStream.h"


//==============================================================================
class ZipFile::ZipEntryInfo
{
public:
    ZipFile::ZipEntry entry;
    int streamOffset;
    int compressedSize;
    bool compressed;
};

//==============================================================================
class ZipFile::ZipInputStream  : public InputStream
{
public:
    //==============================================================================
    ZipInputStream (ZipFile& file_, ZipFile::ZipEntryInfo& zei)
        : file (file_),
          zipEntryInfo (zei),
          pos (0),
          headerSize (0),
          inputStream (file_.inputStream)
    {
        if (file_.inputSource != nullptr)
        {
            inputStream = streamToDelete = file.inputSource->createInputStream();
        }
        else
        {
           #if JUCE_DEBUG
            file_.numOpenStreams++;
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
            file.numOpenStreams--;
       #endif
    }

    int64 getTotalLength()
    {
        return zipEntryInfo.compressedSize;
    }

    int read (void* buffer, int howMany)
    {
        if (headerSize <= 0)
            return 0;

        howMany = (int) jmin ((int64) howMany, zipEntryInfo.compressedSize - pos);

        if (inputStream == nullptr)
            return 0;

        int num;

        if (inputStream == file.inputStream)
        {
            const ScopedLock sl (file.lock);
            inputStream->setPosition (pos + zipEntryInfo.streamOffset + headerSize);
            num = inputStream->read (buffer, howMany);
        }
        else
        {
            inputStream->setPosition (pos + zipEntryInfo.streamOffset + headerSize);
            num = inputStream->read (buffer, howMany);
        }

        pos += num;
        return num;
    }

    bool isExhausted()
    {
        return headerSize <= 0 || pos >= zipEntryInfo.compressedSize;
    }

    int64 getPosition()
    {
        return pos;
    }

    bool setPosition (int64 newPos)
    {
        pos = jlimit ((int64) 0, (int64) zipEntryInfo.compressedSize, newPos);
        return true;
    }


private:
    //==============================================================================
    ZipFile& file;
    ZipEntryInfo zipEntryInfo;
    int64 pos;
    int headerSize;
    InputStream* inputStream;
    ScopedPointer<InputStream> streamToDelete;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZipInputStream);
};


//==============================================================================
ZipFile::ZipFile (InputStream* const source_, const bool deleteStreamWhenDestroyed)
   : inputStream (source_)
    #if JUCE_DEBUG
     , numOpenStreams (0)
    #endif
{
    if (deleteStreamWhenDestroyed)
        streamToDelete = inputStream;

    init();
}

ZipFile::ZipFile (const File& file)
    : inputStream (nullptr)
     #if JUCE_DEBUG
      , numOpenStreams (0)
     #endif
{
    inputSource = new FileInputSource (file);
    init();
}

ZipFile::ZipFile (InputSource* const inputSource_)
    : inputStream (nullptr),
      inputSource (inputSource_)
     #if JUCE_DEBUG
      , numOpenStreams (0)
     #endif
{
    init();
}

ZipFile::~ZipFile()
{
   #if JUCE_DEBUG
    entries.clear();

    /* If you hit this assertion, it means you've created a stream to read one of the items in the
       zipfile, but you've forgotten to delete that stream object before deleting the file..
       Streams can't be kept open after the file is deleted because they need to share the input
       stream that the file uses to read itself.
    */
    jassert (numOpenStreams == 0);
   #endif
}

//==============================================================================
int ZipFile::getNumEntries() const noexcept
{
    return entries.size();
}

const ZipFile::ZipEntry* ZipFile::getEntry (const int index) const noexcept
{
    ZipEntryInfo* const zei = entries [index];
    return zei != nullptr ? &(zei->entry) : nullptr;
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
    ZipEntryInfo* const zei = entries[index];
    InputStream* stream = nullptr;

    if (zei != nullptr)
    {
        stream = new ZipInputStream (*this, *zei);

        if (zei->compressed)
        {
            stream = new GZIPDecompressorInputStream (stream, true, true,
                                                      zei->entry.uncompressedSize);

            // (much faster to unzip in big blocks using a buffer..)
            stream = new BufferedInputStream (stream, 32768, true);
        }
    }

    return stream;
}

class ZipFile::ZipFilenameComparator
{
public:
    int compareElements (const ZipFile::ZipEntryInfo* first, const ZipFile::ZipEntryInfo* second)
    {
        return first->entry.filename.compare (second->entry.filename);
    }
};

void ZipFile::sortEntriesByFilename()
{
    ZipFilenameComparator sorter;
    entries.sort (sorter);
}

//==============================================================================
void ZipFile::init()
{
    ScopedPointer <InputStream> toDelete;
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

            if (in->readIntoMemoryBlock (headerData, size) == size)
            {
                pos = 0;

                for (int i = 0; i < numEntries; ++i)
                {
                    if (pos + 46 > size)
                        break;

                    const char* const buffer = static_cast <const char*> (headerData.getData()) + pos;

                    const int fileNameLen = ByteOrder::littleEndianShort (buffer + 28);

                    if (pos + 46 + fileNameLen > size)
                        break;

                    ZipEntryInfo* const zei = new ZipEntryInfo();
                    zei->entry.filename = String::fromUTF8 (buffer + 46, fileNameLen);

                    const int time = ByteOrder::littleEndianShort (buffer + 12);
                    const int date = ByteOrder::littleEndianShort (buffer + 14);

                    const int year      = 1980 + (date >> 9);
                    const int month     = ((date >> 5) & 15) - 1;
                    const int day       = date & 31;
                    const int hours     = time >> 11;
                    const int minutes   = (time >> 5) & 63;
                    const int seconds   = (time & 31) << 1;

                    zei->entry.fileTime = Time (year, month, day, hours, minutes, seconds);

                    zei->compressed = ByteOrder::littleEndianShort (buffer + 10) != 0;
                    zei->compressedSize = ByteOrder::littleEndianInt (buffer + 20);
                    zei->entry.uncompressedSize = ByteOrder::littleEndianInt (buffer + 24);

                    zei->streamOffset = ByteOrder::littleEndianInt (buffer + 42);
                    entries.add (zei);

                    pos += 46 + fileNameLen
                            + ByteOrder::littleEndianShort (buffer + 30)
                            + ByteOrder::littleEndianShort (buffer + 32);
                }
            }
        }
    }
}

int ZipFile::findEndOfZipEntryTable (InputStream& input, int& numEntries)
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

                return ByteOrder::littleEndianInt (buffer + 16);
            }
        }
    }

    return 0;
}

bool ZipFile::uncompressTo (const File& targetDirectory,
                            const bool shouldOverwriteFiles)
{
    for (int i = 0; i < entries.size(); ++i)
        if (! uncompressEntry (i, targetDirectory, shouldOverwriteFiles))
            return false;

    return true;
}

bool ZipFile::uncompressEntry (const int index,
                               const File& targetDirectory,
                               bool shouldOverwriteFiles)
{
    const ZipEntryInfo* zei = entries [index];

    if (zei != nullptr)
    {
        const File targetFile (targetDirectory.getChildFile (zei->entry.filename));

        if (zei->entry.filename.endsWithChar ('/'))
        {
            return targetFile.createDirectory(); // (entry is a directory, not a file)
        }
        else
        {
            ScopedPointer<InputStream> in (createStreamForEntry (index));

            if (in != nullptr)
            {
                if (shouldOverwriteFiles && ! targetFile.deleteFile())
                    return false;

                if ((! targetFile.exists()) && targetFile.getParentDirectory().createDirectory())
                {
                    ScopedPointer<FileOutputStream> out (targetFile.createOutputStream());

                    if (out != nullptr)
                    {
                        out->writeFromInputStream (*in, -1);
                        out = nullptr;

                        targetFile.setCreationTime (zei->entry.fileTime);
                        targetFile.setLastModificationTime (zei->entry.fileTime);
                        targetFile.setLastAccessTime (zei->entry.fileTime);

                        return true;
                    }
                }
            }
        }
    }

    return false;
}


//=============================================================================
extern unsigned long juce_crc32 (unsigned long crc, const unsigned char* buf, unsigned len);

class ZipFile::Builder::Item
{
public:
    Item (const File& file_, const int compressionLevel_, const String& storedPathName_)
        : file (file_),
          storedPathname (storedPathName_.isEmpty() ? file_.getFileName() : storedPathName_),
          compressionLevel (compressionLevel_),
          compressedSize (0),
          headerStart (0)
    {
    }

    bool writeData (OutputStream& target, const int64 overallStartPosition)
    {
        MemoryOutputStream compressedData;

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

        compressedSize = (int) compressedData.getDataSize();
        headerStart = (int) (target.getPosition() - overallStartPosition);

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
        target.writeInt (headerStart);
        target << storedPathname;

        return true;
    }

private:
    const File file;
    String storedPathname;
    int compressionLevel, compressedSize, headerStart;
    uint32 checksum;

    void writeTimeAndDate (OutputStream& target) const
    {
        const Time t (file.getLastModificationTime());
        target.writeShort ((short) (t.getSeconds() + (t.getMinutes() << 5) + (t.getHours() << 11)));
        target.writeShort ((short) (t.getDayOfMonth() + ((t.getMonth() + 1) << 5) + ((t.getYear() - 1980) << 9)));
    }

    bool writeSource (OutputStream& target)
    {
        checksum = 0;
        ScopedPointer<FileInputStream> input (file.createInputStream());

        if (input == nullptr)
            return false;

        const int bufferSize = 2048;
        HeapBlock<unsigned char> buffer (bufferSize);

        while (! input->isExhausted())
        {
            const int bytesRead = input->read (buffer, bufferSize);

            if (bytesRead < 0)
                return false;

            checksum = juce_crc32 (checksum, buffer, bytesRead);
            target.write (buffer, bytesRead);
        }

        return true;
    }

    void writeFlagsAndSizes (OutputStream& target) const
    {
        target.writeShort (10); // version needed
        target.writeShort (0); // flags
        target.writeShort (compressionLevel > 0 ? (short) 8 : (short) 0);
        writeTimeAndDate (target);
        target.writeInt (checksum);
        target.writeInt (compressedSize);
        target.writeInt ((int) file.getSize());
        target.writeShort ((short) storedPathname.toUTF8().sizeInBytes() - 1);
        target.writeShort (0); // extra field length
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Item);
};

//=============================================================================
ZipFile::Builder::Builder() {}
ZipFile::Builder::~Builder() {}

void ZipFile::Builder::addFile (const File& fileToAdd, const int compressionLevel, const String& storedPathName)
{
    items.add (new Item (fileToAdd, compressionLevel, storedPathName));
}

bool ZipFile::Builder::writeToStream (OutputStream& target) const
{
    const int64 fileStart = target.getPosition();

    int i;
    for (i = 0; i < items.size(); ++i)
        if (! items.getUnchecked (i)->writeData (target, fileStart))
            return false;

    const int64 directoryStart = target.getPosition();

    for (i = 0; i < items.size(); ++i)
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
    target.flush();

    return true;
}


END_JUCE_NAMESPACE
