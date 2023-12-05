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

namespace juce
{

class SVGState
{
public:
    //==============================================================================
    explicit SVGState (const XmlElement* topLevel, const File& svgFile = {})
       : originalFile (svgFile), topLevelXml (topLevel, nullptr)
    {
    }

    struct XmlPath
    {
        XmlPath (const XmlElement* e, const XmlPath* p) noexcept : xml (e), parent (p)  {}

        const XmlElement& operator*() const noexcept            { jassert (xml != nullptr); return *xml; }
        const XmlElement* operator->() const noexcept           { return xml; }
        XmlPath getChild (const XmlElement* e) const noexcept   { return XmlPath (e, this); }

        template <typename OperationType>
        bool applyOperationToChildWithID (const String& id, OperationType& op) const
        {
            for (auto* e : xml->getChildIterator())
            {
                XmlPath child (e, this);

                if (e->compareAttribute ("id", id)
                      && ! child->hasTagName ("defs"))
                    return op (child);

                if (child.applyOperationToChildWithID (id, op))
                    return true;
            }

            return false;
        }

        const XmlElement* xml;
        const XmlPath* parent;
    };

    //==============================================================================
    struct UsePathOp
    {
        const SVGState* state;
        Path* targetPath;

        bool operator() (const XmlPath& xmlPath) const
        {
            return state->parsePathElement (xmlPath, *targetPath);
        }
    };

    struct UseTextOp
    {
        const SVGState* state;
        AffineTransform* transform;
        Drawable* target;

        bool operator() (const XmlPath& xmlPath)
        {
            target = state->parseText (xmlPath, true, transform);
            return target != nullptr;
        }
    };

    struct UseImageOp
    {
        const SVGState* state;
        AffineTransform* transform;
        Drawable* target;

        bool operator() (const XmlPath& xmlPath)
        {
            target = state->parseImage (xmlPath, true, transform);
            return target != nullptr;
        }
    };

    struct GetClipPathOp
    {
        SVGState* state;
        Drawable* target;

        bool operator() (const XmlPath& xmlPath)
        {
            return state->applyClipPath (*target, xmlPath);
        }
    };

    struct SetGradientStopsOp
    {
        const SVGState* state;
        ColourGradient* gradient;

        bool operator() (const XmlPath& xml) const
        {
            return state->addGradientStopsIn (*gradient, xml);
        }
    };

    struct GetFillTypeOp
    {
        const SVGState* state;
        const Path* path;
        float opacity;
        FillType fillType;

        bool operator() (const XmlPath& xml)
        {
            if (xml->hasTagNameIgnoringNamespace ("linearGradient")
                 || xml->hasTagNameIgnoringNamespace ("radialGradient"))
            {
                fillType = state->getGradientFillType (xml, *path, opacity);
                return true;
            }

            return false;
        }
    };

    //==============================================================================
    Drawable* parseSVGElement (const XmlPath& xml)
    {
        auto drawable = new DrawableComposite();
        setCommonAttributes (*drawable, xml);

        SVGState newState (*this);

        if (xml->hasAttribute ("transform"))
            newState.addTransform (xml);

        newState.width  = getCoordLength (xml->getStringAttribute ("width",  String (newState.width)),  viewBoxW);
        newState.height = getCoordLength (xml->getStringAttribute ("height", String (newState.height)), viewBoxH);

        if (newState.width  <= 0) newState.width  = 100;
        if (newState.height <= 0) newState.height = 100;

        Point<float> viewboxXY;

        if (xml->hasAttribute ("viewBox"))
        {
            auto viewBoxAtt = xml->getStringAttribute ("viewBox");
            auto viewParams = viewBoxAtt.getCharPointer();
            Point<float> vwh;

            if (parseCoords (viewParams, viewboxXY, true)
                 && parseCoords (viewParams, vwh, true)
                 && vwh.x > 0
                 && vwh.y > 0)
            {
                newState.viewBoxW = vwh.x;
                newState.viewBoxH = vwh.y;

                auto placementFlags = parsePlacementFlags (xml->getStringAttribute ("preserveAspectRatio").trim());

                if (placementFlags != 0)
                    newState.transform = RectanglePlacement (placementFlags)
                                            .getTransformToFit (Rectangle<float> (viewboxXY.x, viewboxXY.y, vwh.x, vwh.y),
                                                                Rectangle<float> (newState.width, newState.height))
                                            .followedBy (newState.transform);
            }
        }
        else
        {
            if (approximatelyEqual (viewBoxW, 0.0f))  newState.viewBoxW = newState.width;
            if (approximatelyEqual (viewBoxH, 0.0f))  newState.viewBoxH = newState.height;
        }

        newState.parseSubElements (xml, *drawable);

        drawable->setContentArea ({ viewboxXY.x, viewboxXY.y, newState.viewBoxW, newState.viewBoxH });
        drawable->resetBoundingBoxToContentArea();

        return drawable;
    }

