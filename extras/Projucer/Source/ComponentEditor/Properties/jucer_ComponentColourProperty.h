/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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

    ~ComponentColourProperty() override
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
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
    Colour getColour() const override
    {
        return component->findColour (colourId);
    }

    void setColour (Colour newColour) override
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

    void resetToDefault() override
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

        bool perform() override
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

        bool undo() override
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
