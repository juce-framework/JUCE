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

#ifndef __JUCE_REDUCEOPACITYEFFECT_JUCEHEADER__
#define __JUCE_REDUCEOPACITYEFFECT_JUCEHEADER__

#include "juce_ImageEffectFilter.h"


//==============================================================================
/**
    An effect filter that reduces the image's opacity.

    This can be used to make a component (and its child components) more
    transparent.

    @see Component::setComponentEffect
*/
class JUCE_API  ReduceOpacityEffect  : public ImageEffectFilter
{
public:
    //==============================================================================
    /** Creates the effect object.

        The opacity of the component to which the effect is applied will be
        scaled by the given factor (in the range 0 to 1.0f).
    */
    ReduceOpacityEffect (const float opacity = 1.0f);

    /** Destructor. */
    ~ReduceOpacityEffect();

    /** Sets how much to scale the component's opacity.

        @param newOpacity   should be between 0 and 1.0f
    */
    void setOpacity (const float newOpacity);


    //==============================================================================
    /** @internal */
    void applyEffect (Image& sourceImage, Graphics& destContext);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    float opacity;
};


#endif   // __JUCE_REDUCEOPACITYEFFECT_JUCEHEADER__
