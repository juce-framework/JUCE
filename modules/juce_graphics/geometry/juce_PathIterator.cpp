/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#if JUCE_MSVC && JUCE_DEBUG
 #pragma optimize ("t", on)
#endif

//==============================================================================
PathFlatteningIterator::PathFlatteningIterator (const Path& path_,
                                                const AffineTransform& transform_,
                                                const float tolerance)
    : x2 (0),
      y2 (0),
      closesSubPath (false),
      subPathIndex (-1),
      path (path_),
      transform (transform_),
      points (path_.data.elements),
      toleranceSquared (tolerance * tolerance),
      subPathCloseX (0),
      subPathCloseY (0),
      isIdentityTransform (transform_.isIdentity()),
      stackBase (32),
      index (0),
      stackSize (32)
{
    stackPos = stackBase;
}

PathFlatteningIterator::~PathFlatteningIterator()
{
}

bool PathFlatteningIterator::isLastInSubpath() const noexcept
{
    return stackPos == stackBase.getData()
             && (index >= path.numElements || points [index] == Path::moveMarker);
}

bool PathFlatteningIterator::next()
{
    x1 = x2;
    y1 = y2;

    float x3 = 0;
    float y3 = 0;
    float x4 = 0;
    float y4 = 0;

    for (;;)
    {
        float type;

        if (stackPos == stackBase)
        {
            if (index >= path.numElements)
                return false;

            type = points [index++];

            if (type != Path::closeSubPathMarker)
            {
                x2 = points [index++];
                y2 = points [index++];

                if (type == Path::quadMarker)
                {
                    x3 = points [index++];
                    y3 = points [index++];

                    if (! isIdentityTransform)
                        transform.transformPoints (x2, y2, x3, y3);
                }
                else if (type == Path::cubicMarker)
                {
                    x3 = points [index++];
                    y3 = points [index++];
                    x4 = points [index++];
                    y4 = points [index++];

                    if (! isIdentityTransform)
                        transform.transformPoints (x2, y2, x3, y3, x4, y4);
                }
                else
                {
                    if (! isIdentityTransform)
                        transform.transformPoint (x2, y2);
                }
            }
        }
        else
        {
            type = *--stackPos;

            if (type != Path::closeSubPathMarker)
            {
                x2 = *--stackPos;
                y2 = *--stackPos;

                if (type == Path::quadMarker)
                {
                    x3 = *--stackPos;
                    y3 = *--stackPos;
                }
                else if (type == Path::cubicMarker)
                {
                    x3 = *--stackPos;
                    y3 = *--stackPos;
                    x4 = *--stackPos;
                    y4 = *--stackPos;
                }
            }
        }

        if (type == Path::lineMarker)
        {
            ++subPathIndex;

            closesSubPath = (stackPos == stackBase)
                             && (index < path.numElements)
                             && (points [index] == Path::closeSubPathMarker)
                             && x2 == subPathCloseX
                             && y2 == subPathCloseY;

            return true;
        }

        if (type == Path::quadMarker)
        {
            const size_t offset = (size_t) (stackPos - stackBase);

            if (offset >= stackSize - 10)
            {
                stackSize <<= 1;
                stackBase.realloc (stackSize);
                stackPos = stackBase + offset;
            }

            const float m1x = (x1 + x2) * 0.5f;
            const float m1y = (y1 + y2) * 0.5f;
            const float m2x = (x2 + x3) * 0.5f;
            const float m2y = (y2 + y3) * 0.5f;
            const float m3x = (m1x + m2x) * 0.5f;
            const float m3y = (m1y + m2y) * 0.5f;

            const float errorX = m3x - x2;
            const float errorY = m3y - y2;

            if (errorX * errorX + errorY * errorY > toleranceSquared)
            {
                *stackPos++ = y3;
                *stackPos++ = x3;
                *stackPos++ = m2y;
                *stackPos++ = m2x;
                *stackPos++ = Path::quadMarker;

                *stackPos++ = m3y;
                *stackPos++ = m3x;
                *stackPos++ = m1y;
                *stackPos++ = m1x;
                *stackPos++ = Path::quadMarker;
            }
            else
            {
                *stackPos++ = y3;
                *stackPos++ = x3;
                *stackPos++ = Path::lineMarker;

                *stackPos++ = m3y;
                *stackPos++ = m3x;
                *stackPos++ = Path::lineMarker;
            }

            jassert (stackPos < stackBase + stackSize);
        }
        else if (type == Path::cubicMarker)
        {
            const size_t offset = (size_t) (stackPos - stackBase);

            if (offset >= stackSize - 16)
            {
                stackSize <<= 1;
                stackBase.realloc (stackSize);
                stackPos = stackBase + offset;
            }

            const float m1x = (x1 + x2) * 0.5f;
            const float m1y = (y1 + y2) * 0.5f;
            const float m2x = (x3 + x2) * 0.5f;
            const float m2y = (y3 + y2) * 0.5f;
            const float m3x = (x3 + x4) * 0.5f;
            const float m3y = (y3 + y4) * 0.5f;
            const float m4x = (m1x + m2x) * 0.5f;
            const float m4y = (m1y + m2y) * 0.5f;
            const float m5x = (m3x + m2x) * 0.5f;
            const float m5y = (m3y + m2y) * 0.5f;

            const float error1X = m4x - x2;
            const float error1Y = m4y - y2;
            const float error2X = m5x - x3;
            const float error2Y = m5y - y3;

            if (error1X * error1X + error1Y * error1Y > toleranceSquared
                 || error2X * error2X + error2Y * error2Y > toleranceSquared)
            {
                *stackPos++ = y4;
                *stackPos++ = x4;
                *stackPos++ = m3y;
                *stackPos++ = m3x;
                *stackPos++ = m5y;
                *stackPos++ = m5x;
                *stackPos++ = Path::cubicMarker;

                *stackPos++ = (m4y + m5y) * 0.5f;
                *stackPos++ = (m4x + m5x) * 0.5f;
                *stackPos++ = m4y;
                *stackPos++ = m4x;
                *stackPos++ = m1y;
                *stackPos++ = m1x;
                *stackPos++ = Path::cubicMarker;
            }
            else
            {
                *stackPos++ = y4;
                *stackPos++ = x4;
                *stackPos++ = Path::lineMarker;

                *stackPos++ = m5y;
                *stackPos++ = m5x;
                *stackPos++ = Path::lineMarker;

                *stackPos++ = m4y;
                *stackPos++ = m4x;
                *stackPos++ = Path::lineMarker;
            }
        }
        else if (type == Path::closeSubPathMarker)
        {
            if (x2 != subPathCloseX || y2 != subPathCloseY)
            {
                x1 = x2;
                y1 = y2;
                x2 = subPathCloseX;
                y2 = subPathCloseY;
                closesSubPath = true;

                return true;
            }
        }
        else
        {
            jassert (type == Path::moveMarker);

            subPathIndex = -1;
            subPathCloseX = x1 = x2;
            subPathCloseY = y1 = y2;
        }
    }
}

#if JUCE_MSVC && JUCE_DEBUG
  #pragma optimize ("", on)  // resets optimisations to the project defaults
#endif
