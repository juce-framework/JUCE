/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_ColourPropertyComponent.h"

//==============================================================================
template <class ComponentType>
class ComponentColourProperty  : public JucerColourPropertyComponent,
                                 private ChangeListener
{
public:
    ComponentColourProperty (const String& name,
                             ComponentType* comp,
                             JucerDocument& doc,
                             const bool canResetToDefault)
        : JucerColourPropertyComponent (name, canResetToDefault),
          component (comp),
          document (doc)
    {
        document.addChangeListener (this);
    }

    ~ComponentColourProperty()
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

protected:
    ComponentType* component;
    JucerDocument& document;
};


//==============================================================================
class ComponentColourIdProperty : public ComponentColourProperty <Component>
{
public:
    //==============================================================================
    ComponentColourIdProperty (Component* const comp,
                               JucerDocument& doc,
                               const int colourId_,
                               const String& name,
                               const bool canResetToDefault)
        : ComponentColourProperty <Component> (name, comp, doc, canResetToDefault),
          colourId (colourId_)
    {
    }

    //==============================================================================
    Colour getColour() const
    {
        return component->findColour (colourId);
    }

    void setColour (Colour newColour)
    {
        if (component->findColour (colourId) != newColour)
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new ColourChangeAction (component,
                                                      *document.getComponentLayout(),
                                                      colourId,
                                                      newColour,
                                                      false),
                              "Change colour");
        }
    }

    void resetToDefault()
    {
        document.getUndoManager().undoCurrentTransactionOnly();

        document.perform (new ColourChangeAction (component,
                                                  *document.getComponentLayout(),
                                                  colourId,
                                                  Colours::black,
                                                  true),
                          "Reset colour");
    }

private:
    const int colourId;

    class ColourChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        ColourChangeAction (Component* const comp,
                            ComponentLayout& l,
                            const int colourId_,
                            Colour newColour_,
                            const bool newColourIsDefault)
            : ComponentUndoableAction<Component> (comp, l),
              colourId (colourId_),
              newColour (newColour_),
              isDefault (newColourIsDefault)
        {
        }

        bool perform()
        {
            showCorrectTab();

            wasSpecified = getComponent()->isColourSpecified (colourId);
            oldColour = getComponent()->findColour (colourId);

            if (isDefault)
                getComponent()->removeColour (colourId);
            else
                getComponent()->setColour (colourId, newColour);

            changed();
            return true;
        }

        bool undo()
        {
            showCorrectTab();

            if (wasSpecified)
                getComponent()->setColour (colourId, oldColour);
            else
                getComponent()->removeColour (colourId);

            if (TextEditor* const te = dynamic_cast<TextEditor*> (getComponent()))
                te->applyFontToAllText (te->getFont());

            changed();
            return true;
        }

        int colourId;
        Colour newColour, oldColour;
        bool isDefault, wasSpecified;
    };
};
