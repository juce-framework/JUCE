/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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
        return interestedInFileDrag;
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

    void setInterestedInFileDrag (bool isInterested)
    {
        interestedInFileDrag = isInterested;
    }

private:
    TextPropertyComponent& owner;
    int maxChars;
    bool isMultiline;
    bool interestedInFileDrag = true;
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

void TextPropertyComponent::setInterestedInFileDrag (bool isInterested)
{
    if (textEditor != nullptr)
        textEditor->setInterestedInFileDrag (isInterested);
}

} // namespace juce
