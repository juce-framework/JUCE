/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_DrawableImage.h"
#include "../imaging/juce_ImageCache.h"


//==============================================================================
DrawableImage::DrawableImage()
    : image (0),
      canDeleteImage (false),
      opacity (1.0f),
      overlayColour (0x00000000)
{
}

DrawableImage::~DrawableImage()
{
    clearImage();
}

//==============================================================================
void DrawableImage::clearImage()
{
    if (canDeleteImage && image != 0)
    {
        if (ImageCache::isImageInCache (image))
            ImageCache::release (image);
        else
            delete image;
    }

    image = 0;
}

void DrawableImage::setImage (const Image& imageToCopy)
{
    clearImage();
    image = new Image (imageToCopy);
    canDeleteImage = true;
}

void DrawableImage::setImage (Image* imageToUse,
                              const bool releaseWhenNotNeeded)
{
    clearImage();
    image = imageToUse;
    canDeleteImage = releaseWhenNotNeeded;
}

void DrawableImage::setOpacity (const float newOpacity)
{
    opacity = newOpacity;
}

void DrawableImage::setOverlayColour (const Colour& newOverlayColour)
{
    overlayColour = newOverlayColour;
}

//==============================================================================
void DrawableImage::render (const Drawable::RenderingContext& context) const
{
    if (image != 0)
    {
        if (opacity > 0.0f && ! overlayColour.isOpaque())
        {
            context.g.setOpacity (context.opacity * opacity);
            context.g.drawImageTransformed (image,
                                            0, 0, image->getWidth(), image->getHeight(),
                                            context.transform, false);
        }

        if (! overlayColour.isTransparent())
        {
            context.g.setColour (overlayColour.withMultipliedAlpha (context.opacity));
            context.g.drawImageTransformed (image,
                                            0, 0, image->getWidth(), image->getHeight(),
                                            context.transform, true);
        }
    }
}

void DrawableImage::getBounds (float& x, float& y, float& width, float& height) const
{
    x = 0.0f;
    y = 0.0f;
    width = 0.0f;
    height = 0.0f;

    if (image != 0)
    {
        width = (float) image->getWidth();
        height = (float) image->getHeight();
    }
}

bool DrawableImage::hitTest (float x, float y) const
{
    return image != 0
            && x >= 0.0f
            && y >= 0.0f
            && x < image->getWidth()
            && y < image->getHeight()
            && image->getPixelAt (roundFloatToInt (x), roundFloatToInt (y)).getAlpha() >= 127;
}

Drawable* DrawableImage::createCopy() const
{
    DrawableImage* const di = new DrawableImage();

    di->opacity = opacity;
    di->overlayColour = overlayColour;

    if (image != 0)
    {
        if ((! canDeleteImage) || ! ImageCache::isImageInCache (image))
        {
            di->setImage (*image);
        }
        else
        {
            ImageCache::incReferenceCount (image);
            di->setImage (image, true);
        }
    }

    return di;
}


END_JUCE_NAMESPACE
