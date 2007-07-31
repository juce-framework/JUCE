/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_PathIterator.h"
#include "juce_Line.h"
#include "../../../../juce_core/io/streams/juce_MemoryInputStream.h"

// tests that some co-ords aren't NaNs
#define CHECK_COORDS_ARE_VALID(x, y) \
    jassert (x == x && y == y);

//==============================================================================
static const float lineMarker           = 100001.0f;
static const float moveMarker           = 100002.0f;
static const float quadMarker           = 100003.0f;
static const float cubicMarker          = 100004.0f;
static const float closeSubPathMarker   = 100005.0f;

static const int defaultGranularity = 32;

//==============================================================================
Path::Path() throw()
    : ArrayAllocationBase <float> (defaultGranularity),
      numElements (0),
      pathXMin (0),
      pathXMax (0),
      pathYMin (0),
      pathYMax (0),
      useNonZeroWinding (true)
{
}

Path::~Path() throw()
{
}

Path::Path (const Path& other) throw()
    : ArrayAllocationBase <float> (defaultGranularity),
      numElements (other.numElements),
      pathXMin (other.pathXMin),
      pathXMax (other.pathXMax),
      pathYMin (other.pathYMin),
      pathYMax (other.pathYMax),
      useNonZeroWinding (other.useNonZeroWinding)
{
    if (numElements > 0)
    {
        setAllocatedSize (numElements);
        memcpy (elements, other.elements, numElements * sizeof (float));
    }
}

const Path& Path::operator= (const Path& other) throw()
{
    if (this != &other)
    {
        ensureAllocatedSize (other.numElements);

        numElements = other.numElements;
        pathXMin = other.pathXMin;
        pathXMax = other.pathXMax;
        pathYMin = other.pathYMin;
        pathYMax = other.pathYMax;
        useNonZeroWinding = other.useNonZeroWinding;

        if (numElements > 0)
            memcpy (elements, other.elements, numElements * sizeof (float));
    }

    return *this;
}

void Path::clear() throw()
{
    numElements = 0;
    pathXMin = 0;
    pathYMin = 0;
    pathYMax = 0;
    pathXMax = 0;
}

void Path::swapWithPath (Path& other)
{
    swapVariables <int> (this->numAllocated, other.numAllocated);
    swapVariables <float*> (this->elements, other.elements);
    swapVariables <int> (this->numElements, other.numElements);
    swapVariables <float> (this->pathXMin, other.pathXMin);
    swapVariables <float> (this->pathXMax, other.pathXMax);
    swapVariables <float> (this->pathYMin, other.pathYMin);
    swapVariables <float> (this->pathYMax, other.pathYMax);
    swapVariables <bool> (this->useNonZeroWinding, other.useNonZeroWinding);
}

//==============================================================================
void Path::setUsingNonZeroWinding (const bool isNonZero) throw()
{
    useNonZeroWinding = isNonZero;
}

void Path::scaleToFit (const float x, const float y, const float w, const float h,
                       const bool preserveProportions) throw()
{
    applyTransform (getTransformToScaleToFit (x, y, w, h, preserveProportions));
}

//==============================================================================
bool Path::isEmpty() const throw()
{
    int i = 0;

    while (i < numElements)
    {
        const float type = elements [i++];

        if (type == moveMarker)
        {
            i += 2;
        }
        else if (type == lineMarker
                 || type == quadMarker
                 || type == cubicMarker)
        {
            return false;
        }
    }

    return true;
}

void Path::getBounds (float& x, float& y,
                      float& w, float& h) const throw()
{
    x = pathXMin;
    y = pathYMin;
    w = pathXMax - pathXMin;
    h = pathYMax - pathYMin;
}


void Path::getBoundsTransformed (const AffineTransform& transform,
                                 float& x, float& y,
                                 float& w, float& h) const throw()
{
    float x1 = pathXMin;
    float y1 = pathYMin;
    transform.transformPoint (x1, y1);

    float x2 = pathXMax;
    float y2 = pathYMin;
    transform.transformPoint (x2, y2);

    float x3 = pathXMin;
    float y3 = pathYMax;
    transform.transformPoint (x3, y3);

    float x4 = pathXMax;
    float y4 = pathYMax;
    transform.transformPoint (x4, y4);

    x = jmin (x1, x2, x3, x4);
    y = jmin (y1, y2, y3, y4);
    w = jmax (x1, x2, x3, x4) - x;
    h = jmax (y1, y2, y3, y4) - y;
}

//==============================================================================
void Path::startNewSubPath (const float x,
                            const float y) throw()
{
    CHECK_COORDS_ARE_VALID (x, y);

    if (numElements == 0)
    {
        pathXMin = pathXMax = x;
        pathYMin = pathYMax = y;
    }
    else
    {
        pathXMin = jmin (pathXMin, x);
        pathXMax = jmax (pathXMax, x);
        pathYMin = jmin (pathYMin, y);
        pathYMax = jmax (pathYMax, y);
    }

    ensureAllocatedSize (numElements + 3);

    elements [numElements++] = moveMarker;
    elements [numElements++] = x;
    elements [numElements++] = y;
}

