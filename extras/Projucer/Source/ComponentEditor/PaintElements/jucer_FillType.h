/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../jucer_JucerDocument.h"
#include "../jucer_UtilityFunctions.h"
#include "../../ProjectSaving/jucer_ResourceFile.h"

//==============================================================================
class JucerFillType
{
public:
    JucerFillType()
    {
        reset();
    }

    JucerFillType (const JucerFillType& other)
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

    JucerFillType& operator= (const JucerFillType& other)
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

    bool operator== (const JucerFillType& other) const
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

    bool operator!= (const JucerFillType& other) const
    {
        return ! operator== (other);
    }

    //==============================================================================
    void setFillType (Graphics& g, JucerDocument* const document, const Rectangle<int>& parentArea)
    {
        if (document == nullptr)
        {
            jassertfalse;
            return;
        }

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

    String generateVariablesCode (String type) const
    {
        String s;

        switch (mode)
        {
            case solidColour:
                s << "juce::Colour " << type << "Colour = " << CodeHelpers::colourToCode (colour) << ";\n";
                break;

            case linearGradient:
            case radialGradient:
                s << "juce::Colour " << type << "Colour1 = " << CodeHelpers::colourToCode (gradCol1) << ", " << type << "Colour2 = " << CodeHelpers::colourToCode (gradCol2) << ";\n";
                break;

            case imageBrush:
                break;

            default:
                jassertfalse;
                break;
        }

        return s;
    }

    void fillInGeneratedCode (String type, RelativePositionedRectangle relativeTo, GeneratedCode& code, String& paintMethodCode) const
    {
        String s;

        switch (mode)
        {
            case solidColour:
                s << "g.setColour (" << type << "Colour);\n";
                break;

            case linearGradient:
            case radialGradient:
                {
                    String x0, y0, x1, y1, w, h, x2, y2;
                    positionToCode (relativeTo, code.document->getComponentLayout(), x0, y0, w, h);
                    positionToCode (gradPos1, code.document->getComponentLayout(), x1, y1, w, h);
                    positionToCode (gradPos2, code.document->getComponentLayout(), x2, y2, w, h);

                    s << "g.setGradientFill (juce::ColourGradient (";

                    auto indent = String::repeatedString (" ", s.length());

                    s << type << "Colour1,\n"
                      << indent << castToFloat (x1) << " - " << castToFloat (x0) << " + x,\n"
                      << indent << castToFloat (y1) << " - " << castToFloat (y0) << " + y,\n"
                      << indent << type << "Colour2,\n"
                      << indent << castToFloat (x2) << " - " << castToFloat (x0) << " + x,\n"
                      << indent << castToFloat (y2) << " - " << castToFloat (y0) << " + y,\n"
                      << indent << CodeHelpers::boolLiteral (mode == radialGradient) << "));\n";
                    break;
                }

            case imageBrush:
                {
                    auto imageVariable = "cachedImage_" + imageResourceName.replace ("::", "_") + "_" + String (code.getUniqueSuffix());

                    code.addImageResourceLoader (imageVariable, imageResourceName);

                    String x0, y0, x1, y1, w, h;
                    positionToCode (relativeTo, code.document->getComponentLayout(), x0, y0, w, h);
                    positionToCode (imageAnchor, code.document->getComponentLayout(), x1, y1, w, h);

                    s << "g.setTiledImageFill (";

                    const String indent (String::repeatedString (" ", s.length()));

                    s << imageVariable << ",\n"
                      << indent << x1 << " - " << x0 << " + x,\n"
                      << indent << y1 << " - " << y0 << " + y,\n"
                      << indent << CodeHelpers::floatLiteral (imageOpacity, 4) << ");\n";

                    break;
                }

            default:
                jassertfalse;
                break;
        }

        paintMethodCode += s;
    }

    String toString() const
    {
        switch (mode)
        {
            case solidColour:
                return "solid: " + colour.toString();

            case linearGradient:
            case radialGradient:
                return (mode == linearGradient ? "linear: "
                                               : " radial: ")
                        + gradPos1.toString()
                        + ", "
                        + gradPos2.toString()
                        + ", 0=" + gradCol1.toString()
                        + ", 1=" + gradCol2.toString();

            case imageBrush:
                return "image: " + imageResourceName.replaceCharacter (':', '#')
                        + ", "
                        + String (imageOpacity)
                        + ", "
                        + imageAnchor.toString();

            default:
                jassertfalse;
                break;
        }

        return {};
    }

    void restoreFromString (const String& s)
    {
        reset();

        if (s.isNotEmpty())
        {
            StringArray toks;
            toks.addTokens (s, ",:", StringRef());
            toks.trim();

            if (toks[0] == "solid")
            {
                mode = solidColour;
                colour = Colour::fromString (toks[1]);
            }
            else if (toks[0] == "linear"
                      || toks[0] == "radial")
            {
                mode = (toks[0] == "linear") ? linearGradient : radialGradient;

                gradPos1 = RelativePositionedRectangle();
                gradPos1.rect = PositionedRectangle (toks[1]);
                gradPos2 = RelativePositionedRectangle();
                gradPos2.rect = PositionedRectangle (toks[2]);

                gradCol1 = Colour::fromString (toks[3].fromFirstOccurrenceOf ("=", false, false));
                gradCol2 = Colour::fromString (toks[4].fromFirstOccurrenceOf ("=", false, false));
            }
            else if (toks[0] == "image")
            {
                mode = imageBrush;
                imageResourceName = toks[1].replaceCharacter ('#', ':');
                imageOpacity = toks[2].getDoubleValue();
                imageAnchor= RelativePositionedRectangle();
                imageAnchor.rect = PositionedRectangle (toks[3]);
            }
            else
            {
                jassertfalse;
            }
        }
    }

    bool isOpaque() const
    {
        switch (mode)
        {
        case solidColour:
            return colour.isOpaque();

        case linearGradient:
        case radialGradient:
            return gradCol1.isOpaque() && gradCol2.isOpaque();

        case imageBrush:
            return image.isValid()
                     && imageOpacity >= 1.0f
                     && ! image.hasAlphaChannel();

        default:
            jassertfalse;
            break;
        }

        return false;
    }

    bool isInvisible() const
    {
        switch (mode)
        {
        case solidColour:
            return colour.isTransparent();

        case linearGradient:
        case radialGradient:
            return gradCol1.isTransparent() && gradCol2.isTransparent();

        case imageBrush:
            return imageOpacity == 0.0;

        default:
            jassertfalse;
            break;
        }

        return false;
    }

    //==============================================================================
    enum FillMode
    {
        solidColour,
        linearGradient,
        radialGradient,
        imageBrush
    };

    FillMode mode;
    Colour colour, gradCol1, gradCol2;

    // just the x, y, of these are used
    RelativePositionedRectangle gradPos1, gradPos2;

    String imageResourceName;
    double imageOpacity;
    RelativePositionedRectangle imageAnchor;

    //==============================================================================
private:
    Image image;

    void reset()
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

        imageResourceName.clear();
        imageOpacity = 1.0;
        imageAnchor = RelativePositionedRectangle();
        imageAnchor.rect = PositionedRectangle ("0 0");
    }

