/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_COLOUREDITORCOMPONENT_JUCEHEADER__
#define __JUCER_COLOUREDITORCOMPONENT_JUCEHEADER__


//==============================================================================
/**
    A component that shows a colour swatch with hex ARGB value, and which pops up
    a colour selector when you click it.
*/
class ColourEditorComponent    : public Component,
                                 public ChangeListener
{
public:
    ColourEditorComponent (const bool canResetToDefault_)
        : canResetToDefault (canResetToDefault_)
    {
    }

    ~ColourEditorComponent()
    {
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::grey);

        g.fillCheckerBoard (2, 2, getWidth() - 4, getHeight() - 4,
                            10, 10,
                            Colour (0xffdddddd).overlaidWith (colour),
                            Colour (0xffffffff).overlaidWith (colour));

        g.setColour (Colours::white.overlaidWith (colour).contrasting());
        g.setFont (getHeight() * 0.6f, Font::bold);
        g.drawFittedText (String::formatted (T("%02X%02X%02X%02X"),
                                             (int) colour.getAlpha(),
                                             (int) colour.getRed(),
                                             (int) colour.getGreen(),
                                             (int) colour.getBlue()),
                          2, 1, getWidth() - 4, getHeight() - 1,
                          Justification::centred, 1);
    }

    virtual void setColour (const Colour& newColour) = 0;
    virtual void resetToDefault() = 0;
    virtual const Colour getColour() const = 0;

    void refresh()
    {
        const Colour col (getColour());

        if (col != colour)
        {
            colour = col;
            repaint();
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        ColourSelectorComp colourSelector (this, canResetToDefault);

        PopupMenu m;
        m.addCustomItem (1234, &colourSelector, 300, 400, false);
        m.showAt (this);
    }

    void changeListenerCallback (void* source)
    {
        const ColourSelector* const cs = (const ColourSelector*) source;

        if (cs->getCurrentColour() != getColour())
            setColour (cs->getCurrentColour());
    }

    juce_UseDebuggingNewOperator

private:
    Colour colour;
    bool canResetToDefault;

    class ColourSelectorComp   : public Component,
                                 public ButtonListener
    {
    public:
        ColourSelectorComp (ColourEditorComponent* owner_,
                            const bool canResetToDefault)
            : owner (owner_),
              defaultButton (0)
        {
            addAndMakeVisible (selector = new ColourSelectorWithSwatches());
            selector->setName (T("Colour"));
            selector->setCurrentColour (owner->getColour());
            selector->addChangeListener (owner);

            if (canResetToDefault)
            {
                addAndMakeVisible (defaultButton = new TextButton (T("Reset to Default")));
                defaultButton->addButtonListener (this);
            }
        }

        ~ColourSelectorComp()
        {
            deleteAllChildren();
        }

        void resized()
        {
            if (defaultButton != 0)
            {
                selector->setBounds (0, 0, getWidth(), getHeight() - 30);
                defaultButton->changeWidthToFitText (22);
                defaultButton->setTopLeftPosition (10, getHeight() - 26);
            }
            else
            {
                selector->setBounds (0, 0, getWidth(), getHeight());
            }
        }

        void buttonClicked (Button*)
        {
            owner->resetToDefault();
            owner->refresh();
            selector->setCurrentColour (owner->getColour());
        }

    private:
        class ColourSelectorWithSwatches    : public ColourSelector
        {
        public:
            ColourSelectorWithSwatches()
            {
            }

            ~ColourSelectorWithSwatches()
            {
            }

            int getNumSwatches() const
            {
                return StoredSettings::getInstance()->swatchColours.size();
            }

            const Colour getSwatchColour (const int index) const
            {
                return StoredSettings::getInstance()->swatchColours [index];
            }

            void setSwatchColour (const int index, const Colour& newColour) const
            {
                StoredSettings::getInstance()->swatchColours.set (index, newColour);
            }
        };

        ColourEditorComponent* owner;
        ColourSelectorWithSwatches* selector;
        TextButton* defaultButton;
    };
};


#endif   // __JUCER_COLOUREDITORCOMPONENT_JUCEHEADER__
