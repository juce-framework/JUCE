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

#include "juce_Drawable.h"
#include "juce_DrawableComposite.h"
#include "juce_DrawablePath.h"


//==============================================================================
class SVGState
{
public:
    //==============================================================================
    SVGState (const XmlElement* const topLevel)
        : topLevelXml (topLevel),
          elementX (0), elementY (0),
          width (512), height (512),
          viewBoxW (0), viewBoxH (0)
    {
    }

    ~SVGState()
    {
    }

    //==============================================================================
    Drawable* parseSVGElement (const XmlElement& xml)
    {
        if (! xml.hasTagName (T("svg")))
            return 0;

        DrawableComposite* const drawable = new DrawableComposite();

        drawable->setName (xml.getStringAttribute (T("id")));

        SVGState newState (*this);

        if (xml.hasAttribute (T("transform")))
            newState.addTransform (xml);

        newState.elementX = getCoordLength (xml.getStringAttribute (T("x"), String (newState.elementX)), viewBoxW);
        newState.elementY = getCoordLength (xml.getStringAttribute (T("y"), String (newState.elementY)), viewBoxH);
        newState.width = getCoordLength (xml.getStringAttribute (T("width"), String (newState.width)), viewBoxW);
        newState.height = getCoordLength (xml.getStringAttribute (T("height"), String (newState.height)), viewBoxH);

        if (xml.hasAttribute (T("viewBox")))
        {
            const String viewParams (xml.getStringAttribute (T("viewBox")));
            int i = 0;
            float vx, vy, vw, vh;

            if (parseCoords (viewParams, vx, vy, i, true)
                 && parseCoords (viewParams, vw, vh, i, true)
                 && vw > 0
                 && vh > 0)
            {
                newState.viewBoxW = vw;
                newState.viewBoxH = vh;

                int placementFlags = 0;

                const String aspect (xml.getStringAttribute (T("preserveAspectRatio")));

                if (aspect.containsIgnoreCase (T("none")))
                {
                    placementFlags = RectanglePlacement::stretchToFit;
                }
                else
                {
                    if (aspect.containsIgnoreCase (T("slice")))
                        placementFlags |= RectanglePlacement::fillDestination;

                    if (aspect.containsIgnoreCase (T("xMin")))
                        placementFlags |= RectanglePlacement::xLeft;
                    else if (aspect.containsIgnoreCase (T("xMax")))
                        placementFlags |= RectanglePlacement::xRight;
                    else
                        placementFlags |= RectanglePlacement::xMid;

                    if (aspect.containsIgnoreCase (T("yMin")))
                        placementFlags |= RectanglePlacement::yTop;
                    else if (aspect.containsIgnoreCase (T("yMax")))
                        placementFlags |= RectanglePlacement::yBottom;
                    else
                        placementFlags |= RectanglePlacement::yMid;
                }

                const RectanglePlacement placement (placementFlags);

                newState.transform
                    = placement.getTransformToFit (vx, vy, vw, vh,
                                                   0.0f, 0.0f, newState.width, newState.height)
                               .followedBy (newState.transform);
            }
        }
        else
        {
            if (viewBoxW == 0)
                newState.viewBoxW = newState.width;

            if (viewBoxH == 0)
                newState.viewBoxH = newState.height;
        }

        newState.parseSubElements (xml, drawable);

        return drawable;
    }

private:
    //==============================================================================
    const XmlElement* const topLevelXml;
    float elementX, elementY, width, height, viewBoxW, viewBoxH;
    AffineTransform transform;
    String cssStyleText;

    //==============================================================================
    void parseSubElements (const XmlElement& xml, DrawableComposite* const parentDrawable)
    {
        forEachXmlChildElement (xml, e)
        {
            Drawable* d = 0;

            if (e->hasTagName (T("g")))
                d = parseGroupElement (*e);
            else if (e->hasTagName (T("svg")))
                d = parseSVGElement (*e);
            else if (e->hasTagName (T("path")))
                d = parsePath (*e);
            else if (e->hasTagName (T("rect")))
                d = parseRect (*e);
            else if (e->hasTagName (T("circle")))
                d = parseCircle (*e);
            else if (e->hasTagName (T("ellipse")))
                d = parseEllipse (*e);
            else if (e->hasTagName (T("line")))
                d = parseLine (*e);
            else if (e->hasTagName (T("polyline")))
                d = parsePolygon (*e, true);
            else if (e->hasTagName (T("polygon")))
                d = parsePolygon (*e, false);
            else if (e->hasTagName (T("text")))
                d = parseText (*e);
            else if (e->hasTagName (T("switch")))
                d = parseSwitch (*e);
            else if (e->hasTagName (T("style")))
                parseCSSStyle (*e);

            parentDrawable->insertDrawable (d);
        }
    }

