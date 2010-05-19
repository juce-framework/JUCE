/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
    controlPoints[1].setXY (1.0f, 0.0f);
    controlPoints[2].setXY (0.0f, 1.0f);
}

DrawableImage::~DrawableImage()
{
    clearImage();
}

//==============================================================================
void DrawableImage::clearImage()
{
    if (canDeleteImage && image != 0)
        ImageCache::releaseOrDelete (image);

    image = 0;
}

void DrawableImage::setImage (const Image& imageToCopy)
{
    clearImage();
    image = new Image (imageToCopy);
    canDeleteImage = true;

    controlPoints[0].setXY (0.0f, 0.0f);
    controlPoints[1].setXY ((float) image->getWidth(), 0.0f);
    controlPoints[2].setXY (0.0f, (float) image->getHeight());
}

void DrawableImage::setImage (Image* imageToUse,
                              const bool releaseWhenNotNeeded)
{
    clearImage();
    image = imageToUse;
    canDeleteImage = releaseWhenNotNeeded;

    if (image != 0)
    {
        controlPoints[0].setXY (0.0f, 0.0f);
        controlPoints[1].setXY ((float) image->getWidth(), 0.0f);
        controlPoints[2].setXY (0.0f, (float) image->getHeight());
    }
}

void DrawableImage::setOpacity (const float newOpacity)
{
    opacity = newOpacity;
}

void DrawableImage::setOverlayColour (const Colour& newOverlayColour)
{
    overlayColour = newOverlayColour;
}

void DrawableImage::setTransform (const Point<float>& imageTopLeftPosition,
                                  const Point<float>& imageTopRightPosition,
                                  const Point<float>& imageBottomLeftPosition)
{
    controlPoints[0] = imageTopLeftPosition;
    controlPoints[1] = imageTopRightPosition;
    controlPoints[2] = imageBottomLeftPosition;
}

//==============================================================================
const AffineTransform DrawableImage::getTransform() const
{
    if (image == 0)
        return AffineTransform::identity;

    const Point<float> tr (controlPoints[0] + (controlPoints[1] - controlPoints[0]) / image->getWidth());
    const Point<float> bl (controlPoints[0] + (controlPoints[2] - controlPoints[0]) / image->getHeight());

    return AffineTransform::fromTargetPoints (controlPoints[0].getX(), controlPoints[0].getY(),
                                              tr.getX(), tr.getY(),
                                              bl.getX(), bl.getY());
}

void DrawableImage::render (const Drawable::RenderingContext& context) const
{
    if (image != 0)
    {
        const AffineTransform t (getTransform().followedBy (context.transform));

        if (opacity > 0.0f && ! overlayColour.isOpaque())
        {
            context.g.setOpacity (context.opacity * opacity);
            context.g.drawImageTransformed (image, image->getBounds(), t, false);
        }

        if (! overlayColour.isTransparent())
        {
            context.g.setColour (overlayColour.withMultipliedAlpha (context.opacity));
            context.g.drawImageTransformed (image, image->getBounds(), t, true);
        }
    }
}

const Rectangle<float> DrawableImage::getBounds() const
{
    if (image == 0)
        return Rectangle<float>();

    const Point<float> bottomRight (controlPoints[1] + (controlPoints[2] - controlPoints[0]));
    float minX = bottomRight.getX();
    float maxX = minX;
    float minY = bottomRight.getY();
    float maxY = minY;

    for (int i = 0; i < 3; ++i)
    {
        minX = jmin (minX, controlPoints[i].getX());
        maxX = jmax (maxX, controlPoints[i].getX());
        minY = jmin (minY, controlPoints[i].getY());
        maxY = jmax (maxY, controlPoints[i].getY());
    }

    return Rectangle<float> (minX, minY, maxX - minX, maxY - minY);
}

bool DrawableImage::hitTest (float x, float y) const
{
    if (image == 0)
        return false;

    getTransform().inverted().transformPoint (x, y);

    const int ix = roundToInt (x);
    const int iy = roundToInt (y);

    return ix >= 0
            && iy >= 0
            && ix < image->getWidth()
            && iy < image->getHeight()
            && image->getPixelAt (ix, iy).getAlpha() >= 127;
}

