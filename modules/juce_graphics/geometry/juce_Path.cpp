/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

// tests that some coordinates aren't NaNs
#define JUCE_CHECK_COORDS_ARE_VALID(x, y) \
    jassert (x == x && y == y);

//==============================================================================
namespace PathHelpers
{
    const float ellipseAngularIncrement = 0.05f;

    static String nextToken (String::CharPointerType& t)
    {
        t = t.findEndOfWhitespace();

        auto start = t;
        size_t numChars = 0;

        while (! (t.isEmpty() || t.isWhitespace()))
        {
            ++t;
            ++numChars;
        }

        return { start, numChars };
    }

    inline double lengthOf (float x1, float y1, float x2, float y2) noexcept
    {
        return juce_hypot ((double) (x1 - x2), (double) (y1 - y2));
    }
}

//==============================================================================
const float Path::lineMarker           = 100001.0f;
const float Path::moveMarker           = 100002.0f;
const float Path::quadMarker           = 100003.0f;
const float Path::cubicMarker          = 100004.0f;
const float Path::closeSubPathMarker   = 100005.0f;

const float Path::defaultToleranceForTesting = 1.0f;
const float Path::defaultToleranceForMeasurement = 0.6f;

static bool isMarker (float value, float marker) noexcept
{
    return value == marker;
}

//==============================================================================
Path::PathBounds::PathBounds() noexcept
{
}

Rectangle<float> Path::PathBounds::getRectangle() const noexcept
{
    return { pathXMin, pathYMin, pathXMax - pathXMin, pathYMax - pathYMin };
}

void Path::PathBounds::reset() noexcept
{
    pathXMin = pathYMin = pathYMax = pathXMax = 0;
}

void Path::PathBounds::reset (float x, float y) noexcept
{
    pathXMin = pathXMax = x;
    pathYMin = pathYMax = y;
}

void Path::PathBounds::extend (float x, float y) noexcept
{
    if (x < pathXMin)      pathXMin = x;
    else if (x > pathXMax) pathXMax = x;

    if (y < pathYMin)      pathYMin = y;
    else if (y > pathYMax) pathYMax = y;
}

//==============================================================================
Path::Path()
{
}

Path::~Path()
{
}

Path::Path (const Path& other)
    : data (other.data),
      bounds (other.bounds),
      useNonZeroWinding (other.useNonZeroWinding)
{
}

Path& Path::operator= (const Path& other)
{
    if (this != &other)
    {
        data = other.data;
        bounds = other.bounds;
        useNonZeroWinding = other.useNonZeroWinding;
    }

    return *this;
}

Path::Path (Path&& other) noexcept
    : data (std::move (other.data)),
      bounds (other.bounds),
      useNonZeroWinding (other.useNonZeroWinding)
{
}

Path& Path::operator= (Path&& other) noexcept
{
    data = std::move (other.data);
    bounds = other.bounds;
    useNonZeroWinding = other.useNonZeroWinding;
    return *this;
}

bool Path::operator== (const Path& other) const noexcept    { return useNonZeroWinding == other.useNonZeroWinding && data == other.data; }
bool Path::operator!= (const Path& other) const noexcept    { return ! operator== (other); }

void Path::clear() noexcept
{
    data.clearQuick();
    bounds.reset();
}

void Path::swapWithPath (Path& other) noexcept
{
    data.swapWith (other.data);
    std::swap (bounds.pathXMin, other.bounds.pathXMin);
    std::swap (bounds.pathXMax, other.bounds.pathXMax);
    std::swap (bounds.pathYMin, other.bounds.pathYMin);
    std::swap (bounds.pathYMax, other.bounds.pathYMax);
    std::swap (useNonZeroWinding, other.useNonZeroWinding);
}

//==============================================================================
void Path::setUsingNonZeroWinding (const bool isNonZero) noexcept
{
    useNonZeroWinding = isNonZero;
}

void Path::scaleToFit (float x, float y, float w, float h, bool preserveProportions) noexcept
{
    applyTransform (getTransformToScaleToFit (x, y, w, h, preserveProportions));
}

//==============================================================================
bool Path::isEmpty() const noexcept
{
    for (auto i = data.begin(), e = data.end(); i != e; ++i)
    {
        auto type = *i;

        if (isMarker (type, moveMarker))
        {
            i += 2;
        }
        else if (isMarker (type, lineMarker)
                 || isMarker (type, quadMarker)
                 || isMarker (type, cubicMarker))
        {
            return false;
        }
    }

    return true;
}

Rectangle<float> Path::getBounds() const noexcept
{
    return bounds.getRectangle();
}

Rectangle<float> Path::getBoundsTransformed (const AffineTransform& transform) const noexcept
{
    return getBounds().transformedBy (transform);
}

//==============================================================================
void Path::preallocateSpace (int numExtraCoordsToMakeSpaceFor)
{
    data.ensureStorageAllocated (data.size() + numExtraCoordsToMakeSpaceFor);
}

void Path::startNewSubPath (const float x, const float y)
{
    JUCE_CHECK_COORDS_ARE_VALID (x, y)

    if (data.isEmpty())
        bounds.reset (x, y);
    else
        bounds.extend (x, y);

    data.add (moveMarker, x, y);
}

void Path::startNewSubPath (Point<float> start)
{
    startNewSubPath (start.x, start.y);
}

