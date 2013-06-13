/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

    void changeListenerCallback (ChangeBroadcaster*)
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
    Colour getColour() const
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
