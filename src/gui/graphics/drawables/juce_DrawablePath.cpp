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

#include "juce_DrawablePath.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
DrawablePath::DrawablePath()
    : fillColour (0xff000000),
      fillGradient (0),
      strokeGradient (0),
      strokeType (0.0f)
{
}

DrawablePath::~DrawablePath()
{
    delete fillGradient;
    delete strokeGradient;
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath) throw()
{
    path = newPath;
    updateOutline();
}

void DrawablePath::setFillColour (const Colour& newColour) throw()
{
    deleteAndZero (fillGradient);
    fillColour = newColour;
}

void DrawablePath::setFillGradient (const ColourGradient& gradient) throw()
{
    delete fillGradient;
    fillGradient = new ColourGradient (gradient);
}

void DrawablePath::setStrokeType (const PathStrokeType& newStrokeType) throw()
{
    strokeType = newStrokeType;
    updateOutline();
}

void DrawablePath::setStrokeThickness (const float newThickness) throw()
{
    setStrokeType (PathStrokeType (newThickness, strokeType.getJointStyle(), strokeType.getEndStyle()));
}

void DrawablePath::setStrokeColour (const Colour& newStrokeColour) throw()
{
    deleteAndZero (strokeGradient);
    strokeColour = newStrokeColour;
}

void DrawablePath::setStrokeGradient (const ColourGradient& newStrokeGradient) throw()
{
    delete strokeGradient;
    strokeGradient = new ColourGradient (newStrokeGradient);
}

//==============================================================================
void DrawablePath::render (const Drawable::RenderingContext& context) const
{
    if (fillGradient != 0)
    {
        ColourGradient cg (*fillGradient);
        cg.transform = cg.transform.followedBy (context.transform);
        cg.multiplyOpacity (context.opacity);
        context.g.setGradientFill (cg);
    }
    else
    {
        context.g.setColour (fillColour);
    }

    context.g.fillPath (path, context.transform);

    if (strokeType.getStrokeThickness() > 0.0f)
    {
        if (strokeGradient != 0)
        {
            ColourGradient cg (*strokeGradient);
            cg.transform = cg.transform.followedBy (context.transform);
            cg.multiplyOpacity (context.opacity);
            context.g.setGradientFill (cg);
        }
        else
        {
            context.g.setColour (strokeColour);
        }

        context.g.fillPath (outline, context.transform);
    }
}

void DrawablePath::updateOutline()
{
    outline.clear();
    strokeType.createStrokedPath (outline, path, AffineTransform::identity, 4.0f);
}

void DrawablePath::getBounds (float& x, float& y, float& width, float& height) const
{
    if (strokeType.getStrokeThickness() > 0.0f)
        outline.getBounds (x, y, width, height);
    else
        path.getBounds (x, y, width, height);
}

bool DrawablePath::hitTest (float x, float y) const
{
    return path.contains (x, y)
        || outline.contains (x, y);
}

Drawable* DrawablePath::createCopy() const
{
    DrawablePath* const dp = new DrawablePath();

    dp->path = path;
    dp->outline = outline;
    dp->fillColour = fillColour;
    dp->strokeColour = strokeColour;
    dp->fillGradient = (fillGradient != 0) ? new ColourGradient (*fillGradient) : 0;
    dp->strokeGradient = (strokeGradient != 0) ? new ColourGradient (*strokeGradient) : 0;
    dp->strokeType = strokeType;
    return dp;
}

//==============================================================================
static const Colour readColourFromBinary (InputStream& input, ColourGradient*& gradient)
{
    deleteAndZero (gradient);

    switch (input.readByte())
    {
        case 1:
            return Colour ((uint32) input.readInt());

        case 2:
        {
            gradient = new ColourGradient();
            gradient->x1 = input.readFloat();
            gradient->y1 = input.readFloat();
            gradient->x2 = input.readFloat();
            gradient->y2 = input.readFloat();
            gradient->isRadial = input.readByte() != 0;

            const int numColours = input.readCompressedInt();
            for (int i = 0; i < numColours; ++i)
            {
                double proportion = (double) input.readFloat();
                const Colour col ((uint32) input.readInt());
                gradient->addColour (proportion, col);
            }

            break;
        }

        default:
            break;
    }

    return Colours::black;
}

static void writeColourToBinary (OutputStream& output, const Colour& colour, const ColourGradient* const gradient)
{
    if (gradient == 0)
    {
        output.writeByte (1);
        output.writeInt ((int) colour.getARGB());
    }
    else
    {
        output.writeByte (2);

        output.writeFloat (gradient->x1);
        output.writeFloat (gradient->y1);
        output.writeFloat (gradient->x2);
        output.writeFloat (gradient->y2);
        output.writeByte (gradient->isRadial ? 1 : 0);

        output.writeCompressedInt (gradient->getNumColours());

        for (int i = 0; i < gradient->getNumColours(); ++i)
        {
            output.writeFloat ((float) gradient->getColourPosition (i));
            output.writeInt ((int) gradient->getColour (i).getARGB());
        }
    }
}