void Path::lineTo (const float x, const float y)
{
    JUCE_CHECK_COORDS_ARE_VALID (x, y)

    if (data.isEmpty())
        startNewSubPath (0, 0);

    data.add (lineMarker, x, y);
    bounds.extend (x, y);
}

void Path::lineTo (Point<float> end)
{
    lineTo (end.x, end.y);
}

void Path::quadraticTo (const float x1, const float y1,
                        const float x2, const float y2)
{
    JUCE_CHECK_COORDS_ARE_VALID (x1, y1)
    JUCE_CHECK_COORDS_ARE_VALID (x2, y2)

    if (data.isEmpty())
        startNewSubPath (0, 0);

    data.add (quadMarker, x1, y1, x2, y2);
    bounds.extend (x1, y1, x2, y2);
}

void Path::quadraticTo (Point<float> controlPoint, Point<float> endPoint)
{
    quadraticTo (controlPoint.x, controlPoint.y,
                 endPoint.x, endPoint.y);
}

void Path::cubicTo (const float x1, const float y1,
                    const float x2, const float y2,
                    const float x3, const float y3)
{
    JUCE_CHECK_COORDS_ARE_VALID (x1, y1)
    JUCE_CHECK_COORDS_ARE_VALID (x2, y2)
    JUCE_CHECK_COORDS_ARE_VALID (x3, y3)

    if (data.isEmpty())
        startNewSubPath (0, 0);

    data.add (cubicMarker, x1, y1, x2, y2, x3, y3);
    bounds.extend (x1, y1, x2, y2, x3, y3);
}

void Path::cubicTo (Point<float> controlPoint1,
                    Point<float> controlPoint2,
                    Point<float> endPoint)
{
    cubicTo (controlPoint1.x, controlPoint1.y,
             controlPoint2.x, controlPoint2.y,
             endPoint.x, endPoint.y);
}

void Path::closeSubPath()
{
    if (! (data.isEmpty() || isMarker (data.getLast(), closeSubPathMarker)))
        data.add (closeSubPathMarker);
}

Point<float> Path::getCurrentPosition() const
{
    if (data.isEmpty())
        return {};

    auto* i = data.end() - 1;

    if (isMarker (*i, closeSubPathMarker))
    {
        while (i != data.begin())
        {
            if (isMarker (*--i, moveMarker))
            {
                i += 2;
                break;
            }
        }
    }

    if (i != data.begin())
        return { *(i - 1), *i };

    return {};
}

void Path::addRectangle (float x, float y, float w, float h)
{
    auto x1 = x, y1 = y, x2 = x + w, y2 = y + h;

    if (w < 0) std::swap (x1, x2);
    if (h < 0) std::swap (y1, y2);

    if (data.isEmpty())
    {
        bounds.pathXMin = x1;
        bounds.pathXMax = x2;
        bounds.pathYMin = y1;
        bounds.pathYMax = y2;
    }
    else
    {
        bounds.pathXMin = jmin (bounds.pathXMin, x1);
        bounds.pathXMax = jmax (bounds.pathXMax, x2);
        bounds.pathYMin = jmin (bounds.pathYMin, y1);
        bounds.pathYMax = jmax (bounds.pathYMax, y2);
    }

    data.add (moveMarker, x1, y2,
              lineMarker, x1, y1,
              lineMarker, x2, y1,
              lineMarker, x2, y2,
              closeSubPathMarker);
}

void Path::addRoundedRectangle (float x, float y, float w, float h, float csx, float csy)
{
    addRoundedRectangle (x, y, w, h, csx, csy, true, true, true, true);
}

void Path::addRoundedRectangle (const float x, const float y, const float w, const float h,
                                float csx, float csy,
                                const bool curveTopLeft, const bool curveTopRight,
                                const bool curveBottomLeft, const bool curveBottomRight)
{
    csx = jmin (csx, w * 0.5f);
    csy = jmin (csy, h * 0.5f);
    auto cs45x = csx * 0.45f;
    auto cs45y = csy * 0.45f;
    auto x2 = x + w;
    auto y2 = y + h;

    if (curveTopLeft)
    {
        startNewSubPath (x, y + csy);
        cubicTo (x, y + cs45y, x + cs45x, y, x + csx, y);
    }
    else
    {
        startNewSubPath (x, y);
    }

    if (curveTopRight)
    {
        lineTo (x2 - csx, y);
        cubicTo (x2 - cs45x, y, x2, y + cs45y, x2, y + csy);
    }
    else
    {
        lineTo (x2, y);
    }

    if (curveBottomRight)
    {
        lineTo (x2, y2 - csy);
        cubicTo (x2, y2 - cs45y, x2 - cs45x, y2, x2 - csx, y2);
    }
    else
    {
        lineTo (x2, y2);
    }

    if (curveBottomLeft)
    {
        lineTo (x + csx, y2);
        cubicTo (x + cs45x, y2, x, y2 - cs45y, x, y2 - csy);
    }
    else
    {
        lineTo (x, y2);
    }

    closeSubPath();
}

void Path::addRoundedRectangle (float x, float y, float w, float h, float cs)
{
    addRoundedRectangle (x, y, w, h, cs, cs);
}

void Path::addTriangle (float x1, float y1,
                        float x2, float y2,
                        float x3, float y3)
{
    addTriangle ({ x1, y1 },
                 { x2, y2 },
                 { x3, y3 });
}

