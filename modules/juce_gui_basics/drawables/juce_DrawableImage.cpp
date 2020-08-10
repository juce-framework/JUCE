/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

DrawableImage::DrawableImage() : bounds ({ 0.0f, 0.0f, 1.0f, 1.0f })
{
}

DrawableImage::DrawableImage (const DrawableImage& other)
    : Drawable (other),
      image (other.image),
      opacity (other.opacity),
      overlayColour (other.overlayColour),
      bounds (other.bounds)
{
    setBounds (other.getBounds());
}

DrawableImage::~DrawableImage()
{
}

std::unique_ptr<Drawable> DrawableImage::createCopy() const
{
    return std::make_unique<DrawableImage> (*this);
}

//==============================================================================
void DrawableImage::setImage (const Image& imageToUse)
{
    if (image != imageToUse)
    {
        image = imageToUse;
        setBounds (image.getBounds());
        setBoundingBox (image.getBounds().toFloat());
        repaint();
    }
}

void DrawableImage::setOpacity (const float newOpacity)
{
    opacity = newOpacity;
}

void DrawableImage::setOverlayColour (Colour newOverlayColour)
{
    overlayColour = newOverlayColour;
}

void DrawableImage::setBoundingBox (Rectangle<float> newBounds)
{
    setBoundingBox (Parallelogram<float> (newBounds));
}

void DrawableImage::setBoundingBox (Parallelogram<float> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;

        if (image.isValid())
        {
            auto tr = bounds.topLeft + (bounds.topRight   - bounds.topLeft) / (float) image.getWidth();
            auto bl = bounds.topLeft + (bounds.bottomLeft - bounds.topLeft) / (float) image.getHeight();

            auto t = AffineTransform::fromTargetPoints (bounds.topLeft.x, bounds.topLeft.y,
                                                        tr.x, tr.y,
                                                        bl.x, bl.y);

            if (t.isSingularity())
                t = {};

            setTransform (t);
        }
    }
}

//==============================================================================
void DrawableImage::paint (Graphics& g)
{
    if (image.isValid())
    {
        if (opacity > 0.0f && ! overlayColour.isOpaque())
        {
            g.setOpacity (opacity);
            g.drawImageAt (image, 0, 0, false);
        }

        if (! overlayColour.isTransparent())
        {
            g.setColour (overlayColour.withMultipliedAlpha (opacity));
            g.drawImageAt (image, 0, 0, true);
        }
    }
}

Rectangle<float> DrawableImage::getDrawableBounds() const
{
    return image.getBounds().toFloat();
}

bool DrawableImage::hitTest (int x, int y)
{
    return Drawable::hitTest (x, y) && image.isValid() && image.getPixelAt (x, y).getAlpha() >= 127;
}

Path DrawableImage::getOutlineAsPath() const
{
    return {}; // not applicable for images
}

} // namespace juce
