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

#if JUCE_MSVC
  #pragma optimize ("t", on)
#endif

//==============================================================================
static const float lineMarker       = 100001.0f;
static const float moveMarker       = 100002.0f;
static const float quadMarker       = 100003.0f;
static const float cubicMarker      = 100004.0f;
static const float closePathMarker  = 100005.0f;


//==============================================================================
PathFlatteningIterator::PathFlatteningIterator (const Path& path_,
                                                const AffineTransform& transform_,
                                                float tolerence_) throw()
    : x2 (0),
      y2 (0),
      closesSubPath (false),
      subPathIndex (-1),
      path (path_),
      transform (transform_),
      points (path_.elements),
      tolerence (tolerence_ * tolerence_),
      subPathCloseX (0),
      subPathCloseY (0),
      index (0),
      stackSize (32)
{
    stackBase = (float*) juce_malloc (stackSize * sizeof (float));
    isIdentityTransform = transform.isIdentity();
    stackPos = stackBase;
}

PathFlatteningIterator::~PathFlatteningIterator() throw()
{
    juce_free (stackBase);
}

bool PathFlatteningIterator::next() throw()
{
    x1 = x2;
    y1 = y2;

    float x3 = 0;
    float y3 = 0;
    float x4 = 0;
    float y4 = 0;
    float type;

    for (;;)
    {
        if (stackPos == stackBase)
        {
            if (index >= path.numElements)
            {
                return false;
            }
            else
            {
                type = points [index++];

                if (type != closePathMarker)
                {
                    x2 = points [index++];
                    y2 = points [index++];

                    if (! isIdentityTransform)
                        transform.transformPoint (x2, y2);

                    if (type == quadMarker)
                    {
                        x3 = points [index++];
                        y3 = points [index++];

                        if (! isIdentityTransform)
                            transform.transformPoint (x3, y3);
                    }
                    else if (type == cubicMarker)
                    {
                        x3 = points [index++];
                        y3 = points [index++];
                        x4 = points [index++];
                        y4 = points [index++];

                        if (! isIdentityTransform)
                        {
                            transform.transformPoint (x3, y3);
                            transform.transformPoint (x4, y4);
                        }
                    }
                }
            }
        }
        else
        {
            type = *--stackPos;

            if (type != closePathMarker)
            {
                x2 = *--stackPos;
                y2 = *--stackPos;

                if (type == quadMarker)
                {
                    x3 = *--stackPos;
                    y3 = *--stackPos;
                }
                else if (type == cubicMarker)
                {
                    x3 = *--stackPos;
                    y3 = *--stackPos;
                    x4 = *--stackPos;
                    y4 = *--stackPos;
                }
            }
        }

        if (type == lineMarker)
        {
            ++subPathIndex;

            closesSubPath = (stackPos == stackBase)
                             && (index < path.numElements)
                             && (points [index] == closePathMarker)
                             && x2 == subPathCloseX
                             && y2 == subPathCloseY;

            return true;
        }
        else if (type == quadMarker)
        {
            const int offset = (int) (stackPos - stackBase);

            if (offset >= stackSize - 10)
            {
                stackSize <<= 1;
                stackBase = (float*) juce_realloc (stackBase, stackSize * sizeof (float));
                stackPos = stackBase + offset;
            }

            const float dx1 = x1 - x2;
            const float dy1 = y1 - y2;
            const float dx2 = x2 - x3;
            const float dy2 = y2 - y3;

            const float m1x = (x1 + x2) * 0.5f;
            const float m1y = (y1 + y2) * 0.5f;
            const float m2x = (x2 + x3) * 0.5f;
            const float m2y = (y2 + y3) * 0.5f;
            const float m3x = (m1x + m2x) * 0.5f;
            const float m3y = (m1y + m2y) * 0.5f;

            if (dx1*dx1 + dy1*dy1 + dx2*dx2 + dy2*dy2 > tolerence)
            {
                *stackPos++ = y3;
                *stackPos++ = x3;
                *stackPos++ = m2y;
                *stackPos++ = m2x;
                *stackPos++ = quadMarker;

                *stackPos++ = m3y;
                *stackPos++ = m3x;
                *stackPos++ = m1y;
                *stackPos++ = m1x;
                *stackPos++ = quadMarker;
            }
            else
            {
                *stackPos++ = y3;
                *stackPos++ = x3;
                *stackPos++ = lineMarker;

                *stackPos++ = m3y;
                *stackPos++ = m3x;
                *stackPos++ = lineMarker;
            }

            jassert (stackPos < stackBase + stackSize);
        }
        else if (type == cubicMarker)
        {
            const int offset = (int) (stackPos - stackBase);

            if (offset >= stackSize - 16)
            {
                stackSize <<= 1;
                stackBase = (float*) juce_realloc (stackBase, stackSize * sizeof (float));
                stackPos = stackBase + offset;
            }

            const float dx1 = x1 - x2;
            const float dy1 = y1 - y2;
            const float dx2 = x2 - x3;
            const float dy2 = y2 - y3;
            const float dx3 = x3 - x4;
            const float dy3 = y3 - y4;

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

            if (dx1*dx1 + dy1*dy1 + dx2*dx2
                + dy2*dy2 + dx3*dx3 + dy3*dy3 > tolerence)
            {
                *stackPos++ = y4;
                *stackPos++ = x4;
                *stackPos++ = m3y;
                *stackPos++ = m3x;
                *stackPos++ = m5y;
                *stackPos++ = m5x;
                *stackPos++ = cubicMarker;

                *stackPos++ = (m4y + m5y) * 0.5f;
                *stackPos++ = (m4x + m5x) * 0.5f;
                *stackPos++ = m4y;
                *stackPos++ = m4x;
                *stackPos++ = m1y;
                *stackPos++ = m1x;
                *stackPos++ = cubicMarker;
            }
            else
            {
                *stackPos++ = y4;
                *stackPos++ = x4;
                *stackPos++ = lineMarker;

                *stackPos++ = m5y;
                *stackPos++ = m5x;
                *stackPos++ = lineMarker;

                *stackPos++ = m4y;
                *stackPos++ = m4x;
                *stackPos++ = lineMarker;
            }
        }
        else if (type == closePathMarker)
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
            subPathIndex = -1;
            subPathCloseX = x1 = x2;
            subPathCloseY = y1 = y2;
        }
    }
}

END_JUCE_NAMESPACE
