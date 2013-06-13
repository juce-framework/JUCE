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
    virtual Colour getColour() const = 0;
    virtual void resetToDefault() = 0;

    void refresh()
    {
        ((ColourPropEditorComponent*) getChildComponent (0))->refresh();
    }

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

        Colour getColour() const
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