static const Colour readColourFromXml (const XmlElement* xml, ColourGradient*& gradient)
{
    deleteAndZero (gradient);

    if (xml != 0)
    {
        const String type (xml->getStringAttribute (T("type")));

        if (type.equalsIgnoreCase (T("solid")))
        {
            return Colour ((uint32) xml->getStringAttribute (T("colour"), T("ff000000")).getHexValue32());
        }
        else if (type.equalsIgnoreCase (T("gradient")))
        {
            gradient = new ColourGradient();
            gradient->x1 = (float) xml->getDoubleAttribute (T("x1"));
            gradient->y1 = (float) xml->getDoubleAttribute (T("y1"));
            gradient->x2 = (float) xml->getDoubleAttribute (T("x2"));
            gradient->y2 = (float) xml->getDoubleAttribute (T("y2"));
            gradient->isRadial = xml->getBoolAttribute (T("radial"), false);

            StringArray colours;
            colours.addTokens (xml->getStringAttribute (T("colours")), false);

            for (int i = 0; i < colours.size() / 2; ++i)
                gradient->addColour (colours[i * 2].getDoubleValue(),
                                     Colour ((uint32)  colours[i * 2 + 1].getHexValue32()));
        }
        else
        {
            jassertfalse
        }
    }

    return Colours::black;
}

static XmlElement* writeColourToXml (const String& tagName, const Colour& colour, const ColourGradient* const gradient)
{
    XmlElement* const xml = new XmlElement (tagName);

    if (gradient == 0)
    {
        xml->setAttribute (T("type"), T("solid"));
        xml->setAttribute (T("colour"), String::toHexString ((int) colour.getARGB()));
    }
    else
    {
        xml->setAttribute (T("type"), T("gradient"));

        xml->setAttribute (T("x1"), gradient->x1);
        xml->setAttribute (T("y1"), gradient->y1);
        xml->setAttribute (T("x2"), gradient->x2);
        xml->setAttribute (T("y2"), gradient->y2);
        xml->setAttribute (T("radial"), gradient->isRadial);

        String s;
        for (int i = 0; i < gradient->getNumColours(); ++i)
            s << " " << gradient->getColourPosition (i) << " " << String::toHexString ((int) gradient->getColour(i).getARGB());

        xml->setAttribute (T("colours"), s.trimStart());
    }

    return xml;
}

bool DrawablePath::readBinary (InputStream& input)
{
    fillColour = readColourFromBinary (input, fillGradient);
    strokeColour = readColourFromBinary (input, strokeGradient);

    const float strokeThickness = input.readFloat();
    const int jointStyle = input.readByte();
    const int endStyle = input.readByte();

    strokeType = PathStrokeType (strokeThickness,
                                 jointStyle == 1 ? PathStrokeType::curved
                                                 : (jointStyle == 2 ? PathStrokeType::beveled
                                                                    : PathStrokeType::mitered),
                                 endStyle == 1 ? PathStrokeType::square
                                               : (endStyle == 2 ? PathStrokeType::rounded
                                                                : PathStrokeType::butt));

    const int pathSize = input.readInt();
    MemoryBlock pathData;
    input.readIntoMemoryBlock (pathData, pathSize);

    if (pathData.getSize() != pathSize)
        return false;

    path.clear();
    path.loadPathFromData ((const uint8*) pathData.getData(), pathSize);
    updateOutline();
    return true;
}

bool DrawablePath::writeBinary (OutputStream& output) const
{
    writeColourToBinary (output, fillColour, fillGradient);
    writeColourToBinary (output, strokeColour, strokeGradient);

    output.writeFloat (strokeType.getStrokeThickness());
    output.writeByte (strokeType.getJointStyle() == PathStrokeType::mitered ? 0
                        : (strokeType.getJointStyle() == PathStrokeType::curved ? 1 : 2));
    output.writeByte (strokeType.getEndStyle() == PathStrokeType::butt ? 0
                        : (strokeType.getEndStyle() == PathStrokeType::square ? 1 : 2));

    MemoryOutputStream out;
    path.writePathToStream (out);
    output.writeInt (out.getDataSize());
    output.write (out.getData(), out.getDataSize());
    return true;
}

bool DrawablePath::readXml (const XmlElement& xml)
{
    fillColour = readColourFromXml (xml.getChildByName (T("fill")), fillGradient);
    strokeColour = readColourFromXml (xml.getChildByName (T("stroke")), strokeGradient);

    const String jointStyle (xml.getStringAttribute (T("jointStyle"), String::empty));
    const String endStyle (xml.getStringAttribute (T("capStyle"), String::empty));
    strokeType = PathStrokeType ((float) xml.getDoubleAttribute (T("strokeWidth"), 0.0),
                                 jointStyle.equalsIgnoreCase (T("curved")) ? PathStrokeType::curved
                                                                           : (jointStyle.equalsIgnoreCase (T("bevel")) ? PathStrokeType::beveled
                                                                                                                       : PathStrokeType::mitered),
                                 endStyle.equalsIgnoreCase (T("square")) ? PathStrokeType::square
                                                                         : (endStyle.equalsIgnoreCase (T("round")) ? PathStrokeType::rounded
                                                                                                                   : PathStrokeType::butt));

    path.clear();
    path.restoreFromString (xml.getAllSubText());
    updateOutline();
    return true;
}

void DrawablePath::writeXml (XmlElement& xml) const
{
    xml.addChildElement (writeColourToXml (T("fill"), fillColour, fillGradient));
    xml.addChildElement (writeColourToXml (T("stroke"), strokeColour, strokeGradient));

    xml.setAttribute (T("strokeWidth"), (double) strokeType.getStrokeThickness());
    xml.setAttribute (T("jointStyle"),
                      strokeType.getJointStyle() == PathStrokeType::mitered ? T("miter")
                        : (strokeType.getJointStyle() == PathStrokeType::curved ? T("curved") : T("bevel")));
    xml.setAttribute (T("capStyle"),
                      strokeType.getEndStyle() == PathStrokeType::butt ? T("butt")
                        : (strokeType.getEndStyle() == PathStrokeType::square ? T("square") : T("round")));

    xml.addTextElement (path.toString());
}


END_JUCE_NAMESPACE
