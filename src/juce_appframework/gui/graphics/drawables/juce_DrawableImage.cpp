/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

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
void DrawableImage::draw (Graphics& g, const AffineTransform& transform) const
{
    if (image != 0)
    {
        const Colour oldColour (g.getCurrentColour()); // save this so we can restore it later

        if (opacity > 0.0f && ! overlayColour.isOpaque())
        {
            g.setColour (oldColour.withMultipliedAlpha (opacity));

            g.drawImageTransformed (image,
                                    0, 0, image->getWidth(), image->getHeight(),
                                    transform, false);
        }

        if (! overlayColour.isTransparent())
        {
            g.setColour (overlayColour.withMultipliedAlpha (oldColour.getFloatAlpha()));

            g.drawImageTransformed (image,
                                    0, 0, image->getWidth(), image->getHeight(),
                                    transform, true);
        }

        g.setColour (oldColour);
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