void Path::lineTo (const float x, const float y) throw()
{
    CHECK_COORDS_ARE_VALID (x, y);

    if (numElements == 0)
        startNewSubPath (0, 0);

    ensureAllocatedSize (numElements + 3);

    elements [numElements++] = lineMarker;
    elements [numElements++] = x;
    elements [numElements++] = y;

    pathXMin = jmin (pathXMin, x);
    pathXMax = jmax (pathXMax, x);
    pathYMin = jmin (pathYMin, y);
    pathYMax = jmax (pathYMax, y);
}

void Path::quadraticTo (const float x1, const float y1,
                        const float x2, const float y2) throw()
{
    CHECK_COORDS_ARE_VALID (x1, y1);
    CHECK_COORDS_ARE_VALID (x2, y2);

    if (numElements == 0)
        startNewSubPath (0, 0);

    ensureAllocatedSize (numElements + 5);

    elements [numElements++] = quadMarker;
    elements [numElements++] = x1;
    elements [numElements++] = y1;
    elements [numElements++] = x2;
    elements [numElements++] = y2;

    pathXMin = jmin (pathXMin, x1, x2);
    pathXMax = jmax (pathXMax, x1, x2);
    pathYMin = jmin (pathYMin, y1, y2);
    pathYMax = jmax (pathYMax, y1, y2);
}

void Path::cubicTo (const float x1, const float y1,
                    const float x2, const float y2,
                    const float x3, const float y3) throw()
{
    CHECK_COORDS_ARE_VALID (x1, y1);
    CHECK_COORDS_ARE_VALID (x2, y2);
    CHECK_COORDS_ARE_VALID (x3, y3);

    if (numElements == 0)
        startNewSubPath (0, 0);

    ensureAllocatedSize (numElements + 7);

    elements [numElements++] = cubicMarker;
    elements [numElements++] = x1;
    elements [numElements++] = y1;
    elements [numElements++] = x2;
    elements [numElements++] = y2;
    elements [numElements++] = x3;
    elements [numElements++] = y3;

    pathXMin = jmin (pathXMin, x1, x2, x3);
    pathXMax = jmax (pathXMax, x1, x2, x3);
    pathYMin = jmin (pathYMin, y1, y2, y3);
    pathYMax = jmax (pathYMax, y1, y2, y3);
}

void Path::closeSubPath() throw()
{
    if (numElements > 0
         && elements [numElements - 1] != closeSubPathMarker)
    {
        ensureAllocatedSize (numElements + 1);
        elements [numElements++] = closeSubPathMarker;
    }
}

const Point Path::getCurrentPosition() const
{
    int i = numElements - 1;

    if (i > 0 && elements[i] == closeSubPathMarker)
    {
        while (i >= 0)
        {
            if (elements[i] == moveMarker)
            {
                i += 2;
                break;
            }

            --i;
        }
    }

    if (i > 0)
        return Point (elements [i - 1], elements [i]);

    return Point (0.0f, 0.0f);
}

void Path::addRectangle (const float x, const float y,
                         const float w, const float h) throw()
{
    startNewSubPath (x, y + h);
    lineTo (x, y);
    lineTo (x + w, y);
    lineTo (x + w, y + h);
    closeSubPath();
}

void Path::addRoundedRectangle (const float x, const float y,
                                const float w, const float h,
                                float csx,
                                float csy) throw()
{
    csx = jmin (csx, w * 0.5f);
    csy = jmin (csy, h * 0.5f);
    const float cs45x = csx * 0.45f;
    const float cs45y = csy * 0.45f;
    const float x2 = x + w;
    const float y2 = y + h;

    startNewSubPath (x + csx, y);
    lineTo (x2 - csx, y);
    cubicTo (x2 - cs45x, y, x2, y + cs45y, x2, y + csy);
    lineTo (x2, y2 - csy);
    cubicTo (x2, y2 - cs45y, x2 - cs45x, y2, x2 - csx, y2);
    lineTo (x + csx, y2);
    cubicTo (x + cs45x, y2, x, y2 - cs45y, x, y2 - csy);
    lineTo (x, y + csy);
    cubicTo (x, y + cs45y, x + cs45x, y, x + csx, y);
    closeSubPath();
}

void Path::addRoundedRectangle (const float x, const float y,
                                const float w, const float h,
                                float cs) throw()
{
    addRoundedRectangle (x, y, w, h, cs, cs);
}

void Path::addTriangle (const float x1, const float y1,
                        const float x2, const float y2,
                        const float x3, const float y3) throw()
{
    startNewSubPath (x1, y1);
    lineTo (x2, y2);
    lineTo (x3, y3);
    closeSubPath();
}

