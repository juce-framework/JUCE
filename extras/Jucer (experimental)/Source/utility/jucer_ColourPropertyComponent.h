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
class PopupColourSelector   : public Component,
                              public ChangeListener,
                              public Value::Listener,
                              public ButtonListener
{
public:
    PopupColourSelector (const Value& colourValue_,
                         const Colour& defaultColour_,
                         const bool canResetToDefault)
        : defaultButton ("Reset to Default"),
          colourValue (colourValue_),
          defaultColour (defaultColour_)
    {
        addAndMakeVisible (&selector);
        selector.setName ("Colour");
        selector.setCurrentColour (getColour());
        selector.addChangeListener (this);

        if (canResetToDefault)
        {
            addAndMakeVisible (&defaultButton);
            defaultButton.addButtonListener (this);
        }

        colourValue.addListener (this);
    }

    ~PopupColourSelector()
    {
    }

    static void showAt (Component* comp, const Value& colourValue,
                        const Colour& defaultColour, const bool canResetToDefault)
    {
        PopupColourSelector colourSelector (colourValue, defaultColour, canResetToDefault);

        PopupMenu m;
        m.addCustomItem (1234, &colourSelector, 300, 400, false);
        m.showAt (comp);
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
            if (newColour == defaultColour && defaultButton.isVisible())
                colourValue = var::null;
            else
                colourValue = newColour.toDisplayString (true);
        }
    }

    void buttonClicked (Button*)
    {
        setColour (defaultColour);
        selector.setCurrentColour (defaultColour);
    }

    void changeListenerCallback (void* source)
    {
        if (selector.getCurrentColour() != getColour())
            setColour (selector.getCurrentColour());
    }

    void valueChanged (Value&)
    {
        selector.setCurrentColour (getColour());
    }

private:
    StoredSettings::ColourSelectorWithSwatches selector;
    TextButton defaultButton;
    Value colourValue;
    Colour defaultColour;
};

//==============================================================================
/**
    A component that shows a colour swatch with hex ARGB value, and which pops up
    a colour selector when you click it.
*/
class ColourEditorComponent    : public Component,
                                 public Value::Listener
{
public:
    ColourEditorComponent (UndoManager* undoManager_, const Value& colourValue_,
                           const Colour& defaultColour_, const bool canResetToDefault_)
        : undoManager (undoManager_), colourValue (colourValue_), defaultColour (defaultColour_),
          canResetToDefault (canResetToDefault_)
    {
        colourValue.addListener (this);
    }

    ~ColourEditorComponent()
    {
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
                colourValue = var::null;
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
        undoManager->beginNewTransaction();
        PopupColourSelector::showAt (this, colourValue, defaultColour, canResetToDefault);
    }

    void valueChanged (Value&)
    {
        refresh();
    }

    juce_UseDebuggingNewOperator

private:
    UndoManager* undoManager;
    Value colourValue;
    Colour lastColour;
    const Colour defaultColour;
    const bool canResetToDefault;
};

//==============================================================================
class ColourPropertyComponent  : public PropertyComponent
{
public:
    //==============================================================================
    ColourPropertyComponent (UndoManager* undoManager, const String& name, const Value& colour,
                             const Colour& defaultColour, bool canResetToDefault)
        : PropertyComponent (name),
          colourEditor (undoManager, colour, defaultColour, canResetToDefault)
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
