/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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
struct ColorPropertyComponent  : public PropertyComponent
{
    ColorPropertyComponent (UndoManager* undoManager, const String& name, const Value& color,
                             Color defaultColor, bool canResetToDefault)
        : PropertyComponent (name),
          colorEditor (undoManager, color, defaultColor, canResetToDefault)
    {
        addAndMakeVisible (colorEditor);
    }

    void resized() override
    {
        colorEditor.setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
    }

    void refresh() override {}

private:
    /**
        A component that shows a color swatch with hex ARGB value, and which pops up
        a color selector when you click it.
    */
    struct ColorEditorComponent    : public Component,
                                      public Value::Listener
    {
        ColorEditorComponent (UndoManager* um, const Value& color,
                               Color defaultCol, const bool canReset)
            : undoManager (um), colorValue (color), defaultColor (defaultCol),
              canResetToDefault (canReset)
        {
            colorValue.addListener (this);
        }

        void paint (Graphics& g) override
        {
            const Color color (getColor());

            g.fillAll (Colors::gray);
            g.fillCheckerBoard (getLocalBounds().reduced (2).toFloat(),
                                10.0f, 10.0f,
                                Color (0xffdddddd).overlaidWith (color),
                                Color (0xffffffff).overlaidWith (color));

            g.setColor (Colors::white.overlaidWith (color).contrasting());
            g.setFont (Font (getHeight() * 0.6f, Font::bold));
            g.drawFittedText (color.toDisplayString (true), getLocalBounds().reduced (2, 1),
                              Justification::centered, 1);
        }

        Color getColor() const
        {
            if (colorValue.toString().isEmpty())
                return defaultColor;

            return Color::fromString (colorValue.toString());
        }

        void setColor (Color newColor)
        {
            if (getColor() != newColor)
            {
                if (newColor == defaultColor && canResetToDefault)
                    colorValue = var();
                else
                    colorValue = newColor.toDisplayString (true);
            }
        }

        void resetToDefault()
        {
            setColor (defaultColor);
        }

        void refresh()
        {
            const Color col (getColor());

            if (col != lastColor)
            {
                lastColor = col;
                repaint();
            }
        }

        void mouseDown (const MouseEvent&) override
        {
            if (undoManager != nullptr)
                undoManager->beginNewTransaction();

            CallOutBox::launchAsynchronously (new PopupColorSelector (colorValue,
                                                                       defaultColor,
                                                                       canResetToDefault),
                                              getScreenBounds(), nullptr);
        }

        void valueChanged (Value&) override
        {
            refresh();
        }

        UndoManager* undoManager;
        Value colorValue;
        Color lastColor;
        const Color defaultColor;
        const bool canResetToDefault;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorEditorComponent)
    };

    //==============================================================================
    struct PopupColorSelector   : public Component,
                                   public ChangeListener,
                                   public Value::Listener
    {
        PopupColorSelector (const Value& color,
                             Color defaultCol,
                             const bool canResetToDefault)
            : defaultButton ("Reset to Default"),
              colorValue (color),
              defaultColor (defaultCol)
        {
            addAndMakeVisible (selector);
            selector.setName ("Color");
            selector.setCurrentColor (getColor());
            selector.addChangeListener (this);

            if (canResetToDefault)
            {
                addAndMakeVisible (defaultButton);
                defaultButton.onClick = [this]
                {
                    setColor (defaultColor);
                    selector.setCurrentColor (defaultColor);
                };
            }

            colorValue.addListener (this);
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

        Color getColor() const
        {
            if (colorValue.toString().isEmpty())
                return defaultColor;

            return Color::fromString (colorValue.toString());
        }

        void setColor (Color newColor)
        {
            if (getColor() != newColor)
            {
                if (newColor == defaultColor && defaultButton.isVisible())
                    colorValue = var();
                else
                    colorValue = newColor.toDisplayString (true);
            }
        }

        void changeListenerCallback (ChangeBroadcaster*) override
        {
            if (selector.getCurrentColor() != getColor())
                setColor (selector.getCurrentColor());
        }

        void valueChanged (Value&) override
        {
            selector.setCurrentColor (getColor());
        }

    private:
        StoredSettings::ColorSelectorWithSwatches selector;
        TextButton defaultButton;
        Value colorValue;
        Color defaultColor;
    };

    ColorEditorComponent colorEditor;
};
