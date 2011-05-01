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

#include "../../jucer_Headers.h"
#include "jucer_FillType.h"


//==============================================================================
JucerFillType::JucerFillType()
    : image (0)
{
    reset();
}

JucerFillType::JucerFillType (const JucerFillType& other)
{
    image = Image();
    mode = other.mode;
    colour = other.colour;
    gradCol1 = other.gradCol1;
    gradCol2 = other.gradCol2;
    gradPos1 = other.gradPos1;
    gradPos2 = other.gradPos2;
    imageResourceName = other.imageResourceName;
    imageOpacity = other.imageOpacity;
    imageAnchor = other.imageAnchor;
}

JucerFillType& JucerFillType::operator= (const JucerFillType& other)
{
    image = Image();
    mode = other.mode;
    colour = other.colour;
    gradCol1 = other.gradCol1;
    gradCol2 = other.gradCol2;
    gradPos1 = other.gradPos1;
    gradPos2 = other.gradPos2;
    imageResourceName = other.imageResourceName;
    imageOpacity = other.imageOpacity;
    imageAnchor = other.imageAnchor;

    return *this;
}

JucerFillType::~JucerFillType()
{
}

bool JucerFillType::operator== (const JucerFillType& other) const throw()
{
    return mode == other.mode
        && colour == other.colour
        && gradCol1 == other.gradCol1
        && gradCol2 == other.gradCol2
        && gradPos1 == other.gradPos1
        && gradPos2 == other.gradPos2
        && imageResourceName == other.imageResourceName
        && imageOpacity == other.imageOpacity
        && imageAnchor == other.imageAnchor;
}

bool JucerFillType::operator!= (const JucerFillType& other) const throw()
{
    return ! operator== (other);
}

void JucerFillType::reset()
{
    image = Image();
    mode = solidColour;
    colour = Colours::brown.withHue (Random::getSystemRandom().nextFloat());
    gradCol1 = Colours::red;
    gradCol2 = Colours::green;
    gradPos1 = RelativePositionedRectangle();
    gradPos1.rect = PositionedRectangle ("50 50");
    gradPos2 = RelativePositionedRectangle();
    gradPos2.rect = PositionedRectangle ("100 100");

    imageResourceName = String::empty;
    imageOpacity = 1.0;
    imageAnchor = RelativePositionedRectangle();
    imageAnchor.rect = PositionedRectangle ("0 0");
}

//==============================================================================
void JucerFillType::setFillType (Graphics& g, JucerDocument* const document, const Rectangle<int>& parentArea)
{
    if (mode == solidColour)
    {
        image = Image();
        g.setColour (colour);
    }
    else if (mode == imageBrush)
    {
        loadImage (document);

        Rectangle<int> r (imageAnchor.getRectangle (parentArea, document->getComponentLayout()));

        g.setTiledImageFill (image, r.getX(), r.getY(), (float) imageOpacity);
    }
    else
    {
        image = Image();

        Rectangle<int> r1 (gradPos1.getRectangle (parentArea, document->getComponentLayout()));
        Rectangle<int> r2 (gradPos2.getRectangle (parentArea, document->getComponentLayout()));

        g.setGradientFill (ColourGradient (gradCol1, (float) r1.getX(), (float) r1.getY(),
                                           gradCol2, (float) r2.getX(), (float) r2.getY(),
                                           mode == radialGradient));
    }
}

void JucerFillType::fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) const
{
    String s;

    switch (mode)
    {
    case solidColour:
        s << "g.setColour (" << colourToCode (colour) << ");\n";
        break;

    case linearGradient:
    case radialGradient:
        {
            String x1, y1, w, h, x2, y2;
            positionToCode (gradPos1, code.document->getComponentLayout(), x1, y1, w, h);
            positionToCode (gradPos2, code.document->getComponentLayout(), x2, y2, w, h);

            s << "g.setGradientFill (ColourGradient (";

            const String indent (String::repeatedString (" ", s.length()));

            s << colourToCode (gradCol1) << ",\n"
              << indent << castToFloat (x1) << ", " << castToFloat (y1) << ",\n"
              << indent << colourToCode (gradCol2) << ",\n"
              << indent << castToFloat (x2) << ", " << castToFloat (y2) << ",\n"
              << indent << boolToString (mode == radialGradient) << "));\n";
            break;
        }

    case imageBrush:
        {
            const String imageVariable ("cachedImage_" + imageResourceName + "_" + String (code.getUniqueSuffix()));

            code.addImageResourceLoader (imageVariable, imageResourceName);

            String x, y, w, h;
            positionToCode (imageAnchor, code.document->getComponentLayout(), x, y, w, h);

            s << "g.setTiledImageFill (";

            const String indent (String::repeatedString (" ", s.length()));

            s << imageVariable << ",\n"
              << indent << x << ", " << y << ",\n"
              << indent << valueToFloat (imageOpacity) << ");\n";

            break;
        }

    default:
        jassertfalse
        break;
    }

    paintMethodCode += s;
}

