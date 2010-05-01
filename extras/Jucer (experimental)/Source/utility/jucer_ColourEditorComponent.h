/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
                                 public ChangeListener,
                                 public Value::Listener
{
public:
    ColourEditorComponent (ComponentDocument& document_, const Value& colourValue_,
                           const Colour& defaultColour_, const bool canResetToDefault_)
        : document (document_), colourValue (colourValue_), defaultColour (defaultColour_),
          canResetToDefault (canResetToDefault_)
    {
        colourValue.addListener (this);
    }

    ~ColourEditorComponent()
    {
        colourValue.removeListener (this);
    }

    void paint (Graphics& g)
    {
        const Colour colour (getColour());

        g.fillAll (Colours::grey);
        g.fillCheckerBoard (2, 2, getWidth() - 4, getHeight() - 4,
                            10, 10,
                            Colour (0xffdddddd).overlaidWith (colour),
                            Colour (0xffffffff).overlaidWith (colour));

        g.setColour (Colours::white.overlaidWith (colour).contrasting());
        g.setFont (getHeight() * 0.6f, Font::bold);
        g.drawFittedText (colour.toDisplayString (true),
                          2, 1, getWidth() - 4, getHeight() - 1,
                          Justification::centred, 1);
    }

    const Colour getColour() const
    {
        if (colourValue.toString().isEmpty())
            return defaultColour;

        return Colour::fromString (colourValue.toString());
    }

    void setColour (const Colour& newColour)
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

    void mouseDown (const MouseEvent& e)
    {
        SafePointer<Component> deletionChecker (this);

        {
            ColourSelectorComp colourSelector (this, canResetToDefault);

            PopupMenu m;
            m.addCustomItem (1234, &colourSelector, 300, 400, false);
            m.showAt (this);

            if (deletionChecker == 0)
                return;
        }

        const Colour newColour (getColour());
        document.getUndoManager()->undoCurrentTransactionOnly();
        setColour (newColour);
    }

    void valueChanged (Value&)
    {
        refresh();
    }

    void changeListenerCallback (void* source)
    {
        ColourSelector* cs = static_cast <ColourSelector*> (source);

        if (cs->getCurrentColour() != getColour())
        {
            document.getUndoManager()->undoCurrentTransactionOnly();
            setColour (cs->getCurrentColour());
        }
    }

    juce_UseDebuggingNewOperator

private:
    ComponentDocument& document;
    Value colourValue;
    Colour lastColour;
    const Colour defaultColour;
    const bool canResetToDefault;

    class ColourSelectorComp   : public Component,
                                 public ButtonListener
    {
    public:
        ColourSelectorComp (ColourEditorComponent* owner_,
                            const bool canResetToDefault)
            : owner (owner_),
              defaultButton ("Reset to Default")
        {
            addAndMakeVisible (&selector);
            selector.setName ("Colour");
            selector.setCurrentColour (owner->getColour());
            selector.addChangeListener (owner);

            if (canResetToDefault)
            {
                addAndMakeVisible (&defaultButton);
                defaultButton.addButtonListener (this);
            }
        }

        ~ColourSelectorComp()
        {
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
                selector.setBounds (0, 0, getWidth(), getHeight());
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
        ColourSelectorWithSwatches selector;
        TextButton defaultButton;
    };
};

//==============================================================================
class ColourPropertyComponent  : public PropertyComponent
{
public:
    //==============================================================================
    ColourPropertyComponent (ComponentDocument& document, const String& name, const Value& colour,
                             const Colour& defaultColour, bool canResetToDefault)
        : PropertyComponent (name),
          colourEditor (document, colour, defaultColour, canResetToDefault)
    {
        addAndMakeVisible (&colourEditor);
    }

    ~ColourPropertyComponent()
    {
    }

    void resized()
    {
        colourEditor.setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
    }

    void refresh() {}

protected:
    ColourEditorComponent colourEditor;
};


#endif   // __JUCER_COLOUREDITORCOMPONENT_JUCEHEADER__