void Path::addTriangle (Point<float> p1, Point<float> p2, Point<float> p3)
{
    startNewSubPath (p1);
    lineTo (p2);
    lineTo (p3);
    closeSubPath();
}

void Path::addQuadrilateral (float x1, float y1,
                             float x2, float y2,
                             float x3, float y3,
                             float x4, float y4)
{
    startNewSubPath (x1, y1);
    lineTo (x2, y2);
    lineTo (x3, y3);
    lineTo (x4, y4);
    closeSubPath();
}

void Path::addEllipse (float x, float y, float w, float h)
{
    addEllipse ({ x, y, w, h });
}

void Path::addEllipse (Rectangle<float> area)
{
    auto hw = area.getWidth() * 0.5f;
    auto hw55 = hw * 0.55f;
    auto hh = area.getHeight() * 0.5f;
    auto hh55 = hh * 0.55f;
    auto cx = area.getX() + hw;
    auto cy = area.getY() + hh;

    startNewSubPath (cx, cy - hh);
    cubicTo (cx + hw55, cy - hh, cx + hw, cy - hh55, cx + hw, cy);
    cubicTo (cx + hw, cy + hh55, cx + hw55, cy + hh, cx, cy + hh);
    cubicTo (cx - hw55, cy + hh, cx - hw, cy + hh55, cx - hw, cy);
    cubicTo (cx - hw, cy - hh55, cx - hw55, cy - hh, cx, cy - hh);
    closeSubPath();
}

void Path::addArc (float x, float y, float w, float h,
                   float fromRadians, float toRadians,
                   bool startAsNewSubPath)
{
    auto radiusX = w / 2.0f;
    auto radiusY = h / 2.0f;

    addCentredArc (x + radiusX,
                   y + radiusY,
                   radiusX, radiusY,
                   0.0f,
                   fromRadians, toRadians,
                   startAsNewSubPath);
}

void Path::addCentredArc (float centreX, float centreY,
                          float radiusX, float radiusY,
                          float rotationOfEllipse,
                          float fromRadians, float toRadians,
                          bool startAsNewSubPath)
{
    if (radiusX > 0.0f && radiusY > 0.0f)
    {
        Point<float> centre (centreX, centreY);
        auto rotation = AffineTransform::rotation (rotationOfEllipse, centreX, centreY);
        auto angle = fromRadians;

        if (startAsNewSubPath)
            startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));

        if (fromRadians < toRadians)
        {
            if (startAsNewSubPath)
                angle += PathHelpers::ellipseAngularIncrement;

            while (angle < toRadians)
            {
                lineTo (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));
                angle += PathHelpers::ellipseAngularIncrement;
            }
        }
        else
        {
            if (startAsNewSubPath)
                angle -= PathHelpers::ellipseAngularIncrement;

            while (angle > toRadians)
            {
                lineTo (centre.getPointOnCircumference (radiusX, radiusY, angle).transformedBy (rotation));
                angle -= PathHelpers::ellipseAngularIncrement;
            }
        }

        lineTo (centre.getPointOnCircumference (radiusX, radiusY, toRadians).transformedBy (rotation));
    }
}

void Path::addPieSegment (float x, float y, float width, float height,
                          float fromRadians, float toRadians,
                          float innerCircleProportionalSize)
{
    auto radiusX = width * 0.5f;
    auto radiusY = height * 0.5f;
    Point<float> centre (x + radiusX, y + radiusY);

    startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, fromRadians));
    addArc (x, y, width, height, fromRadians, toRadians);

    if (std::abs (fromRadians - toRadians) > MathConstants<float>::pi * 1.999f)
    {
        closeSubPath();

        if (innerCircleProportionalSize > 0)
        {
            radiusX *= innerCircleProportionalSize;
            radiusY *= innerCircleProportionalSize;

            startNewSubPath (centre.getPointOnCircumference (radiusX, radiusY, toRadians));
            addArc (centre.x - radiusX, centre.y - radiusY, radiusX * 2.0f, radiusY * 2.0f, toRadians, fromRadians);
        }
    }
    else
    {
        if (innerCircleProportionalSize > 0)
        {
            radiusX *= innerCircleProportionalSize;
            radiusY *= innerCircleProportionalSize;

            addArc (centre.x - radiusX, centre.y - radiusY, radiusX * 2.0f, radiusY * 2.0f, toRadians, fromRadians);
        }
        else
        {
            lineTo (centre);
        }
    }

    closeSubPath();
}

void Path::addPieSegment (Rectangle<float> segmentBounds,
                          float fromRadians, float toRadians,
                          float innerCircleProportionalSize)
{
    addPieSegment (segmentBounds.getX(),
                   segmentBounds.getY(),
                   segmentBounds.getWidth(),
                   segmentBounds.getHeight(),
                   fromRadians,
                   toRadians,
                   innerCircleProportionalSize);
}

//==============================================================================
void Path::addLineSegment (Line<float> line, float lineThickness)
{
    auto reversed = line.reversed();
    lineThickness *= 0.5f;

    startNewSubPath (line.getPointAlongLine (0, lineThickness));
    lineTo (line.getPointAlongLine (0, -lineThickness));
    lineTo (reversed.getPointAlongLine (0, lineThickness));
    lineTo (reversed.getPointAlongLine (0, -lineThickness));
    closeSubPath();
}

