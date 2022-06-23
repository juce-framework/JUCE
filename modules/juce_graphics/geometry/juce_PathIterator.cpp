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

#if JUCE_MSVC && JUCE_DEBUG
 #pragma optimize ("t", on)
#endif

//==============================================================================
PathFlatteningIterator::PathFlatteningIterator (const Path& pathToUse,
                                                const AffineTransform& t,
                                                float tolerance)
    : x2 (0),
      y2 (0),
      closesSubPath (false),
      subPathIndex (-1),
      path (pathToUse),
      transform (t),
      source (path.data.begin()),
      toleranceSquared (tolerance * tolerance),
      isIdentityTransform (t.isIdentity())
{
    stackPos = stackBase;
}

PathFlatteningIterator::~PathFlatteningIterator()
{
}

bool PathFlatteningIterator::isLastInSubpath() const noexcept
{
    return stackPos == stackBase.get()
             && (source == path.data.end() || isMarker (*source, Path::moveMarker));
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

        if (stackPos == stackBase.get())
        {
            if (source == path.data.end())
                return false;

            type = *source++;

            if (! isMarker (type, Path::closeSubPathMarker))
            {
                x2 = *source++;
                y2 = *source++;

                if (isMarker (type, Path::quadMarker))
                {
                    x3 = *source++;
                    y3 = *source++;

                    if (! isIdentityTransform)
                        transform.transformPoints (x2, y2, x3, y3);
                }
                else if (isMarker (type, Path::cubicMarker))
                {
                    x3 = *source++;
                    y3 = *source++;
                    x4 = *source++;
                    y4 = *source++;

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

            if (! isMarker (type, Path::closeSubPathMarker))
            {
                x2 = *--stackPos;
                y2 = *--stackPos;

                if (isMarker (type, Path::quadMarker))
                {
                    x3 = *--stackPos;
                    y3 = *--stackPos;
                }
                else if (isMarker (type, Path::cubicMarker))
                {
                    x3 = *--stackPos;
                    y3 = *--stackPos;
                    x4 = *--stackPos;
                    y4 = *--stackPos;
                }
            }
        }

        if (isMarker (type, Path::lineMarker))
        {
            ++subPathIndex;

            closesSubPath = stackPos == stackBase.get()
                             && source != path.data.end()
                             && *source == Path::closeSubPathMarker
                             && x2 == subPathCloseX
                             && y2 == subPathCloseY;

            return true;
        }

        if (isMarker (type, Path::quadMarker))
        {
            const size_t offset = (size_t) (stackPos - stackBase);

            if (offset >= stackSize - 10)
            {
                stackSize <<= 1;
                stackBase.realloc (stackSize);
                stackPos = stackBase + offset;
            }

            auto m1x = (x1 + x2) * 0.5f;
            auto m1y = (y1 + y2) * 0.5f;
            auto m2x = (x2 + x3) * 0.5f;
            auto m2y = (y2 + y3) * 0.5f;
            auto m3x = (m1x + m2x) * 0.5f;
            auto m3y = (m1y + m2y) * 0.5f;

            auto errorX = m3x - x2;
            auto errorY = m3y - y2;

            auto outsideTolerance = errorX * errorX + errorY * errorY > toleranceSquared;
            auto canBeSubdivided = (m3x != m1x && m3x != m2x)
                                || (m3y != m1y && m3y != m2y);

            if (outsideTolerance && canBeSubdivided)
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
        else if (isMarker (type, Path::cubicMarker))
        {
            const size_t offset = (size_t) (stackPos - stackBase);

            if (offset >= stackSize - 16)
            {
                stackSize <<= 1;
                stackBase.realloc (stackSize);
                stackPos = stackBase + offset;
            }

            auto m1x = (x1 + x2) * 0.5f;
            auto m1y = (y1 + y2) * 0.5f;
            auto m2x = (x3 + x2) * 0.5f;
            auto m2y = (y3 + y2) * 0.5f;
            auto m3x = (x3 + x4) * 0.5f;
            auto m3y = (y3 + y4) * 0.5f;
            auto m4x = (m1x + m2x) * 0.5f;
            auto m4y = (m1y + m2y) * 0.5f;
            auto m5x = (m3x + m2x) * 0.5f;
            auto m5y = (m3y + m2y) * 0.5f;

            auto error1X = m4x - x2;
            auto error1Y = m4y - y2;
            auto error2X = m5x - x3;
            auto error2Y = m5y - y3;

            auto outsideTolerance = error1X * error1X + error1Y * error1Y > toleranceSquared
                                 || error2X * error2X + error2Y * error2Y > toleranceSquared;
            auto canBeSubdivided = (m4x != m1x && m4x != m2x)
                                || (m4y != m1y && m4y != m2y)
                                || (m5x != m3x && m5x != m2x)
                                || (m5y != m3y && m5y != m2y);

            if (outsideTolerance && canBeSubdivided)
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
        else if (isMarker (type, Path::closeSubPathMarker))
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
            jassert (isMarker (type, Path::moveMarker));

            subPathIndex = -1;
            subPathCloseX = x1 = x2;
            subPathCloseY = y1 = y2;
        }
    }
}

#if JUCE_MSVC && JUCE_DEBUG
  #pragma optimize ("", on)  // resets optimisations to the project defaults
#endif

} // namespace juce
