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

DrawableImage::DrawableImage (const Image& imageToUse)
{
    setImageInternal (imageToUse);
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
    if (setImageInternal (imageToUse))
        repaint();
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

//==============================================================================
bool DrawableImage::setImageInternal (const Image& imageToUse)
{
    if (image != imageToUse)
    {
        image = imageToUse;
        setBounds (image.getBounds());
        setBoundingBox (image.getBounds().toFloat());
        return true;
    }

    return false;
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> DrawableImage::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::image);
}

} // namespace juce