const String JucerFillType::toString() const
{
    switch (mode)
    {
    case solidColour:
        return "solid: " + colourToHex (colour);

    case linearGradient:
    case radialGradient:
        return (mode == linearGradient ? "linear: "
                                       : " radial: ")
                + positionToString (gradPos1)
                + ", "
                + positionToString (gradPos2)
                + ", 0=" + colourToHex (gradCol1)
                + ", 1=" + colourToHex (gradCol2);

    case imageBrush:
        return "image: " + imageResourceName
                + ", "
                + String (imageOpacity)
                + ", "
                + positionToString (imageAnchor);

    default:
        jassertfalse
        break;
    }

    return String::empty;
}

void JucerFillType::restoreFromString (const String& s)
{
    reset();

    if (s.isNotEmpty())
    {
        StringArray toks;
        toks.addTokens (s, ",:", String::empty);
        toks.trim();

        if (toks[0] == "solid")
        {
            mode = solidColour;
            colour = Colour (toks[1].getHexValue32());
        }
        else if (toks[0] == "linear"
                  || toks[0] == "radial")
        {
            mode = (toks[0] == "linear") ? linearGradient : radialGradient;

            gradPos1 = RelativePositionedRectangle();
            gradPos1.rect = PositionedRectangle (toks[1]);
            gradPos2 = RelativePositionedRectangle();
            gradPos2.rect = PositionedRectangle (toks[2]);

            gradCol1 = Colour (toks[3].fromFirstOccurrenceOf ("=", false, false).getHexValue32());
            gradCol2 = Colour (toks[4].fromFirstOccurrenceOf ("=", false, false).getHexValue32());
        }
        else if (toks[0] == "image")
        {
            mode = imageBrush;
            imageResourceName = toks[1];
            imageOpacity = toks[2].getDoubleValue();
            imageAnchor= RelativePositionedRectangle();
            imageAnchor.rect = PositionedRectangle (toks[3]);
        }
        else
        {
            jassertfalse
        }
    }
}

bool JucerFillType::isOpaque() const
{
    switch (mode)
    {
    case solidColour:
        return colour.isOpaque();

    case linearGradient:
    case radialGradient:
        return gradCol1.isOpaque() && gradCol1.isOpaque();

    case imageBrush:
        return image.isValid()
                 && imageOpacity >= 1.0f
                 && ! image.hasAlphaChannel();

    default:
        jassertfalse
        break;
    }

    return false;
}

bool JucerFillType::isInvisible() const
{
    switch (mode)
    {
    case solidColour:
        return colour.isTransparent();

    case linearGradient:
    case radialGradient:
        return gradCol1.isTransparent() && gradCol2.isTransparent();

    case imageBrush:
        return imageOpacity == 0;

    default:
        jassertfalse
        break;
    }

    return false;
}

void JucerFillType::loadImage (JucerDocument* const document)
{
    if (image.isNull())
    {
        if (document != 0)
            image = document->getResources().getImageFromCache (imageResourceName);

        if (image.isNull())
        {
            const int hashCode = 0x3437856f;
            image = ImageCache::getFromHashCode (hashCode);

            if (image.isNull())
            {
                image = Image (Image::RGB, 100, 100, true);

                Graphics g (image);
                g.fillCheckerBoard (image.getBounds(),
                                    image.getWidth() / 2, image.getHeight() / 2,
                                    Colours::white, Colours::lightgrey);

                g.setFont (12.0f);
                g.setColour (Colours::grey);
                g.drawText ("(image missing)", 0, 0, image.getWidth(), image.getHeight() / 2, Justification::centred, true);

                ImageCache::addImageToCache (image, hashCode);
            }
        }
    }
}
