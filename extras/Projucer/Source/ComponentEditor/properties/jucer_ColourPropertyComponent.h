/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCER_COLOURPROPERTYCOMPONENT_H_INCLUDED
#define JUCER_COLOURPROPERTYCOMPONENT_H_INCLUDED


class JucerColourPropertyComponent  : public PropertyComponent
{
public:
    JucerColourPropertyComponent (const String& name,
                                  const bool canReset)
        : PropertyComponent (name)
    {
        colourPropEditor = new ColourPropEditorComponent (this, canReset);
        addAndMakeVisible (colourPropEditor);
    }

    virtual void setColour (Colour newColour) = 0;
    virtual Colour getColour() const = 0;
    virtual void resetToDefault() = 0;

    void refresh() override
    {
        ((ColourPropEditorComponent*) getChildComponent (0))->refresh();
    }

    class ColourEditorComponent    : public Component,
                                     public ChangeListener
    {
    public:
        ColourEditorComponent (const bool canReset)
            : canResetToDefault (canReset)
        {
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::grey);

            g.fillCheckerBoard (getLocalBounds().reduced (2, 2),
                                10, 10,
                                Colour (0xffdddddd).overlaidWith (colour),
                                Colour (0xffffffff).overlaidWith (colour));

            g.setColour (Colours::white.overlaidWith (colour).contrasting());
            g.setFont (Font (getHeight() * 0.6f, Font::bold));
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
            CallOutBox::launchAsynchronously (new ColourSelectorComp (this, canResetToDefault),
                                              getScreenBounds(), nullptr);
        }

        void changeListenerCallback (ChangeBroadcaster* source) override
        {
            const ColourSelector* const cs = (const ColourSelector*) source;

            if (cs->getCurrentColour() != getColour())
                setColour (cs->getCurrentColour());
        }

        class ColourSelectorComp   : public Component,
                                     public ButtonListener
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
                    defaultButton.addListener (this);
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

            void buttonClicked (Button*) override
            {
                owner->resetToDefault();
                owner->refresh();
                selector.setCurrentColour (owner->getColour());
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

                void setSwatchColour (int index, const Colour& newColour) const override
                {
                    getAppSettings().swatchColours.set (index, newColour);
                }
            };

            ColourEditorComponent* owner;
            ColourSelectorWithSwatches selector;
            TextButton defaultButton;
        };

    private:
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

    ScopedPointer<ColourPropEditorComponent> colourPropEditor;
};


#endif   // JUCER_COLOURPROPERTYCOMPONENT_H_INCLUDED
