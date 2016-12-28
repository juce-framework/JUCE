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

#ifndef JUCE_FILEINPUTSOURCE_H_INCLUDED
#define JUCE_FILEINPUTSOURCE_H_INCLUDED


//==============================================================================
/**
    A type of InputSource that represents a normal file.

    @see InputSource
*/
class JUCE_API  FileInputSource     : public InputSource
{
public:
    //==============================================================================
    /** Creates a FileInputSource for a file.
        If the useFileTimeInHashGeneration parameter is true, then this object's
        hashCode() method will incorporate the file time into its hash code; if
        false, only the file name will be used for the hash.
    */
    FileInputSource (const File& file, bool useFileTimeInHashGeneration = false);

    /** Destructor. */
    ~FileInputSource();

    InputStream* createInputStream();
    InputStream* createInputStreamFor (const String& relatedItemPath);
    int64 hashCode() const;

private:
    //==============================================================================
    const File file;
    bool useFileTimeInHashGeneration;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInputSource)
};


#endif   // JUCE_FILEINPUTSOURCE_H_INCLUDED
