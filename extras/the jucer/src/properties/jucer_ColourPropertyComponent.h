/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