void Path::addArrow (Line<float> line, float lineThickness,
                     float arrowheadWidth, float arrowheadLength)
{
    auto reversed = line.reversed();
    lineThickness *= 0.5f;
    arrowheadWidth *= 0.5f;
    arrowheadLength = jmin (arrowheadLength, 0.8f * line.getLength());

    startNewSubPath (line.getPointAlongLine (0, lineThickness));
    lineTo (line.getPointAlongLine (0, -lineThickness));
    lineTo (reversed.getPointAlongLine (arrowheadLength, lineThickness));
    lineTo (reversed.getPointAlongLine (arrowheadLength, arrowheadWidth));
    lineTo (line.getEnd());
    lineTo (reversed.getPointAlongLine (arrowheadLength, -arrowheadWidth));
    lineTo (reversed.getPointAlongLine (arrowheadLength, -lineThickness));
    closeSubPath();
}

void Path::addPolygon (Point<float> centre, int numberOfSides,
                       float radius, float startAngle)
{
    jassert (numberOfSides > 1); // this would be silly.

    if (numberOfSides > 1)
    {
        auto angleBetweenPoints = MathConstants<float>::twoPi / (float) numberOfSides;

        for (int i = 0; i < numberOfSides; ++i)
        {
            auto angle = startAngle + (float) i * angleBetweenPoints;
            auto p = centre.getPointOnCircumference (radius, angle);

            if (i == 0)
                startNewSubPath (p);
            else
                lineTo (p);
        }

        closeSubPath();
    }
}

void Path::addStar (Point<float> centre, int numberOfPoints, float innerRadius,
                    float outerRadius, float startAngle)
{
    jassert (numberOfPoints > 1); // this would be silly.

    if (numberOfPoints > 1)
    {
        auto angleBetweenPoints = MathConstants<float>::twoPi / (float) numberOfPoints;

        for (int i = 0; i < numberOfPoints; ++i)
        {
            auto angle = startAngle + (float) i * angleBetweenPoints;
            auto p = centre.getPointOnCircumference (outerRadius, angle);

            if (i == 0)
                startNewSubPath (p);
            else
                lineTo (p);

            lineTo (centre.getPointOnCircumference (innerRadius, angle + angleBetweenPoints * 0.5f));
        }

        closeSubPath();
    }
}

void Path::addBubble (Rectangle<float> bodyArea,
                      Rectangle<float> maximumArea,
                      Point<float> arrowTip,
                      float cornerSize,
                      float arrowBaseWidth)
{
    auto halfW = bodyArea.getWidth() / 2.0f;
    auto halfH = bodyArea.getHeight() / 2.0f;
    auto cornerSizeW = jmin (cornerSize, halfW);
    auto cornerSizeH = jmin (cornerSize, halfH);
    auto cornerSizeW2 = 2.0f * cornerSizeW;
    auto cornerSizeH2 = 2.0f * cornerSizeH;

    startNewSubPath (bodyArea.getX() + cornerSizeW, bodyArea.getY());

    auto targetLimit = bodyArea.reduced (jmin (halfW - 1.0f, cornerSizeW + arrowBaseWidth),
                                         jmin (halfH - 1.0f, cornerSizeH + arrowBaseWidth));

    if (Rectangle<float> (targetLimit.getX(), maximumArea.getY(),
                          targetLimit.getWidth(), bodyArea.getY() - maximumArea.getY()).contains (arrowTip))
    {
        lineTo (arrowTip.x - arrowBaseWidth, bodyArea.getY());
        lineTo (arrowTip.x, arrowTip.y);
        lineTo (arrowTip.x + arrowBaseWidth, bodyArea.getY());
    }

    lineTo (bodyArea.getRight() - cornerSizeW, bodyArea.getY());
    addArc (bodyArea.getRight() - cornerSizeW2, bodyArea.getY(), cornerSizeW2, cornerSizeH2, 0, MathConstants<float>::halfPi);

    if (Rectangle<float> (bodyArea.getRight(), targetLimit.getY(),
                          maximumArea.getRight() - bodyArea.getRight(), targetLimit.getHeight()).contains (arrowTip))
    {
        lineTo (bodyArea.getRight(), arrowTip.y - arrowBaseWidth);
        lineTo (arrowTip.x, arrowTip.y);
        lineTo (bodyArea.getRight(), arrowTip.y + arrowBaseWidth);
    }

    lineTo (bodyArea.getRight(), bodyArea.getBottom() - cornerSizeH);
    addArc (bodyArea.getRight() - cornerSizeW2, bodyArea.getBottom() - cornerSizeH2, cornerSizeW2, cornerSizeH2, MathConstants<float>::halfPi, MathConstants<float>::pi);

    if (Rectangle<float> (targetLimit.getX(), bodyArea.getBottom(),
                          targetLimit.getWidth(), maximumArea.getBottom() - bodyArea.getBottom()).contains (arrowTip))
    {
        lineTo (arrowTip.x + arrowBaseWidth, bodyArea.getBottom());
        lineTo (arrowTip.x, arrowTip.y);
        lineTo (arrowTip.x - arrowBaseWidth, bodyArea.getBottom());
    }

    lineTo (bodyArea.getX() + cornerSizeW, bodyArea.getBottom());
    addArc (bodyArea.getX(), bodyArea.getBottom() - cornerSizeH2, cornerSizeW2, cornerSizeH2, MathConstants<float>::pi, MathConstants<float>::pi * 1.5f);

    if (Rectangle<float> (maximumArea.getX(), targetLimit.getY(),
                          bodyArea.getX() - maximumArea.getX(), targetLimit.getHeight()).contains (arrowTip))
    {
        lineTo (bodyArea.getX(), arrowTip.y + arrowBaseWidth);
        lineTo (arrowTip.x, arrowTip.y);
        lineTo (bodyArea.getX(), arrowTip.y - arrowBaseWidth);
    }

    lineTo (bodyArea.getX(), bodyArea.getY() + cornerSizeH);
    addArc (bodyArea.getX(), bodyArea.getY(), cornerSizeW2, cornerSizeH2, MathConstants<float>::pi * 1.5f, MathConstants<float>::twoPi - 0.05f);

    closeSubPath();
}

