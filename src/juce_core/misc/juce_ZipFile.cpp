/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_ZipFile.h"
#include "../io/streams/juce_GZIPDecompressorInputStream.h"
#include "../io/streams/juce_BufferedInputStream.h"
#include "../io/files/juce_FileInputStream.h"
#include "../io/files/juce_FileOutputStream.h"
#include "../threads/juce_ScopedLock.h"


//==============================================================================
struct ZipEntryInfo
{
    ZipFile::ZipEntry entry;
    int streamOffset;
    int compressedSize;
    bool compressed;
};


//==============================================================================
class ZipInputStream  : public InputStream
{
public:
    //==============================================================================
    ZipInputStream (ZipFile& file_,
                    ZipEntryInfo& zei) throw()
        : file (file_),
          zipEntryInfo (zei),
          pos (0),
          headerSize (0)
    {
        char buffer [30];

        if (file.source->setPosition (zei.streamOffset)
             && file.source->read (buffer, 30) == 30
             && littleEndianInt (buffer) == 0x04034b50)
        {
            headerSize = 30 + littleEndianShort (buffer + 26)
                            + littleEndianShort (buffer + 28);
        }
    }

    ~ZipInputStream() throw()
    {
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

        const ScopedLock sl (file.lock);

        file.source->setPosition (pos + zipEntryInfo.streamOffset + headerSize);
        const int num = file.source->read (buffer, howMany);

        pos += num;
        return num;
    }

    bool isExhausted() throw()
    {
        return pos >= zipEntryInfo.compressedSize;
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
    ZipEntryInfo& zipEntryInfo;
    int64 pos;
    int headerSize;

    ZipInputStream (const ZipInputStream&);
    const ZipInputStream& operator= (const ZipInputStream&);
};


//==============================================================================
ZipFile::ZipFile (InputStream* const source_,
                  const bool deleteStreamWhenDestroyed_) throw()
   : source (source_),
     deleteStreamWhenDestroyed (deleteStreamWhenDestroyed_)
{
    init();
}

ZipFile::ZipFile (const File& file)
    : deleteStreamWhenDestroyed (true)
{
    source = file.createInputStream();
    init();
}

ZipFile::~ZipFile() throw()
{
    for (int i = entries.size(); --i >= 0;)
    {
        ZipEntryInfo* const zei = (ZipEntryInfo*)(entries [i]);
        delete zei;
    }

    if (deleteStreamWhenDestroyed && (source != 0))
        delete source;
}

//==============================================================================
int ZipFile::getNumEntries() const throw()
{
    return entries.size();
}

const ZipFile::ZipEntry* ZipFile::getEntry (const int index) const throw()
{
    ZipEntryInfo* const zei = (ZipEntryInfo*)(entries [index]);

    return (zei != 0) ? &(zei->entry)
                      : 0;
}

int ZipFile::getIndexOfFileName (const String& fileName) const throw()
{
    for (int i = 0; i < entries.size(); ++i)
        if (((ZipEntryInfo*) entries.getUnchecked (i))->entry.filename == fileName)
            return i;

    return -1;
}

const ZipFile::ZipEntry* ZipFile::getEntry (const String& fileName) const throw()
{
    return getEntry (getIndexOfFileName (fileName));
}

InputStream* ZipFile::createStreamForEntry (const int index)
{
    ZipEntryInfo* const zei = (ZipEntryInfo*) (entries[index]);

    InputStream* stream = 0;

    if (zei != 0)
    {
        stream = new ZipInputStream (*this, *zei);

        if (zei->compressed)
        {
            stream = new GZIPDecompressorInputStream (stream, true, true);

            // (much faster to unzip in big blocks using a buffer..)
            stream = new BufferedInputStream (stream, 32768, true);
        }
    }

    return stream;
}

class ZipFilenameComparator
{
public:
    static int compareElements (const void* const first, const void* const second) throw()
    {
        return ((const ZipEntryInfo*) first)->entry.filename
                    .compare (((const ZipEntryInfo*) second)->entry.filename);
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
    if (source != 0)
    {
        numEntries = 0;
        int pos = findEndOfZipEntryTable();
        int size = (int) (source->getTotalLength() - pos);

        source->setPosition (pos);
        MemoryBlock headerData;

        if (source->readIntoMemoryBlock (headerData, size) != size)
            return;

        pos = 0;

        for (int i = 0; i < numEntries; ++i)
        {
            if (pos + 46 > size)
                break;

            const char* const buffer = ((const char*) headerData.getData()) + pos;

            const int fileNameLen = littleEndianShort (buffer + 28);

            if (pos + 46 + fileNameLen > size)
                break;

            ZipEntryInfo* const zei = new ZipEntryInfo();
            zei->entry.filename = String (buffer + 46, fileNameLen);

            const int time = littleEndianShort (buffer + 12);
            const int date = littleEndianShort (buffer + 14);

            const int year      = 1980 + (date >> 9);
            const int month     = ((date >> 5) & 15) - 1;
            const int day       = date & 31;
            const int hours     = time >> 11;
            const int minutes   = (time >> 5) & 63;
            const int seconds   = (time & 31) << 1;

            zei->entry.fileTime = Time (year, month, day, hours, minutes, seconds);

            zei->compressed = littleEndianShort (buffer + 10) != 0;
            zei->compressedSize = littleEndianInt (buffer + 20);
            zei->entry.uncompressedSize = littleEndianInt (buffer + 24);

            zei->streamOffset = littleEndianInt (buffer + 42);
            entries.add (zei);

            pos += 46 + fileNameLen
                    + littleEndianShort (buffer + 30)
                    + littleEndianShort (buffer + 32);
        }
    }
}

int ZipFile::findEndOfZipEntryTable()
{
    BufferedInputStream in (source, 8192, false);

    in.setPosition (in.getTotalLength());
    int64 pos = in.getPosition();

    char buffer [32];
    zeromem (buffer, sizeof (buffer));

    while (pos > 0)
    {
        in.setPosition (pos - 22);
        pos = in.getPosition();
        memcpy (buffer + 22, buffer, 4);

        if (in.read (buffer, 22) != 22)
            return 0;

        for (int i = 0; i < 22; ++i)
        {
            if (littleEndianInt (buffer + i) == 0x06054b50)
            {
                in.setPosition (pos + i);
                in.read (buffer, 22);
                numEntries = littleEndianShort (buffer + 10);

                return littleEndianInt (buffer + 16);
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
        const ZipEntryInfo& zei = *(ZipEntryInfo*) entries[i];

        const File targetFile (targetDirectory.getChildFile (zei.entry.filename));

        if (zei.entry.filename.endsWithChar (T('/')))
        {
            targetFile.createDirectory(); // (entry is a directory, not a file)
        }
        else
        {
            InputStream* const in = createStreamForEntry (i);

            if (in != 0)
            {
                if (shouldOverwriteFiles)
                    targetFile.deleteFile();

                if ((! targetFile.exists())
                     && targetFile.getParentDirectory().createDirectory())
                {
                    FileOutputStream* const out = targetFile.createOutputStream();

                    if (out != 0)
                    {
                        out->writeFromInputStream (*in, -1);
                        delete out;

                        targetFile.setCreationTime (zei.entry.fileTime);
                        targetFile.setLastModificationTime (zei.entry.fileTime);
                        targetFile.setLastAccessTime (zei.entry.fileTime);
                    }
                }

                delete in;
            }
        }
    }
}

END_JUCE_NAMESPACE
