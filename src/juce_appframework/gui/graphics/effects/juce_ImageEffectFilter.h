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

#ifndef __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__
#define __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__

#include "../../graphics/contexts/juce_Graphics.h"


//==============================================================================
/**
    A graphical effect filter that can be applied to components.

    An ImageEffectFilter can be applied to the image that a component
    paints before it hits the screen.

    This is used for adding effects like shadows, blurs, etc.

    @see Component::setComponentEffect
*/
class JUCE_API  ImageEffectFilter
{
public:
    //==============================================================================
    /** Overridden to render the effect.

        The implementation of this method must use the image that is passed in
        as its source, and should render its output to the graphics context passed in.

        @param sourceImage      the image that the source component has just rendered with
                                its paint() method. The image may or may not have an alpha
                                channel, depending on whether the component is opaque.
        @param destContext      the graphics context to use to draw the resultant image.
    */
    virtual void applyEffect (Image& sourceImage,
                              Graphics& destContext) = 0;

    /** Destructor. */
    virtual ~ImageEffectFilter() {}

};

#endif   // __JUCE_IMAGEEFFECTFILTER_JUCEHEADER__