void Path::addPath (const Path& other)
{
    const auto* d = other.data.begin();

    for (int i = 0; i < other.data.size();)
    {
        auto type = d[i++];

        if (isMarker (type, moveMarker))
        {
            startNewSubPath (d[i], d[i + 1]);
            i += 2;
        }
        else if (isMarker (type, lineMarker))
        {
            lineTo (d[i], d[i + 1]);
            i += 2;
        }
        else if (isMarker (type, quadMarker))
        {
            quadraticTo (d[i], d[i + 1], d[i + 2], d[i + 3]);
            i += 4;
        }
        else if (isMarker (type, cubicMarker))
        {
            cubicTo (d[i], d[i + 1], d[i + 2], d[i + 3], d[i + 4], d[i + 5]);
            i += 6;
        }
        else if (isMarker (type, closeSubPathMarker))
        {
            closeSubPath();
        }
        else
        {
            // something's gone wrong with the element list!
            jassertfalse;
        }
    }
}

void Path::addPath (const Path& other,
                    const AffineTransform& transformToApply)
{
    const auto* d = other.data.begin();

    for (int i = 0; i < other.data.size();)
    {
        auto type = d[i++];

        if (isMarker (type, closeSubPathMarker))
        {
            closeSubPath();
        }
        else
        {
            auto x = d[i++];
            auto y = d[i++];
            transformToApply.transformPoint (x, y);

            if (isMarker (type, moveMarker))
            {
                startNewSubPath (x, y);
            }
            else if (isMarker (type, lineMarker))
            {
                lineTo (x, y);
            }
            else if (isMarker (type, quadMarker))
            {
                auto x2 = d[i++];
                auto y2 = d[i++];
                transformToApply.transformPoint (x2, y2);

                quadraticTo (x, y, x2, y2);
            }
            else if (isMarker (type, cubicMarker))
            {
                auto x2 = d[i++];
                auto y2 = d[i++];
                auto x3 = d[i++];
                auto y3 = d[i++];
                transformToApply.transformPoints (x2, y2, x3, y3);

                cubicTo (x, y, x2, y2, x3, y3);
            }
            else
            {
                // something's gone wrong with the element list!
                jassertfalse;
            }
        }
    }
}

//==============================================================================
void Path::applyTransform (const AffineTransform& transform) noexcept
{
    bounds.reset();
    bool firstPoint = true;
    float* d = data.begin();
    auto* end = data.end();

    while (d < end)
    {
        auto type = *d++;

        if (isMarker (type, moveMarker))
        {
            transform.transformPoint (d[0], d[1]);
            JUCE_CHECK_COORDS_ARE_VALID (d[0], d[1])

            if (firstPoint)
            {
                firstPoint = false;
                bounds.reset (d[0], d[1]);
            }
            else
            {
                bounds.extend (d[0], d[1]);
            }

            d += 2;
        }
        else if (isMarker (type, lineMarker))
        {
            transform.transformPoint (d[0], d[1]);
            JUCE_CHECK_COORDS_ARE_VALID (d[0], d[1])
            bounds.extend (d[0], d[1]);
            d += 2;
        }
        else if (isMarker (type, quadMarker))
        {
            transform.transformPoints (d[0], d[1], d[2], d[3]);
            JUCE_CHECK_COORDS_ARE_VALID (d[0], d[1])
            JUCE_CHECK_COORDS_ARE_VALID (d[2], d[3])
            bounds.extend (d[0], d[1], d[2], d[3]);
            d += 4;
        }
        else if (isMarker (type, cubicMarker))
        {
            transform.transformPoints (d[0], d[1], d[2], d[3], d[4], d[5]);
            JUCE_CHECK_COORDS_ARE_VALID (d[0], d[1])
            JUCE_CHECK_COORDS_ARE_VALID (d[2], d[3])
            JUCE_CHECK_COORDS_ARE_VALID (d[4], d[5])
            bounds.extend (d[0], d[1], d[2], d[3], d[4], d[5]);
            d += 6;
        }
    }
}


//==============================================================================
AffineTransform Path::getTransformToScaleToFit (Rectangle<float> area, bool preserveProportions,
                                                Justification justification) const
{
    return getTransformToScaleToFit (area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                                     preserveProportions, justification);
}

