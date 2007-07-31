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
