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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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
    DrawableImage() = default;
    DrawableImage (const DrawableImage&) = default;

    /** Sets the image that this drawable will render. */
    explicit DrawableImage (const Image& imageToUse);

    /** Sets the image that this drawable will render. Only the part specified by sourceBounds will
        be drawn in the area specified by destinationBounds.
    */
    DrawableImage (const Image& image,
                   Rectangle<int> destinationBounds,
                   Rectangle<int> sourceBounds);

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

    /** Sets the resampling quality to use when drawing the image.

        This will have the most noticeable effect when the DrawableImage size does not match the
        image size.

        Defaults to highResamplingQuality.
    */
    void setImageResamplingQuality (Graphics::ResamplingQuality newQuality);

    //==============================================================================
    /** @internal */
    bool hitTest (Point<float>) const override;
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;
    /** @internal */
    Rectangle<float> getDrawableBoundsUntransformed() const override;
    /** @internal */
    Path getOutlineAsPath() const override;
    /** @internal */
    bool isImage() const override;
    /** @internal */
    void paint (Graphics& g) const override;

private:
    //==============================================================================
    bool setImageInternal (const Image&);

    //==============================================================================
    Image image;
    Rectangle<int> destinationBounds;
    Rectangle<int> sourceBounds;
    float opacity = 1.0f;
    Colour overlayColour { 0 };
    Parallelogram<float> bounds { { 0.0f, 0.0f, 1.0f, 1.0f } };
    Graphics::ResamplingQuality resamplingQuality = Graphics::ResamplingQuality::highResamplingQuality;

    JUCE_LEAK_DETECTOR (DrawableImage)
};

} // namespace juce
