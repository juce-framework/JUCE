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

#ifndef __JUCE_ZIPFILE_JUCEHEADER__
#define __JUCE_ZIPFILE_JUCEHEADER__

#include "../io/files/juce_File.h"
#include "../io/juce_InputStream.h"
#include "../threads/juce_CriticalSection.h"


//==============================================================================
/**
    Decodes a ZIP file from a stream.

    This can enumerate the items in a ZIP file and can create suitable stream objects
    to read each one.
*/
class JUCE_API  ZipFile
{
public:
    //==============================================================================
    /** Creates a ZipFile for a given stream.

        @param inputStream                  the stream to read from
        @param deleteStreamWhenDestroyed    if set to true, the object passed-in
                                            will be deleted when this ZipFile object is deleted
    */
    ZipFile (InputStream* const inputStream,
             const bool deleteStreamWhenDestroyed) throw();

    /** Creates a ZipFile based for a file. */
    ZipFile (const File& file);

    /** Destructor. */
    ~ZipFile() throw();

    //==============================================================================
    /**
        Contains information about one of the entries in a ZipFile.

        @see ZipFile::getEntry
    */
    struct ZipEntry
    {
        /** The name of the file, which may also include a partial pathname. */
        String filename;

        /** The file's original size. */
        unsigned int uncompressedSize;

        /** The last time the file was modified. */
        Time fileTime;
    };

    //==============================================================================
    /** Returns the number of items in the zip file. */
    int getNumEntries() const throw();

    /** Returns a structure that describes one of the entries in the zip file.

        This may return zero if the index is out of range.

        @see ZipFile::ZipEntry
    */
    const ZipEntry* getEntry (const int index) const throw();

    /** Returns the index of the first entry with a given filename.

        This uses a case-sensitive comparison to look for a filename in the
        list of entries. It might return -1 if no match is found.

        @see ZipFile::ZipEntry
    */
    int getIndexOfFileName (const String& fileName) const throw();

    /** Returns a structure that describes one of the entries in the zip file.

        This uses a case-sensitive comparison to look for a filename in the
        list of entries. It might return 0 if no match is found.

        @see ZipFile::ZipEntry
    */
    const ZipEntry* getEntry (const String& fileName) const throw();

    /** Sorts the list of entries, based on the filename.
    */
    void sortEntriesByFilename();

    //==============================================================================
    /** Creates a stream that can read from one of the zip file's entries.

        The stream that is returned must be deleted by the caller (and
        zero might be returned if a stream can't be opened for some reason).

        The stream must not be used after the ZipFile object that created
        has been deleted.
    */
    InputStream* createStreamForEntry (const int index);

    //==============================================================================
    /** Uncompresses all of the files in the zip file.

        This will expand all the entires into a target directory. The relative
        paths of the entries are used.

        @param targetDirectory      the root folder to uncompress to
        @param shouldOverwriteFiles whether to overwrite existing files with similarly-named ones
    */
    void uncompressTo (const File& targetDirectory,
                       const bool shouldOverwriteFiles = true);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    VoidArray entries;
    friend class ZipInputStream;
    CriticalSection lock;
    InputStream* source;
    bool deleteStreamWhenDestroyed;
    int numEntries, centralRecStart;

    void init();
    int findEndOfZipEntryTable();

    ZipFile (const ZipFile&);
    const ZipFile& operator= (const ZipFile&);
};

#endif   // __JUCE_ZIPFILE_JUCEHEADER__