void Path::addQuadrilateral (const float x1, const float y1,
                             const float x2, const float y2,
                             const float x3, const float y3,
                             const float x4, const float y4) throw()
{
    startNewSubPath (x1, y1);
    lineTo (x2, y2);
    lineTo (x3, y3);
    lineTo (x4, y4);
    closeSubPath();
}

void Path::addEllipse (const float x, const float y,
                       const float w, const float h) throw()
{
    const float hw = w * 0.5f;
    const float hw55 = hw * 0.55f;
    const float hh = h * 0.5f;
    const float hh45 = hh * 0.55f;
    const float cx = x + hw;
    const float cy = y + hh;

    startNewSubPath (cx, cy - hh);
    cubicTo (cx + hw55, cy - hh, cx + hw, cy - hh45, cx + hw, cy);
    cubicTo (cx + hw, cy + hh45, cx + hw55, cy + hh, cx, cy + hh);
    cubicTo (cx - hw55, cy + hh, cx - hw, cy + hh45, cx - hw, cy);
    cubicTo (cx - hw, cy - hh45, cx - hw55, cy - hh, cx, cy - hh);
    closeSubPath();
}

void Path::addArc (const float x, const float y,
                   const float w, const float h,
                   const float fromRadians,
                   const float toRadians,
                   const bool startAsNewSubPath) throw()
{
    const float radiusX = w / 2.0f;
    const float radiusY = h / 2.0f;

    addCentredArc (x + radiusX,
                   y + radiusY,
                   radiusX, radiusY,
                   0.0f,
                   fromRadians, toRadians,
                   startAsNewSubPath);
}

static const float ellipseAngularIncrement = 0.05f;

void Path::addCentredArc (const float centreX, const float centreY,
                          const float radiusX, const float radiusY,
                          const float rotationOfEllipse,
                          const float fromRadians,
                          const float toRadians,
                          const bool startAsNewSubPath) throw()
{
    if (radiusX > 0.0f && radiusY > 0.0f)
    {
        const AffineTransform rotation (AffineTransform::rotation (rotationOfEllipse, centreX, centreY));
        float angle = fromRadians;

        if (startAsNewSubPath)
        {
            float x = centreX + radiusX * sinf (angle);
            float y = centreY - radiusY * cosf (angle);

            if (rotationOfEllipse != 0)
                rotation.transformPoint (x, y);

            startNewSubPath (x, y);
        }

        if (fromRadians < toRadians)
        {
            if (startAsNewSubPath)
                angle += ellipseAngularIncrement;

            while (angle < toRadians)
            {
                float x = centreX + radiusX * sinf (angle);
                float y = centreY - radiusY * cosf (angle);

                if (rotationOfEllipse != 0)
                    rotation.transformPoint (x, y);

                lineTo (x, y);

                angle += ellipseAngularIncrement;
            }
        }
        else
        {
            if (startAsNewSubPath)
                angle -= ellipseAngularIncrement;

            while (angle > toRadians)
            {
                float x = centreX + radiusX * sinf (angle);
                float y = centreY - radiusY * cosf (angle);

                if (rotationOfEllipse != 0)
                    rotation.transformPoint (x, y);

                lineTo (x, y);

                angle -= ellipseAngularIncrement;
            }
        }

        float x = centreX + radiusX * sinf (toRadians);
        float y = centreY - radiusY * cosf (toRadians);

        if (rotationOfEllipse != 0)
            rotation.transformPoint (x, y);

        lineTo (x, y);
    }
}

void Path::addPieSegment (const float x, const float y,
                          const float width, const float height,
                          const float fromRadians,
                          const float toRadians,
                          const float innerCircleProportionalSize)
{
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    const float centreX = x + hw;
    const float centreY = y + hh;

    startNewSubPath (centreX + hw * sinf (fromRadians),
                     centreY - hh * cosf (fromRadians));

    addArc (x, y, width, height, fromRadians, toRadians);

    if (fabs (fromRadians - toRadians) > float_Pi * 1.999f)
    {
        closeSubPath();

        if (innerCircleProportionalSize > 0)
        {
            hw *= innerCircleProportionalSize;
            hh *= innerCircleProportionalSize;

            startNewSubPath (centreX + hw * sinf (toRadians),
                             centreY - hh * cosf (toRadians));

            addArc (centreX - hw, centreY - hh, hw * 2.0f, hh * 2.0f,
                    toRadians, fromRadians);
        }
    }
    else
    {
        if (innerCircleProportionalSize > 0)
        {
            hw *= innerCircleProportionalSize;
            hh *= innerCircleProportionalSize;

            addArc (centreX - hw, centreY - hh, hw * 2.0f, hh * 2.0f,
                    toRadians, fromRadians);
        }
        else
        {
            lineTo (centreX, centreY);
        }
    }

    closeSubPath();
}