    DrawableComposite* parseSwitch (const XmlElement& xml)
    {
        const XmlElement* const group = xml.getChildByName (T("g"));

        if (group != 0)
            return parseGroupElement (*group);

        return 0;
    }

    DrawableComposite* parseGroupElement (const XmlElement& xml)
    {
        DrawableComposite* const drawable = new DrawableComposite();

        drawable->setName (xml.getStringAttribute (T("id")));

        if (xml.hasAttribute (T("transform")))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            newState.parseSubElements (xml, drawable);
        }
        else
        {
            parseSubElements (xml, drawable);
        }

        return drawable;
    }

    //==============================================================================
    Drawable* parsePath (const XmlElement& xml) const
    {
        const String d (xml.getStringAttribute (T("d")).trimStart());
        Path path;

        if (getStyleAttribute (&xml, T("fill-rule")).trim().equalsIgnoreCase (T("evenodd")))
            path.setUsingNonZeroWinding (false);

        int index = 0;
        float lastX = 0, lastY = 0;
        float lastX2 = 0, lastY2 = 0;
        tchar lastCommandChar = 0;
        bool carryOn = true;

        const String validCommandChars (T("MmLlHhVvCcSsQqTtAaZz"));

        for (;;)
        {
            float x, y, x2, y2, x3, y3;
            const bool isRelative = (d[index] >= 'a' && d[index] <= 'z');

            if (validCommandChars.containsChar (d[index]))
                lastCommandChar = d [index++];

            switch (lastCommandChar)
            {
            case T('M'):
            case T('m'):
            case T('L'):
            case T('l'):
                if (parseCoords (d, x, y, index, false))
                {
                    if (isRelative)
                    {
                        x += lastX;
                        y += lastY;
                    }

                    if (lastCommandChar == T('M') || lastCommandChar == T('m'))
                        path.startNewSubPath (x, y);
                    else
                        path.lineTo (x, y);

                    lastX2 = lastX;
                    lastY2 = lastY;
                    lastX = x;
                    lastY = y;
                }
                else
                {
                    ++index;
                }

                break;

            case T('H'):
            case T('h'):
                if (parseCoord (d, x, index, false, true))
                {
                    if (isRelative)
                        x += lastX;

                    path.lineTo (x, lastY);

                    lastX2 = lastX;
                    lastX = x;
                }
                else
                {
                    ++index;
                }
                break;

            case T('V'):
            case T('v'):
                if (parseCoord (d, y, index, false, false))
                {
                    if (isRelative)
                        y += lastY;

                    path.lineTo (lastX, y);

                    lastY2 = lastY;
                    lastY = y;
                }
                else
                {
                    ++index;
                }
                break;

            case T('C'):
            case T('c'):
                if (parseCoords (d, x, y, index, false)
                     && parseCoords (d, x2, y2, index, false)
                     && parseCoords (d, x3, y3, index, false))
                {
                    if (isRelative)
                    {
                        x += lastX;
                        y += lastY;
                        x2 += lastX;
                        y2 += lastY;
                        x3 += lastX;
                        y3 += lastY;
                    }

                    path.cubicTo (x, y, x2, y2, x3, y3);

                    lastX2 = x2;
                    lastY2 = y2;
                    lastX = x3;
                    lastY = y3;
                }
                else
                {
                    ++index;
                }
                break;

            case T('S'):
            case T('s'):
                if (parseCoords (d, x, y, index, false)
                     && parseCoords (d, x3, y3, index, false))
                {
                    if (isRelative)
                    {
                        x += lastX;
                        y += lastY;
                        x3 += lastX;
                        y3 += lastY;
                    }

                    x2 = lastX + (lastX - lastX2);
                    y2 = lastY + (lastY - lastY2);
                    path.cubicTo (x2, y2, x, y, x3, y3);

                    lastX2 = x;
                    lastY2 = y;
                    lastX = x3;
                    lastY = y3;
                }
                else
                {
                    ++index;
                }
                break;

            case T('Q'):
            case T('q'):
                if (parseCoords (d, x, y, index, false)
                     && parseCoords (d, x2, y2, index, false))
                {
                    if (isRelative)
                    {
                        x += lastX;
                        y += lastY;
                        x2 += lastX;
                        y2 += lastY;
                    }

                    path.quadraticTo (x, y, x2, y2);

                    lastX2 = x;
                    lastY2 = y;
                    lastX = x2;
                    lastY = y2;
                }
                else
                {
                    ++index;
                }
                break;

            case T('T'):
            case T('t'):
                if (parseCoords (d, x, y, index, false))
                {
                    if (isRelative)
                    {
                        x += lastX;
                        y += lastY;
                    }

                    x2 = lastX + (lastX - lastX2);
                    y2 = lastY + (lastY - lastY2);
                    path.quadraticTo (x2, y2, x, y);

                    lastX2 = x2;
                    lastY2 = y2;
                    lastX = x;
                    lastY = y;
                }
                else
                {
                    ++index;
                }
                break;

            case T('A'):
            case T('a'):
                if (parseCoords (d, x, y, index, false))
                {
                    String num;

                    if (parseNextNumber (d, num, index, false))
                    {
                        const float angle = num.getFloatValue() * (180.0f / float_Pi);

                        if (parseNextNumber (d, num, index, false))
                        {
                            const bool largeArc = num.getIntValue() != 0;

                            if (parseNextNumber (d, num, index, false))
                            {
                                const bool sweep = num.getIntValue() != 0;

                                if (parseCoords (d, x2, y2, index, false))
                                {
                                    if (isRelative)
                                    {
                                        x2 += lastX;
                                        y2 += lastY;
                                    }

                                    if (lastX != x2 || lastY != y2)
                                    {
                                        double centreX, centreY, startAngle, deltaAngle;
                                        double rx = x, ry = y;

                                        endpointToCentreParameters (lastX, lastY, x2, y2,
                                                                    angle, largeArc, sweep,
                                                                    rx, ry, centreX, centreY,
                                                                    startAngle, deltaAngle);

                                        path.addCentredArc ((float) centreX, (float) centreY,
                                                            (float) rx, (float) ry,
                                                            angle, (float) startAngle, (float) (startAngle + deltaAngle),
                                                            false);

                                        path.lineTo (x2, y2);
                                    }

                                    lastX2 = lastX;
                                    lastY2 = lastY;
                                    lastX = x2;
                                    lastY = y2;
                                }
                            }
                        }
                    }
                }
                else
                {
                    ++index;
                }

                break;

            case T('Z'):
            case T('z'):
                path.closeSubPath();
                while (CharacterFunctions::isWhitespace (d [index]))
                    ++index;

                break;

            default:
                carryOn = false;
                break;
            }

            if (! carryOn)
                break;
        }

        return parseShape (xml, path);
    }

    Drawable* parseRect (const XmlElement& xml) const
    {
        Path rect;

        const bool hasRX = xml.hasAttribute (T("rx"));
        const bool hasRY = xml.hasAttribute (T("ry"));

        if (hasRX || hasRY)
        {
            float rx = getCoordLength (xml.getStringAttribute (T("rx")), viewBoxW);
            float ry = getCoordLength (xml.getStringAttribute (T("ry")), viewBoxH);

            if (! hasRX)
                rx = ry;
            else if (! hasRY)
                ry = rx;

            rect.addRoundedRectangle (getCoordLength (xml.getStringAttribute (T("x")), viewBoxW),
                                      getCoordLength (xml.getStringAttribute (T("y")), viewBoxH),
                                      getCoordLength (xml.getStringAttribute (T("width")), viewBoxW),
                                      getCoordLength (xml.getStringAttribute (T("height")), viewBoxH),
                                      rx, ry);
        }
        else
        {
            rect.addRectangle (getCoordLength (xml.getStringAttribute (T("x")), viewBoxW),
                               getCoordLength (xml.getStringAttribute (T("y")), viewBoxH),
                               getCoordLength (xml.getStringAttribute (T("width")), viewBoxW),
                               getCoordLength (xml.getStringAttribute (T("height")), viewBoxH));
        }

        return parseShape (xml, rect);
    }

    Drawable* parseCircle (const XmlElement& xml) const
    {
        Path circle;

        const float cx = getCoordLength (xml.getStringAttribute (T("cx")), viewBoxW);
        const float cy = getCoordLength (xml.getStringAttribute (T("cy")), viewBoxH);
        const float radius = getCoordLength (xml.getStringAttribute (T("r")), viewBoxW);

        circle.addEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        return parseShape (xml, circle);
    }

    Drawable* parseEllipse (const XmlElement& xml) const
    {
        Path ellipse;

        const float cx = getCoordLength (xml.getStringAttribute (T("cx")), viewBoxW);
        const float cy = getCoordLength (xml.getStringAttribute (T("cy")), viewBoxH);
        const float radiusX = getCoordLength (xml.getStringAttribute (T("rx")), viewBoxW);
        const float radiusY = getCoordLength (xml.getStringAttribute (T("ry")), viewBoxH);

        ellipse.addEllipse (cx - radiusX, cy - radiusY, radiusX * 2.0f, radiusY * 2.0f);

        return parseShape (xml, ellipse);
    }

    Drawable* parseLine (const XmlElement& xml) const
    {
        Path line;

        const float x1 = getCoordLength (xml.getStringAttribute (T("x1")), viewBoxW);
        const float y1 = getCoordLength (xml.getStringAttribute (T("y1")), viewBoxH);
        const float x2 = getCoordLength (xml.getStringAttribute (T("x2")), viewBoxW);
        const float y2 = getCoordLength (xml.getStringAttribute (T("y2")), viewBoxH);

        line.startNewSubPath (x1, y1);
        line.lineTo (x2, y2);

        return parseShape (xml, line);
    }

    Drawable* parsePolygon (const XmlElement& xml, const bool isPolyline) const
    {
        const String points (xml.getStringAttribute (T("points")));
        Path path;

        int index = 0;
        float x, y;

        if (parseCoords (points, x, y, index, true))
        {
            float firstX = x;
            float firstY = y;
            float lastX = 0, lastY = 0;

            path.startNewSubPath (x, y);

            while (parseCoords (points, x, y, index, true))
            {
                lastX = x;
                lastY = y;
                path.lineTo (x, y);
            }

            if ((! isPolyline) || (firstX == lastX && firstY == lastY))
                path.closeSubPath();
        }

        return parseShape (xml, path);
    }

    //==============================================================================
    Drawable* parseShape (const XmlElement& xml, Path& path,
                          const bool shouldParseTransform = true) const
    {
        if (shouldParseTransform && xml.hasAttribute (T("transform")))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseShape (xml, path, false);
        }

        DrawablePath* dp = new DrawablePath();
        dp->setName (xml.getStringAttribute (T("id")));
        dp->setFill (FillType (Colours::transparentBlack));

        path.applyTransform (transform);
        dp->setPath (path);

        Path::Iterator iter (path);

        bool containsClosedSubPath = false;
        while (iter.next())
        {
            if (iter.elementType == Path::Iterator::closePath)
            {
                containsClosedSubPath = true;
                break;
            }
        }

        dp->setFill (getPathFillType (path,
                                      getStyleAttribute (&xml, T("fill")),
                                      getStyleAttribute (&xml, T("fill-opacity")),
                                      getStyleAttribute (&xml, T("opacity")),
                                      containsClosedSubPath ? Colours::black
                                                            : Colours::transparentBlack));

        const String strokeType (getStyleAttribute (&xml, T("stroke")));

        if (strokeType.isNotEmpty() && ! strokeType.equalsIgnoreCase (T("none")))
        {
            dp->setStrokeFill (getPathFillType (path, strokeType,
                                                getStyleAttribute (&xml, T("stroke-opacity")),
                                                getStyleAttribute (&xml, T("opacity")),
                                                Colours::transparentBlack));

            dp->setStrokeType (getStrokeFor (&xml));
        }

        return dp;
    }

    const XmlElement* findLinkedElement (const XmlElement* e) const
    {
        const String id (e->getStringAttribute (T("xlink:href")));

        if (! id.startsWithChar (T('#')))
            return 0;

        return findElementForId (topLevelXml, id.substring (1));
    }

    void addGradientStopsIn (ColourGradient& cg, const XmlElement* const fillXml) const
    {
        if (fillXml == 0)
            return;

        forEachXmlChildElementWithTagName (*fillXml, e, T("stop"))
        {
            int index = 0;
            Colour col (parseColour (getStyleAttribute  (e, T("stop-color")), index, Colours::black));

            const String opacity (getStyleAttribute (e, T("stop-opacity"), T("1")));
            col = col.withMultipliedAlpha (jlimit (0.0f, 1.0f, opacity.getFloatValue()));

            double offset = e->getDoubleAttribute (T("offset"));

            if (e->getStringAttribute (T("offset")).containsChar (T('%')))
                offset *= 0.01;

            cg.addColour (jlimit (0.0, 1.0, offset), col);
        }
    }

    const FillType getPathFillType (const Path& path,
                                    const String& fill,
                                    const String& fillOpacity,
                                    const String& overallOpacity,
                                    const Colour& defaultColour) const
    {
        float opacity = 1.0f;

        if (overallOpacity.isNotEmpty())
            opacity = jlimit (0.0f, 1.0f, overallOpacity.getFloatValue());

        if (fillOpacity.isNotEmpty())
            opacity *= (jlimit (0.0f, 1.0f, fillOpacity.getFloatValue()));

        if (fill.startsWithIgnoreCase (T("url")))
        {
            const String id (fill.fromFirstOccurrenceOf (T("#"), false, false)
                                 .upToLastOccurrenceOf (T(")"), false, false).trim());

            const XmlElement* const fillXml = findElementForId (topLevelXml, id);

            if (fillXml != 0
                 && (fillXml->hasTagName (T("linearGradient"))
                      || fillXml->hasTagName (T("radialGradient"))))
            {
                const XmlElement* inheritedFrom = findLinkedElement (fillXml);

                ColourGradient gradient;

                addGradientStopsIn (gradient, inheritedFrom);
                addGradientStopsIn (gradient, fillXml);

                if (gradient.getNumColours() > 0)
                {
                    gradient.addColour (0.0, gradient.getColour (0));
                    gradient.addColour (1.0, gradient.getColour (gradient.getNumColours() - 1));
                }
                else
                {
                    gradient.addColour (0.0, Colours::black);
                    gradient.addColour (1.0, Colours::black);
                }

                if (overallOpacity.isNotEmpty())
                    gradient.multiplyOpacity (overallOpacity.getFloatValue());

                jassert (gradient.getNumColours() > 0);

                gradient.isRadial = fillXml->hasTagName (T("radialGradient"));

                float width = viewBoxW;
                float height = viewBoxH;
                float dx = 0.0f;
                float dy = 0.0f;

                const bool userSpace = fillXml->getStringAttribute (T("gradientUnits")).equalsIgnoreCase (T("userSpaceOnUse"));

                if (! userSpace)
                    path.getBounds (dx, dy, width, height);

                if (gradient.isRadial)
                {
                    gradient.x1 = dx + getCoordLength (fillXml->getStringAttribute (T("cx"), T("50%")), width);
                    gradient.y1 = dy + getCoordLength (fillXml->getStringAttribute (T("cy"), T("50%")), height);

                    const float radius = getCoordLength (fillXml->getStringAttribute (T("r"), T("50%")), width);

                    gradient.x2 = gradient.x1 + radius;
                    gradient.y2 = gradient.y1;

                    //xxx (the fx, fy focal point isn't handled properly here..)
                }
                else
                {
                    gradient.x1 = dx + getCoordLength (fillXml->getStringAttribute (T("x1"), T("0%")), width);
                    gradient.y1 = dy + getCoordLength (fillXml->getStringAttribute (T("y1"), T("0%")), height);

                    gradient.x2 = dx + getCoordLength (fillXml->getStringAttribute (T("x2"), T("100%")), width);
                    gradient.y2 = dy + getCoordLength (fillXml->getStringAttribute (T("y2"), T("0%")), height);

                    if (gradient.x1 == gradient.x2 && gradient.y1 == gradient.y2)
                        return Colour (gradient.getColour (gradient.getNumColours() - 1));
                }

                FillType type (gradient);
                type.transform = parseTransform (fillXml->getStringAttribute (T("gradientTransform")))
                                   .followedBy (transform);
                return type;
            }
        }

        if (fill.equalsIgnoreCase (T("none")))
            return Colours::transparentBlack;

        int i = 0;
        const Colour colour (parseColour (fill, i, defaultColour));
        return colour.withMultipliedAlpha (opacity);
    }

    const PathStrokeType getStrokeFor (const XmlElement* const xml) const
    {
        const String width (getStyleAttribute (xml, T("stroke-width")));
        const String cap (getStyleAttribute (xml, T("stroke-linecap")));
        const String join (getStyleAttribute (xml, T("stroke-linejoin")));

        //const String mitreLimit (getStyleAttribute (xml, T("stroke-miterlimit")));
        //const String dashArray (getStyleAttribute (xml, T("stroke-dasharray")));
        //const String dashOffset (getStyleAttribute (xml, T("stroke-dashoffset")));

        PathStrokeType::JointStyle joinStyle = PathStrokeType::mitered;
        PathStrokeType::EndCapStyle capStyle = PathStrokeType::butt;

        if (join.equalsIgnoreCase (T("round")))
            joinStyle = PathStrokeType::curved;
        else if (join.equalsIgnoreCase (T("bevel")))
            joinStyle = PathStrokeType::beveled;

        if (cap.equalsIgnoreCase (T("round")))
            capStyle = PathStrokeType::rounded;
        else if (cap.equalsIgnoreCase (T("square")))
            capStyle = PathStrokeType::square;

        float ox = 0.0f, oy = 0.0f;
        transform.transformPoint (ox, oy);
        float x = getCoordLength (width, viewBoxW), y = 0.0f;
        transform.transformPoint (x, y);

        return PathStrokeType (width.isNotEmpty() ? juce_hypotf (x - ox, y - oy) : 1.0f,
                               joinStyle, capStyle);
    }

    //==============================================================================
    Drawable* parseText (const XmlElement& xml)
    {
        Array <float> xCoords, yCoords, dxCoords, dyCoords;

        getCoordList (xCoords, getInheritedAttribute (&xml, T("x")), true, true);
        getCoordList (yCoords, getInheritedAttribute (&xml, T("y")), true, false);
        getCoordList (dxCoords, getInheritedAttribute (&xml, T("dx")), true, true);
        getCoordList (dyCoords, getInheritedAttribute (&xml, T("dy")), true, false);


        //xxx not done text yet!


        forEachXmlChildElement (xml, e)
        {
            if (e->isTextElement())
            {
                const String text (e->getText());

                Path path;
                Drawable* s = parseShape (*e, path);
                delete s;
            }
            else if (e->hasTagName (T("tspan")))
            {
                Drawable* s = parseText (*e);
                delete s;
            }
        }

        return 0;
    }

    //==============================================================================
    void addTransform (const XmlElement& xml)
    {
        transform = parseTransform (xml.getStringAttribute (T("transform")))
                        .followedBy (transform);
    }

    //==============================================================================
    bool parseCoord (const String& s, float& value, int& index,
                     const bool allowUnits, const bool isX) const
    {
        String number;

        if (! parseNextNumber (s, number, index, allowUnits))
        {
            value = 0;
            return false;
        }

        value = getCoordLength (number, isX ? viewBoxW : viewBoxH);
        return true;
    }

    bool parseCoords (const String& s, float& x, float& y,
                      int& index, const bool allowUnits) const
    {
        return parseCoord (s, x, index, allowUnits, true)
            && parseCoord (s, y, index, allowUnits, false);
    }

    float getCoordLength (const String& s, const float sizeForProportions) const
    {
        float n = s.getFloatValue();
        const int len = s.length();

        if (len > 2)
        {
            const float dpi = 96.0f;

            const tchar n1 = s [len - 2];
            const tchar n2 = s [len - 1];

            if (n1 == T('i') && n2 == T('n'))
                n *= dpi;
            else if (n1 == T('m') && n2 == T('m'))
                n *= dpi / 25.4f;
            else if (n1 == T('c') && n2 == T('m'))
                n *= dpi / 2.54f;
            else if (n1 == T('p') && n2 == T('c'))
                n *= 15.0f;
            else if (n2 == T('%'))
                n *= 0.01f * sizeForProportions;
        }

        return n;
    }

    void getCoordList (Array <float>& coords, const String& list,
                       const bool allowUnits, const bool isX) const
    {
        int index = 0;
        float value;

        while (parseCoord (list, value, index, allowUnits, isX))
            coords.add (value);
    }

    //==============================================================================
    void parseCSSStyle (const XmlElement& xml)
    {
        cssStyleText = xml.getAllSubText() + T("\n") + cssStyleText;
    }

    const String getStyleAttribute (const XmlElement* xml, const String& attributeName,
                                    const String& defaultValue = String::empty) const
    {
        if (xml->hasAttribute (attributeName))
            return xml->getStringAttribute (attributeName, defaultValue);

        const String styleAtt (xml->getStringAttribute (T("style")));

        if (styleAtt.isNotEmpty())
        {
            const String value (getAttributeFromStyleList (styleAtt, attributeName, String::empty));

            if (value.isNotEmpty())
                return value;
        }
        else if (xml->hasAttribute (T("class")))
        {
            const String className (T(".") + xml->getStringAttribute (T("class")));

            int index = cssStyleText.indexOfIgnoreCase (className + T(" "));

            if (index < 0)
                index = cssStyleText.indexOfIgnoreCase (className + T("{"));

            if (index >= 0)
            {
                const int openBracket = cssStyleText.indexOfChar (index, T('{'));

                if (openBracket > index)
                {
                    const int closeBracket = cssStyleText.indexOfChar (openBracket, T('}'));

                    if (closeBracket > openBracket)
                    {
                        const String value (getAttributeFromStyleList (cssStyleText.substring (openBracket + 1, closeBracket), attributeName, defaultValue));

                        if (value.isNotEmpty())
                            return value;
                    }
                }
            }
        }

        xml = const_cast <XmlElement*> (topLevelXml)->findParentElementOf (xml);

        if (xml != 0)
            return getStyleAttribute (xml, attributeName, defaultValue);

        return defaultValue;
    }

    const String getInheritedAttribute (const XmlElement* xml, const String& attributeName) const
    {
        if (xml->hasAttribute (attributeName))
            return xml->getStringAttribute (attributeName);

        xml = const_cast <XmlElement*> (topLevelXml)->findParentElementOf (xml);

        if (xml != 0)
            return getInheritedAttribute  (xml, attributeName);

        return String::empty;
    }

    //==============================================================================
    static bool isIdentifierChar (const tchar c)
    {
        return CharacterFunctions::isLetter (c) || c == T('-');
    }

    static const String getAttributeFromStyleList (const String& list, const String& attributeName, const String& defaultValue)
    {
        int i = 0;

        for (;;)
        {
            i = list.indexOf (i, attributeName);

            if (i < 0)
                break;

            if ((i == 0 || (i > 0 && ! isIdentifierChar (list [i - 1])))
                 && ! isIdentifierChar (list [i + attributeName.length()]))
            {
                i = list.indexOfChar (i, T(':'));

                if (i < 0)
                    break;

                int end = list.indexOfChar (i, T(';'));

                if (end < 0)
                    end = 0x7ffff;

                return list.substring (i + 1, end).trim();
            }

            ++i;
        }

        return defaultValue;
    }

    //==============================================================================
    static bool parseNextNumber (const String& source, String& value, int& index, const bool allowUnits)
    {
        const tchar* const s = (const tchar*) source;

        while (CharacterFunctions::isWhitespace (s[index]) || s[index] == T(','))
            ++index;

        int start = index;

        if (CharacterFunctions::isDigit (s[index]) || s[index] == T('.') || s[index] == T('-'))
            ++index;

        while (CharacterFunctions::isDigit (s[index]) || s[index] == T('.'))
            ++index;

        if ((s[index] == T('e') || s[index] == T('E'))
             && (CharacterFunctions::isDigit (s[index + 1])
                  || s[index + 1] == T('-')
                  || s[index + 1] == T('+')))
        {
            index += 2;

            while (CharacterFunctions::isDigit (s[index]))
                ++index;
        }

        if (allowUnits)
        {
            while (CharacterFunctions::isLetter (s[index]))
                ++index;
        }

        if (index == start)
            return false;

        value = String (s + start, index - start);

        while (CharacterFunctions::isWhitespace (s[index]) || s[index] == T(','))
            ++index;

        return true;
    }

    //==============================================================================
    static const Colour parseColour (const String& s, int& index, const Colour& defaultColour)
    {
        if (s [index] == T('#'))
        {
            uint32 hex [6];
            zeromem (hex, sizeof (hex));
            int numChars = 0;

            for (int i = 6; --i >= 0;)
            {
                const int hexValue = CharacterFunctions::getHexDigitValue (s [++index]);

                if (hexValue >= 0)
                    hex [numChars++] = hexValue;
                else
                    break;
            }

            if (numChars <= 3)
                return Colour ((uint8) (hex [0] * 0x11),
                               (uint8) (hex [1] * 0x11),
                               (uint8) (hex [2] * 0x11));
            else
                return Colour ((uint8) ((hex [0] << 4) + hex [1]),
                               (uint8) ((hex [2] << 4) + hex [3]),
                               (uint8) ((hex [4] << 4) + hex [5]));
        }
        else if (s [index] == T('r')
                  && s [index + 1] == T('g')
                  && s [index + 2] == T('b'))
        {
            const int openBracket = s.indexOfChar (index, T('('));
            const int closeBracket = s.indexOfChar (openBracket, T(')'));

            if (openBracket >= 3 && closeBracket > openBracket)
            {
                index = closeBracket;

                StringArray tokens;
                tokens.addTokens (s.substring (openBracket + 1, closeBracket), T(","), T(""));
                tokens.trim();
                tokens.removeEmptyStrings();

                if (tokens[0].containsChar (T('%')))
                    return Colour ((uint8) roundToInt (2.55 * tokens[0].getDoubleValue()),
                                   (uint8) roundToInt (2.55 * tokens[1].getDoubleValue()),
                                   (uint8) roundToInt (2.55 * tokens[2].getDoubleValue()));
                else
                    return Colour ((uint8) tokens[0].getIntValue(),
                                   (uint8) tokens[1].getIntValue(),
                                   (uint8) tokens[2].getIntValue());
            }
        }

        return Colours::findColourForName (s, defaultColour);
    }

    static const AffineTransform parseTransform (String t)
    {
        AffineTransform result;

        while (t.isNotEmpty())
        {
            StringArray tokens;
            tokens.addTokens (t.fromFirstOccurrenceOf (T("("), false, false)
                               .upToFirstOccurrenceOf (T(")"), false, false),
                              T(", "), 0);

            tokens.removeEmptyStrings (true);

            float numbers [6];

            for (int i = 0; i < 6; ++i)
                numbers[i] = tokens[i].getFloatValue();

            AffineTransform trans;

            if (t.startsWithIgnoreCase (T("matrix")))
            {
                trans = AffineTransform (numbers[0], numbers[2], numbers[4],
                                         numbers[1], numbers[3], numbers[5]);
            }
            else if (t.startsWithIgnoreCase (T("translate")))
            {
                trans = trans.translated (numbers[0], numbers[1]);
            }
            else if (t.startsWithIgnoreCase (T("scale")))
            {
                if (tokens.size() == 1)
                    trans = trans.scaled (numbers[0], numbers[0]);
                else
                    trans = trans.scaled (numbers[0], numbers[1]);
            }
            else if (t.startsWithIgnoreCase (T("rotate")))
            {
                if (tokens.size() != 3)
                    trans = trans.rotated (numbers[0] / (180.0f / float_Pi));
                else
                    trans = trans.rotated (numbers[0] / (180.0f / float_Pi),
                                           numbers[1], numbers[2]);
            }
            else if (t.startsWithIgnoreCase (T("skewX")))
            {
                trans = AffineTransform (1.0f, tanf (numbers[0] * (float_Pi / 180.0f)), 0.0f,
                                         0.0f, 1.0f, 0.0f);
            }
            else if (t.startsWithIgnoreCase (T("skewY")))
            {
                trans = AffineTransform (1.0f, 0.0f, 0.0f,
                                         tanf (numbers[0] * (float_Pi / 180.0f)), 1.0f, 0.0f);
            }

            result = trans.followedBy (result);
            t = t.fromFirstOccurrenceOf (T(")"), false, false).trimStart();
        }

        return result;
    }

    static void endpointToCentreParameters (const double x1, const double y1,
                                            const double x2, const double y2,
                                            const double angle,
                                            const bool largeArc, const bool sweep,
                                            double& rx, double& ry,
                                            double& centreX, double& centreY,
                                            double& startAngle, double& deltaAngle)
    {
        const double midX = (x1 - x2) * 0.5;
        const double midY = (y1 - y2) * 0.5;

        const double cosAngle = cos (angle);
        const double sinAngle = sin (angle);
        const double xp = cosAngle * midX + sinAngle * midY;
        const double yp = cosAngle * midY - sinAngle * midX;
        const double xp2 = xp * xp;
        const double yp2 = yp * yp;

        double rx2 = rx * rx;
        double ry2 = ry * ry;

        const double s = (xp2 / rx2) + (yp2 / ry2);
        double c;

        if (s <= 1.0)
        {
            c = sqrt (jmax (0.0, ((rx2 * ry2) - (rx2 * yp2) - (ry2 * xp2))
                                   / (( rx2 * yp2) + (ry2 * xp2))));

            if (largeArc == sweep)
                c = -c;
        }
        else
        {
            const double s2 = sqrt (s);
            rx *= s2;
            ry *= s2;
            rx2 = rx * rx;
            ry2 = ry * ry;
            c = 0;
        }

        const double cpx = ((rx * yp) / ry) * c;
        const double cpy = ((-ry * xp) / rx) * c;

        centreX = ((x1 + x2) * 0.5) + (cosAngle * cpx) - (sinAngle * cpy);
        centreY = ((y1 + y2) * 0.5) + (sinAngle * cpx) + (cosAngle * cpy);

        const double ux = (xp - cpx) / rx;
        const double uy = (yp - cpy) / ry;
        const double vx = (-xp - cpx) / rx;
        const double vy = (-yp - cpy) / ry;

        const double length = juce_hypot (ux, uy);

        startAngle = acos (jlimit (-1.0, 1.0, ux / length));

        if (uy < 0)
            startAngle = -startAngle;

        startAngle += double_Pi * 0.5;

        deltaAngle = acos (jlimit (-1.0, 1.0, ((ux * vx) + (uy * vy))
                                                / (length * juce_hypot (vx, vy))));

        if ((ux * vy) - (uy * vx) < 0)
            deltaAngle = -deltaAngle;

        if (sweep)
        {
            if (deltaAngle < 0)
                deltaAngle += double_Pi * 2.0;
        }
        else
        {
            if (deltaAngle > 0)
                deltaAngle -= double_Pi * 2.0;
        }

        deltaAngle = fmod (deltaAngle, double_Pi * 2.0);
    }

    static const XmlElement* findElementForId (const XmlElement* const parent, const String& id)
    {
        forEachXmlChildElement (*parent, e)
        {
            if (e->compareAttribute (T("id"), id))
                return e;

            const XmlElement* const found = findElementForId (e, id);

            if (found != 0)
                return found;
        }

        return 0;
    }

    const SVGState& operator= (const SVGState&);
};


//==============================================================================
Drawable* Drawable::createFromSVG (const XmlElement& svgDocument)
{
    SVGState state (&svgDocument);
    return state.parseSVGElement (svgDocument);
}


END_JUCE_NAMESPACE
