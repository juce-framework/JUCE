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
#include "../brushes/juce_SolidColourBrush.h"
#include "../brushes/juce_GradientBrush.h"
#include "../brushes/juce_ImageBrush.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
DrawablePath::DrawablePath()
    : fillBrush (new SolidColourBrush (Colours::black)),
      strokeBrush (0),
      strokeType (0.0f)
{
}

DrawablePath::~DrawablePath()
{
    delete fillBrush;
    delete strokeBrush;
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath)
{
    path = newPath;
    updateOutline();
}

void DrawablePath::setSolidFill (const Colour& newColour)
{
    delete fillBrush;
    fillBrush = new SolidColourBrush (newColour);
}

void DrawablePath::setFillBrush (const Brush& newBrush)
{
    delete fillBrush;
    fillBrush = newBrush.createCopy();
}

void DrawablePath::setOutline (const float thickness, const Colour& colour)
{
    strokeType = PathStrokeType (thickness);
    delete strokeBrush;
    strokeBrush = new SolidColourBrush (colour);
    updateOutline();
}

void DrawablePath::setOutline (const PathStrokeType& strokeType_, const Brush& newStrokeBrush)
{
    strokeType = strokeType_;
    delete strokeBrush;
    strokeBrush = newStrokeBrush.createCopy();
    updateOutline();
}