//==============================================================================
static void perpendicularOffset (const float x1, const float y1,
                                 const float x2, const float y2,
                                 const float offsetX, const float offsetY,
                                 float& resultX, float& resultY) throw()
{
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float len = juce_hypotf (dx, dy);

    if (len == 0)
    {
        resultX = x1;
        resultY = y1;
    }
    else
    {
        resultX = x1 + ((dx * offsetX) - (dy * offsetY)) / len;
        resultY = y1 + ((dy * offsetX) + (dx * offsetY)) / len;
    }
}

//==============================================================================
void Path::addLineSegment (const float startX, const float startY,
                           const float endX, const float endY,
                           float lineThickness) throw()
{
    lineThickness *= 0.5f;

    float x, y;

    perpendicularOffset (startX, startY, endX, endY,
                         0, lineThickness, x, y);
    startNewSubPath (x, y);

    perpendicularOffset (startX, startY, endX, endY,
                         0, -lineThickness, x, y);
    lineTo (x, y);

    perpendicularOffset (endX, endY, startX, startY,
                         0, lineThickness, x, y);
    lineTo (x, y);

    perpendicularOffset (endX, endY, startX, startY,
                         0, -lineThickness, x, y);
    lineTo (x, y);

    closeSubPath();
}

void Path::addArrow (const float startX, const float startY,
                     const float endX, const float endY,
                     float lineThickness,
                     float arrowheadWidth,
                     float arrowheadLength) throw()
{
    lineThickness *= 0.5f;
    arrowheadWidth *= 0.5f;
    arrowheadLength = jmin (arrowheadLength, 0.8f * juce_hypotf (startX - endX,
                                                                 startY - endY));

    float x, y;

    perpendicularOffset (startX, startY, endX, endY,
                         0, lineThickness, x, y);
    startNewSubPath (x, y);

    perpendicularOffset (startX, startY, endX, endY,
                         0, -lineThickness, x, y);
    lineTo (x, y);

    perpendicularOffset (endX, endY, startX, startY,
                         arrowheadLength, lineThickness, x, y);
    lineTo (x, y);

    perpendicularOffset (endX, endY, startX, startY,
                         arrowheadLength, arrowheadWidth, x, y);
    lineTo (x, y);

    perpendicularOffset (endX, endY, startX, startY,
                         0, 0, x, y);
    lineTo (x, y);

    perpendicularOffset (endX, endY, startX, startY,
                         arrowheadLength, -arrowheadWidth, x, y);
    lineTo (x, y);

    perpendicularOffset (endX, endY, startX, startY,
                         arrowheadLength, -lineThickness, x, y);
    lineTo (x, y);

    closeSubPath();
}

void Path::addStar (const float centreX,
                    const float centreY,
                    const int numberOfPoints,
                    const float innerRadius,
                    const float outerRadius,
                    const float startAngle)
{
    jassert (numberOfPoints > 1); // this would be silly.

    if (numberOfPoints > 1)
    {
        const float angleBetweenPoints = float_Pi * 2.0f / numberOfPoints;

        for (int i = 0; i < numberOfPoints; ++i)
        {
            float angle = startAngle + i * angleBetweenPoints;

            const float x = centreX + outerRadius * sinf (angle);
            const float y = centreY - outerRadius * cosf (angle);

            if (i == 0)
                startNewSubPath (x, y);
            else
                lineTo (x, y);

            angle += angleBetweenPoints * 0.5f;

            lineTo (centreX + innerRadius * sinf (angle),
                    centreY - innerRadius * cosf (angle));
        }

        closeSubPath();
    }
}

