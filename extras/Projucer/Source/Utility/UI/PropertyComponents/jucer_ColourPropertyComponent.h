/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
struct ColourPropertyComponent  : public PropertyComponent
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
    struct ColourEditorComponent    : public Component,
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
            g.setFont (Font ((float) getHeight() * 0.6f, Font::bold));
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
    struct PopupColourSelector   : public Component,
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
