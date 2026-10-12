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

DrawableImage::DrawableImage (const Image& imageToUse)
{
    setImageInternal (imageToUse);
}

DrawableImage::DrawableImage (const Image& imageIn,
                              Rectangle<int> destinationBoundsIn,
                              Rectangle<int> sourceBoundsIn)
    : image (imageIn),
      destinationBounds (destinationBoundsIn),
      sourceBounds (sourceBoundsIn)
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
        callListeners();
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
            auto tr = bounds.topLeft + (bounds.topRight - bounds.topLeft) / (float) image.getWidth();
            auto bl = bounds.topLeft + (bounds.bottomLeft - bounds.topLeft) / (float) image.getHeight();

            auto t = AffineTransform::fromTargetPoints (
                bounds.topLeft.x, bounds.topLeft.y, tr.x, tr.y, bl.x, bl.y);

            if (t.isSingularity())
                t = {};

            setDrawableTransform (t);
        }
    }
}

//==============================================================================
void DrawableImage::paint (Graphics& g) const
{
    const Graphics::ScopedSaveState ss (g);

    if (! image.isValid())
        return;

    g.setImageResamplingQuality (resamplingQuality);

    const auto dst = destinationBounds.isEmpty() ? image.getBounds() : destinationBounds;
    const auto src = sourceBounds.isEmpty() ? image.getBounds() : sourceBounds;

    if (opacity > 0.0f && ! overlayColour.isOpaque())
    {
        g.setOpacity (opacity);

        g.drawImage (image,
                     dst.getX(),
                     dst.getY(),
                     dst.getWidth(),
                     dst.getHeight(),
                     src.getX(),
                     src.getY(),
                     src.getWidth(),
                     src.getHeight(),
                     false);
    }

    if (! overlayColour.isTransparent())
    {
        g.setColour (overlayColour.withMultipliedAlpha (opacity));

        g.drawImage (image,
                     dst.getX(),
                     dst.getY(),
                     dst.getWidth(),
                     dst.getHeight(),
                     src.getX(),
                     src.getY(),
                     src.getWidth(),
                     src.getHeight(),
                     true);
    }
}

Rectangle<float> DrawableImage::getDrawableBoundsUntransformed() const
{
    return image.getBounds().toFloat();
}

bool DrawableImage::hitTest (Point<float> p) const
{
    const auto pInt = p.roundToInt();
    return image.isValid() && image.getPixelAt (pInt.x, pInt.y).getAlpha() >= 127;
}

Path DrawableImage::getOutlineAsPath() const
{
    return {}; // not applicable for images
}

bool DrawableImage::isImage() const
{
    return true;
}

void DrawableImage::setImageResamplingQuality (Graphics::ResamplingQuality newQuality)
{
    resamplingQuality = newQuality;
}

//==============================================================================
bool DrawableImage::setImageInternal (const Image& imageToUse)
{
    if (image != imageToUse)
    {
        image = imageToUse;
        setBoundingBox (image.getBounds().toFloat());
        return true;
    }

    return false;
}

} // namespace juce