AffineTransform Path::getTransformToScaleToFit (float x, float y, float w, float h,
                                                bool preserveProportions,
                                                Justification justification) const
{
    auto boundsRect = getBounds();

    if (preserveProportions)
    {
        if (w <= 0 || h <= 0 || boundsRect.isEmpty())
            return AffineTransform();

        float newW, newH;
        auto srcRatio = boundsRect.getHeight() / boundsRect.getWidth();

        if (srcRatio > h / w)
        {
            newW = h / srcRatio;
            newH = h;
        }
        else
        {
            newW = w;
            newH = w * srcRatio;
        }

        auto newXCentre = x;
        auto newYCentre = y;

        if (justification.testFlags (Justification::left))          newXCentre += newW * 0.5f;
        else if (justification.testFlags (Justification::right))    newXCentre += w - newW * 0.5f;
        else                                                        newXCentre += w * 0.5f;

        if (justification.testFlags (Justification::top))           newYCentre += newH * 0.5f;
        else if (justification.testFlags (Justification::bottom))   newYCentre += h - newH * 0.5f;
        else                                                        newYCentre += h * 0.5f;

        return AffineTransform::translation (boundsRect.getWidth()  * -0.5f - boundsRect.getX(),
                                             boundsRect.getHeight() * -0.5f - boundsRect.getY())
                    .scaled (newW / boundsRect.getWidth(),
                             newH / boundsRect.getHeight())
                    .translated (newXCentre, newYCentre);
    }
    else
    {
        return AffineTransform::translation (-boundsRect.getX(), -boundsRect.getY())
                    .scaled (w / boundsRect.getWidth(),
                             h / boundsRect.getHeight())
                    .translated (x, y);
    }
}

//==============================================================================
bool Path::contains (float x, float y, float tolerance) const
{
    if (x <= bounds.pathXMin || x >= bounds.pathXMax
         || y <= bounds.pathYMin || y >= bounds.pathYMax)
        return false;

    PathFlatteningIterator i (*this, AffineTransform(), tolerance);

    int positiveCrossings = 0;
    int negativeCrossings = 0;

    while (i.next())
    {
        if ((i.y1 <= y && i.y2 > y) || (i.y2 <= y && i.y1 > y))
        {
            auto intersectX = i.x1 + (i.x2 - i.x1) * (y - i.y1) / (i.y2 - i.y1);

            if (intersectX <= x)
            {
                if (i.y1 < i.y2)
                    ++positiveCrossings;
                else
                    ++negativeCrossings;
            }
        }
    }

    return useNonZeroWinding ? (negativeCrossings != positiveCrossings)
                             : ((negativeCrossings + positiveCrossings) & 1) != 0;
}

bool Path::contains (Point<float> point, float tolerance) const
{
    return contains (point.x, point.y, tolerance);
}

bool Path::intersectsLine (Line<float> line, float tolerance)
{
    PathFlatteningIterator i (*this, AffineTransform(), tolerance);
    Point<float> intersection;

    while (i.next())
        if (line.intersects (Line<float> (i.x1, i.y1, i.x2, i.y2), intersection))
            return true;

    return false;
}

Line<float> Path::getClippedLine (Line<float> line, bool keepSectionOutsidePath) const
{
    Line<float> result (line);
    const bool startInside = contains (line.getStart());
    const bool endInside   = contains (line.getEnd());

    if (startInside == endInside)
    {
        if (keepSectionOutsidePath == startInside)
            result = Line<float>();
    }
    else
    {
        PathFlatteningIterator i (*this, AffineTransform());
        Point<float> intersection;

        while (i.next())
        {
            if (line.intersects ({ i.x1, i.y1, i.x2, i.y2 }, intersection))
            {
                if ((startInside && keepSectionOutsidePath) || (endInside && ! keepSectionOutsidePath))
                    result.setStart (intersection);
                else
                    result.setEnd (intersection);
            }
        }
    }

    return result;
}

float Path::getLength (const AffineTransform& transform, float tolerance) const
{
    float length = 0;
    PathFlatteningIterator i (*this, transform, tolerance);

    while (i.next())
        length += Line<float> (i.x1, i.y1, i.x2, i.y2).getLength();

    return length;
}

Point<float> Path::getPointAlongPath (float distanceFromStart,
                                      const AffineTransform& transform,
                                      float tolerance) const
{
    PathFlatteningIterator i (*this, transform, tolerance);

    while (i.next())
    {
        const Line<float> line (i.x1, i.y1, i.x2, i.y2);
        auto lineLength = line.getLength();

        if (distanceFromStart <= lineLength)
            return line.getPointAlongLine (distanceFromStart);

        distanceFromStart -= lineLength;
    }

    return { i.x2, i.y2 };
}

float Path::getNearestPoint (Point<float> targetPoint, Point<float>& pointOnPath,
                             const AffineTransform& transform,
                             float tolerance) const
{
    PathFlatteningIterator i (*this, transform, tolerance);
    float bestPosition = 0, bestDistance = std::numeric_limits<float>::max();
    float length = 0;
    Point<float> pointOnLine;

    while (i.next())
    {
        const Line<float> line (i.x1, i.y1, i.x2, i.y2);
        auto distance = line.getDistanceFromPoint (targetPoint, pointOnLine);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestPosition = length + pointOnLine.getDistanceFrom (line.getStart());
            pointOnPath = pointOnLine;
        }

        length += line.getLength();
    }

    return bestPosition;
}

