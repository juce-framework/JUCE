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

#pragma once


//==============================================================================
template<typename Type>
class TextWithDefaultPropertyComponent  : public PropertyComponent,
                                          private Label::Listener
{
    //==========================================================================
    class LabelComp  : public Label,
                       public FileDragAndDropTarget
    {
    public:
        LabelComp (TextWithDefaultPropertyComponent& tpc, const int charLimit)
            : Label (String(), String()),
              owner (tpc),
              maxChars (charLimit)
        {
            setEditable (true, true, false);
            addListener (&tpc);

            updateColours();
        }

        bool isInterestedInFileDrag (const StringArray&) override
        {
            return true;
        }

        void filesDropped (const StringArray& files, int, int) override
        {
            setText (getText() + files.joinIntoString (", "), sendNotificationSync);
            showEditor();
        }

        TextEditor* createEditorComponent() override
        {
            TextEditor* const ed = Label::createEditorComponent();
            ed->setInputRestrictions (maxChars);
            return ed;
        }

        void textWasEdited() override
        {
            owner.textWasEdited();
        }

        void updateColours()
        {
            setColour (backgroundColourId, owner.findColour (TextWithDefaultPropertyComponent::backgroundColourId));
            setColour (outlineColourId,    owner.findColour (TextWithDefaultPropertyComponent::outlineColourId));
            setColour (textColourId,       owner.findColour (TextWithDefaultPropertyComponent::textColourId));
            repaint();
        }

    private:
        TextWithDefaultPropertyComponent& owner;
        int maxChars;
    };

protected:
    //==========================================================================
    TextWithDefaultPropertyComponent (const String& propertyName, int maxNumChars)
        : PropertyComponent (propertyName)
    {
        createEditor (maxNumChars);
    }

public:
    //==========================================================================
    TextWithDefaultPropertyComponent (CachedValue<Type>& valueToControl, const String& propertyName, int maxNumChars)
        : PropertyComponent (propertyName),
          cachedValue (valueToControl)
    {
        createEditor (maxNumChars);
        refresh();
    }

    virtual String getText() const
    {
        return cachedValue.get();
    }

    enum ColourIds
    {
        backgroundColourId          = 0x100e401,    /**< The colour to fill the background of the text area. */
        textColourId                = 0x100e402,    /**< The colour to use for the editable text. */
        outlineColourId             = 0x100e403,    /**< The colour to use to draw an outline around the text area. */
    };

    void colourChanged() override
    {
        PropertyComponent::colourChanged();
        textEditor->updateColours();
    }

    //==============================================================================
    void refresh() override
    {
        if (cachedValue.isUsingDefault())
            setColour (textColourId, findColour (widgetTextColourId).withMultipliedAlpha (0.5f));
        else
            setColour (textColourId, findColour (widgetTextColourId));

        textEditor->setText (getText(), dontSendNotification);
    }

    virtual void textWasEdited()
    {
        String textDisplayed = textEditor->getText();

        if (textDisplayed.isEmpty())
            cachedValue.resetToDefault();
        else
            cachedValue = textDisplayed;

        refresh();
    }

private:
    //==============================================================================
    friend class LabelComp;
    CachedValue<Type>& cachedValue;

    ScopedPointer<LabelComp> textEditor;

    void createEditor (int maxNumChars)
    {
        addAndMakeVisible (textEditor = new LabelComp (*this, maxNumChars));
    }

    void labelTextChanged (Label*) override {}

    void editorShown (Label*, TextEditor& editor) override
    {
        if (cachedValue.isUsingDefault())
            editor.setText (String(), dontSendNotification);
    }

    void editorHidden (Label*, TextEditor&) override {}

    void lookAndFeelChanged() override { refresh(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextWithDefaultPropertyComponent)
};


//==============================================================================
class TextWithDefaultPropertyComponentWithEnablement : public TextWithDefaultPropertyComponent<String>,
                                                       private Value::Listener
{
public:
    TextWithDefaultPropertyComponentWithEnablement (CachedValue<String>& valueToControl,
                                                    const Value& valueToListenTo,
                                                    const String& propertyName,
                                                    int maxNumChars)
        : TextWithDefaultPropertyComponent<String> (valueToControl, propertyName, maxNumChars),
          value (valueToListenTo)
    {
        value.addListener (this);
        setEnabled (value.getValue());
    }

    ~TextWithDefaultPropertyComponentWithEnablement()
    {
        value.removeListener (this);
    }

private:
    Value value;

    void valueChanged (Value& v) override
    {
        setEnabled (v.getValue());
    }
};