    //==============================================================================
    void parsePathString (Path& path, const String& pathString) const
    {
        auto d = pathString.getCharPointer().findEndOfWhitespace();

        Point<float> subpathStart, last, last2, p1, p2, p3;
        juce_wchar currentCommand = 0, previousCommand = 0;
        bool isRelative = true;
        bool carryOn = true;

        while (! d.isEmpty())
        {
            if (CharPointer_ASCII ("MmLlHhVvCcSsQqTtAaZz").indexOf (*d) >= 0)
            {
                currentCommand = d.getAndAdvance();
                isRelative = currentCommand >= 'a';
            }

            switch (currentCommand)
            {
            case 'M':
            case 'm':
            case 'L':
            case 'l':
                if (parseCoordsOrSkip (d, p1, false))
                {
                    if (isRelative)
                        p1 += last;

                    if (currentCommand == 'M' || currentCommand == 'm')
                    {
                        subpathStart = p1;
                        path.startNewSubPath (p1);
                        currentCommand = 'l';
                    }
                    else
                        path.lineTo (p1);

                    last2 = last = p1;
                }
                break;

            case 'H':
            case 'h':
                if (parseCoord (d, p1.x, false, Axis::x))
                {
                    if (isRelative)
                        p1.x += last.x;

                    path.lineTo (p1.x, last.y);

                    last2.x = last.x;
                    last.x = p1.x;
                }
                else
                {
                    ++d;
                }
                break;

            case 'V':
            case 'v':
                if (parseCoord (d, p1.y, false, Axis::y))
                {
                    if (isRelative)
                        p1.y += last.y;

                    path.lineTo (last.x, p1.y);

                    last2.y = last.y;
                    last.y = p1.y;
                }
                else
                {
                    ++d;
                }
                break;

            case 'C':
            case 'c':
                if (parseCoordsOrSkip (d, p1, false)
                     && parseCoordsOrSkip (d, p2, false)
                     && parseCoordsOrSkip (d, p3, false))
                {
                    if (isRelative)
                    {
                        p1 += last;
                        p2 += last;
                        p3 += last;
                    }

                    path.cubicTo (p1, p2, p3);

                    last2 = p2;
                    last = p3;
                }
                break;

            case 'S':
            case 's':
                if (parseCoordsOrSkip (d, p1, false)
                     && parseCoordsOrSkip (d, p3, false))
                {
                    if (isRelative)
                    {
                        p1 += last;
                        p3 += last;
                    }

                    p2 = last;

                    if (CharPointer_ASCII ("CcSs").indexOf (previousCommand) >= 0)
                        p2 += (last - last2);

                    path.cubicTo (p2, p1, p3);

                    last2 = p1;
                    last = p3;
                }
                break;

            case 'Q':
            case 'q':
                if (parseCoordsOrSkip (d, p1, false)
                     && parseCoordsOrSkip (d, p2, false))
                {
                    if (isRelative)
                    {
                        p1 += last;
                        p2 += last;
                    }

                    path.quadraticTo (p1, p2);

                    last2 = p1;
                    last = p2;
                }
                break;

            case 'T':
            case 't':
                if (parseCoordsOrSkip (d, p1, false))
                {
                    if (isRelative)
                        p1 += last;

                    p2 = last;

                    if (CharPointer_ASCII ("QqTt").indexOf (previousCommand) >= 0)
                        p2 += (last - last2);

                    path.quadraticTo (p2, p1);

                    last2 = p2;
                    last = p1;
                }
                break;

            case 'A':
            case 'a':
                if (parseCoordsOrSkip (d, p1, false))
                {
                    String num;
                    bool flagValue = false;

                    if (parseNextNumber (d, num, false))
                    {
                        auto angle = degreesToRadians (parseSafeFloat (num));

                        if (parseNextFlag (d, flagValue))
                        {
                            auto largeArc = flagValue;

                            if (parseNextFlag (d, flagValue))
                            {
                                auto sweep = flagValue;

                                if (parseCoordsOrSkip (d, p2, false))
                                {
                                    if (isRelative)
                                        p2 += last;

                                    if (last != p2)
                                    {
                                        double centreX, centreY, startAngle, deltaAngle;
                                        double rx = p1.x, ry = p1.y;

                                        endpointToCentreParameters (last.x, last.y, p2.x, p2.y,
                                                                    angle, largeArc, sweep,
                                                                    rx, ry, centreX, centreY,
                                                                    startAngle, deltaAngle);

                                        path.addCentredArc ((float) centreX, (float) centreY,
                                                            (float) rx, (float) ry,
                                                            angle, (float) startAngle, (float) (startAngle + deltaAngle),
                                                            false);

                                        path.lineTo (p2);
                                    }

                                    last2 = last;
                                    last = p2;
                                }
                            }
                        }
                    }
                }

                break;

            case 'Z':
            case 'z':
                path.closeSubPath();
                last = last2 = subpathStart;
                d.incrementToEndOfWhitespace();
                currentCommand = 'M';
                break;

            default:
                carryOn = false;
                break;
            }

            if (! carryOn)
                break;

            previousCommand = currentCommand;
        }

        // paths that finish back at their start position often seem to be
        // left without a 'z', so need to be closed explicitly..
        if (path.getCurrentPosition() == subpathStart)
            path.closeSubPath();
    }

private:
    //==============================================================================
    const File originalFile;
    const XmlPath topLevelXml;
    float width = 512, height = 512, viewBoxW = 0, viewBoxH = 0;
    AffineTransform transform;
    String cssStyleText;

    static bool isNone (const String& s) noexcept
    {
        return s.equalsIgnoreCase ("none");
    }

    static void setCommonAttributes (Drawable& d, const XmlPath& xml)
    {
        auto compID = xml->getStringAttribute ("id");
        d.setName (compID);
        d.setComponentID (compID);

        if (isNone (xml->getStringAttribute ("display")))
            d.setVisible (false);
    }

    //==============================================================================
    void parseSubElements (const XmlPath& xml, DrawableComposite& parentDrawable, bool shouldParseClip = true)
    {
        for (auto* e : xml->getChildIterator())
        {
            const XmlPath child (xml.getChild (e));

            if (auto* drawable = parseSubElement (child))
            {
                parentDrawable.addChildComponent (drawable);

                if (! isNone (getStyleAttribute (child, "display")))
                    drawable->setVisible (true);

                if (shouldParseClip)
                    parseClipPath (child, *drawable);
            }
        }
    }

    Drawable* parseSubElement (const XmlPath& xml)
    {
        {
            Path path;
            if (parsePathElement (xml, path))
                return parseShape (xml, path);
        }

        auto tag = xml->getTagNameWithoutNamespace();

        if (tag == "g")         return parseGroupElement (xml, true);
        if (tag == "svg")       return parseSVGElement (xml);
        if (tag == "text")      return parseText (xml, true, nullptr);
        if (tag == "image")     return parseImage (xml, true);
        if (tag == "switch")    return parseSwitch (xml);
        if (tag == "a")         return parseLinkElement (xml);
        if (tag == "use")       return parseUseOther (xml);
        if (tag == "style")     parseCSSStyle (xml);
        if (tag == "defs")      parseDefs (xml);

        return nullptr;
    }

    bool parsePathElement (const XmlPath& xml, Path& path) const
    {
        auto tag = xml->getTagNameWithoutNamespace();

        if (tag == "path")      { parsePath (xml, path);           return true; }
        if (tag == "rect")      { parseRect (xml, path);           return true; }
        if (tag == "circle")    { parseCircle (xml, path);         return true; }
        if (tag == "ellipse")   { parseEllipse (xml, path);        return true; }
        if (tag == "line")      { parseLine (xml, path);           return true; }
        if (tag == "polyline")  { parsePolygon (xml, true, path);  return true; }
        if (tag == "polygon")   { parsePolygon (xml, false, path); return true; }
        if (tag == "use")       { return parseUsePath (xml, path); }

        return false;
    }

    DrawableComposite* parseSwitch (const XmlPath& xml)
    {
        if (auto* group = xml->getChildByName ("g"))
            return parseGroupElement (xml.getChild (group), true);

        return nullptr;
    }

    DrawableComposite* parseGroupElement (const XmlPath& xml, bool shouldParseTransform)
    {
        if (shouldParseTransform && xml->hasAttribute ("transform"))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseGroupElement (xml, false);
        }

        auto* drawable = new DrawableComposite();
        setCommonAttributes (*drawable, xml);
        parseSubElements (xml, *drawable);

        drawable->resetContentAreaAndBoundingBoxToFitChildren();
        return drawable;
    }

