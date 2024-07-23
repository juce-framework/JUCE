/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An InputSource backed by an AndroidDocument.

    @see InputSource, AndroidDocument

    @tags{Core}
*/
class JUCE_API  AndroidDocumentInputSource   : public InputSource
{
public:
    //==============================================================================
    /** Creates a new AndroidDocumentInputSource, backed by the provided document.
    */
    explicit AndroidDocumentInputSource (const AndroidDocument& doc)
        : document (doc) {}

    //==============================================================================
    /** Returns a new InputStream to read this item.

        @returns            an inputstream that the caller will delete, or nullptr if
                            the document can't be opened.
    */
    InputStream* createInputStream() override
    {
        return document.createInputStream().release();
    }

    /** @internal

        An AndroidDocument doesn't use conventional filesystem paths.
        Use the member functions of AndroidDocument to locate relative items.

        @param relatedItemPath  the relative pathname of the resource that is required
        @returns            an input stream if relatedItemPath was empty, otherwise
                            nullptr.
    */
    InputStream* createInputStreamFor (const String& relatedItemPath) override
    {
        return relatedItemPath.isEmpty() ? document.createInputStream().release() : nullptr;
    }

    /** Returns a hash code that uniquely represents this item.
    */
    int64 hashCode() const override
    {
        return document.getUrl().toString (true).hashCode64();
    }

private:
    AndroidDocument document;
};

} // namespace juce