    void loadImage (JucerDocument* const document)
    {
        if (image.isNull())
        {
            if (document != nullptr)
            {
                if (imageResourceName.contains ("::"))
                {
                    if (Project* project = document->getCppDocument().getProject())
                    {
                        JucerResourceFile resourceFile (*project);

                        for (int i = 0; i < resourceFile.getNumFiles(); ++i)
                        {
                            const File& file = resourceFile.getFile(i);

                            if (imageResourceName == resourceFile.getClassName() + "::" + resourceFile.getDataVariableFor (file))
                            {
                                image = ImageCache::getFromFile (file);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    image = document->getResources().getImageFromCache (imageResourceName);
                }
            }

            if (image.isNull())
            {
                const int hashCode = 0x3437856f;
                image = ImageCache::getFromHashCode (hashCode);

                if (image.isNull())
                {
                    image = Image (Image::RGB, 100, 100, true);

                    Graphics g (image);
                    g.fillCheckerBoard (image.getBounds().toFloat(),
                                        (float) image.getWidth() * 0.5f, (float) image.getHeight() * 0.5f,
                                        Colours::white, Colours::lightgrey);

                    g.setFont (12.0f);
                    g.setColour (Colours::grey);
                    g.drawText ("(image missing)", 0, 0, image.getWidth(), image.getHeight() / 2, Justification::centred, true);

                    ImageCache::addImageToCache (image, hashCode);
                }
            }
        }
    }
};
