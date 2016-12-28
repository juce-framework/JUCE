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