//==============================================================================
Path Path::createPathWithRoundedCorners (const float cornerRadius) const
{
    if (cornerRadius <= 0.01f)
        return *this;

    Path p;
    int n = 0, indexOfPathStart = 0, indexOfPathStartThis = 0;
    auto* elements = data.begin();
    bool lastWasLine = false, firstWasLine = false;

    while (n < data.size())
    {
        auto type = elements[n++];

        if (isMarker (type, moveMarker))
        {
            indexOfPathStart = p.data.size();
            indexOfPathStartThis = n - 1;
            auto x = elements[n++];
            auto y = elements[n++];
            p.startNewSubPath (x, y);
            lastWasLine = false;
            firstWasLine = (isMarker (elements[n], lineMarker));
        }
        else if (isMarker (type, lineMarker) || isMarker (type, closeSubPathMarker))
        {
            float startX = 0, startY = 0, joinX = 0, joinY = 0, endX, endY;

            if (isMarker (type, lineMarker))
            {
                endX = elements[n++];
                endY = elements[n++];

                if (n > 8)
                {
                    startX = elements[n - 8];
                    startY = elements[n - 7];
                    joinX  = elements[n - 5];
                    joinY  = elements[n - 4];
                }
            }
            else
            {
                endX = elements[indexOfPathStartThis + 1];
                endY = elements[indexOfPathStartThis + 2];

                if (n > 6)
                {
                    startX = elements[n - 6];
                    startY = elements[n - 5];
                    joinX  = elements[n - 3];
                    joinY  = elements[n - 2];
                }
            }

            if (lastWasLine)
            {
                auto len1 = PathHelpers::lengthOf (startX, startY, joinX, joinY);

                if (len1 > 0)
                {
                    auto propNeeded = jmin (0.5, cornerRadius / len1);

                    *(p.data.end() - 2) = (float) (joinX - (joinX - startX) * propNeeded);
                    *(p.data.end() - 1) = (float) (joinY - (joinY - startY) * propNeeded);
                }

                auto len2 = PathHelpers::lengthOf (endX, endY, joinX, joinY);

                if (len2 > 0)
                {
                    auto propNeeded = jmin (0.5, cornerRadius / len2);

                    p.quadraticTo (joinX, joinY,
                                   (float) (joinX + (endX - joinX) * propNeeded),
                                   (float) (joinY + (endY - joinY) * propNeeded));
                }

                p.lineTo (endX, endY);
            }
            else if (isMarker (type, lineMarker))
            {
                p.lineTo (endX, endY);
                lastWasLine = true;
            }

            if (isMarker (type, closeSubPathMarker))
            {
                if (firstWasLine)
                {
                    startX = elements[n - 3];
                    startY = elements[n - 2];
                    joinX = endX;
                    joinY = endY;
                    endX = elements[indexOfPathStartThis + 4];
                    endY = elements[indexOfPathStartThis + 5];

                    auto len1 = PathHelpers::lengthOf (startX, startY, joinX, joinY);

                    if (len1 > 0)
                    {
                        auto propNeeded = jmin (0.5, cornerRadius / len1);

                        *(p.data.end() - 2) = (float) (joinX - (joinX - startX) * propNeeded);
                        *(p.data.end() - 1) = (float) (joinY - (joinY - startY) * propNeeded);
                    }

                    auto len2 = PathHelpers::lengthOf (endX, endY, joinX, joinY);

                    if (len2 > 0)
                    {
                        auto propNeeded = jmin (0.5, cornerRadius / len2);

                        endX = (float) (joinX + (endX - joinX) * propNeeded);
                        endY = (float) (joinY + (endY - joinY) * propNeeded);

                        p.quadraticTo (joinX, joinY, endX, endY);

                        p.data.begin()[indexOfPathStart + 1] = endX;
                        p.data.begin()[indexOfPathStart + 2] = endY;
                    }
                }

                p.closeSubPath();
            }
        }
        else if (isMarker (type, quadMarker))
        {
            lastWasLine = false;
            auto x1 = elements[n++];
            auto y1 = elements[n++];
            auto x2 = elements[n++];
            auto y2 = elements[n++];
            p.quadraticTo (x1, y1, x2, y2);
        }
        else if (isMarker (type, cubicMarker))
        {
            lastWasLine = false;
            auto x1 = elements[n++];
            auto y1 = elements[n++];
            auto x2 = elements[n++];
            auto y2 = elements[n++];
            auto x3 = elements[n++];
            auto y3 = elements[n++];
            p.cubicTo (x1, y1, x2, y2, x3, y3);
        }
    }

    return p;
}

//==============================================================================
void Path::loadPathFromStream (InputStream& source)
{
    while (! source.isExhausted())
    {
        switch (source.readByte())
        {
        case 'm':
        {
            auto x = source.readFloat();
            auto y = source.readFloat();
            startNewSubPath (x, y);
            break;
        }

        case 'l':
        {
            auto x = source.readFloat();
            auto y = source.readFloat();
            lineTo (x, y);
            break;
        }

        case 'q':
        {
            auto x1 = source.readFloat();
            auto y1 = source.readFloat();
            auto x2 = source.readFloat();
            auto y2 = source.readFloat();
            quadraticTo (x1, y1, x2, y2);
            break;
        }

        case 'b':
        {
            auto x1 = source.readFloat();
            auto y1 = source.readFloat();
            auto x2 = source.readFloat();
            auto y2 = source.readFloat();
            auto x3 = source.readFloat();
            auto y3 = source.readFloat();
            cubicTo (x1, y1, x2, y2, x3, y3);
            break;
        }

        case 'c':
            closeSubPath();
            break;

        case 'n':
            useNonZeroWinding = true;
            break;

        case 'z':
            useNonZeroWinding = false;
            break;

        case 'e':
            return; // end of path marker

        default:
            jassertfalse; // illegal char in the stream
            break;
        }
    }
}

