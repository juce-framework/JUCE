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
                                      public Value::Listener
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
            g.fillCheckerBoard (getLocalBounds().reduced (2),
                                10, 10,
                                Colour (0xffdddddd).overlaidWith (colour),
                                Colour (0xffffffff).overlaidWith (colour));

            g.setColour (Colours::white.overlaidWith (colour).contrasting());
            g.setFont (Font (getHeight() * 0.6f, Font::bold));
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

            CallOutBox::launchAsynchronously (new PopupColourSelector (colourValue,
                                                                       defaultColour,
                                                                       canResetToDefault),
                                              getScreenBounds(), nullptr);
        }

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
                                   public ChangeListener,
                                   public Value::Listener,
                                   public ButtonListener
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
                defaultButton.addListener (this);
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

        void buttonClicked (Button*) override
        {
            setColour (defaultColour);
            selector.setCurrentColour (defaultColour);
        }

        void changeListenerCallback (ChangeBroadcaster*) override
        {
            if (selector.getCurrentColour() != getColour())
                setColour (selector.getCurrentColour());
        }

        void valueChanged (Value&) override
        {
            selector.setCurrentColour (getColour());
        }

    private:
        StoredSettings::ColourSelectorWithSwatches selector;
        TextButton defaultButton;
        Value colourValue;
        Colour defaultColour;
    };

    ColourEditorComponent colourEditor;
};


#endif  // JUCER_COLOURPROPERTYCOMPONENT_H_INCLUDED
