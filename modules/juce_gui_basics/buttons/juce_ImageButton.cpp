/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

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
                               bool isMouseOverButton,
                               bool isButtonDown)
{
    if (! isEnabled())
    {
        isMouseOverButton = false;
        isButtonDown = false;
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
                const float imRatio = ih / (float) iw;
                const float destRatio = h / (float) w;

                if (imRatio > destRatio)
                {
                    newW = roundToInt (h / imRatio);
                    newH = h;
                }
                else
                {
                    newW = w;
                    newH = roundToInt (w * imRatio);
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

        const bool useDownImage = isButtonDown || getToggleState();

        getLookAndFeel().drawImageButton (g, &im, x, y, w, h,
                                          useDownImage ? downOverlay
                                                       : (isMouseOverButton ? overOverlay
                                                                            : normalOverlay),
                                          useDownImage ? downOpacity
                                                       : (isMouseOverButton ? overOpacity
                                                                            : normalOpacity),
                                          *this);
    }
}

bool ImageButton::hitTest (int x, int y)
{
    if (alphaThreshold == 0)
        return true;

    Image im (getCurrentImage());

    return im.isNull() || ((! imageBounds.isEmpty())
                            && alphaThreshold < im.getPixelAt (((x - imageBounds.getX()) * im.getWidth()) / imageBounds.getWidth(),
                                                               ((y - imageBounds.getY()) * im.getHeight()) / imageBounds.getHeight()).getAlpha());
}
