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

#ifndef JUCE_CODETOKENISER_H_INCLUDED
#define JUCE_CODETOKENISER_H_INCLUDED


//==============================================================================
/**
    A base class for tokenising code so that the syntax can be displayed in a
    code editor.

    @see CodeDocument, CodeEditorComponent
*/
class JUCE_API  CodeTokeniser
{
public:
    CodeTokeniser()                 {}
    virtual ~CodeTokeniser()        {}

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


#endif   // JUCE_CODETOKENISER_H_INCLUDED
