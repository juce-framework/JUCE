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

#pragma once


//==============================================================================
struct ColourPropertyComponent final : public PropertyComponent
{
    ColourPropertyComponent (UndoManager* undoManager, const String& name, const Value& colour,
                             Colour defaultColour, bool canResetToDefault)
        : PropertyComponent (name),
          colourEditor (undoManager, colour, defaultColour, canResetToDefault)
    {
        addAndMakeVisible (colourEditor);
    }

    void resized() override
    {
        colourEditor.setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
    }

    void refresh() override {}

private:
    /**
        A component that shows a colour swatch with hex ARGB value, and which pops up
        a colour selector when you click it.
    */
    struct ColourEditorComponent final : public Component,
                                         private Value::Listener
    {
        ColourEditorComponent (UndoManager* um, const Value& colour,
                               Colour defaultCol, const bool canReset)
            : undoManager (um), colourValue (colour), defaultColour (defaultCol),
              canResetToDefault (canReset)
        {
            colourValue.addListener (this);
        }

        void paint (Graphics& g) override
        {
            const Colour colour (getColour());

            g.fillAll (Colours::grey);
            g.fillCheckerBoard (getLocalBounds().reduced (2).toFloat(),
                                10.0f, 10.0f,
                                Colour (0xffdddddd).overlaidWith (colour),
                                Colour (0xffffffff).overlaidWith (colour));

            g.setColour (Colours::white.overlaidWith (colour).contrasting());
            g.setFont (FontOptions ((float) getHeight() * 0.6f, Font::bold));
            g.drawFittedText (colour.toDisplayString (true), getLocalBounds().reduced (2, 1),
                              Justification::centred, 1);
        }

        Colour getColour() const
        {
            if (colourValue.toString().isEmpty())
                return defaultColour;

            return Colour::fromString (colourValue.toString());
        }

        void setColour (Colour newColour)
        {
            if (getColour() != newColour)
            {
                if (newColour == defaultColour && canResetToDefault)
                    colourValue = var();
                else
                    colourValue = newColour.toDisplayString (true);
            }
        }

        void resetToDefault()
        {
            setColour (defaultColour);
        }

        void refresh()
        {
            const Colour col (getColour());

            if (col != lastColour)
            {
                lastColour = col;
                repaint();
            }
        }

        void mouseDown (const MouseEvent&) override
        {
            if (undoManager != nullptr)
                undoManager->beginNewTransaction();

            CallOutBox::launchAsynchronously (std::make_unique<PopupColourSelector> (colourValue,
                                                                                     defaultColour,
                                                                                     canResetToDefault),
                                              getScreenBounds(),
                                              nullptr);
        }

    private:
        void valueChanged (Value&) override
        {
            refresh();
        }

        UndoManager* undoManager;
        Value colourValue;
        Colour lastColour;
        const Colour defaultColour;
        const bool canResetToDefault;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourEditorComponent)
    };

    //==============================================================================
    struct PopupColourSelector final : public Component,
                                       private ChangeListener,
                                       private Value::Listener
    {
        PopupColourSelector (const Value& colour,
                             Colour defaultCol,
                             const bool canResetToDefault)
            : defaultButton ("Reset to Default"),
              colourValue (colour),
              defaultColour (defaultCol)
        {
            addAndMakeVisible (selector);
            selector.setName ("Colour");
            selector.setCurrentColour (getColour());
            selector.addChangeListener (this);

            if (canResetToDefault)
            {
                addAndMakeVisible (defaultButton);
                defaultButton.onClick = [this]
                {
                    setColour (defaultColour);
                    selector.setCurrentColour (defaultColour);
                };
            }

            colourValue.addListener (this);
            setSize (300, 400);
        }

        void resized() override
        {
            if (defaultButton.isVisible())
            {
                selector.setBounds (0, 0, getWidth(), getHeight() - 30);
                defaultButton.changeWidthToFitText (22);
                defaultButton.setTopLeftPosition (10, getHeight() - 26);
            }
            else
            {
                selector.setBounds (getLocalBounds());
            }
        }

        Colour getColour() const
        {
            if (colourValue.toString().isEmpty())
                return defaultColour;

            return Colour::fromString (colourValue.toString());
        }

        void setColour (Colour newColour)
        {
            if (getColour() != newColour)
            {
                if (newColour == defaultColour && defaultButton.isVisible())
                    colourValue = var();
                else
                    colourValue = newColour.toDisplayString (true);
            }
        }

    private:
        void changeListenerCallback (ChangeBroadcaster*) override
        {
            if (selector.getCurrentColour() != getColour())
                setColour (selector.getCurrentColour());
        }

        void valueChanged (Value&) override
        {
            selector.setCurrentColour (getColour());
        }

        StoredSettings::ColourSelectorWithSwatches selector;
        TextButton defaultButton;
        Value colourValue;
        Colour defaultColour;
    };

    ColourEditorComponent colourEditor;
};
