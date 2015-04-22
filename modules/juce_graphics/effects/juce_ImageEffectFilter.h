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

#ifndef JUCE_IMAGEEFFECTFILTER_H_INCLUDED
#define JUCE_IMAGEEFFECTFILTER_H_INCLUDED


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
        @param scaleFactor      a scale factor that has been applied to the image - e.g. if
                                this is 2, then the image is actually scaled-up to twice the
                                original resolution
        @param alpha            the alpha with which to draw the resultant image to the
                                target context
    */
    virtual void applyEffect (Image& sourceImage,
                              Graphics& destContext,
                              float scaleFactor,
                              float alpha) = 0;

    /** Destructor. */
    virtual ~ImageEffectFilter() {}

};

#endif   // JUCE_IMAGEEFFECTFILTER_H_INCLUDED
