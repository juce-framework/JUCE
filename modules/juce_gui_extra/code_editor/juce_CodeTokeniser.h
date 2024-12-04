/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
