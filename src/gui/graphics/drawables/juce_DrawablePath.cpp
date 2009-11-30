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
    : mainFill (FillType (Colours::black)),
      strokeFill (FillType (Colours::transparentBlack)),
      strokeType (0.0f)
{
}

DrawablePath::~DrawablePath()
{
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath) throw()
{
    path = newPath;
    updateOutline();
}

void DrawablePath::setFill (const FillType& newFill) throw()
{
    mainFill = newFill;
}

void DrawablePath::setStrokeFill (const FillType& newFill) throw()
{
    strokeFill = newFill;
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

//==============================================================================
void DrawablePath::render (const Drawable::RenderingContext& context) const
{
    FillType f (mainFill);
    if (f.isGradient())
        f.gradient->multiplyOpacity (context.opacity);

    f.transform = f.transform.followedBy (context.transform);
    context.g.setFillType (f);
    context.g.fillPath (path, context.transform);

    if (strokeType.getStrokeThickness() > 0.0f)
    {
        FillType f (strokeFill);
        if (f.isGradient())
            f.gradient->multiplyOpacity (context.opacity);

        f.transform = f.transform.followedBy (context.transform);
        context.g.setFillType (f);
        context.g.fillPath (stroke, context.transform);
    }
}

void DrawablePath::updateOutline()
{
    stroke.clear();
    strokeType.createStrokedPath (stroke, path, AffineTransform::identity, 4.0f);
}

void DrawablePath::getBounds (float& x, float& y, float& width, float& height) const
{
    if (strokeType.getStrokeThickness() > 0.0f)
        stroke.getBounds (x, y, width, height);
    else
        path.getBounds (x, y, width, height);
}

bool DrawablePath::hitTest (float x, float y) const
{
    return path.contains (x, y)
            || stroke.contains (x, y);
}

Drawable* DrawablePath::createCopy() const
{
    DrawablePath* const dp = new DrawablePath();

    dp->path = path;
    dp->stroke = stroke;
    dp->mainFill = mainFill;
    dp->strokeFill = strokeFill;
    dp->strokeType = strokeType;
    return dp;
}

//==============================================================================
static const FillType readColourFromBinary (InputStream& input)
{
    switch (input.readByte())
    {
        case 1:
            return FillType (Colour ((uint32) input.readInt()));

        case 2:
        {
            ColourGradient g;
            g.x1 = input.readFloat();
            g.y1 = input.readFloat();
            g.x2 = input.readFloat();
            g.y2 = input.readFloat();
            g.isRadial = input.readByte() != 0;

            const int numColours = input.readCompressedInt();
            for (int i = 0; i < numColours; ++i)
            {
                double proportion = (double) input.readFloat();
                const Colour col ((uint32) input.readInt());
                g.addColour (proportion, col);
            }

            return FillType (g);
        }

        default:
            break;
    }

    return FillType();
}

static void writeColourToBinary (OutputStream& output, const FillType& fillType)
{
    if (fillType.isColour())
    {
        output.writeByte (1);
        output.writeInt ((int) fillType.colour.getARGB());
    }
    else if (fillType.isGradient())
    {
        output.writeByte (2);

        output.writeFloat (fillType.gradient->x1);
        output.writeFloat (fillType.gradient->y1);
        output.writeFloat (fillType.gradient->x2);
        output.writeFloat (fillType.gradient->y2);
        output.writeByte (fillType.gradient->isRadial ? 1 : 0);

        output.writeCompressedInt (fillType.gradient->getNumColours());

        for (int i = 0; i < fillType.gradient->getNumColours(); ++i)
        {
            output.writeFloat ((float) fillType.gradient->getColourPosition (i));
            output.writeInt ((int) fillType.gradient->getColour (i).getARGB());
        }
    }
    else
    {
        jassertfalse // xxx
    }

}

static const FillType readColourFromXml (const XmlElement* xml)
{
    if (xml != 0)
    {
        const String type (xml->getStringAttribute (T("type")));

        if (type.equalsIgnoreCase (T("solid")))
        {
            return FillType (Colour ((uint32) xml->getStringAttribute (T("colour"), T("ff000000")).getHexValue32()));
        }
        else if (type.equalsIgnoreCase (T("gradient")))
        {
            ColourGradient g;
            g.x1 = (float) xml->getDoubleAttribute (T("x1"));
            g.y1 = (float) xml->getDoubleAttribute (T("y1"));
            g.x2 = (float) xml->getDoubleAttribute (T("x2"));
            g.y2 = (float) xml->getDoubleAttribute (T("y2"));
            g.isRadial = xml->getBoolAttribute (T("radial"), false);

            StringArray colours;
            colours.addTokens (xml->getStringAttribute (T("colours")), false);

            for (int i = 0; i < colours.size() / 2; ++i)
                g.addColour (colours[i * 2].getDoubleValue(),
                             Colour ((uint32)  colours[i * 2 + 1].getHexValue32()));

            return FillType (g);
        }
        else
        {
            jassertfalse
        }
    }

    return FillType();
}

static XmlElement* writeColourToXml (const String& tagName, const FillType& fillType)
{
    XmlElement* const xml = new XmlElement (tagName);

    if (fillType.isColour())
    {
        xml->setAttribute (T("type"), T("solid"));
        xml->setAttribute (T("colour"), String::toHexString ((int) fillType.colour.getARGB()));
    }
    else if (fillType.isGradient())
    {
        xml->setAttribute (T("type"), T("gradient"));

        xml->setAttribute (T("x1"), fillType.gradient->x1);
        xml->setAttribute (T("y1"), fillType.gradient->y1);
        xml->setAttribute (T("x2"), fillType.gradient->x2);
        xml->setAttribute (T("y2"), fillType.gradient->y2);
        xml->setAttribute (T("radial"), fillType.gradient->isRadial);

        String s;
        for (int i = 0; i < fillType.gradient->getNumColours(); ++i)
            s << " " << fillType.gradient->getColourPosition (i)
              << " " << String::toHexString ((int) fillType.gradient->getColour(i).getARGB());

        xml->setAttribute (T("colours"), s.trimStart());
    }
    else
    {
        jassertfalse //xxx
    }

    return xml;
}

bool DrawablePath::readBinary (InputStream& input)
{
    mainFill = readColourFromBinary (input);
    strokeFill = readColourFromBinary (input);

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
    writeColourToBinary (output, mainFill);
    writeColourToBinary (output, strokeFill);

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
    mainFill = readColourFromXml (xml.getChildByName (T("fill")));
    strokeFill = readColourFromXml (xml.getChildByName (T("stroke")));

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
    xml.addChildElement (writeColourToXml (T("fill"), mainFill));
    xml.addChildElement (writeColourToXml (T("stroke"), strokeFill));

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
