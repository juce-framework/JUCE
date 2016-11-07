/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

class TextPropertyComponent::LabelComp  : public Label,
                                          public FileDragAndDropTarget
{
public:
    LabelComp (TextPropertyComponent& tpc, const int charLimit, const bool multiline)
        : Label (String(), String()),
          owner (tpc),
          maxChars (charLimit),
          isMultiline (multiline)
    {
        setEditable (true, true, false);

        updateColours();
    }

    bool isInterestedInFileDrag (const StringArray&) override
    {
        return true;
    }

    void filesDropped (const StringArray& files, int, int) override
    {
        setText (getText() + files.joinIntoString (isMultiline ? "\n" : ", "), sendNotificationSync);
        showEditor();
    }

    TextEditor* createEditorComponent() override
    {
        TextEditor* const ed = Label::createEditorComponent();
        ed->setInputRestrictions (maxChars);

        if (isMultiline)
        {
            ed->setMultiLine (true, true);
            ed->setReturnKeyStartsNewLine (true);
        }

        return ed;
    }

    void textWasEdited() override
    {
        owner.textWasEdited();
    }

    void updateColours()
    {
        setColour (backgroundColourId, owner.findColour (TextPropertyComponent::backgroundColourId));
        setColour (outlineColourId,    owner.findColour (TextPropertyComponent::outlineColourId));
        setColour (textColourId,       owner.findColour (TextPropertyComponent::textColourId));
        repaint();
    }

private:
    TextPropertyComponent& owner;
    int maxChars;
    bool isMultiline;
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
}

void TextPropertyComponent::setText (const String& newText)
{
    textEditor->setText (newText, sendNotificationSync);
}

String TextPropertyComponent::getText() const
{
    return textEditor->getText();
}

Value& TextPropertyComponent::getValue() const
{
    return textEditor->getTextValue();
}

void TextPropertyComponent::createEditor (const int maxNumChars, const bool isMultiLine)
{
    addAndMakeVisible (textEditor = new LabelComp (*this, maxNumChars, isMultiLine));

    if (isMultiLine)
    {
        textEditor->setJustificationType (Justification::topLeft);
        preferredHeight = 100;
    }
}

void TextPropertyComponent::refresh()
{
    textEditor->setText (getText(), dontSendNotification);
}

void TextPropertyComponent::textWasEdited()
{
    const String newText (textEditor->getText());

    if (getText() != newText)
        setText (newText);

    callListeners();
}

void TextPropertyComponent::addListener (TextPropertyComponentListener* const listener)
{
    listenerList.add (listener);
}

void TextPropertyComponent::removeListener (TextPropertyComponentListener* const listener)
{
    listenerList.remove (listener);
}

void TextPropertyComponent::callListeners()
{
    Component::BailOutChecker checker (this);
    listenerList.callChecked (checker, &TextPropertyComponentListener::textPropertyComponentChanged, this);
}

void TextPropertyComponent::colourChanged()
{
    PropertyComponent::colourChanged();
    textEditor->updateColours();
}
