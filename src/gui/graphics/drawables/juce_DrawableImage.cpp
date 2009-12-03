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
#include "../imaging/juce_ImageFileFormat.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"

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
            context.g.drawImageTransformed (image, image->getBounds(),
                                            context.transform, false);
        }

        if (! overlayColour.isTransparent())
        {
            context.g.setColour (overlayColour.withMultipliedAlpha (context.opacity));
            context.g.drawImageTransformed (image, image->getBounds(),
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

//==============================================================================
bool DrawableImage::readBinary (InputStream& input)
{
    opacity = input.readFloat();
    overlayColour = Colour (input.readInt());

    const int dataLen = input.readInt();

    if (dataLen > 0)
    {
        MemoryBlock imageData;
        input.readIntoMemoryBlock (imageData, dataLen);

        Image* im = ImageFileFormat::loadFrom (imageData.getData(), imageData.getSize());
        if (im == 0)
            return false;

        setImage (im, true);
    }

    return true;
}

bool DrawableImage::writeBinary (OutputStream& output) const
{
    MemoryOutputStream imageData;

    if (image != 0)
    {
        PNGImageFormat pngFormat;
        if (! pngFormat.writeImageToStream (*image, imageData))
            return false;
    }

    output.writeFloat (opacity);
    output.writeInt (overlayColour.getARGB());
    output.writeInt (imageData.getDataSize());
    output.write (imageData.getData(), imageData.getDataSize());
    return true;
}

bool DrawableImage::readXml (const XmlElement& xml)
{
    opacity = (float) xml.getDoubleAttribute (T("opacity"), 1.0);
    overlayColour = Colour (xml.getStringAttribute (T("overlay"), T("0")).getHexValue32());

    MemoryBlock imageData;
    if (imageData.fromBase64Encoding (xml.getAllSubText()))
    {
        Image* const im = ImageFileFormat::loadFrom (imageData.getData(), imageData.getSize());
        if (im == 0)
            return false;

        setImage (im, true);
    }

    return true;
}

void DrawableImage::writeXml (XmlElement& xml) const
{
    if (opacity < 1.0f)
        xml.setAttribute (T("opacity"), (double) opacity);

    if (! overlayColour.isTransparent())
        xml.setAttribute (T("overlay"), String::toHexString ((int) overlayColour.getARGB()));

    if (image != 0)
    {
        MemoryOutputStream imageData;
        PNGImageFormat pngFormat;
        if (pngFormat.writeImageToStream (*image, imageData))
        {
            String base64 (MemoryBlock (imageData.getData(), imageData.getDataSize()).toBase64Encoding());

            for (int i = (base64.length() & ~127); i >= 0; i -= 128)
                base64 = base64.substring (0, i) + "\n" + base64.substring (i);

            xml.addTextElement (base64);
        }
    }
}

END_JUCE_NAMESPACE
