/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

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
    addAndMakeVisible (textEditor = new TextPropLabel (*this, maxNumChars, isMultiLine));

    if (isMultiLine)
    {
        textEditor->setJustificationType (Justification::topLeft);
        preferredHeight = 120;
    }
}

TextPropertyComponent::~TextPropertyComponent()
{
    deleteAllChildren();
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