void Path::addBubble (float x, float y,
                      float w, float h,
                      float cs,
                      float tipX,
                      float tipY,
                      int whichSide,
                      float arrowPos,
                      float arrowWidth)
{
    if (w > 1.0f && h > 1.0f)
    {
        cs = jmin (cs, w * 0.5f, h * 0.5f);
        const float cs2 = 2.0f * cs;

        startNewSubPath (x + cs, y);

        if (whichSide == 0)
        {
            const float halfArrowW = jmin (arrowWidth, w - cs2) * 0.5f;
            const float arrowX1 = x + cs + jmax (0.0f, (w - cs2) * arrowPos - halfArrowW);
            lineTo (arrowX1, y);
            lineTo (tipX, tipY);
            lineTo (arrowX1 + halfArrowW * 2.0f, y);
        }

        lineTo (x + w - cs, y);

        if (cs > 0.0f)
            addArc (x + w - cs2, y, cs2, cs2, 0, float_Pi * 0.5f);

        if (whichSide == 3)
        {
            const float halfArrowH = jmin (arrowWidth, h - cs2) * 0.5f;
            const float arrowY1 = y + cs + jmax (0.0f, (h - cs2) * arrowPos - halfArrowH);
            lineTo (x + w, arrowY1);
            lineTo (tipX, tipY);
            lineTo (x + w, arrowY1 + halfArrowH * 2.0f);
        }

        lineTo (x + w, y + h - cs);

        if (cs > 0.0f)
            addArc (x + w - cs2, y + h - cs2, cs2, cs2, float_Pi * 0.5f, float_Pi);

        if (whichSide == 2)
        {
            const float halfArrowW = jmin (arrowWidth, w - cs2) * 0.5f;
            const float arrowX1 = x + cs + jmax (0.0f, (w - cs2) * arrowPos - halfArrowW);
            lineTo (arrowX1 + halfArrowW * 2.0f, y + h);
            lineTo (tipX, tipY);
            lineTo (arrowX1, y + h);
        }

        lineTo (x + cs, y + h);

        if (cs > 0.0f)
            addArc (x, y + h - cs2, cs2, cs2, float_Pi, float_Pi * 1.5f);

        if (whichSide == 1)
        {
            const float halfArrowH = jmin (arrowWidth, h - cs2) * 0.5f;
            const float arrowY1 = y + cs + jmax (0.0f, (h - cs2) * arrowPos - halfArrowH);
            lineTo (x, arrowY1 + halfArrowH * 2.0f);
            lineTo (tipX, tipY);
            lineTo (x, arrowY1);
        }

        lineTo (x, y + cs);

        if (cs > 0.0f)
            addArc (x, y, cs2, cs2, float_Pi * 1.5f, float_Pi * 2.0f - ellipseAngularIncrement);

        closeSubPath();
    }
}

void Path::addPath (const Path& other) throw()
{
    int i = 0;

    while (i < other.numElements)
    {
        const float type = other.elements [i++];

        if (type == moveMarker)
        {
            startNewSubPath (other.elements [i],
                             other.elements [i + 1]);

            i += 2;
        }
        else if (type == lineMarker)
        {
            lineTo (other.elements [i],
                    other.elements [i + 1]);

            i += 2;
        }
        else if (type == quadMarker)
        {
            quadraticTo (other.elements [i],
                         other.elements [i + 1],
                         other.elements [i + 2],
                         other.elements [i + 3]);
            i += 4;
        }
        else if (type == cubicMarker)
        {
            cubicTo (other.elements [i],
                     other.elements [i + 1],
                     other.elements [i + 2],
                     other.elements [i + 3],
                     other.elements [i + 4],
                     other.elements [i + 5]);

            i += 6;
        }
        else if (type == closeSubPathMarker)
        {
            closeSubPath();
        }
        else
        {
            // something's gone wrong with the element list!
            jassertfalse
        }
    }
}

void Path::addPath (const Path& other,
                    const AffineTransform& transformToApply) throw()
{
    int i = 0;

    while (i < other.numElements)
    {
        const float type = other.elements [i++];

        if (type == closeSubPathMarker)
        {
            closeSubPath();
        }
        else
        {
            float x = other.elements [i++];
            float y = other.elements [i++];
            transformToApply.transformPoint (x, y);

            if (type == moveMarker)
            {
                startNewSubPath (x, y);
            }
            else if (type == lineMarker)
            {
                lineTo (x, y);
            }
            else if (type == quadMarker)
            {
                float x2 = other.elements [i++];
                float y2 = other.elements [i++];
                transformToApply.transformPoint (x2, y2);

                quadraticTo (x, y, x2, y2);
            }
            else if (type == cubicMarker)
            {
                float x2 = other.elements [i++];
                float y2 = other.elements [i++];
                float x3 = other.elements [i++];
                float y3 = other.elements [i++];
                transformToApply.transformPoint (x2, y2);
                transformToApply.transformPoint (x3, y3);

                cubicTo (x, y, x2, y2, x3, y3);
            }
            else
            {
                // something's gone wrong with the element list!
                jassertfalse
            }
        }
    }
}

