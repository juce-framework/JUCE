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

#ifndef __JUCE_ZIPFILE_JUCEHEADER__
#define __JUCE_ZIPFILE_JUCEHEADER__

#include "juce_File.h"
#include "../streams/juce_InputStream.h"
#include "../streams/juce_InputSource.h"
#include "../../threads/juce_CriticalSection.h"
#include "../../containers/juce_VoidArray.h"


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

    /** Creates a ZipFile for an input source.

        The inputSource object will be owned by the zip file, which will delete
        it later when not needed.
    */
    ZipFile (InputSource* const inputSource);

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
    class ZipInputStream;
    class ZipFilenameComparator;
    class ZipEntryInfo;

    OwnedArray <ZipEntryInfo> entries;
    CriticalSection lock;
    InputStream* inputStream;
    ScopedPointer <InputStream> streamToDelete;
    ScopedPointer <InputSource> inputSource;

#ifdef JUCE_DEBUG
    int numOpenStreams;
#endif

    void init();
    int findEndOfZipEntryTable (InputStream* in, int& numEntries);
    static int compareElements (const ZipEntryInfo* first, const ZipEntryInfo* second);

    ZipFile (const ZipFile&);
    const ZipFile& operator= (const ZipFile&);
};

#endif   // __JUCE_ZIPFILE_JUCEHEADER__
