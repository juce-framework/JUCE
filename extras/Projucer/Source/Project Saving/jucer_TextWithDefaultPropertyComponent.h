/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

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


#ifndef JUCER_TEXTWITHDEFAULTPROPERTYCOMPONENT_H_INCLUDED
#define JUCER_TEXTWITHDEFAULTPROPERTYCOMPONENT_H_INCLUDED

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
            setColour (textColourId, Colours::grey);
        else
            setColour (textColourId, Colours::black);

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextWithDefaultPropertyComponent)
};

#endif  // JUCER_TEXTWITHDEFAULTPROPERTYCOMPONENT_H_INCLUDED
