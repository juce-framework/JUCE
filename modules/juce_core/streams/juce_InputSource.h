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

#ifndef JUCE_INPUTSOURCE_H_INCLUDED
#define JUCE_INPUTSOURCE_H_INCLUDED


//==============================================================================
/**
    A lightweight object that can create a stream to read some kind of resource.

    This may be used to refer to a file, or some other kind of source, allowing a
    caller to create an input stream that can read from it when required.

    @see FileInputSource
*/
class JUCE_API  InputSource
{
public:
    //==============================================================================
    InputSource() noexcept      {}

    /** Destructor. */
    virtual ~InputSource()      {}

    //==============================================================================
    /** Returns a new InputStream to read this item.

        @returns            an inputstream that the caller will delete, or nullptr if
                            the filename isn't found.
    */
    virtual InputStream* createInputStream() = 0;

    /** Returns a new InputStream to read an item, relative.

        @param relatedItemPath  the relative pathname of the resource that is required
        @returns            an inputstream that the caller will delete, or nullptr if
                            the item isn't found.
    */
    virtual InputStream* createInputStreamFor (const String& relatedItemPath) = 0;

    /** Returns a hash code that uniquely represents this item.
    */
    virtual int64 hashCode() const = 0;


private:
    //==============================================================================
    JUCE_LEAK_DETECTOR (InputSource)
};


#endif   // JUCE_INPUTSOURCE_H_INCLUDED
