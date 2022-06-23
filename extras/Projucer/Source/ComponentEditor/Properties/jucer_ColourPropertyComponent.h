/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
class JucerColourPropertyComponent  : public PropertyComponent
{
public:
    JucerColourPropertyComponent (const String& name,
                                  const bool canReset)
        : PropertyComponent (name)
    {
        colourPropEditor.reset (new ColourPropEditorComponent (this, canReset));
        addAndMakeVisible (colourPropEditor.get());
    }

    virtual void setColour (Colour newColour) = 0;
    virtual Colour getColour() const = 0;
    virtual void resetToDefault() = 0;

    void refresh() override
    {
        ((ColourPropEditorComponent*) getChildComponent (0))->refresh();
    }

    class ColourEditorComponent    : public Component,
                                     private ChangeListener
    {
    public:
        ColourEditorComponent (const bool canReset)
            : canResetToDefault (canReset)
        {
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::grey);

            g.fillCheckerBoard (getLocalBounds().reduced (2, 2).toFloat(),
                                10.0f, 10.0f,
                                Colour (0xffdddddd).overlaidWith (colour),
                                Colour (0xffffffff).overlaidWith (colour));

            g.setColour (Colours::white.overlaidWith (colour).contrasting());
            g.setFont (Font ((float) getHeight() * 0.6f, Font::bold));
            g.drawFittedText (colour.toDisplayString (true),
                              2, 1, getWidth() - 4, getHeight() - 1,
                              Justification::centred, 1);
        }

        virtual void setColour (Colour newColour) = 0;
        virtual void resetToDefault() = 0;
        virtual Colour getColour() const = 0;

        void refresh()
        {
            const Colour col (getColour());

            if (col != colour)
            {
                colour = col;
                repaint();
            }
        }

        void mouseDown (const MouseEvent&) override
        {
            CallOutBox::launchAsynchronously (std::make_unique<ColourSelectorComp> (this, canResetToDefault),
                                              getScreenBounds(),
                                              nullptr);
        }

        class ColourSelectorComp   : public Component
        {
        public:
            ColourSelectorComp (ColourEditorComponent* owner_,
                                const bool canReset)
                : owner (owner_),
                  defaultButton ("Reset to Default")
            {
                addAndMakeVisible (selector);
                selector.setName ("Colour");
                selector.setCurrentColour (owner->getColour());
                selector.addChangeListener (owner);

                if (canReset)
                {
                    addAndMakeVisible (defaultButton);

                    defaultButton.onClick = [this]
                    {
                        owner->resetToDefault();
                        owner->refresh();
                        selector.setCurrentColour (owner->getColour());
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
            class ColourSelectorWithSwatches    : public ColourSelector
            {
            public:
                ColourSelectorWithSwatches()
                {
                }

                int getNumSwatches() const override
                {
                    return getAppSettings().swatchColours.size();
                }

                Colour getSwatchColour (int index) const override
                {
                    return getAppSettings().swatchColours [index];
                }

                void setSwatchColour (int index, const Colour& newColour) override
                {
                    getAppSettings().swatchColours.set (index, newColour);
                }
            };

            ColourEditorComponent* owner;
            ColourSelectorWithSwatches selector;
            TextButton defaultButton;
        };

    private:
        void changeListenerCallback (ChangeBroadcaster* source) override
        {
            const ColourSelector* const cs = (const ColourSelector*) source;

            if (cs->getCurrentColour() != getColour())
                setColour (cs->getCurrentColour());
        }

        Colour colour;
        bool canResetToDefault;
    };

    class ColourPropEditorComponent     : public ColourEditorComponent
    {
        JucerColourPropertyComponent* const owner;

    public:
        ColourPropEditorComponent (JucerColourPropertyComponent* const owner_,
                                   const bool canReset)
            : ColourEditorComponent (canReset),
              owner (owner_)
        {}

        void setColour (Colour newColour) override
        {
            owner->setColour (newColour);
        }

        Colour getColour() const override
        {
            return owner->getColour();
        }

        void resetToDefault() override
        {
            owner->resetToDefault();
        }
    };

    std::unique_ptr<ColourPropEditorComponent> colourPropEditor;
};
