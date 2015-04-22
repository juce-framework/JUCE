/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_FILEFILTER_H_INCLUDED
#define JUCE_FILEFILTER_H_INCLUDED


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


#endif   // JUCE_FILEFILTER_H_INCLUDED
