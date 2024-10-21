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

namespace juce
{

//==============================================================================
/**
    A graphical effect filter that can be applied to components.

    An ImageEffectFilter can be applied to the image that a component
    paints before it hits the screen.

    This is used for adding effects like shadows, blurs, etc.

    @see Component::setComponentEffect

    @tags{Graphics}
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
    virtual ~ImageEffectFilter() = default;

};

} // namespace juce
