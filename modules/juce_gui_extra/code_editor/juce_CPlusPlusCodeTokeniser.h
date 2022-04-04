/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A simple lexical analyser for syntax colouring of C++ code.

    @see CodeEditorComponent, CodeDocument

    @tags{GUI}
*/
class JUCE_API  CPlusPlusCodeTokeniser    : public CodeTokeniser
{
public:
    //==============================================================================
    CPlusPlusCodeTokeniser();
    ~CPlusPlusCodeTokeniser() override;

    //==============================================================================
    int readNextToken (CodeDocument::Iterator&) override;
    CodeEditorComponent::ColourScheme getDefaultColourScheme() override;

    /** This is a handy method for checking whether a string is a c++ reserved keyword. */
    static bool isReservedKeyword (const String& token) noexcept;

    /** The token values returned by this tokeniser. */
    enum TokenType
    {
        tokenType_error = 0,
        tokenType_comment,
        tokenType_keyword,
        tokenType_operator,
        tokenType_identifier,
        tokenType_integer,
        tokenType_float,
        tokenType_string,
        tokenType_bracket,
        tokenType_punctuation,
        tokenType_preprocessor
    };

private:
    //==============================================================================
    JUCE_LEAK_DETECTOR (CPlusPlusCodeTokeniser)
};

} // namespace juce