//==============================================================================
void Path::applyTransform (const AffineTransform& transform) throw()
{
    int i = 0;
    pathYMin = pathXMin = 0;
    pathYMax = pathXMax = 0;
    bool setMaxMin = false;

    while (i < numElements)
    {
        const float type = elements [i++];

        if (type == moveMarker)
        {
            transform.transformPoint (elements [i],
                                      elements [i + 1]);

            if (setMaxMin)
            {
                pathXMin = jmin (pathXMin, elements [i]);
                pathXMax = jmax (pathXMax, elements [i]);
                pathYMin = jmin (pathYMin, elements [i + 1]);
                pathYMax = jmax (pathYMax, elements [i + 1]);
            }
            else
            {
                pathXMin = pathXMax = elements [i];
                pathYMin = pathYMax = elements [i + 1];
                setMaxMin = true;
            }

            i += 2;
        }
        else if (type == lineMarker)
        {
            transform.transformPoint (elements [i],
                                      elements [i + 1]);

            pathXMin = jmin (pathXMin, elements [i]);
            pathXMax = jmax (pathXMax, elements [i]);
            pathYMin = jmin (pathYMin, elements [i + 1]);
            pathYMax = jmax (pathYMax, elements [i + 1]);

            i += 2;
        }
        else if (type == quadMarker)
        {
            transform.transformPoint (elements [i],
                                      elements [i + 1]);

            transform.transformPoint (elements [i + 2],
                                      elements [i + 3]);

            pathXMin = jmin (pathXMin, elements [i], elements [i + 2]);
            pathXMax = jmax (pathXMax, elements [i], elements [i + 2]);
            pathYMin = jmin (pathYMin, elements [i + 1], elements [i + 3]);
            pathYMax = jmax (pathYMax, elements [i + 1], elements [i + 3]);

            i += 4;
        }
        else if (type == cubicMarker)
        {
            transform.transformPoint (elements [i],
                                      elements [i + 1]);

            transform.transformPoint (elements [i + 2],
                                      elements [i + 3]);

            transform.transformPoint (elements [i + 4],
                                      elements [i + 5]);

            pathXMin = jmin (pathXMin, elements [i], elements [i + 2], elements [i + 4]);
            pathXMax = jmax (pathXMax, elements [i], elements [i + 2], elements [i + 4]);
            pathYMin = jmin (pathYMin, elements [i + 1], elements [i + 3], elements [i + 5]);
            pathYMax = jmax (pathYMax, elements [i + 1], elements [i + 3], elements [i + 5]);

            i += 6;
        }
    }
}


//==============================================================================
const AffineTransform Path::getTransformToScaleToFit (const float x, const float y,
                                                      const float w, const float h,
                                                      const bool preserveProportions,
                                                      const Justification& justification) const throw()
{
    float sx, sy, sw, sh;
    getBounds (sx, sy, sw, sh);

    if (preserveProportions)
    {
        if (w <= 0 || h <= 0 || sw <= 0 || sh <= 0)
            return AffineTransform::identity;

        float newW, newH;
        const float srcRatio = sh / sw;

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

        float newXCentre = x;
        float newYCentre = y;

        if (justification.testFlags (Justification::left))
            newXCentre += newW * 0.5f;
        else if (justification.testFlags (Justification::right))
            newXCentre += w - newW * 0.5f;
        else
            newXCentre += w * 0.5f;

        if (justification.testFlags (Justification::top))
            newYCentre += newH * 0.5f;
        else if (justification.testFlags (Justification::bottom))
            newYCentre += h - newH * 0.5f;
        else
            newYCentre += h * 0.5f;

        return AffineTransform::translation (sw * -0.5f - sx, sh * -0.5f - sy)
                    .scaled (newW / sw, newH / sh)
                    .translated (newXCentre, newYCentre);
    }
    else
    {
        return AffineTransform::translation (-sx, -sy)
                    .scaled (w / sw, h / sh)
                    .translated (x, y);
    }
}

//==============================================================================
static const float collisionDetectionTolerence = 20.0f;

bool Path::contains (const float x, const float y) const throw()
{
    if (x <= pathXMin || x >= pathXMax
         || y <= pathYMin || y >= pathYMax)
        return false;

    PathFlatteningIterator i (*this, AffineTransform::identity, collisionDetectionTolerence);

    int positiveCrossings = 0;
    int negativeCrossings = 0;

    while (i.next())
    {
        if ((i.y1 <= y && i.y2 > y)
             || (i.y2 <= y && i.y1 > y))
        {
            const float intersectX = i.x1 + (i.x2 - i.x1) * (y - i.y1) / (i.y2 - i.y1);

            if (intersectX <= x)
            {
                if (i.y1 < i.y2)
                    ++positiveCrossings;
                else
                    ++negativeCrossings;
            }
        }
    }

    return (useNonZeroWinding) ? (negativeCrossings != positiveCrossings)
                               : ((negativeCrossings + positiveCrossings) & 1) != 0;
}

bool Path::intersectsLine (const float x1, const float y1,
                           const float x2, const float y2) throw()
{
    PathFlatteningIterator i (*this, AffineTransform::identity, collisionDetectionTolerence);

    const Line line1 (x1, y1, x2, y2);

    while (i.next())
    {
        const Line line2 (i.x1, i.y1, i.x2, i.y2);

        float ix, iy;
        if (line1.intersects (line2, ix, iy))
            return true;
    }

    return false;
}

