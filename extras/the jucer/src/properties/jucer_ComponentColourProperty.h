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

#ifndef __JUCER_COMPONENTCOLOURPROPERTY_JUCEHEADER__
#define __JUCER_COMPONENTCOLOURPROPERTY_JUCEHEADER__

#include "jucer_ColourPropertyComponent.h"


//==============================================================================
/**
*/
template <class ComponentType>
class ComponentColourProperty  : public ColourPropertyComponent,
                                 private ChangeListener
{
public:
    ComponentColourProperty (const String& name,
                             ComponentType* component_,
                             JucerDocument& document_,
                             const bool canResetToDefault)
        : ColourPropertyComponent (name, canResetToDefault),
          component (component_),
          document (document_)
    {
        document.addChangeListener (this);
    }

    ~ComponentColourProperty()
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

protected:
    ComponentType* component;
    JucerDocument& document;
};


//==============================================================================
/**
*/
class ComponentColourIdProperty : public ComponentColourProperty <Component>
{
public:
    //==============================================================================
    ComponentColourIdProperty (Component* const component_,
                               JucerDocument& document_,
                               const int colourId_,
                               const String& name,
                               const bool canResetToDefault)
        : ComponentColourProperty <Component> (name, component_, document_, canResetToDefault),
          colourId (colourId_)
    {
    }

    ~ComponentColourIdProperty()
    {
    }

    //==============================================================================
    const Colour getColour() const
    {
        return component->findColour (colourId);
    }

    void setColour (const Colour& newColour)
    {
        if (component->findColour (colourId) != newColour)
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new ColourChangeAction (component,
                                                      *document.getComponentLayout(),
                                                      colourId,
                                                      newColour,
                                                      false),
                              T("Change colour"));
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
                          T("Reset colour"));
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    const int colourId;

    class ColourChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        ColourChangeAction (Component* const comp,
                            ComponentLayout& layout,
                            const int colourId_,
                            const Colour& newColour_,
                            const bool newColourIsDefault)
            : ComponentUndoableAction <Component> (comp, layout),
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

            TextEditor* const te = dynamic_cast <TextEditor*> (getComponent());
            if (te != 0)
                te->applyFontToAllText (te->getFont());

            changed();
            return true;
        }

        int colourId;
        Colour newColour, oldColour;
        bool isDefault, wasSpecified;
    };
};


#endif   // __JUCER_COMPONENTCOLOURPROPERTY_JUCEHEADER__
