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
    Defines a drop-shadow effect.

    @tags{Graphics}
*/
struct JUCE_API  DropShadow
{
    /** Creates a default drop-shadow effect. */
    DropShadow() = default;

    /** Creates a drop-shadow object with the given parameters. */
    DropShadow (Colour shadowColour, int radius, Point<int> offset) noexcept;

    /** Renders a drop-shadow based on the alpha-channel of the given image. */
    void drawForImage (Graphics& g, const Image& srcImage) const;

    /** Renders a drop-shadow based on the shape of a path. */
    void drawForPath (Graphics& g, const Path& path) const;

    /** Renders a drop-shadow for a rectangle.
        Note that for speed, this approximates the shadow using gradients.
    */
    void drawForRectangle (Graphics& g, const Rectangle<int>& area) const;

    /** The colour with which to render the shadow.
        In most cases you'll probably want to leave this as black with an alpha
        value of around 0.5
    */
    Colour colour { 0x90000000 };

    /** The approximate spread of the shadow. */
    int radius { 4 };

    /** The offset of the shadow. */
    Point<int> offset;
};

//==============================================================================
/**
    An effect filter that adds a drop-shadow behind the image's content.

    (This will only work on images/components that aren't opaque, of course).

    When added to a component, this effect will draw a soft-edged
    shadow based on what gets drawn inside it. The shadow will also
    be applied to the component's children.

    For speed, this doesn't use a proper gaussian blur, but cheats by
    using a simple bilinear filter. If you need a really high-quality
    shadow, check out ImageConvolutionKernel::createGaussianBlur()

    @see Component::setComponentEffect

    @tags{Graphics}
*/
class JUCE_API  DropShadowEffect  : public ImageEffectFilter
{
public:
    //==============================================================================
    /** Creates a default drop-shadow effect.
        To customise the shadow's appearance, use the setShadowProperties() method.
    */
    DropShadowEffect();

    /** Destructor. */
    ~DropShadowEffect() override;

    //==============================================================================
    /** Sets up parameters affecting the shadow's appearance. */
    void setShadowProperties (const DropShadow& newShadow);

    //==============================================================================
    /** @internal */
    void applyEffect (Image& sourceImage, Graphics& destContext, float scaleFactor, float alpha) override;


private:
    //==============================================================================
    DropShadow shadow;

    JUCE_LEAK_DETECTOR (DropShadowEffect)
};

} // namespace juce
