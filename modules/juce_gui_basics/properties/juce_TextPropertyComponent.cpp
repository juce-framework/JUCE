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
class TextPropertyComponent::LabelComp final : public Label,
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
            auto& lf = owner.getLookAndFeel();
            auto textArea = lf.getLabelBorderSize (*this).subtractedFrom (getLocalBounds());
            auto labelFont = lf.getLabelFont (*this);

            g.setColour (owner.findColour (TextPropertyComponent::textColourId).withAlpha (alphaToUseForEmptyText));
            g.setFont (labelFont);

            g.drawFittedText (textToDisplayWhenEmpty, textArea, getJustificationType(),
                              jmax (1, (int) ((float) textArea.getHeight() / labelFont.getHeight())),
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
class TextRemapperValueSourceWithDefault final : public Value::ValueSource
{
public:
    TextRemapperValueSourceWithDefault (const ValueTreePropertyWithDefault& v)
        : value (v)
    {
    }

    var getValue() const override
    {
        if (value.isUsingDefault())
            return {};

        return value.get();
    }

    void setValue (const var& newValue) override
    {
        if (newValue.toString().isEmpty())
        {
            value.resetToDefault();
            return;
        }

        value = newValue;
    }

private:
    ValueTreePropertyWithDefault value;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextRemapperValueSourceWithDefault)
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

TextPropertyComponent::TextPropertyComponent (const Value& valueToControl, const String& name,
                                              int maxNumChars, bool multiLine, bool isEditable)
    : TextPropertyComponent (name, maxNumChars, multiLine, isEditable)
{
    textEditor->getTextValue().referTo (valueToControl);
}

TextPropertyComponent::TextPropertyComponent (const ValueTreePropertyWithDefault& valueToControl, const String& name,
                                              int maxNumChars, bool multiLine, bool isEditable)
    : TextPropertyComponent (name, maxNumChars, multiLine, isEditable)
{
    value = valueToControl;

    textEditor->getTextValue().referTo (Value (new TextRemapperValueSourceWithDefault (value)));
    textEditor->setTextToDisplayWhenEmpty (value.getDefault(), 0.5f);

    value.onDefaultChange = [this]
    {
        textEditor->setTextToDisplayWhenEmpty (value.getDefault(), 0.5f);
        repaint();
    };
}

TextPropertyComponent::~TextPropertyComponent()  {}

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
