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
    A drawable object which is a bitmap image.

    @see Drawable

    @tags{GUI}
*/
class JUCE_API  DrawableImage  : public Drawable
{
public:
    //==============================================================================
    DrawableImage();
    DrawableImage (const DrawableImage&);

    /** Sets the image that this drawable will render. */
    explicit DrawableImage (const Image& imageToUse);

    /** Destructor. */
    ~DrawableImage() override;

    //==============================================================================
    /** Sets the image that this drawable will render. */
    void setImage (const Image& imageToUse);

    /** Returns the current image. */
    const Image& getImage() const noexcept                      { return image; }

    /** Sets the opacity to use when drawing the image. */
    void setOpacity (float newOpacity);

    /** Returns the image's opacity. */
    float getOpacity() const noexcept                           { return opacity; }

    /** Sets a colour to draw over the image's alpha channel.

        By default this is transparent so isn't drawn, but if you set a non-transparent
        colour here, then it will be overlaid on the image, using the image's alpha
        channel as a mask.

        This is handy for doing things like darkening or lightening an image by overlaying
        it with semi-transparent black or white.
    */
    void setOverlayColour (Colour newOverlayColour);

    /** Returns the overlay colour. */
    Colour getOverlayColour() const noexcept                    { return overlayColour; }

    /** Sets the bounding box within which the image should be displayed. */
    void setBoundingBox (Parallelogram<float> newBounds);

    /** Sets the bounding box within which the image should be displayed. */
    void setBoundingBox (Rectangle<float> newBounds);

    /** Returns the position to which the image's top-left corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    Parallelogram<float> getBoundingBox() const noexcept        { return bounds; }

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    bool hitTest (int x, int y) override;
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;
    /** @internal */
    Rectangle<float> getDrawableBounds() const override;
    /** @internal */
    Path getOutlineAsPath() const override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    bool setImageInternal (const Image&);

    //==============================================================================
    Image image;
    float opacity = 1.0f;
    Colour overlayColour { 0 };
    Parallelogram<float> bounds;

    DrawableImage& operator= (const DrawableImage&);
    JUCE_LEAK_DETECTOR (DrawableImage)
};

} // namespace juce