void Path::loadPathFromData (const void* const pathData, const size_t numberOfBytes)
{
    MemoryInputStream in (pathData, numberOfBytes, false);
    loadPathFromStream (in);
}

void Path::writePathToStream (OutputStream& dest) const
{
    dest.writeByte (useNonZeroWinding ? 'n' : 'z');

    for (auto* i = data.begin(); i != data.end();)
    {
        auto type = *i++;

        if (isMarker (type, moveMarker))
        {
            dest.writeByte ('m');
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
        }
        else if (isMarker (type, lineMarker))
        {
            dest.writeByte ('l');
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
        }
        else if (isMarker (type, quadMarker))
        {
            dest.writeByte ('q');
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
        }
        else if (isMarker (type, cubicMarker))
        {
            dest.writeByte ('b');
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
            dest.writeFloat (*i++);
        }
        else if (isMarker (type, closeSubPathMarker))
        {
            dest.writeByte ('c');
        }
    }

    dest.writeByte ('e'); // marks the end-of-path
}

String Path::toString() const
{
    MemoryOutputStream s (2048);
    if (! useNonZeroWinding)
        s << 'a';

    float lastMarker = 0.0f;

    for (int i = 0; i < data.size();)
    {
        auto type = data.begin()[i++];
        char markerChar = 0;
        int numCoords = 0;

        if (isMarker (type, moveMarker))
        {
            markerChar = 'm';
            numCoords = 2;
        }
        else if (isMarker (type, lineMarker))
        {
            markerChar = 'l';
            numCoords = 2;
        }
        else if (isMarker (type, quadMarker))
        {
            markerChar = 'q';
            numCoords = 4;
        }
        else if (isMarker (type, cubicMarker))
        {
            markerChar = 'c';
            numCoords = 6;
        }
        else
        {
            jassert (isMarker (type, closeSubPathMarker));
            markerChar = 'z';
        }

        if (! isMarker (type, lastMarker))
        {
            if (s.getDataSize() != 0)
                s << ' ';

            s << markerChar;
            lastMarker = type;
        }

        while (--numCoords >= 0 && i < data.size())
        {
            String coord (data.begin()[i++], 3);

            while (coord.endsWithChar ('0') && coord != "0")
                coord = coord.dropLastCharacters (1);

            if (coord.endsWithChar ('.'))
                coord = coord.dropLastCharacters (1);

            if (s.getDataSize() != 0)
                s << ' ';

            s << coord;
        }
    }

    return s.toUTF8();
}

void Path::restoreFromString (StringRef stringVersion)
{
    clear();
    setUsingNonZeroWinding (true);

    auto t = stringVersion.text;
    juce_wchar marker = 'm';
    int numValues = 2;
    float values[6];

    for (;;)
    {
        auto token = PathHelpers::nextToken (t);
        auto firstChar = token[0];
        int startNum = 0;

        if (firstChar == 0)
            break;

        if (firstChar == 'm' || firstChar == 'l')
        {
            marker = firstChar;
            numValues = 2;
        }
        else if (firstChar == 'q')
        {
            marker = firstChar;
            numValues = 4;
        }
        else if (firstChar == 'c')
        {
            marker = firstChar;
            numValues = 6;
        }
        else if (firstChar == 'z')
        {
            marker = firstChar;
            numValues = 0;
        }
        else if (firstChar == 'a')
        {
            setUsingNonZeroWinding (false);
            continue;
        }
        else
        {
            ++startNum;
            values [0] = token.getFloatValue();
        }

        for (int i = startNum; i < numValues; ++i)
            values [i] = PathHelpers::nextToken (t).getFloatValue();

        switch (marker)
        {
            case 'm':   startNewSubPath (values[0], values[1]); break;
            case 'l':   lineTo (values[0], values[1]); break;
            case 'q':   quadraticTo (values[0], values[1], values[2], values[3]); break;
            case 'c':   cubicTo (values[0], values[1], values[2], values[3], values[4], values[5]); break;
            case 'z':   closeSubPath(); break;
            default:    jassertfalse; break; // illegal string format?
        }
    }
}

//==============================================================================
Path::Iterator::Iterator (const Path& p) noexcept
    : elementType (startNewSubPath), path (p), index (path.data.begin())
{
}

Path::Iterator::~Iterator() noexcept
{
}

bool Path::Iterator::next() noexcept
{
    if (index != path.data.end())
    {
        auto type = *index++;

        if (isMarker (type, moveMarker))
        {
            elementType = startNewSubPath;
            x1 = *index++;
            y1 = *index++;
        }
        else if (isMarker (type, lineMarker))
        {
            elementType = lineTo;
            x1 = *index++;
            y1 = *index++;
        }
        else if (isMarker (type, quadMarker))
        {
            elementType = quadraticTo;
            x1 = *index++;
            y1 = *index++;
            x2 = *index++;
            y2 = *index++;
        }
        else if (isMarker (type, cubicMarker))
        {
            elementType = cubicTo;
            x1 = *index++;
            y1 = *index++;
            x2 = *index++;
            y2 = *index++;
            x3 = *index++;
            y3 = *index++;
        }
        else if (isMarker (type, closeSubPathMarker))
        {
            elementType = closePath;
        }

        return true;
    }

    return false;
}

#undef JUCE_CHECK_COORDS_ARE_VALID

} // namespace juce
