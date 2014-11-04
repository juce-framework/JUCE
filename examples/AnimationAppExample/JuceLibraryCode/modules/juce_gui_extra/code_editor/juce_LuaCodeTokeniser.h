/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_LUACODETOKENISER_H_INCLUDED
#define JUCE_LUACODETOKENISER_H_INCLUDED


//==============================================================================
/**
*/
class JUCE_API  LuaTokeniser   : public CodeTokeniser
{
public:
    //==============================================================================
    LuaTokeniser();
    ~LuaTokeniser();

    //==============================================================================
    int readNextToken (CodeDocument::Iterator&) override;
    CodeEditorComponent::ColourScheme getDefaultColourScheme() override;

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
        tokenType_punctuation
    };

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LuaTokeniser)
};

#endif   // JUCE_LUACODETOKENISER_H_INCLUDED