//==============================================================================
const Path Path::createPathWithRoundedCorners (const float cornerRadius) const throw()
{
    if (cornerRadius <= 0.01f)
        return *this;

    int indexOfPathStart = 0, indexOfPathStartThis = 0;
    int n = 0;
    bool lastWasLine = false, firstWasLine = false;
    Path p;

    while (n < numElements)
    {
        const float type = elements [n++];

        if (type == moveMarker)
        {
            indexOfPathStart = p.numElements;
            indexOfPathStartThis = n - 1;
            const float x = elements [n++];
            const float y = elements [n++];
            p.startNewSubPath (x, y);
            lastWasLine = false;
            firstWasLine = (elements [n] == lineMarker);
        }
        else if (type == lineMarker || type == closeSubPathMarker)
        {
            float startX = 0, startY = 0, joinX = 0, joinY = 0, endX, endY;

            if (type == lineMarker)
            {
                endX = elements [n++];
                endY = elements [n++];

                if (n > 8)
                {
                    startX = elements [n - 8];
                    startY = elements [n - 7];
                    joinX = elements [n - 5];
                    joinY = elements [n - 4];
                }
            }
            else
            {
                endX = elements [indexOfPathStartThis + 1];
                endY = elements [indexOfPathStartThis + 2];

                if (n > 6)
                {
                    startX = elements [n - 6];
                    startY = elements [n - 5];
                    joinX = elements [n - 3];
                    joinY = elements [n - 2];
                }
            }

            if (lastWasLine)
            {
                const double len1 = juce_hypot (startX - joinX,
                                                startY - joinY);

                if (len1 > 0)
                {
                    const double propNeeded = jmin (0.5, cornerRadius / len1);

                    p.elements [p.numElements - 2] = (float) (joinX - (joinX - startX) * propNeeded);
                    p.elements [p.numElements - 1] = (float) (joinY - (joinY - startY) * propNeeded);
                }

                const double len2 = juce_hypot (endX - joinX,
                                                endY - joinY);

                if (len2 > 0)
                {
                    const double propNeeded = jmin (0.5, cornerRadius / len2);

                    p.quadraticTo (joinX, joinY,
                                   (float) (joinX + (endX - joinX) * propNeeded),
                                   (float) (joinY + (endY - joinY) * propNeeded));
                }

                p.lineTo (endX, endY);
            }
            else if (type == lineMarker)
            {
                p.lineTo (endX, endY);
                lastWasLine = true;
            }

            if (type == closeSubPathMarker)
            {
                if (firstWasLine)
                {
                    startX = elements [n - 3];
                    startY = elements [n - 2];
                    joinX = endX;
                    joinY = endY;
                    endX = elements [indexOfPathStartThis + 4];
                    endY = elements [indexOfPathStartThis + 5];

                    const double len1 = juce_hypot (startX - joinX,
                                                    startY - joinY);

                    if (len1 > 0)
                    {
                        const double propNeeded = jmin (0.5, cornerRadius / len1);

                        p.elements [p.numElements - 2] = (float) (joinX - (joinX - startX) * propNeeded);
                        p.elements [p.numElements - 1] = (float) (joinY - (joinY - startY) * propNeeded);
                    }

                    const double len2 = juce_hypot (endX - joinX,
                                                    endY - joinY);

                    if (len2 > 0)
                    {
                        const double propNeeded = jmin (0.5, cornerRadius / len2);

                        endX = (float) (joinX + (endX - joinX) * propNeeded);
                        endY = (float) (joinY + (endY - joinY) * propNeeded);

                        p.quadraticTo (joinX, joinY, endX, endY);

                        p.elements [indexOfPathStart + 1] = endX;
                        p.elements [indexOfPathStart + 2] = endY;
                    }
                }

                p.closeSubPath();
            }
        }
        else if (type == quadMarker)
        {
            lastWasLine = false;
            const float x1 = elements [n++];
            const float y1 = elements [n++];
            const float x2 = elements [n++];
            const float y2 = elements [n++];
            p.quadraticTo (x1, y1, x2, y2);
        }
        else if (type == cubicMarker)
        {
            lastWasLine = false;
            const float x1 = elements [n++];
            const float y1 = elements [n++];
            const float x2 = elements [n++];
            const float y2 = elements [n++];
            const float x3 = elements [n++];
            const float y3 = elements [n++];
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
            const float x = source.readFloat();
            const float y = source.readFloat();
            startNewSubPath (x, y);
            break;
        }

        case 'l':
        {
            const float x = source.readFloat();
            const float y = source.readFloat();
            lineTo (x, y);
            break;
        }

        case 'q':
        {
            const float x1 = source.readFloat();
            const float y1 = source.readFloat();
            const float x2 = source.readFloat();
            const float y2 = source.readFloat();
            quadraticTo (x1, y1, x2, y2);
            break;
        }

        case 'b':
        {
            const float x1 = source.readFloat();
            const float y1 = source.readFloat();
            const float x2 = source.readFloat();
            const float y2 = source.readFloat();
            const float x3 = source.readFloat();
            const float y3 = source.readFloat();
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
            jassertfalse // illegal char in the stream
            break;
        }
    }
}

void Path::loadPathFromData (const unsigned char* const data,
                             const int numberOfBytes) throw()
{
    MemoryInputStream in ((const char*) data, numberOfBytes, false);
    loadPathFromStream (in);
}