Drawable* DrawableImage::createCopy() const
{
    DrawableImage* const di = new DrawableImage();

    di->opacity = opacity;
    di->overlayColour = overlayColour;

    for (int i = 0; i < 4; ++i)
        di->controlPoints[i] = controlPoints[i];

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

//==============================================================================
const Identifier DrawableImage::valueTreeType ("Image");

namespace DrawableImageHelpers
{
    static const Identifier opacity ("opacity");
    static const Identifier overlay ("overlay");
    static const Identifier image ("image");
    static const Identifier topLeft ("topLeft");
    static const Identifier topRight ("topRight");
    static const Identifier bottomLeft ("bottomLeft");

    static void stringToPoint (const String& coords, Point<float>& point)
    {
        if (coords.isNotEmpty())
        {
            const int comma = coords.indexOfChar (',');
            point.setXY (coords.substring (0, comma).getFloatValue(),
                         coords.substring (comma).getFloatValue());
        }
    }

    static const var pointToString (const Point<float>& point)
    {
        return String (point.getX()) + ", " + String (point.getY());
    }
}

const Rectangle<float> DrawableImage::refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    jassert (tree.hasType (valueTreeType));

    setName (tree [idProperty]);

    const float newOpacity = tree.getProperty (DrawableImageHelpers::opacity, 1.0);
    const Colour newOverlayColour (tree [DrawableImageHelpers::overlay].toString().getHexValue32());

    Image* newImage = 0;
    const String imageIdentifier (tree [DrawableImageHelpers::image].toString());
    if (imageIdentifier.isNotEmpty())
    {
        jassert (imageProvider != 0); // if you're using images, you need to provide something that can load and save them!

        if (imageProvider != 0)
            newImage = imageProvider->getImageForIdentifier (imageIdentifier);
    }

    Point<float> newControlPoint[3];
    DrawableImageHelpers::stringToPoint (tree [DrawableImageHelpers::topLeft].toString(), newControlPoint[0]);
    DrawableImageHelpers::stringToPoint (tree [DrawableImageHelpers::topRight].toString(), newControlPoint[1]);
    DrawableImageHelpers::stringToPoint (tree [DrawableImageHelpers::bottomLeft].toString(), newControlPoint[2]);

    if (newOpacity != opacity || overlayColour != newOverlayColour || image != newImage
         || controlPoints[0] != newControlPoint[0]
         || controlPoints[1] != newControlPoint[1]
         || controlPoints[2] != newControlPoint[2])
    {
        opacity = newOpacity;
        overlayColour = newOverlayColour;
        controlPoints[0] = newControlPoint[0];
        controlPoints[1] = newControlPoint[1];
        controlPoints[2] = newControlPoint[2];

        if (image != newImage)
        {
            ImageCache::release (image);
            image = newImage;
        }

        return getBounds();
    }

    ImageCache::release (newImage);
    return Rectangle<float>();
}

const ValueTree DrawableImage::createValueTree (ImageProvider* imageProvider) const
{
    ValueTree v (valueTreeType);

    if (getName().isNotEmpty())
        v.setProperty (idProperty, getName(), 0);

    if (opacity < 1.0f)
        v.setProperty (DrawableImageHelpers::opacity, (double) opacity, 0);

    if (! overlayColour.isTransparent())
        v.setProperty (DrawableImageHelpers::overlay, String::toHexString ((int) overlayColour.getARGB()), 0);

    if (! getTransform().isIdentity())
    {
        v.setProperty (DrawableImageHelpers::topLeft, DrawableImageHelpers::pointToString (controlPoints[0]), 0);
        v.setProperty (DrawableImageHelpers::topRight, DrawableImageHelpers::pointToString (controlPoints[1]), 0);
        v.setProperty (DrawableImageHelpers::bottomLeft, DrawableImageHelpers::pointToString (controlPoints[2]), 0);
    }

    if (image != 0)
    {
        jassert (imageProvider != 0); // if you're using images, you need to provide something that can load and save them!

        if (imageProvider != 0)
            v.setProperty (DrawableImageHelpers::image, imageProvider->getIdentifierForImage (image), 0);
    }

    return v;
}


END_JUCE_NAMESPACE
