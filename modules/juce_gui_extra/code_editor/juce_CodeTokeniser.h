/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A base class for tokenising code so that the syntax can be displayed in a
    code editor.

    @see CodeDocument, CodeEditorComponent

    @tags{GUI}
*/
class JUCE_API  CodeTokeniser
{
public:
    CodeTokeniser() = default;
    virtual ~CodeTokeniser() = default;

    //==============================================================================
    /** Reads the next token from the source and returns its token type.

        This must leave the source pointing to the first character in the
        next token.
    */
    virtual int readNextToken (CodeDocument::Iterator& source) = 0;

    /** Returns a suggested syntax highlighting colour scheme. */
    virtual CodeEditorComponent::ColourScheme getDefaultColourScheme() = 0;

private:
    JUCE_LEAK_DETECTOR (CodeTokeniser)
};

} // namespace juce