    DrawableComposite* parseLinkElement (const XmlPath& xml)
    {
        return parseGroupElement (xml, true); // TODO: support for making this clickable
    }

    //==============================================================================
    void parsePath (const XmlPath& xml, Path& path) const
    {
        parsePathString (path, xml->getStringAttribute ("d"));

        if (getStyleAttribute (xml, "fill-rule").trim().equalsIgnoreCase ("evenodd"))
            path.setUsingNonZeroWinding (false);
    }

    void parseRect (const XmlPath& xml, Path& rect) const
    {
        const bool hasRX = xml->hasAttribute ("rx");
        const bool hasRY = xml->hasAttribute ("ry");

        if (hasRX || hasRY)
        {
            float rx = getCoordLength (xml, "rx", viewBoxW);
            float ry = getCoordLength (xml, "ry", viewBoxH);

            if (! hasRX)
                rx = ry;
            else if (! hasRY)
                ry = rx;

            rect.addRoundedRectangle (getCoordLength (xml, "x", viewBoxW),
                                      getCoordLength (xml, "y", viewBoxH),
                                      getCoordLength (xml, "width", viewBoxW),
                                      getCoordLength (xml, "height", viewBoxH),
                                      rx, ry);
        }
        else
        {
            rect.addRectangle (getCoordLength (xml, "x", viewBoxW),
                               getCoordLength (xml, "y", viewBoxH),
                               getCoordLength (xml, "width", viewBoxW),
                               getCoordLength (xml, "height", viewBoxH));
        }
    }

    void parseCircle (const XmlPath& xml, Path& circle) const
    {
        auto cx = getCoordLength (xml, "cx", viewBoxW);
        auto cy = getCoordLength (xml, "cy", viewBoxH);
        auto radius = getCoordLength (xml, "r", viewBoxW);

        circle.addEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }

    void parseEllipse (const XmlPath& xml, Path& ellipse) const
    {
        auto cx      = getCoordLength (xml, "cx", viewBoxW);
        auto cy      = getCoordLength (xml, "cy", viewBoxH);
        auto radiusX = getCoordLength (xml, "rx", viewBoxW);
        auto radiusY = getCoordLength (xml, "ry", viewBoxH);

        ellipse.addEllipse (cx - radiusX, cy - radiusY, radiusX * 2.0f, radiusY * 2.0f);
    }

    void parseLine (const XmlPath& xml, Path& line) const
    {
        auto x1 = getCoordLength (xml, "x1", viewBoxW);
        auto y1 = getCoordLength (xml, "y1", viewBoxH);
        auto x2 = getCoordLength (xml, "x2", viewBoxW);
        auto y2 = getCoordLength (xml, "y2", viewBoxH);

        line.startNewSubPath (x1, y1);
        line.lineTo (x2, y2);
    }

    void parsePolygon (const XmlPath& xml, bool isPolyline, Path& path) const
    {
        auto pointsAtt = xml->getStringAttribute ("points");
        auto points = pointsAtt.getCharPointer();
        Point<float> p;

        if (parseCoords (points, p, true))
        {
            Point<float> first (p), last;

            path.startNewSubPath (first);

            while (parseCoords (points, p, true))
            {
                last = p;
                path.lineTo (p);
            }

            if ((! isPolyline) || first == last)
                path.closeSubPath();
        }
    }

    static String getLinkedID (const XmlPath& xml)
    {
        auto link = xml->getStringAttribute ("xlink:href");

        if (link.startsWithChar ('#'))
            return link.substring (1);

        return {};
    }

    bool parseUsePath (const XmlPath& xml, Path& path) const
    {
        auto linkedID = getLinkedID (xml);

        if (linkedID.isNotEmpty())
        {
            UsePathOp op = { this, &path };
            return topLevelXml.applyOperationToChildWithID (linkedID, op);
        }

        return false;
    }

    Drawable* parseUseOther (const XmlPath& xml) const
    {
        if (auto* drawableText  = parseText (xml, false, nullptr))    return drawableText;
        if (auto* drawableImage = parseImage (xml, false))   return drawableImage;

        return nullptr;
    }

    static String parseURL (const String& str)
    {
        if (str.startsWithIgnoreCase ("url"))
            return str.fromFirstOccurrenceOf ("#", false, false)
                      .upToLastOccurrenceOf (")", false, false).trim();

        return {};
    }

    //==============================================================================
    Drawable* parseShape (const XmlPath& xml, Path& path,
                          bool shouldParseTransform = true,
                          AffineTransform* additonalTransform = nullptr) const
    {
        if (shouldParseTransform && xml->hasAttribute ("transform"))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseShape (xml, path, false, additonalTransform);
        }

        auto dp = new DrawablePath();
        setCommonAttributes (*dp, xml);
        dp->setFill (Colours::transparentBlack);

        path.applyTransform (transform);

        if (additonalTransform != nullptr)
            path.applyTransform (*additonalTransform);

        dp->setPath (path);

        dp->setFill (getPathFillType (path, xml, "fill",
                                      getStyleAttribute (xml, "fill-opacity"),
                                      getStyleAttribute (xml, "opacity"),
                                      pathContainsClosedSubPath (path) ? Colours::black
                                                                       : Colours::transparentBlack));

        auto strokeType = getStyleAttribute (xml, "stroke");

        if (strokeType.isNotEmpty() && ! isNone (strokeType))
        {
            dp->setStrokeFill (getPathFillType (path, xml, "stroke",
                                                getStyleAttribute (xml, "stroke-opacity"),
                                                getStyleAttribute (xml, "opacity"),
                                                Colours::transparentBlack));

            dp->setStrokeType (getStrokeFor (xml));
        }

        auto strokeDashArray = getStyleAttribute (xml, "stroke-dasharray");

        if (strokeDashArray.isNotEmpty())
            parseDashArray (strokeDashArray, *dp);