void Path::writePathToStream (OutputStream& dest) const
{
    dest.writeByte ((useNonZeroWinding) ? 'n' : 'z');

    int i = 0;
    while (i < numElements)
    {
        const float type = elements [i++];

        if (type == moveMarker)
        {
            dest.writeByte ('m');
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
        }
        else if (type == lineMarker)
        {
            dest.writeByte ('l');
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
        }
        else if (type == quadMarker)
        {
            dest.writeByte ('q');
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
        }
        else if (type == cubicMarker)
        {
            dest.writeByte ('b');
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
            dest.writeFloat (elements [i++]);
        }
        else if (type == closeSubPathMarker)
        {
            dest.writeByte ('c');
        }
    }

    dest.writeByte ('e'); // marks the end-of-path
}

const String Path::toString() const
{
    String s;
    s.preallocateStorage (numElements * 4);
    if (! useNonZeroWinding)
        s << T("a ");

    int i = 0;
    float lastMarker = 0.0f;

    while (i < numElements)
    {
        const float marker = elements [i++];
        tchar markerChar = 0;
        int numCoords = 0;

        if (marker == moveMarker)
        {
            markerChar = T('m');
            numCoords = 2;
        }
        else if (marker == lineMarker)
        {
            markerChar = T('l');
            numCoords = 2;
        }
        else if (marker == quadMarker)
        {
            markerChar = T('q');
            numCoords = 4;
        }
        else if (marker == cubicMarker)
        {
            markerChar = T('c');
            numCoords = 6;
        }
        else
        {
            jassert (marker == closeSubPathMarker);
            markerChar = T('z');
        }

        if (marker != lastMarker)
        {
            s << markerChar << T(' ');
            lastMarker = marker;
        }

        while (--numCoords >= 0 && i < numElements)
        {
            String n (elements [i++], 3);

            while (n.endsWithChar (T('0')))
                n = n.dropLastCharacters (1);

            if (n.endsWithChar (T('.')))
                n = n.dropLastCharacters (1);

            s << n << T(' ');
        }
    }

    return s.trimEnd();
}

static const String nextToken (const tchar*& t)
{
    while (*t == T(' '))
        ++t;

    const tchar* const start = t;

    while (*t != 0 && *t != T(' '))
        ++t;

    const int length = (int) (t - start);

    while (*t == T(' '))
        ++t;

    return String (start, length);
}

void Path::restoreFromString (const String& stringVersion)
{
    clear();
    setUsingNonZeroWinding (true);

    const tchar* t = stringVersion;
    tchar marker = T('m');
    int numValues = 2;
    float values [6];

    while (*t != 0)
    {
        const String token (nextToken (t));
        const tchar firstChar = token[0];
        int startNum = 0;

        if (firstChar == T('m') || firstChar == T('l'))
        {
            marker = firstChar;
            numValues = 2;
        }
        else if (firstChar == T('q'))
        {
            marker = firstChar;
            numValues = 4;
        }
        else if (firstChar == T('c'))
        {
            marker = firstChar;
            numValues = 6;
        }
        else if (firstChar == T('z'))
        {
            marker = firstChar;
            numValues = 0;
        }
        else if (firstChar == T('a'))
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
            values [i] = nextToken (t).getFloatValue();

        switch (marker)
        {
        case T('m'):
            startNewSubPath (values[0], values[1]);
            break;

        case T('l'):
            lineTo (values[0], values[1]);
            break;

        case T('q'):
            quadraticTo (values[0], values[1],
                         values[2], values[3]);
            break;

        case T('c'):
            cubicTo (values[0], values[1],
                     values[2], values[3],
                     values[4], values[5]);
            break;

        case T('z'):
            closeSubPath();
            break;

        default:
            jassertfalse // illegal string format?
            break;
        }
    }
}

//==============================================================================
Path::Iterator::Iterator (const Path& path_)
    : path (path_),
      index (0)
{
}

Path::Iterator::~Iterator()
{
}

bool Path::Iterator::next()
{
    const float* const elements = path.elements;

    if (index < path.numElements)
    {
        const float type = elements [index++];

        if (type == moveMarker)
        {
            elementType = startNewSubPath;
            x1 = elements [index++];
            y1 = elements [index++];
        }
        else if (type == lineMarker)
        {
            elementType = lineTo;
            x1 = elements [index++];
            y1 = elements [index++];
        }
        else if (type == quadMarker)
        {
            elementType = quadraticTo;
            x1 = elements [index++];
            y1 = elements [index++];
            x2 = elements [index++];
            y2 = elements [index++];
        }
        else if (type == cubicMarker)
        {
            elementType = cubicTo;
            x1 = elements [index++];
            y1 = elements [index++];
            x2 = elements [index++];
            y2 = elements [index++];
            x3 = elements [index++];
            y3 = elements [index++];
        }
        else if (type == closeSubPathMarker)
        {
            elementType = closePath;
        }

        return true;
    }

    return false;
}


END_JUCE_NAMESPACE
