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

ImageButton::ImageButton (const String& text_)
    : Button (text_),
      scaleImageToFit (true),
      preserveProportions (true),
      alphaThreshold (0)
{
}

ImageButton::~ImageButton()
{
}

void ImageButton::setImages (const bool resizeButtonNowToFitThisImage,
                             const bool rescaleImagesWhenButtonSizeChanges,
                             const bool preserveImageProportions,
                             const Image& normalImage_,
                             const float imageOpacityWhenNormal,
                             Colour overlayColourWhenNormal,
                             const Image& overImage_,
                             const float imageOpacityWhenOver,
                             Colour overlayColourWhenOver,
                             const Image& downImage_,
                             const float imageOpacityWhenDown,
                             Colour overlayColourWhenDown,
                             const float hitTestAlphaThreshold)
{
    normalImage = normalImage_;
    overImage = overImage_;
    downImage = downImage_;

    if (resizeButtonNowToFitThisImage && normalImage.isValid())
    {
        imageBounds.setSize (normalImage.getWidth(),
                             normalImage.getHeight());

        setSize (imageBounds.getWidth(), imageBounds.getHeight());
    }

    scaleImageToFit = rescaleImagesWhenButtonSizeChanges;
    preserveProportions = preserveImageProportions;

    normalOpacity = imageOpacityWhenNormal;
    normalOverlay = overlayColourWhenNormal;
    overOpacity   = imageOpacityWhenOver;
    overOverlay   = overlayColourWhenOver;
    downOpacity   = imageOpacityWhenDown;
    downOverlay   = overlayColourWhenDown;

    alphaThreshold = (uint8) jlimit (0, 0xff, roundToInt (255.0f * hitTestAlphaThreshold));

    repaint();
}

Image ImageButton::getCurrentImage() const
{
    if (isDown() || getToggleState())
        return getDownImage();

    if (isOver())
        return getOverImage();

    return getNormalImage();
}

Image ImageButton::getNormalImage() const
{
    return normalImage;
}

Image ImageButton::getOverImage() const
{
    return overImage.isValid() ? overImage
                               : normalImage;
}

Image ImageButton::getDownImage() const
{
    return downImage.isValid() ? downImage
                               : getOverImage();
}

void ImageButton::paintButton (Graphics& g,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown)
{
    if (! isEnabled())
    {
        shouldDrawButtonAsHighlighted = false;
        shouldDrawButtonAsDown = false;
    }

    Image im (getCurrentImage());

    if (im.isValid())
    {
        const int iw = im.getWidth();
        const int ih = im.getHeight();
        int w = getWidth();
        int h = getHeight();
        int x = (w - iw) / 2;
        int y = (h - ih) / 2;

        if (scaleImageToFit)
        {
            if (preserveProportions)
            {
                int newW, newH;
                const float imRatio = (float) ih / (float) iw;
                const float destRatio = (float) h / (float) w;

                if (imRatio > destRatio)
                {
                    newW = roundToInt ((float) h / imRatio);
                    newH = h;
                }
                else
                {
                    newW = w;
                    newH = roundToInt ((float) w * imRatio);
                }

                x = (w - newW) / 2;
                y = (h - newH) / 2;
                w = newW;
                h = newH;
            }
            else
            {
                x = 0;
                y = 0;
            }
        }

        if (! scaleImageToFit)
        {
            w = iw;
            h = ih;
        }

        imageBounds.setBounds (x, y, w, h);

        const bool useDownImage = shouldDrawButtonAsDown || getToggleState();

        getLookAndFeel().drawImageButton (g, &im, x, y, w, h,
                                          useDownImage ? downOverlay
                                                       : (shouldDrawButtonAsHighlighted ? overOverlay
                                                                            : normalOverlay),
                                          useDownImage ? downOpacity
                                                       : (shouldDrawButtonAsHighlighted ? overOpacity
                                                                            : normalOpacity),
                                          *this);
    }
}

bool ImageButton::hitTest (int x, int y)
{
    if (! Component::hitTest (x, y)) // handle setInterceptsMouseClicks
        return false;

    if (alphaThreshold == 0)
        return true;

    Image im (getCurrentImage());

    return im.isNull() || ((! imageBounds.isEmpty())
                            && alphaThreshold < im.getPixelAt (((x - imageBounds.getX()) * im.getWidth()) / imageBounds.getWidth(),
                                                               ((y - imageBounds.getY()) * im.getHeight()) / imageBounds.getHeight()).getAlpha());
}

} // namespace juce
