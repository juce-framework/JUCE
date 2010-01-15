/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_TextPropertyComponent.h"
#include "../controls/juce_ComboBox.h"


//==============================================================================
class TextPropLabel  : public Label
{
    TextPropertyComponent& owner;
    int maxChars;
    bool isMultiline;

public:
    TextPropLabel (TextPropertyComponent& owner_,
                   const int maxChars_, const bool isMultiline_)
        : Label (String::empty, String::empty),
          owner (owner_),
          maxChars (maxChars_),
          isMultiline (isMultiline_)
    {
        setEditable (true, true, false);

        setColour (backgroundColourId, Colours::white);
        setColour (outlineColourId, findColour (ComboBox::outlineColourId));
    }

    ~TextPropLabel()
    {
    }

    TextEditor* createEditorComponent()
    {
        TextEditor* const textEditor = Label::createEditorComponent();

        textEditor->setInputRestrictions (maxChars);

        if (isMultiline)
        {
            textEditor->setMultiLine (true, true);
            textEditor->setReturnKeyStartsNewLine (true);
        }

        return textEditor;
    }

    void textWasEdited()
    {
        owner.textWasEdited();
    }
};

//==============================================================================
TextPropertyComponent::TextPropertyComponent (const String& name,
                                              const int maxNumChars,
                                              const bool isMultiLine)
    : PropertyComponent (name)
{
    createEditor (maxNumChars, isMultiLine);
}

TextPropertyComponent::TextPropertyComponent (const Value& valueToControl,
                                              const String& name,
                                              const int maxNumChars,
                                              const bool isMultiLine)
    : PropertyComponent (name)
{
    createEditor (maxNumChars, isMultiLine);

    textEditor->getTextValue().referTo (valueToControl);
}

TextPropertyComponent::~TextPropertyComponent()
{
    deleteAllChildren();
}

void TextPropertyComponent::setText (const String& newText)
{
    textEditor->setText (newText, true);
}

const String TextPropertyComponent::getText() const
{
    return textEditor->getText();
}

void TextPropertyComponent::createEditor (const int maxNumChars, const bool isMultiLine)
{
    addAndMakeVisible (textEditor = new TextPropLabel (*this, maxNumChars, isMultiLine));

    if (isMultiLine)
    {
        textEditor->setJustificationType (Justification::topLeft);
        preferredHeight = 120;
    }
}

void TextPropertyComponent::refresh()
{
    textEditor->setText (getText(), false);
}

void TextPropertyComponent::textWasEdited()
{
    const String newText (textEditor->getText());

    if (getText() != newText)
        setText (newText);
}


END_JUCE_NAMESPACE
