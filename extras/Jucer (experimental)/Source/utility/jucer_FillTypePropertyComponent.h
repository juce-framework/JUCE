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

#ifndef __JUCER_FILLTYPEPROPERTYCOMPONENT_H_88CF1300__
#define __JUCER_FILLTYPEPROPERTYCOMPONENT_H_88CF1300__

//==============================================================================
class PopupFillSelector   : public Component,
                            public ChangeListener,
                            public ValueTree::Listener,
                            public ButtonListener
{
public:
    PopupFillSelector (const ValueTree& fillState_, UndoManager* undoManager_)
        : fillState (fillState_),
          undoManager (undoManager_)
    {
        addAndMakeVisible (&colourPicker);
        colourPicker.setName ("Colour");
        colourPicker.addChangeListener (this);

        fillState.addListener (this);
    }

    ~PopupFillSelector()
    {
    }

    static void showAt (Component* comp, const ValueTree& fill, UndoManager* undoManager)
    {
        PopupFillSelector popup (fill, undoManager);

        PopupMenu m;
        m.addCustomItem (1234, &popup, 300, 400, false);
        m.showAt (comp);
    }

    void resized()
    {
        colourPicker.setBounds (0, 0, getWidth(), getHeight());
    }

    void buttonClicked (Button*)
    {
    }

    void changeListenerCallback (void* source)
    {
        const FillType currentFill (Drawable::ValueTreeWrapperBase::readFillType (fillState));

        if (currentFill.isColour())
        {
            const FillType newFill (colourPicker.getCurrentColour());

            if (currentFill != newFill)
                Drawable::ValueTreeWrapperBase::writeFillType (fillState, newFill, undoManager);
        }
    }

    void refresh()
    {
        const FillType newFill (Drawable::ValueTreeWrapperBase::readFillType (fillState));

        if (newFill.isColour())
            colourPicker.setCurrentColour (newFill.colour);
    }

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property)  { refresh(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)                                 { refresh(); }
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)                                  {}

private:
    StoredSettings::ColourSelectorWithSwatches colourPicker;
    ValueTree fillState;
    UndoManager* undoManager;
};


//==============================================================================
/**
    A component that shows a fill type swatch, and pops up a editor panel
    when you click it.
*/
class FillTypeEditorComponent    : public Component,
                                   public ValueTree::Listener
{
public:
    FillTypeEditorComponent (const ValueTree& fillState_, UndoManager* undoManager_)
        : fillState (fillState_), undoManager (undoManager_)
    {
        fillState.addListener (this);
        refresh();
    }

    ~FillTypeEditorComponent()
    {
    }

    void paint (Graphics& g)
    {
        g.setColour (Colours::grey);
        g.drawRect (0, 0, getWidth(), getHeight(), 2);

        g.fillCheckerBoard (2, 2, getWidth() - 4, getHeight() - 4, 10, 10,
                            Colour (0xffdddddd), Colour (0xffffffff));

        FillType f (fillType);

        if (f.gradient != 0)
        {
            f.gradient->point1.setXY (2.0f, getHeight() / 2.0f);
            f.gradient->point2.setXY (getWidth() - 2.0f, getHeight() / 2.0f);
        }

        g.setFillType (f);
        g.fillRect (2, 2, getWidth() - 4, getHeight() - 4);

        if (fillType.isColour())
        {
            g.setColour (Colours::white.overlaidWith (fillType.colour).contrasting());
            g.setFont (getHeight() * 0.6f, Font::bold);
            g.drawFittedText (fillType.colour.toDisplayString (true),
                              2, 1, getWidth() - 4, getHeight() - 1,
                              Justification::centred, 1);
        }
    }

    void refresh()
    {
        const FillType newFill (Drawable::ValueTreeWrapperBase::readFillType (fillState));

        if (newFill != fillType)
        {
            fillType = newFill;
            repaint();
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        undoManager->beginNewTransaction();
        PopupFillSelector::showAt (this, fillState, undoManager);
    }

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property)  { refresh(); }
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)                                 { refresh(); }
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)                                  {}

    juce_UseDebuggingNewOperator

private:
    ValueTree fillState;
    UndoManager* undoManager;
    FillType fillType;
};

//==============================================================================
class FillTypePropertyComponent  : public PropertyComponent
{
public:
    //==============================================================================
    FillTypePropertyComponent (UndoManager* undoManager, const String& name, const ValueTree& fill)
        : PropertyComponent (name),
          editor (fill, undoManager)
    {
        addAndMakeVisible (&editor);
    }

    ~FillTypePropertyComponent()
    {
    }

    void resized()
    {
        editor.setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
    }

    void refresh() {}

protected:
    FillTypeEditorComponent editor;
};


#endif  // __JUCER_FILLTYPEPROPERTYCOMPONENT_H_88CF1300__
