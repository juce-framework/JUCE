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

#include "juce_ImageButton.h"
#include "../../graphics/imaging/juce_ImageCache.h"


//==============================================================================
ImageButton::ImageButton (const String& text)
    : Button (text),
      scaleImageToFit (true),
      preserveProportions (true),
      alphaThreshold (0),
      imageX (0),
      imageY (0),
      imageW (0),
      imageH (0),
      normalImage (0),
      overImage (0),
      downImage (0)
{
}

ImageButton::~ImageButton()
{
    deleteImages();
}

void ImageButton::deleteImages()
{
    if (normalImage != 0)
    {
        if (ImageCache::isImageInCache (normalImage))
            ImageCache::release (normalImage);
        else
            delete normalImage;
    }

    if (overImage != 0)
    {
        if (ImageCache::isImageInCache (overImage))
            ImageCache::release (overImage);
        else
            delete overImage;
    }

    if (downImage != 0)
    {
        if (ImageCache::isImageInCache (downImage))
            ImageCache::release (downImage);
        else
            delete downImage;
    }
}

void ImageButton::setImages (const bool resizeButtonNowToFitThisImage,
                             const bool rescaleImagesWhenButtonSizeChanges,
                             const bool preserveImageProportions,
                             Image* const normalImage_,
                             const float imageOpacityWhenNormal,
                             const Colour& overlayColourWhenNormal,
                             Image* const overImage_,
                             const float imageOpacityWhenOver,
                             const Colour& overlayColourWhenOver,
                             Image* const downImage_,
                             const float imageOpacityWhenDown,
                             const Colour& overlayColourWhenDown,
                             const float hitTestAlphaThreshold)
{
    deleteImages();

    normalImage = normalImage_;
    overImage = overImage_;
    downImage = downImage_;

    if (resizeButtonNowToFitThisImage && normalImage != 0)
    {
        imageW = normalImage->getWidth();
        imageH = normalImage->getHeight();

        setSize (imageW, imageH);
    }

    scaleImageToFit = rescaleImagesWhenButtonSizeChanges;
    preserveProportions = preserveImageProportions;

    normalOpacity = imageOpacityWhenNormal;
    normalOverlay = overlayColourWhenNormal;
    overOpacity   = imageOpacityWhenOver;
    overOverlay   = overlayColourWhenOver;
    downOpacity   = imageOpacityWhenDown;
    downOverlay   = overlayColourWhenDown;

    alphaThreshold = (unsigned char) jlimit (0, 0xff, roundFloatToInt (255.0f * hitTestAlphaThreshold));

    repaint();
}

Image* ImageButton::getCurrentImage() const
{
    if (isDown())
        return getDownImage();

    if (isOver())
        return getOverImage();

    return getNormalImage();
}

Image* ImageButton::getNormalImage() const throw()
{
    return normalImage;
}

Image* ImageButton::getOverImage() const throw()
{
    return (overImage != 0) ? overImage
                            : normalImage;
}

Image* ImageButton::getDownImage() const throw()
{
    return (downImage != 0) ? downImage
                            : getOverImage();
}

void ImageButton::paintButton (Graphics& g,
                               bool isMouseOverButton,
                               bool isButtonDown)
{
    if (! isEnabled())
    {
        isMouseOverButton = false;
        isButtonDown = false;
    }

    Image* const im = getCurrentImage();

    if (im != 0)
    {
        const int iw = im->getWidth();
        const int ih = im->getHeight();
        imageW = getWidth();
        imageH = getHeight();
        imageX = (imageW - iw) >> 1;
        imageY = (imageH - ih) >> 1;

        if (scaleImageToFit)
        {
            if (preserveProportions)
            {
                int newW, newH;
                const float imRatio = ih / (float)iw;
                const float destRatio = imageH / (float)imageW;

                if (imRatio > destRatio)
                {
                    newW = roundFloatToInt (imageH / imRatio);
                    newH = imageH;
                }
                else
                {
                    newW = imageW;
                    newH = roundFloatToInt (imageW * imRatio);
                }

                imageX = (imageW - newW) / 2;
                imageY = (imageH - newH) / 2;
                imageW = newW;
                imageH = newH;
            }
            else
            {
                imageX = 0;
                imageY = 0;
            }
        }

        const Colour& overlayColour = (isButtonDown) ? downOverlay
                                                     : ((isMouseOverButton) ? overOverlay
                                                                            : normalOverlay);

        if (! overlayColour.isOpaque())
        {
            g.setOpacity ((isButtonDown) ? downOpacity
                                         : ((isMouseOverButton) ? overOpacity
                                                                : normalOpacity));

            if (scaleImageToFit)
                g.drawImage (im, imageX, imageY, imageW, imageH, 0, 0, iw, ih, false);
            else
                g.drawImageAt (im, imageX, imageY, false);
        }

        if (! overlayColour.isTransparent())
        {
            g.setColour (overlayColour);

            if (scaleImageToFit)
                g.drawImage (im, imageX, imageY, imageW, imageH, 0, 0, iw, ih, true);
            else
                g.drawImageAt (im, imageX, imageY, true);
        }
    }
}

bool ImageButton::hitTest (int x, int y)
{
    if (alphaThreshold == 0)
        return true;

    Image* const im = getCurrentImage();

    return im == 0
             || (imageW > 0 && imageH > 0
                  && alphaThreshold < im->getPixelAt (((x - imageX) * im->getWidth()) / imageW,
                                                      ((y - imageY) * im->getHeight()) / imageH).getAlpha());
}

END_JUCE_NAMESPACE
