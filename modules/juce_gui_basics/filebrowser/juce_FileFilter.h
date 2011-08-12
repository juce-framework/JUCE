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

#ifndef __JUCE_FILEFILTER_JUCEHEADER__
#define __JUCE_FILEFILTER_JUCEHEADER__


//==============================================================================
/**
    Interface for deciding which files are suitable for something.

    For example, this is used by DirectoryContentsList to select which files
    go into the list.

    @see WildcardFileFilter, DirectoryContentsList, FileListComponent, FileBrowserComponent
*/
class JUCE_API  FileFilter
{
public:
    //==============================================================================
    /** Creates a filter with the given description.

        The description can be returned later with the getDescription() method.
    */
    FileFilter (const String& filterDescription);

    /** Destructor. */
    virtual ~FileFilter();

    //==============================================================================
    /** Returns the description that the filter was created with. */
    const String& getDescription() const noexcept;

    //==============================================================================
    /** Should return true if this file is suitable for inclusion in whatever context
        the object is being used.
    */
    virtual bool isFileSuitable (const File& file) const = 0;

    /** Should return true if this directory is suitable for inclusion in whatever context
        the object is being used.
    */
    virtual bool isDirectorySuitable (const File& file) const = 0;


protected:
    //==============================================================================
    String description;
};


#endif   // __JUCE_FILEFILTER_JUCEHEADER__
