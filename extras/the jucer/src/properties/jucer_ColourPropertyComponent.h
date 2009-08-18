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

#ifndef __JUCER_COLOURPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCER_COLOURPROPERTYCOMPONENT_JUCEHEADER__

#include "../utility/jucer_ColourEditorComponent.h"


//==============================================================================
/**
*/
class ColourPropertyComponent  : public PropertyComponent
{
public:
    //==============================================================================
    ColourPropertyComponent (const String& name,
                             const bool canResetToDefault)
        : PropertyComponent (name)
    {
        addAndMakeVisible (new ColourPropEditorComponent (this, canResetToDefault));
    }

    ~ColourPropertyComponent()
    {
        deleteAllChildren();
    }

    //==============================================================================
    virtual void setColour (const Colour& newColour) = 0;
    virtual const Colour getColour() const = 0;
    virtual void resetToDefault() = 0;

    void refresh()
    {
        ((ColourPropEditorComponent*) getChildComponent (0))->refresh();
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    class ColourPropEditorComponent     : public ColourEditorComponent
    {
        ColourPropertyComponent* const owner;

    public:
        ColourPropEditorComponent (ColourPropertyComponent* const owner_,
                                   const bool canResetToDefault)
            : ColourEditorComponent (canResetToDefault),
              owner (owner_)
        {}

        ~ColourPropEditorComponent() {}

        void setColour (const Colour& newColour)
        {
            owner->setColour (newColour);
        }

        const Colour getColour() const
        {
            return owner->getColour();
        }

        void resetToDefault()
        {
            owner->resetToDefault();
        }
    };
};


#endif   // __JUCER_COLOURPROPERTYCOMPONENT_JUCEHEADER__