        return dp;
    }

    static bool pathContainsClosedSubPath (const Path& path) noexcept
    {
        for (Path::Iterator iter (path); iter.next();)
            if (iter.elementType == Path::Iterator::closePath)
                return true;

        return false;
    }

    void parseDashArray (const String& dashList, DrawablePath& dp) const
    {
        if (dashList.equalsIgnoreCase ("null") || isNone (dashList))
            return;

        Array<float> dashLengths;

        for (auto t = dashList.getCharPointer();;)
        {
            float value;
            if (! parseCoord (t, value, true, Axis::x))
                break;

            dashLengths.add (value);

            t.incrementToEndOfWhitespace();

            if (*t == ',')
                ++t;
        }

        if (dashLengths.size() > 0)
        {
            auto* dashes = dashLengths.getRawDataPointer();

            for (int i = 0; i < dashLengths.size(); ++i)
            {
                if (dashes[i] <= 0)  // SVG uses zero-length dashes to mean a dotted line
                {
                    if (dashLengths.size() == 1)
                        return;

                    const float nonZeroLength = 0.001f;
                    dashes[i] = nonZeroLength;

                    const int pairedIndex = i ^ 1;

                    if (isPositiveAndBelow (pairedIndex, dashLengths.size())
                          && dashes[pairedIndex] > nonZeroLength)
                        dashes[pairedIndex] -= nonZeroLength;
                }
            }

            dp.setDashLengths (dashLengths);
        }
    }

    bool parseClipPath (const XmlPath& xml, Drawable& d)
    {
        const String clipPath (getStyleAttribute (xml, "clip-path"));

        if (clipPath.isNotEmpty())
        {
            auto urlID = parseURL (clipPath);

            if (urlID.isNotEmpty())
            {
                GetClipPathOp op = { this, &d };
                return topLevelXml.applyOperationToChildWithID (urlID, op);
            }
        }

        return false;
    }

    bool applyClipPath (Drawable& target, const XmlPath& xmlPath)
    {
        if (xmlPath->hasTagNameIgnoringNamespace ("clipPath"))
        {
            std::unique_ptr<DrawableComposite> drawableClipPath (new DrawableComposite());

            parseSubElements (xmlPath, *drawableClipPath, false);

            if (drawableClipPath->getNumChildComponents() > 0)
            {
                setCommonAttributes (*drawableClipPath, xmlPath);
                target.setClipPath (std::move (drawableClipPath));
                return true;
            }
        }

        return false;
    }

    bool addGradientStopsIn (ColourGradient& cg, const XmlPath& fillXml) const
    {
        bool result = false;

        if (fillXml.xml != nullptr)
        {
            for (auto* e : fillXml->getChildWithTagNameIterator ("stop"))
            {
                auto col = parseColour (fillXml.getChild (e), "stop-color", Colours::black);

                auto opacity = getStyleAttribute (fillXml.getChild (e), "stop-opacity", "1");
                col = col.withMultipliedAlpha (jlimit (0.0f, 1.0f, parseSafeFloat (opacity)));

                auto offset = parseSafeFloat (e->getStringAttribute ("offset"));

                if (e->getStringAttribute ("offset").containsChar ('%'))
                    offset *= 0.01f;

                cg.addColour (jlimit (0.0f, 1.0f, offset), col);
                result = true;
            }
        }

        return result;
    }

    FillType getGradientFillType (const XmlPath& fillXml,
                                  const Path& path,
                                  const float opacity) const
    {
        ColourGradient gradient;

        {
            auto linkedID = getLinkedID (fillXml);

            if (linkedID.isNotEmpty())
            {
                SetGradientStopsOp op = { this, &gradient, };
                topLevelXml.applyOperationToChildWithID (linkedID, op);
            }
        }

        addGradientStopsIn (gradient, fillXml);

        if (int numColours = gradient.getNumColours())
        {
            if (gradient.getColourPosition (0) > 0)
                gradient.addColour (0.0, gradient.getColour (0));

            if (gradient.getColourPosition (numColours - 1) < 1.0)
                gradient.addColour (1.0, gradient.getColour (numColours - 1));
        }
        else
        {
            gradient.addColour (0.0, Colours::black);
            gradient.addColour (1.0, Colours::black);
        }

        if (opacity < 1.0f)
            gradient.multiplyOpacity (opacity);

        jassert (gradient.getNumColours() > 0);

        gradient.isRadial = fillXml->hasTagNameIgnoringNamespace ("radialGradient");

        float gradientWidth = viewBoxW;
        float gradientHeight = viewBoxH;
        float dx = 0.0f;
        float dy = 0.0f;

        const bool userSpace = fillXml->getStringAttribute ("gradientUnits").equalsIgnoreCase ("userSpaceOnUse");

        if (! userSpace)
        {
            auto bounds = path.getBounds();
            dx = bounds.getX();
            dy = bounds.getY();
            gradientWidth = bounds.getWidth();
            gradientHeight = bounds.getHeight();
        }

        if (gradient.isRadial)
        {
            if (userSpace)
                gradient.point1.setXY (dx + getCoordLength (fillXml->getStringAttribute ("cx", "50%"), gradientWidth),
                                       dy + getCoordLength (fillXml->getStringAttribute ("cy", "50%"), gradientHeight));
            else
                gradient.point1.setXY (dx + gradientWidth  * getCoordLength (fillXml->getStringAttribute ("cx", "50%"), 1.0f),
                                       dy + gradientHeight * getCoordLength (fillXml->getStringAttribute ("cy", "50%"), 1.0f));

            auto radius = getCoordLength (fillXml->getStringAttribute ("r", "50%"), gradientWidth);
            gradient.point2 = gradient.point1 + Point<float> (radius, 0.0f);

            //xxx (the fx, fy focal point isn't handled properly here..)
        }
        else
        {
            if (userSpace)
            {
                gradient.point1.setXY (dx + getCoordLength (fillXml->getStringAttribute ("x1", "0%"), gradientWidth),
                                       dy + getCoordLength (fillXml->getStringAttribute ("y1", "0%"), gradientHeight));

                gradient.point2.setXY (dx + getCoordLength (fillXml->getStringAttribute ("x2", "100%"), gradientWidth),
                                       dy + getCoordLength (fillXml->getStringAttribute ("y2", "0%"), gradientHeight));
            }
            else
            {
                gradient.point1.setXY (dx + gradientWidth  * getCoordLength (fillXml->getStringAttribute ("x1", "0%"), 1.0f),
                                       dy + gradientHeight * getCoordLength (fillXml->getStringAttribute ("y1", "0%"), 1.0f));

                gradient.point2.setXY (dx + gradientWidth  * getCoordLength (fillXml->getStringAttribute ("x2", "100%"), 1.0f),
                                       dy + gradientHeight * getCoordLength (fillXml->getStringAttribute ("y2", "0%"), 1.0f));
            }

            if (gradient.point1 == gradient.point2)
                return Colour (gradient.getColour (gradient.getNumColours() - 1));
        }

        FillType type (gradient);

        auto gradientTransform = parseTransform (fillXml->getStringAttribute ("gradientTransform"));

        if (gradient.isRadial)
        {
            type.transform = gradientTransform;
        }
        else
        {
            // Transform the perpendicular vector into the new coordinate space for the gradient.
            // This vector is now the slope of the linear gradient as it should appear in the new coord space
            auto perpendicular = Point<float> (gradient.point2.y - gradient.point1.y,
                                               gradient.point1.x - gradient.point2.x)
                                    .transformedBy (gradientTransform.withAbsoluteTranslation (0, 0));

            auto newGradPoint1 = gradient.point1.transformedBy (gradientTransform);
            auto newGradPoint2 = gradient.point2.transformedBy (gradientTransform);

            // Project the transformed gradient vector onto the transformed slope of the linear
            // gradient as it should appear in the new coordinate space
            const float scale = perpendicular.getDotProduct (newGradPoint2 - newGradPoint1)
                                  / perpendicular.getDotProduct (perpendicular);

            type.gradient->point1 = newGradPoint1;
            type.gradient->point2 = newGradPoint2 - perpendicular * scale;
        }

        return type;
    }

    FillType getPathFillType (const Path& path,
                              const XmlPath& xml,
                              StringRef fillAttribute,
                              const String& fillOpacity,
                              const String& overallOpacity,
                              const Colour defaultColour) const
    {
        float opacity = 1.0f;

        if (overallOpacity.isNotEmpty())
            opacity = jlimit (0.0f, 1.0f, parseSafeFloat (overallOpacity));

        if (fillOpacity.isNotEmpty())
            opacity *= jlimit (0.0f, 1.0f, parseSafeFloat (fillOpacity));

        String fill (getStyleAttribute (xml, fillAttribute));
        String urlID = parseURL (fill);

        if (urlID.isNotEmpty())
        {
            GetFillTypeOp op = { this, &path, opacity, FillType() };

            if (topLevelXml.applyOperationToChildWithID (urlID, op))
                return op.fillType;
        }

        if (isNone (fill))
            return Colours::transparentBlack;

        return parseColour (xml, fillAttribute, defaultColour).withMultipliedAlpha (opacity);
    }

    static PathStrokeType::JointStyle getJointStyle (const String& join) noexcept
    {
        if (join.equalsIgnoreCase ("round"))  return PathStrokeType::curved;
        if (join.equalsIgnoreCase ("bevel"))  return PathStrokeType::beveled;

        return PathStrokeType::mitered;
    }

    static PathStrokeType::EndCapStyle getEndCapStyle (const String& cap) noexcept
    {
        if (cap.equalsIgnoreCase ("round"))   return PathStrokeType::rounded;
        if (cap.equalsIgnoreCase ("square"))  return PathStrokeType::square;

        return PathStrokeType::butt;
    }

    float getStrokeWidth (const String& strokeWidth) const noexcept
    {
        auto transformScale = std::sqrt (std::abs (transform.getDeterminant()));
        return transformScale * getCoordLength (strokeWidth, viewBoxW);
    }

    PathStrokeType getStrokeFor (const XmlPath& xml) const
    {
        return PathStrokeType (getStrokeWidth (getStyleAttribute (xml, "stroke-width", "1")),
                               getJointStyle  (getStyleAttribute (xml, "stroke-linejoin")),
                               getEndCapStyle (getStyleAttribute (xml, "stroke-linecap")));
    }

    //==============================================================================
    Drawable* useText (const XmlPath& xml) const
    {
        auto translation = AffineTransform::translation (parseSafeFloat (xml->getStringAttribute ("x")),
                                                         parseSafeFloat (xml->getStringAttribute ("y")));

        UseTextOp op = { this, &translation, nullptr };

        auto linkedID = getLinkedID (xml);

        if (linkedID.isNotEmpty())
            topLevelXml.applyOperationToChildWithID (linkedID, op);

        return op.target;
    }

    /*  Handling the stateful consumption of x and y coordinates added to <text> and <tspan> elements.

        <text> elements must have their own x and y attributes, or be positioned at (0, 0) since groups
        enclosing <text> elements can't have x and y attributes.

        <tspan> elements can be embedded inside <text> elements, and <tspan> elements. <text> elements
        can't be embedded inside <text> or <tspan> elements.

        A <tspan> element can have its own x, y attributes, which it will consume at the same time as
        it consumes its parent's attributes. Its own elements will take precedence, but parent elements
        will be consumed regardless.
    */
    class StringLayoutState
    {
    public:
        StringLayoutState (StringLayoutState* parentIn, Array<float> xIn, Array<float> yIn)
            : parent (parentIn),
              xCoords (std::move (xIn)),
              yCoords (std::move (yIn))
        {
        }

        Point<float> getNextStartingPos() const
        {
            if (parent != nullptr)
                return parent->getNextStartingPos();

            return nextStartingPos;
        }

        void setNextStartingPos (Point<float> newPos)
        {
            nextStartingPos = newPos;

            if (parent != nullptr)
                parent->setNextStartingPos (newPos);
        }

        std::pair<std::optional<float>, std::optional<float>> popCoords()
        {
            auto x = xCoords.isEmpty() ? std::optional<float>{} : std::make_optional (xCoords.removeAndReturn (0));
            auto y = yCoords.isEmpty() ? std::optional<float>{} : std::make_optional (yCoords.removeAndReturn (0));

            if (parent != nullptr)
            {
                auto [parentX, parentY] = parent->popCoords();

                if (! x.has_value())
                    x = parentX;

                if (! y.has_value())
                    y = parentY;
            }

            return { x, y };
        }

        bool hasMoreCoords() const
        {
            if (! xCoords.isEmpty() || ! yCoords.isEmpty())
                return true;

            if (parent != nullptr)
                return parent->hasMoreCoords();

            return false;
        }

    private:
        StringLayoutState* parent = nullptr;
        Point<float> nextStartingPos;
        Array<float> xCoords, yCoords;
    };

    Drawable* parseText (const XmlPath& xml, bool shouldParseTransform,
                         AffineTransform* additonalTransform,
                         StringLayoutState* parentLayoutState = nullptr) const
    {
        if (shouldParseTransform && xml->hasAttribute ("transform"))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseText (xml, false, additonalTransform);
        }

        if (xml->hasTagName ("use"))
            return useText (xml);

        if (! xml->hasTagName ("text") && ! xml->hasTagNameIgnoringNamespace ("tspan"))
            return nullptr;

        // If a <tspan> element has no x, or y attributes of its own, it can still use the
        // parent's yet unconsumed such attributes.
        StringLayoutState layoutState { parentLayoutState,
                                        getCoordList (*xml, Axis::x),
                                        getCoordList (*xml, Axis::y) };

        auto font = getFont (xml);
        auto anchorStr = getStyleAttribute (xml, "text-anchor");

        auto dc = new DrawableComposite();
        setCommonAttributes (*dc, xml);

        for (auto* e : xml->getChildIterator())
        {
            if (e->isTextElement())
            {
                auto fullText = e->getText();

                const auto subtextElements = [&]
                {
                    std::vector<std::tuple<String, std::optional<float>, std::optional<float>>> result;

                    for (auto it = fullText.begin(), end = fullText.end(); it != end;)
                    {
                        const auto pos = layoutState.popCoords();
                        const auto next = layoutState.hasMoreCoords() ? it + 1 : end;
                        result.emplace_back (String (it, next), pos.first, pos.second);
                        it = next;
                    }

                    return result;
                }();

                for (const auto& [text, optX, optY] : subtextElements)
                {
                    auto dt = new DrawableText();
                    dc->addAndMakeVisible (dt);

                    dt->setText (text);
                    dt->setFont (font, true);

                    if (additonalTransform != nullptr)
                        dt->setDrawableTransform (transform.followedBy (*additonalTransform));
                    else
                        dt->setDrawableTransform (transform);

                    dt->setColour (parseColour (xml, "fill", Colours::black)
                                       .withMultipliedAlpha (parseSafeFloat (getStyleAttribute (xml, "fill-opacity", "1"))));

                    const auto x = optX.value_or (layoutState.getNextStartingPos().getX());
                    const auto y = optY.value_or (layoutState.getNextStartingPos().getY());

                    Rectangle<float> bounds (x, y - font.getAscent(),
                                             font.getStringWidthFloat (text), font.getHeight());

                    if (anchorStr == "middle")   bounds.setX (bounds.getX() - bounds.getWidth() / 2.0f);
                    else if (anchorStr == "end") bounds.setX (bounds.getX() - bounds.getWidth());

                    dt->setBoundingBox (bounds);

                    layoutState.setNextStartingPos ({ bounds.getRight(), y });
                }
            }
            else if (e->hasTagNameIgnoringNamespace ("tspan"))
            {
                dc->addAndMakeVisible (parseText (xml.getChild (e), true, nullptr, &layoutState));
            }
        }

        return dc;
    }

    Font getFont (const XmlPath& xml) const
    {
        Font f;
        auto family = getStyleAttribute (xml, "font-family").unquoted();

        if (family.isNotEmpty())
            f.setTypefaceName (family);

        if (getStyleAttribute (xml, "font-style").containsIgnoreCase ("italic"))
            f.setItalic (true);

        if (getStyleAttribute (xml, "font-weight").containsIgnoreCase ("bold"))
            f.setBold (true);

        return f.withPointHeight (getCoordLength (getStyleAttribute (xml, "font-size", "15"), 1.0f));
    }

    //==============================================================================
    Drawable* useImage (const XmlPath& xml) const
    {
        auto translation = AffineTransform::translation (parseSafeFloat (xml->getStringAttribute ("x")),
                                                         parseSafeFloat (xml->getStringAttribute ("y")));

        UseImageOp op = { this, &translation, nullptr };

        auto linkedID = getLinkedID (xml);

        if (linkedID.isNotEmpty())
            topLevelXml.applyOperationToChildWithID (linkedID, op);

        return op.target;
    }

    Drawable* parseImage (const XmlPath& xml, bool shouldParseTransform,
                          AffineTransform* additionalTransform = nullptr) const
    {
        if (shouldParseTransform && xml->hasAttribute ("transform"))
        {
            SVGState newState (*this);
            newState.addTransform (xml);

            return newState.parseImage (xml, false, additionalTransform);
        }

        if (xml->hasTagName ("use"))
            return useImage (xml);

        if (! xml->hasTagName ("image"))
            return nullptr;

        auto link = xml->getStringAttribute ("xlink:href");

        std::unique_ptr<InputStream> inputStream;
        MemoryOutputStream imageStream;

        if (link.startsWith ("data:"))
        {
            const auto indexOfComma = link.indexOf (",");
            auto format = link.substring (5, indexOfComma).trim();
            auto indexOfSemi = format.indexOf (";");

            if (format.substring (indexOfSemi + 1).trim().equalsIgnoreCase ("base64"))
            {
                auto mime = format.substring (0, indexOfSemi).trim();

                if (mime.equalsIgnoreCase ("image/png") || mime.equalsIgnoreCase ("image/jpeg"))
                {
                    auto base64text = link.substring (indexOfComma + 1).removeCharacters ("\t\n\r ");

                    if (Base64::convertFromBase64 (imageStream, base64text))
                        inputStream.reset (new MemoryInputStream (imageStream.getData(), imageStream.getDataSize(), false));
                }
            }
        }
        else
        {
            auto linkedFile = originalFile.getParentDirectory().getChildFile (link);

            if (linkedFile.existsAsFile())
                inputStream = linkedFile.createInputStream();
        }

        if (inputStream != nullptr)
        {
            auto image = ImageFileFormat::loadFrom (*inputStream);

            if (image.isValid())
            {
                auto* di = new DrawableImage();

                setCommonAttributes (*di, xml);

                Rectangle<float> imageBounds (parseSafeFloat (xml->getStringAttribute ("x")),
                                              parseSafeFloat (xml->getStringAttribute ("y")),
                                              parseSafeFloat (xml->getStringAttribute ("width",  String (image.getWidth()))),
                                              parseSafeFloat (xml->getStringAttribute ("height", String (image.getHeight()))));

                di->setImage (image.rescaled ((int) imageBounds.getWidth(),
                                              (int) imageBounds.getHeight()));

                di->setTransformToFit (imageBounds, RectanglePlacement (parsePlacementFlags (xml->getStringAttribute ("preserveAspectRatio").trim())));

                if (additionalTransform != nullptr)
                    di->setTransform (di->getTransform().followedBy (transform).followedBy (*additionalTransform));
                else
                    di->setTransform (di->getTransform().followedBy (transform));

                return di;
            }
        }

        return nullptr;
    }

    //==============================================================================
    void addTransform (const XmlPath& xml)
    {
        transform = parseTransform (xml->getStringAttribute ("transform"))
                        .followedBy (transform);
    }

    //==============================================================================
    enum class Axis { x, y };

    bool parseCoord (String::CharPointerType& s, float& value, bool allowUnits, Axis axis) const
    {
        String number;

        if (! parseNextNumber (s, number, allowUnits))
        {
            value = 0;
            return false;
        }

        value = getCoordLength (number, axis == Axis::x ? viewBoxW : viewBoxH);
        return true;
    }

    bool parseCoords (String::CharPointerType& s, Point<float>& p, bool allowUnits) const
    {
        return parseCoord (s, p.x, allowUnits, Axis::x)
            && parseCoord (s, p.y, allowUnits, Axis::y);
    }

    bool parseCoordsOrSkip (String::CharPointerType& s, Point<float>& p, bool allowUnits) const
    {
        if (parseCoords (s, p, allowUnits))
            return true;

        if (! s.isEmpty()) ++s;
        return false;
    }

    float getCoordLength (const String& s, const float sizeForProportions) const noexcept
    {
        auto n = parseSafeFloat (s);
        auto len = s.length();

        if (len > 2)
        {
            auto dpi = 96.0f;

            auto n1 = s[len - 2];
            auto n2 = s[len - 1];

            if (n1 == 'i' && n2 == 'n')         n *= dpi;
            else if (n1 == 'm' && n2 == 'm')    n *= dpi / 25.4f;
            else if (n1 == 'c' && n2 == 'm')    n *= dpi / 2.54f;
            else if (n1 == 'p' && n2 == 'c')    n *= 15.0f;
            else if (n2 == '%')                 n *= 0.01f * sizeForProportions;
        }

        return n;
    }

    float getCoordLength (const XmlPath& xml, const char* attName, const float sizeForProportions) const noexcept
    {
        return getCoordLength (xml->getStringAttribute (attName), sizeForProportions);
    }

    Array<float> getCoordList (const XmlElement& xml, Axis axis) const
    {
        const String attributeName { axis == Axis::x ? "x" : "y" };

        if (! xml.hasAttribute (attributeName))
            return {};

        return getCoordList (xml.getStringAttribute (attributeName), true, axis);
    }

    Array<float> getCoordList (const String& list, bool allowUnits, Axis axis) const
    {
        auto text = list.getCharPointer();
        float value;
        Array<float> coords;

        while (parseCoord (text, value, allowUnits, axis))
            coords.add (value);

        return coords;
    }

    static float parseSafeFloat (const String& s)
    {
        auto n = s.getFloatValue();
        return (std::isnan (n) || std::isinf (n)) ? 0.0f : n;
    }

    //==============================================================================
    void parseCSSStyle (const XmlPath& xml)
    {
        cssStyleText = xml->getAllSubText() + "\n" + cssStyleText;
    }

    void parseDefs (const XmlPath& xml)
    {
        if (auto* style = xml->getChildByName ("style"))
            parseCSSStyle (xml.getChild (style));
    }

    static String::CharPointerType findStyleItem (String::CharPointerType source, String::CharPointerType name)
    {
        auto nameLength = (int) name.length();

        while (! source.isEmpty())
        {
            if (source.getAndAdvance() == '.'
                 && CharacterFunctions::compareIgnoreCaseUpTo (source, name, nameLength) == 0)
            {
                auto endOfName = (source + nameLength).findEndOfWhitespace();

                if (*endOfName == '{')
                    return endOfName;

                if (*endOfName == ',')
                    return CharacterFunctions::find (endOfName, (juce_wchar) '{');
            }
        }

        return source;
    }

    String getStyleAttribute (const XmlPath& xml, StringRef attributeName, const String& defaultValue = String()) const
    {
        if (xml->hasAttribute (attributeName))
            return xml->getStringAttribute (attributeName, defaultValue);

        auto styleAtt = xml->getStringAttribute ("style");

        if (styleAtt.isNotEmpty())
        {
            auto value = getAttributeFromStyleList (styleAtt, attributeName, {});

            if (value.isNotEmpty())
                return value;
        }
        else if (xml->hasAttribute ("class"))
        {
            for (auto i = cssStyleText.getCharPointer();;)
            {
                auto openBrace = findStyleItem (i, xml->getStringAttribute ("class").getCharPointer());

                if (openBrace.isEmpty())
                    break;

                auto closeBrace = CharacterFunctions::find (openBrace, (juce_wchar) '}');

                if (closeBrace.isEmpty())
                    break;

                auto value = getAttributeFromStyleList (String (openBrace + 1, closeBrace),
                                                        attributeName, defaultValue);
                if (value.isNotEmpty())
                    return value;

                i = closeBrace + 1;
            }
        }

        if (xml.parent != nullptr)
            return getStyleAttribute (*xml.parent, attributeName, defaultValue);

        return defaultValue;
    }

    String getInheritedAttribute (const XmlPath& xml, StringRef attributeName) const
    {
        if (xml->hasAttribute (attributeName))
            return xml->getStringAttribute (attributeName);

        if (xml.parent != nullptr)
            return getInheritedAttribute (*xml.parent, attributeName);

        return {};
    }

    static int parsePlacementFlags (const String& align) noexcept
    {
        if (align.isEmpty())
            return 0;

        if (isNone (align))
            return RectanglePlacement::stretchToFit;

        return (align.containsIgnoreCase ("slice") ? RectanglePlacement::fillDestination : 0)
             | (align.containsIgnoreCase ("xMin")  ? RectanglePlacement::xLeft
                                                   : (align.containsIgnoreCase ("xMax") ? RectanglePlacement::xRight
                                                                                        : RectanglePlacement::xMid))
             | (align.containsIgnoreCase ("yMin")  ? RectanglePlacement::yTop
                                                   : (align.containsIgnoreCase ("yMax") ? RectanglePlacement::yBottom
                                                                                        : RectanglePlacement::yMid));
    }

    //==============================================================================
    static bool isIdentifierChar (juce_wchar c)
    {
        return CharacterFunctions::isLetter (c) || c == '-';
    }

    static String getAttributeFromStyleList (const String& list, StringRef attributeName, const String& defaultValue)
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
                i = list.indexOfChar (i, ':');

                if (i < 0)
                    break;

                int end = list.indexOfChar (i, ';');

                if (end < 0)
                    end = 0x7ffff;

                return list.substring (i + 1, end).trim();
            }

            ++i;
        }

        return defaultValue;
    }

    //==============================================================================
    static bool isStartOfNumber (juce_wchar c) noexcept
    {
        return CharacterFunctions::isDigit (c) || c == '-' || c == '+';
    }

    static bool parseNextNumber (String::CharPointerType& text, String& value, bool allowUnits)
    {
        auto s = text;

        while (s.isWhitespace() || *s == ',')
            ++s;

        auto start = s;

        if (isStartOfNumber (*s))
            ++s;

        while (s.isDigit())
            ++s;

        if (*s == '.')
        {
            ++s;

            while (s.isDigit())
                ++s;
        }

        if ((*s == 'e' || *s == 'E') && isStartOfNumber (s[1]))
        {
            s += 2;

            while (s.isDigit())
                ++s;
        }

        if (allowUnits)
            while (s.isLetter())
                ++s;

        if (s == start)
        {
            text = s;
            return false;
        }

        value = String (start, s);

        while (s.isWhitespace() || *s == ',')
            ++s;

        text = s;
        return true;
    }

    static bool parseNextFlag (String::CharPointerType& text, bool& value)
    {
        while (text.isWhitespace() || *text == ',')
            ++text;

        if (*text != '0' && *text != '1')
            return false;

        value = *(text++) != '0';

        while (text.isWhitespace() || *text == ',')
             ++text;

        return true;
    }

    //==============================================================================
    Colour parseColour (const XmlPath& xml, StringRef attributeName, const Colour defaultColour) const
    {
        auto text = getStyleAttribute (xml, attributeName);

        if (text.startsWithChar ('#'))
        {
            uint32 hex[8] = { 0 };
            hex[6] = hex[7] = 15;

            int numChars = 0;
            auto s = text.getCharPointer();

            while (numChars < 8)
            {
                auto hexValue = CharacterFunctions::getHexDigitValue (*++s);

                if (hexValue >= 0)
                    hex[numChars++] = (uint32) hexValue;
                else
                    break;
            }

            if (numChars <= 3)
                return Colour ((uint8) (hex[0] * 0x11),
                               (uint8) (hex[1] * 0x11),
                               (uint8) (hex[2] * 0x11));

            return Colour ((uint8) ((hex[0] << 4) + hex[1]),
                           (uint8) ((hex[2] << 4) + hex[3]),
                           (uint8) ((hex[4] << 4) + hex[5]),
                           (uint8) ((hex[6] << 4) + hex[7]));
        }

        if (text.startsWith ("rgb") || text.startsWith ("hsl"))
        {
            auto tokens = [&text]
            {
                auto openBracket = text.indexOfChar ('(');
                auto closeBracket = text.indexOfChar (openBracket, ')');

                StringArray arr;

                if (openBracket >= 3 && closeBracket > openBracket)
                {
                    arr.addTokens (text.substring (openBracket + 1, closeBracket), ",", "");
                    arr.trim();
                    arr.removeEmptyStrings();
                }

                return arr;
            }();

            auto alpha = [&tokens, &text]
            {
                if ((text.startsWith ("rgba") || text.startsWith ("hsla")) && tokens.size() == 4)
                    return parseSafeFloat (tokens[3]);

                return 1.0f;
            }();

            if (text.startsWith ("hsl"))
                return Colour::fromHSL (parseSafeFloat (tokens[0]) / 360.0f,
                                        parseSafeFloat (tokens[1]) / 100.0f,
                                        parseSafeFloat (tokens[2]) / 100.0f,
                                        alpha);

            if (tokens[0].containsChar ('%'))
                return Colour ((uint8) roundToInt (2.55f * parseSafeFloat (tokens[0])),
                               (uint8) roundToInt (2.55f * parseSafeFloat (tokens[1])),
                               (uint8) roundToInt (2.55f * parseSafeFloat (tokens[2])),
                               alpha);

            return Colour ((uint8) tokens[0].getIntValue(),
                           (uint8) tokens[1].getIntValue(),
                           (uint8) tokens[2].getIntValue(),
                           alpha);
        }

        if (text == "inherit")
        {
            for (const XmlPath* p = xml.parent; p != nullptr; p = p->parent)
                if (getStyleAttribute (*p, attributeName).isNotEmpty())
                    return parseColour (*p, attributeName, defaultColour);
        }

        return Colours::findColourForName (text, defaultColour);
    }

    static AffineTransform parseTransform (String t)
    {
        AffineTransform result;

        while (t.isNotEmpty())
        {
            StringArray tokens;
            tokens.addTokens (t.fromFirstOccurrenceOf ("(", false, false)
                               .upToFirstOccurrenceOf (")", false, false),
                              ", ", "");

            tokens.removeEmptyStrings (true);

            float numbers[6];

            for (int i = 0; i < numElementsInArray (numbers); ++i)
                numbers[i] = parseSafeFloat (tokens[i]);

            AffineTransform trans;

            if (t.startsWithIgnoreCase ("matrix"))
            {
                trans = AffineTransform (numbers[0], numbers[2], numbers[4],
                                         numbers[1], numbers[3], numbers[5]);
            }
            else if (t.startsWithIgnoreCase ("translate"))
            {
                trans = AffineTransform::translation (numbers[0], numbers[1]);
            }
            else if (t.startsWithIgnoreCase ("scale"))
            {
                trans = AffineTransform::scale (numbers[0], numbers[tokens.size() > 1 ? 1 : 0]);
            }
            else if (t.startsWithIgnoreCase ("rotate"))
            {
                trans = AffineTransform::rotation (degreesToRadians (numbers[0]), numbers[1], numbers[2]);
            }
            else if (t.startsWithIgnoreCase ("skewX"))
            {
                trans = AffineTransform::shear (std::tan (degreesToRadians (numbers[0])), 0.0f);
            }
            else if (t.startsWithIgnoreCase ("skewY"))
            {
                trans = AffineTransform::shear (0.0f, std::tan (degreesToRadians (numbers[0])));
            }

            result = trans.followedBy (result);
            t = t.fromFirstOccurrenceOf (")", false, false).trimStart();
        }

        return result;
    }

    static void endpointToCentreParameters (double x1, double y1,
                                            double x2, double y2,
                                            double angle,
                                            bool largeArc, bool sweep,
                                            double& rx, double& ry,
                                            double& centreX, double& centreY,
                                            double& startAngle, double& deltaAngle) noexcept
    {
        const double midX = (x1 - x2) * 0.5;
        const double midY = (y1 - y2) * 0.5;

        const double cosAngle = std::cos (angle);
        const double sinAngle = std::sin (angle);
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
            c = std::sqrt (jmax (0.0, ((rx2 * ry2) - (rx2 * yp2) - (ry2 * xp2))
                                         / (( rx2 * yp2) + (ry2 * xp2))));

            if (largeArc == sweep)
                c = -c;
        }
        else
        {
            const double s2 = std::sqrt (s);
            rx *= s2;
            ry *= s2;
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

        startAngle += MathConstants<double>::halfPi;

        deltaAngle = acos (jlimit (-1.0, 1.0, ((ux * vx) + (uy * vy))
                                                / (length * juce_hypot (vx, vy))));

        if ((ux * vy) - (uy * vx) < 0)
            deltaAngle = -deltaAngle;

        if (sweep)
        {
            if (deltaAngle < 0)
                deltaAngle += MathConstants<double>::twoPi;
        }
        else
        {
            if (deltaAngle > 0)
                deltaAngle -= MathConstants<double>::twoPi;
        }

        deltaAngle = fmod (deltaAngle, MathConstants<double>::twoPi);
    }

    SVGState (const SVGState&) = default;
    SVGState& operator= (const SVGState&) = delete;
};


//==============================================================================
std::unique_ptr<Drawable> Drawable::createFromSVG (const XmlElement& svgDocument)
{
    if (! svgDocument.hasTagNameIgnoringNamespace ("svg"))
        return {};

    SVGState state (&svgDocument);
    return std::unique_ptr<Drawable> (state.parseSVGElement (SVGState::XmlPath (&svgDocument, {})));
}

std::unique_ptr<Drawable> Drawable::createFromSVGFile (const File& svgFile)
{
    if (auto xml = parseXMLIfTagMatches (svgFile, "svg"))
        return createFromSVG (*xml);

    return {};
}

Path Drawable::parseSVGPath (const String& svgPath)
{
    SVGState state (nullptr);
    Path p;
    state.parsePathString (p, svgPath);
    return p;
}

} // namespace juce