//==============================================================================
void DrawablePath::render (const Drawable::RenderingContext& context) const
{
    {
        Brush* const tempBrush = fillBrush->createCopy();
        tempBrush->applyTransform (context.transform);
        tempBrush->multiplyOpacity (context.opacity);

        context.g.setBrush (tempBrush);
        context.g.fillPath (path, context.transform);

        delete tempBrush;
    }

    if (strokeBrush != 0 && strokeType.getStrokeThickness() > 0.0f)
    {
        Brush* const tempBrush = strokeBrush->createCopy();
        tempBrush->applyTransform (context.transform);
        tempBrush->multiplyOpacity (context.opacity);

        context.g.setBrush (tempBrush);
        context.g.fillPath (outline, context.transform);

        delete tempBrush;
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
    dp->setFillBrush (*fillBrush);

    if (strokeBrush != 0)
        dp->setOutline (strokeType, *strokeBrush);

    return dp;
}

//==============================================================================
static Brush* readBrushFromBinary (InputStream& input)
{
    switch (input.readByte())
    {
        case 1:
            return new SolidColourBrush (Colour ((uint32) input.readInt()));

        case 2:
        {
            ColourGradient gradient;
            gradient.x1 = input.readFloat();
            gradient.y1 = input.readFloat();
            gradient.x2 = input.readFloat();
            gradient.y2 = input.readFloat();
            gradient.isRadial = input.readByte() != 0;
            
            const int numColours = input.readCompressedInt();
            for (int i = 0; i < numColours; ++i)
            {
                double proportion = (double) input.readFloat();
                const Colour colour ((uint32) input.readInt());
                gradient.addColour (proportion, colour);
            }

            return new GradientBrush (gradient);
        }

        case 3:
        {
            jassertfalse; //xxx TODO

            return new ImageBrush (0, 0, 0, 0);
        }

        default:
            break;
    }

    return 0;
}

static void writeBrushToBinary (OutputStream& output, const Brush* const brush)
{
    if (brush == 0)
    {
        output.writeByte (0);
        return;
    }

    const SolidColourBrush* cb;
    const GradientBrush* gb;
    const ImageBrush* ib;

    if ((cb = dynamic_cast <const SolidColourBrush*> (brush)) != 0)
    {
        output.writeByte (1);
        output.writeInt ((int) cb->getColour().getARGB());
    }
    else if ((gb = dynamic_cast <const GradientBrush*> (brush)) != 0)
    {
        output.writeByte (2);

        const ColourGradient& g = gb->getGradient();
        output.writeFloat (g.x1);
        output.writeFloat (g.y1);
        output.writeFloat (g.x2);
        output.writeFloat (g.y2);
        output.writeByte (g.isRadial ? 1 : 0);

        output.writeCompressedInt (g.getNumColours());
        
        for (int i = 0; i < g.getNumColours(); ++i)
        {
            output.writeFloat ((float) g.getColourPosition (i));
            output.writeInt ((int) g.getColour (i).getARGB());
        }
    }
    else if ((ib = dynamic_cast <const ImageBrush*> (brush)) != 0)
    {
        output.writeByte (3);
        jassertfalse; //xxx TODO
    }
}

static Brush* readBrushFromXml (const XmlElement* xml)
{
    if (xml == 0)
        return 0;

    const String type (xml->getStringAttribute (T("type")));

    if (type.equalsIgnoreCase (T("solid")))
        return new SolidColourBrush (Colour ((uint32) xml->getStringAttribute (T("colour"), T("ff000000")).getHexValue32()));

    if (type.equalsIgnoreCase (T("gradient")))
    {
        ColourGradient gradient;
        gradient.x1 = (float) xml->getDoubleAttribute (T("x1"));
        gradient.y1 = (float) xml->getDoubleAttribute (T("y1"));
        gradient.x2 = (float) xml->getDoubleAttribute (T("x2"));
        gradient.y2 = (float) xml->getDoubleAttribute (T("y2"));
        gradient.isRadial = xml->getBoolAttribute (T("radial"), false);

        StringArray colours;
        colours.addTokens (xml->getStringAttribute (T("colours")), false);

        for (int i = 0; i < colours.size() / 2; ++i)
            gradient.addColour (colours[i * 2].getDoubleValue(),
                                Colour ((uint32)  colours[i * 2 + 1].getHexValue32()));

        return new GradientBrush (gradient);
    }

    if (type.equalsIgnoreCase (T("image")))
    {
        jassertfalse; //xxx TODO

        return new ImageBrush (0, 0, 0, 0);
    }

    return 0;
}

static XmlElement* writeBrushToXml (const String& tagName, const Brush* brush)
{
    if (brush == 0)
        return 0;

    XmlElement* const xml = new XmlElement (tagName);
    
    const SolidColourBrush* cb;
    const GradientBrush* gb;
    const ImageBrush* ib;

    if ((cb = dynamic_cast <const SolidColourBrush*> (brush)) != 0)
    {
        xml->setAttribute (T("type"), T("solid"));
        xml->setAttribute (T("colour"), String::toHexString ((int) cb->getColour().getARGB()));
    }
    else if ((gb = dynamic_cast <const GradientBrush*> (brush)) != 0)
    {
        xml->setAttribute (T("type"), T("gradient"));

        const ColourGradient& g = gb->getGradient();
        xml->setAttribute (T("x1"), g.x1);
        xml->setAttribute (T("y1"), g.y1);
        xml->setAttribute (T("x2"), g.x2);
        xml->setAttribute (T("y2"), g.y2);
        xml->setAttribute (T("radial"), g.isRadial);

        String s;
        for (int i = 0; i < g.getNumColours(); ++i)
            s << " " << g.getColourPosition (i) << " " << String::toHexString ((int) g.getColour(i).getARGB());

        xml->setAttribute (T("colours"), s.trimStart());
    }
    else if ((ib = dynamic_cast <const ImageBrush*> (brush)) != 0)
    {
        xml->setAttribute (T("type"), T("image"));

        jassertfalse; //xxx TODO
    }
    
    return xml;
}

bool DrawablePath::readBinary (InputStream& input)
{
    delete fillBrush;
    fillBrush = readBrushFromBinary (input);

    delete strokeBrush;
    strokeBrush = readBrushFromBinary (input);

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
    writeBrushToBinary (output, fillBrush);
    writeBrushToBinary (output, strokeBrush);

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
    delete fillBrush;
    fillBrush = readBrushFromXml (xml.getChildByName (T("fill")));
    delete strokeBrush;
    strokeBrush = readBrushFromXml (xml.getChildByName (T("stroke")));

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
    xml.addChildElement (writeBrushToXml (T("fill"), fillBrush));
    xml.addChildElement (writeBrushToXml (T("stroke"), strokeBrush));

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
