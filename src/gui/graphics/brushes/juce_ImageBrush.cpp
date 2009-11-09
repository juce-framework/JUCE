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


#include "juce_ImageBrush.h"
#include "../contexts/juce_LowLevelGraphicsContext.h"


//==============================================================================
ImageBrush::ImageBrush (Image* const image_,
                        const int anchorX_,
                        const int anchorY_,
                        const float opacity_) throw()
   : image (image_),
     anchorX (anchorX_),
     anchorY (anchorY_),
     opacity (opacity_)
{
    jassert (image != 0); // not much point creating a brush without an image, is there?

    if (image != 0)
    {
        if (image->getWidth() == 0 || image->getHeight() == 0)
        {
            jassertfalse // you've passed in an empty image - not exactly brilliant for tiling.
            image = 0;
        }
    }
}

ImageBrush::~ImageBrush() throw()
{
}

Brush* ImageBrush::createCopy() const throw()
{
    return new ImageBrush (image, anchorX, anchorY, opacity);
}

void ImageBrush::multiplyOpacity (const float multiple) throw()
{
    opacity *= multiple;
}

bool ImageBrush::isInvisible() const throw()
{
    return opacity == 0.0f;
}

void ImageBrush::applyTransform (const AffineTransform& /*transform*/) throw()
{
    //xxx should probably be smarter and warp the image
}

void ImageBrush::getStartXY (int& x, int& y) const throw()
{
    x -= anchorX;
    y -= anchorY;

    const int iw = image->getWidth();
    const int ih = image->getHeight();

    if (x < 0)
        x = ((x / iw) - 1) * iw;
    else
        x = (x / iw) * iw;

    if (y < 0)
        y = ((y / ih) - 1) * ih;
    else
        y = (y / ih) * ih;


    x += anchorX;
    y += anchorY;
}

//==============================================================================
void ImageBrush::paintRectangle (LowLevelGraphicsContext& context,
                                 int x, int y, int w, int h) throw()
{
    context.saveState();

    if (image != 0 && context.reduceClipRegion (x, y, w, h))
    {
        const int right = x + w;
        const int bottom = y + h;

        const int iw = image->getWidth();
        const int ih = image->getHeight();

        int startX = x;
        getStartXY (startX, y);

        while (y < bottom)
        {
            x = startX;

            while (x < right)
            {
                context.setOpacity (opacity);
                context.blendImage (*image, x, y, iw, ih, 0, 0);
                x += iw;
            }

            y += ih;
        }
    }

    context.restoreState();
}

void ImageBrush::paintPath (LowLevelGraphicsContext& context,
                            const Path& path, const AffineTransform& transform) throw()
{
    if (image != 0)
    {
        Rectangle clip (context.getClipBounds());
        context.setOpacity (opacity);

        {
            float x, y, w, h;
            path.getBoundsTransformed (transform, x, y, w, h);

            clip = clip.getIntersection (Rectangle ((int) floorf (x),
                                                    (int) floorf (y),
                                                    2 + (int) floorf (w),
                                                    2 + (int) floorf (h)));
        }

        int x = clip.getX();
        int y = clip.getY();
        const int right = clip.getRight();
        const int bottom = clip.getBottom();

        const int iw = image->getWidth();
        const int ih = image->getHeight();

        int startX = x;
        getStartXY (startX, y);

        while (y < bottom)
        {
            x = startX;

            while (x < right)
            {
                context.fillPathWithImage (path, transform, *image, x, y);
                x += iw;
            }

            y += ih;
        }
    }
}

void ImageBrush::paintAlphaChannel (LowLevelGraphicsContext& context,
                                    const Image& alphaChannelImage, int imageX, int imageY,
                                    int x, int y, int w, int h) throw()
{
    context.saveState();

    if (image != 0 && context.reduceClipRegion (x, y, w, h))
    {
        context.setOpacity (opacity);
        const Rectangle clip (context.getClipBounds());
        x = clip.getX();
        y = clip.getY();
        const int right = clip.getRight();
        const int bottom = clip.getBottom();

        const int iw = image->getWidth();
        const int ih = image->getHeight();

        int startX = x;
        getStartXY (startX, y);

        while (y < bottom)
        {
            x = startX;

            while (x < right)
            {
                context.fillAlphaChannelWithImage (alphaChannelImage,
                                                   imageX, imageY, *image,
                                                   x, y);
                x += iw;
            }

            y += ih;
        }
    }

    context.restoreState();
}


END_JUCE_NAMESPACE
