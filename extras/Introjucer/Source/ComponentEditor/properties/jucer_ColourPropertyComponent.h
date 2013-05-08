/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_COLOURPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCER_COLOURPROPERTYCOMPONENT_JUCEHEADER__

#include "../../Application/jucer_Application.h"


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

    virtual void setColour (const Colour& newColour) = 0;
    virtual Colour getColour() const = 0;
    virtual void resetToDefault() = 0;

    void refresh()
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

        void paint (Graphics& g)
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

        virtual void setColour (const Colour& newColour) = 0;
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

        void mouseDown (const MouseEvent&)
        {
            ColourSelectorComp colourSelector (this, canResetToDefault);

            PopupMenu m;
            m.addCustomItem (1234, &colourSelector, 300, 400, false);
            m.showAt (this);
        }

        void changeListenerCallback (ChangeBroadcaster* source)
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
                addAndMakeVisible (&selector);
                selector.setName ("Colour");
                selector.setCurrentColour (owner->getColour());
                selector.addChangeListener (owner);

                if (canReset)
                {
                    addAndMakeVisible (&defaultButton);
                    defaultButton.addListener (this);
                }
            }

            void resized()
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

            void buttonClicked (Button*)
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

                int getNumSwatches() const
                {
                    return getAppSettings().swatchColours.size();
                }

                Colour getSwatchColour (int index) const
                {
                    return getAppSettings().swatchColours [index];
                }

                void setSwatchColour (int index, const Colour& newColour) const
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

        void setColour (const Colour& newColour)
        {
            owner->setColour (newColour);
        }

        Colour getColour() const
        {
            return owner->getColour();
        }

        void resetToDefault()
        {
            owner->resetToDefault();
        }
    };

    ScopedPointer<ColourPropEditorComponent> colourPropEditor;
};


#endif   // __JUCER_COLOURPROPERTYCOMPONENT_JUCEHEADER__
