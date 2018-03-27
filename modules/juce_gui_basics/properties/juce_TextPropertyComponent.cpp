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

//==============================================================================
class TextPropertyComponent::LabelComp  : public Label,
                                          public FileDragAndDropTarget
{
public:
    LabelComp (TextPropertyComponent& tpc, int charLimit, bool multiline, bool editable)
        : Label ({}, {}),
          owner (tpc),
          maxChars (charLimit),
          isMultiline (multiline)
    {
        setEditable (editable, editable);

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
        auto* ed = Label::createEditorComponent();
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

    void setTextToDisplayWhenEmpty (const String& text, float alpha)
    {
        textToDisplayWhenEmpty = text;
        alphaToUseForEmptyText = alpha;
    }

    void paintOverChildren (Graphics& g) override
    {
        if (getText().isEmpty() && ! isBeingEdited())
        {
            auto textArea = getBorderSize().subtractedFrom (getLocalBounds());
            auto labelFont = owner.getLookAndFeel().getLabelFont (*this);

            g.setColour (owner.findColour (TextPropertyComponent::textColourId).withAlpha (alphaToUseForEmptyText));
            g.setFont (labelFont);

            g.drawFittedText (textToDisplayWhenEmpty, textArea, getJustificationType(),
                              jmax (1, (int) (textArea.getHeight() / labelFont.getHeight())),
                              getMinimumHorizontalScale());
        }
    }

private:
    TextPropertyComponent& owner;

    int maxChars;
    bool isMultiline;
    bool interestedInFileDrag = true;

    String textToDisplayWhenEmpty;
    float alphaToUseForEmptyText = 0.0f;
};

//==============================================================================
class TextPropertyComponent::RemapperValueSourceWithDefault    : public Value::ValueSource
{
public:
    RemapperValueSourceWithDefault (const ValueWithDefault& vwd)
        : valueWithDefault (vwd)
    {
    }

    var getValue() const override
    {
        return valueWithDefault.isUsingDefault() ? var() : valueWithDefault.get();
    }

    void setValue (const var& newValue) override
    {
        if (newValue.toString().isEmpty())
            valueWithDefault.resetToDefault();
        else
            valueWithDefault = newValue;
    }

private:
    ValueWithDefault valueWithDefault;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemapperValueSourceWithDefault)
};

//==============================================================================
TextPropertyComponent::TextPropertyComponent (const String& name,
                                              int maxNumChars,
                                              bool multiLine,
                                              bool isEditable)
    : PropertyComponent (name),
      isMultiLine (multiLine)
{
    createEditor (maxNumChars, isEditable);
}

TextPropertyComponent::TextPropertyComponent (const Value& valueToControl,
                                              const String& name,
                                              int maxNumChars,
                                              bool isMultiLine,
                                              bool isEditable)
    : TextPropertyComponent (name, maxNumChars, isMultiLine, isEditable)
{
    textEditor->getTextValue().referTo (valueToControl);
}

TextPropertyComponent::TextPropertyComponent (ValueWithDefault& valueToControl,
                                              const String& name,
                                              int maxNumChars,
                                              bool isMultiLine,
                                              bool isEditable)
    : TextPropertyComponent (name, maxNumChars, isMultiLine, isEditable)
{
    textEditor->getTextValue().referTo (Value (new RemapperValueSourceWithDefault (valueToControl)));
    textEditor->setTextToDisplayWhenEmpty (valueToControl.getDefault(), 0.5f);

    valueToControl.onDefaultChange = [this, &valueToControl]
    {
        textEditor->setTextToDisplayWhenEmpty (valueToControl.getDefault(), 0.5f);
        repaint();
    };
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

void TextPropertyComponent::createEditor (int maxNumChars, bool isEditable)
{
    textEditor.reset (new LabelComp (*this, maxNumChars, isMultiLine, isEditable));
    addAndMakeVisible (textEditor.get());

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
    auto newText = textEditor->getText();

    if (getText() != newText)
        setText (newText);

    callListeners();
}

void TextPropertyComponent::addListener    (TextPropertyComponent::Listener* l)  { listenerList.add (l); }
void TextPropertyComponent::removeListener (TextPropertyComponent::Listener* l)  { listenerList.remove (l); }

void TextPropertyComponent::callListeners()
{
    Component::BailOutChecker checker (this);
    listenerList.callChecked (checker, [this] (Listener& l) { l.textPropertyComponentChanged (this); });
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

void TextPropertyComponent::setEditable (bool isEditable)
{
    if (textEditor != nullptr)
        textEditor->setEditable (isEditable, isEditable);
}

} // namespace juce
