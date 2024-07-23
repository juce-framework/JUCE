/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/**
    An image that will be resampled before it is drawn.

    A plain Image only stores plain pixels, but does not store any information
    about how these pixels correspond to points. This means that if the image's
    dimensions are interpreted as points, then the image will be blurry when
    drawn on high resolution displays. If the image's dimensions are instead
    interpreted as corresponding to exact pixel positions, then the logical
    size of the image will change depending on the scale factor of the screen
    used to draw it.

    The ScaledImage class is designed to store an image alongside a scale
    factor that informs a renderer how to convert between the image's pixels
    and points.

    @tags{GUI}
*/
class JUCE_API  ScaledImage
{
public:
    /** Creates a ScaledImage with an invalid image and unity scale.
    */
    ScaledImage() = default;

    /** Creates a ScaledImage from an Image, where the dimensions of the image
        in pixels are exactly equal to its dimensions in points.
    */
    explicit ScaledImage (const Image& imageIn)
        : ScaledImage (imageIn, 1.0) {}

    /** Creates a ScaledImage from an Image, using a custom scale factor.

        A scale of 1.0 means that the image's dimensions in pixels is equal to
        its dimensions in points.

        A scale of 2.0 means that the image contains 2 pixels per point in each
        direction.
    */
    ScaledImage (const Image& imageIn, double scaleIn)
        : image (imageIn), scaleFactor (scaleIn) {}

    /** Returns the image at its original dimensions. */
    Image getImage() const { return image; }

    /** Returns the image's scale. */
    double getScale() const { return scaleFactor; }

    /** Returns the bounds of this image expressed in points. */
    Rectangle<double> getScaledBounds() const { return image.getBounds().toDouble() / scaleFactor; }

private:
    Image image;
    double scaleFactor = 1.0;
};

} // namespace juce
