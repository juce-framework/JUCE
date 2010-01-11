/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
#include "juce_FileInputStream.h"
#include "juce_FileOutputStream.h"
#include "../../threads/juce_ScopedLock.h"

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
          inputStream (0)
    {
        inputStream = file_.inputStream;

        if (file_.inputSource != 0)
        {
            inputStream = file.inputSource->createInputStream();
        }
        else
        {
#ifdef JUCE_DEBUG
            file_.numOpenStreams++;
#endif
        }

        char buffer [30];

        if (inputStream != 0
             && inputStream->setPosition (zei.streamOffset)
             && inputStream->read (buffer, 30) == 30
             && ByteOrder::littleEndianInt (buffer) == 0x04034b50)
        {
            headerSize = 30 + ByteOrder::littleEndianShort (buffer + 26)
                            + ByteOrder::littleEndianShort (buffer + 28);
        }
    }

    ~ZipInputStream() throw()
    {
#ifdef JUCE_DEBUG
        if (inputStream != 0 && inputStream == file.inputStream)
            file.numOpenStreams--;
#endif

        if (inputStream != file.inputStream)
            delete inputStream;
    }

    int64 getTotalLength() throw()
    {
        return zipEntryInfo.compressedSize;
    }

    int read (void* buffer, int howMany) throw()
    {
        if (headerSize <= 0)
            return 0;

        howMany = (int) jmin ((int64) howMany, zipEntryInfo.compressedSize - pos);

        if (inputStream == 0)
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

    bool isExhausted() throw()
    {
        return headerSize <= 0 || pos >= zipEntryInfo.compressedSize;
    }

    int64 getPosition() throw()
    {
        return pos;
    }

    bool setPosition (int64 newPos) throw()
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

    ZipInputStream (const ZipInputStream&);
    const ZipInputStream& operator= (const ZipInputStream&);
};


//==============================================================================
ZipFile::ZipFile (InputStream* const source_,
                  const bool deleteStreamWhenDestroyed) throw()
   : inputStream (source_)
#ifdef JUCE_DEBUG
     , numOpenStreams (0)
#endif
{
    if (deleteStreamWhenDestroyed)
        streamToDelete = inputStream;

    init();
}

ZipFile::ZipFile (const File& file)
    : inputStream (0)
#ifdef JUCE_DEBUG
      , numOpenStreams (0)
#endif
{
    inputSource = new FileInputSource (file);
    init();
}

ZipFile::ZipFile (InputSource* const inputSource_)
    : inputStream (0),
      inputSource (inputSource_)
#ifdef JUCE_DEBUG
      , numOpenStreams (0)
#endif
{
    init();
}

ZipFile::~ZipFile() throw()
{
#ifdef JUCE_DEBUG
    entries.clear();

    // If you hit this assertion, it means you've created a stream to read
    // one of the items in the zipfile, but you've forgotten to delete that
    // stream object before deleting the file.. Streams can't be kept open
    // after the file is deleted because they need to share the input
    // stream that the file uses to read itself.
    jassert (numOpenStreams == 0);
#endif
}

//==============================================================================
int ZipFile::getNumEntries() const throw()
{
    return entries.size();
}

const ZipFile::ZipEntry* ZipFile::getEntry (const int index) const throw()
{
    ZipEntryInfo* const zei = (ZipEntryInfo*) entries [index];

    return (zei != 0) ? &(zei->entry)
                      : 0;
}

int ZipFile::getIndexOfFileName (const String& fileName) const throw()
{
    for (int i = 0; i < entries.size(); ++i)
        if (entries.getUnchecked (i)->entry.filename == fileName)
            return i;

    return -1;
}

const ZipFile::ZipEntry* ZipFile::getEntry (const String& fileName) const throw()
{
    return getEntry (getIndexOfFileName (fileName));
}

InputStream* ZipFile::createStreamForEntry (const int index)
{
    ZipEntryInfo* const zei = entries[index];
    InputStream* stream = 0;

    if (zei != 0)
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

    if (inputSource != 0)
    {
        in = inputSource->createInputStream();
        toDelete = in;
    }

    if (in != 0)
    {
        int numEntries = 0;
        int pos = findEndOfZipEntryTable (in, numEntries);

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

                    const char* const buffer = ((const char*) headerData.getData()) + pos;

                    const int fileNameLen = ByteOrder::littleEndianShort (buffer + 28);

                    if (pos + 46 + fileNameLen > size)
                        break;

                    ZipEntryInfo* const zei = new ZipEntryInfo();
                    zei->entry.filename = String::fromUTF8 ((const uint8*) buffer + 46, fileNameLen);

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

int ZipFile::findEndOfZipEntryTable (InputStream* input, int& numEntries)
{
    BufferedInputStream in (input, 8192, false);

    in.setPosition (in.getTotalLength());
    int64 pos = in.getPosition();
    const int64 lowestPos = jmax ((int64) 0, pos - 1024);

    char buffer [32];
    zeromem (buffer, sizeof (buffer));

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

void ZipFile::uncompressTo (const File& targetDirectory,
                            const bool shouldOverwriteFiles)
{
    for (int i = 0; i < entries.size(); ++i)
    {
        const ZipEntry& zei = entries.getUnchecked(i)->entry;

        const File targetFile (targetDirectory.getChildFile (zei.filename));

        if (zei.filename.endsWithChar (T('/')))
        {
            targetFile.createDirectory(); // (entry is a directory, not a file)
        }
        else
        {
            ScopedPointer <InputStream> in (createStreamForEntry (i));

            if (in != 0)
            {
                if (shouldOverwriteFiles)
                    targetFile.deleteFile();

                if ((! targetFile.exists())
                     && targetFile.getParentDirectory().createDirectory())
                {
                    ScopedPointer <FileOutputStream> out (targetFile.createOutputStream());

                    if (out != 0)
                    {
                        out->writeFromInputStream (*in, -1);
                        out = 0;

                        targetFile.setCreationTime (zei.fileTime);
                        targetFile.setLastModificationTime (zei.fileTime);
                        targetFile.setLastAccessTime (zei.fileTime);
                    }
                }
            }
        }
    }
}

END_JUCE_NAMESPACE
