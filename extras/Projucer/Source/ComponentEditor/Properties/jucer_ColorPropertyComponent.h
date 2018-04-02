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
class JucerColorPropertyComponent  : public PropertyComponent
{
public:
    JucerColorPropertyComponent (const String& name,
                                  const bool canReset)
        : PropertyComponent (name)
    {
        colorPropEditor.reset (new ColorPropEditorComponent (this, canReset));
        addAndMakeVisible (colorPropEditor);
    }

    virtual void setColor (Color newColor) = 0;
    virtual Color getColor() const = 0;
    virtual void resetToDefault() = 0;

    void refresh() override
    {
        ((ColorPropEditorComponent*) getChildComponent (0))->refresh();
    }

    class ColorEditorComponent    : public Component,
                                     public ChangeListener
    {
    public:
        ColorEditorComponent (const bool canReset)
            : canResetToDefault (canReset)
        {
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colors::gray);

            g.fillCheckerBoard (getLocalBounds().reduced (2, 2).toFloat(),
                                10.0f, 10.0f,
                                Color (0xffdddddd).overlaidWith (color),
                                Color (0xffffffff).overlaidWith (color));

            g.setColor (Colors::white.overlaidWith (color).contrasting());
            g.setFont (Font (getHeight() * 0.6f, Font::bold));
            g.drawFittedText (color.toDisplayString (true),
                              2, 1, getWidth() - 4, getHeight() - 1,
                              Justification::centered, 1);
        }

        virtual void setColor (Color newColor) = 0;
        virtual void resetToDefault() = 0;
        virtual Color getColor() const = 0;

        void refresh()
        {
            const Color col (getColor());

            if (col != color)
            {
                color = col;
                repaint();
            }
        }

        void mouseDown (const MouseEvent&) override
        {
            CallOutBox::launchAsynchronously (new ColorSelectorComp (this, canResetToDefault),
                                              getScreenBounds(), nullptr);
        }

        void changeListenerCallback (ChangeBroadcaster* source) override
        {
            const ColorSelector* const cs = (const ColorSelector*) source;

            if (cs->getCurrentColor() != getColor())
                setColor (cs->getCurrentColor());
        }

        class ColorSelectorComp   : public Component
        {
        public:
            ColorSelectorComp (ColorEditorComponent* owner_,
                                const bool canReset)
                : owner (owner_),
                  defaultButton ("Reset to Default")
            {
                addAndMakeVisible (selector);
                selector.setName ("Color");
                selector.setCurrentColor (owner->getColor());
                selector.addChangeListener (owner);

                if (canReset)
                {
                    addAndMakeVisible (defaultButton);

                    defaultButton.onClick = [this]
                    {
                        owner->resetToDefault();
                        owner->refresh();
                        selector.setCurrentColor (owner->getColor());
                    };
                }

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

        private:
            class ColorSelectorWithSwatches    : public ColorSelector
            {
            public:
                ColorSelectorWithSwatches()
                {
                }

                int getNumSwatches() const override
                {
                    return getAppSettings().swatchColors.size();
                }

                Color getSwatchColor (int index) const override
                {
                    return getAppSettings().swatchColors [index];
                }

                void setSwatchColor (int index, const Color& newColor) override
                {
                    getAppSettings().swatchColors.set (index, newColor);
                }
            };

            ColorEditorComponent* owner;
            ColorSelectorWithSwatches selector;
            TextButton defaultButton;
        };

    private:
        Color color;
        bool canResetToDefault;
    };

    class ColorPropEditorComponent     : public ColorEditorComponent
    {
        JucerColorPropertyComponent* const owner;

    public:
        ColorPropEditorComponent (JucerColorPropertyComponent* const owner_,
                                   const bool canReset)
            : ColorEditorComponent (canReset),
              owner (owner_)
        {}

        void setColor (Color newColor) override
        {
            owner->setColor (newColor);
        }

        Color getColor() const override
        {
            return owner->getColor();
        }

        void resetToDefault() override
        {
            owner->resetToDefault();
        }
    };

    ScopedPointer<ColorPropEditorComponent> colorPropEditor;
};
