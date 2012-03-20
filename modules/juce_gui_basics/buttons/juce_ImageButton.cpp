/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
                             const Colour& overlayColourWhenNormal,
                             const Image& overImage_,
                             const float imageOpacityWhenOver,
                             const Colour& overlayColourWhenOver,
                             const Image& downImage_,
                             const float imageOpacityWhenDown,
                             const Colour& overlayColourWhenDown,
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

const Identifier ImageButton::Ids::tagType     ("IMAGEBUTTON");
const Identifier ImageButton::Ids::upImage     ("upImage");
const Identifier ImageButton::Ids::overImage   ("overImage");
const Identifier ImageButton::Ids::downImage   ("downImage");
const Identifier ImageButton::Ids::upOverlay   ("upOverlay");
const Identifier ImageButton::Ids::overOverlay ("overOverlay");
const Identifier ImageButton::Ids::downOverlay ("downOverlay");
const Identifier ImageButton::Ids::upOpacity   ("upOpacity");
const Identifier ImageButton::Ids::overOpacity ("overOpacity");
const Identifier ImageButton::Ids::downOpacity ("downOpacity");

namespace ImageButtonHelpers
{
    static Colour getColourFromVar (const var& col)
    {
        return col.isString() ? Colour::fromString (col.toString())
                              : Colours::transparentBlack;
    }

    static float getOpacityFromVar (const var& v)
    {
        return v.isVoid() ? 1.0f : static_cast<float> (v);
    }
}

void ImageButton::refreshFromValueTree (const ValueTree& state, ComponentBuilder& builder)
{
    Button::refreshFromValueTree (state, builder);

    const var upImageIdentifier (state [Ids::upImage]),
              overImageIdentifier (state [Ids::overImage]),
              downImageIdentifier (state [Ids::downImage]);

    ComponentBuilder::ImageProvider* const imageProvider = builder.getImageProvider();
    jassert (imageProvider != nullptr || upImageIdentifier.isVoid());

    Image newUpImage, newOverImage, newDownImage;

    if (imageProvider != nullptr)
    {
        newUpImage   = imageProvider->getImageForIdentifier (upImageIdentifier);
        newOverImage = imageProvider->getImageForIdentifier (overImageIdentifier);
        newDownImage = imageProvider->getImageForIdentifier (downImageIdentifier);
    }

    using namespace ImageButtonHelpers;

    setImages (false, true, true,
               newUpImage,   getOpacityFromVar (state[Ids::upOpacity]),   getColourFromVar (state[Ids::upOverlay]),
               newOverImage, getOpacityFromVar (state[Ids::overOpacity]), getColourFromVar (state[Ids::overOverlay]),
               newDownImage, getOpacityFromVar (state[Ids::downOpacity]), getColourFromVar (state[Ids::downOverlay]));
}
