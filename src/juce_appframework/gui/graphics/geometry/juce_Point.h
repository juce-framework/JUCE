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

#ifndef __JUCE_POINT_JUCEHEADER__
#define __JUCE_POINT_JUCEHEADER__

#include "juce_AffineTransform.h"


//==============================================================================
/**
    A pair of (x, y) co-ordinates.

    Uses 32-bit floating point accuracy.

    @see Line, Path, AffineTransform
*/
class JUCE_API  Point
{
public:
    //==============================================================================
    /** Creates a point with co-ordinates (0, 0). */
    Point() throw();

    /** Creates a copy of another point. */
    Point (const Point& other) throw();

    /** Creates a point from an (x, y) position. */
    Point (const float x, const float y) throw();

    /** Copies this point from another one.
        @see setXY
    */
    const Point& operator= (const Point& other) throw();

    /** Destructor. */
    ~Point() throw();

    //==============================================================================
    /** Returns the point's x co-ordinate. */
    inline float getX() const throw()                   { return x; }

    /** Returns the point's y co-ordinate. */
    inline float getY() const throw()                   { return y; }

    /** Changes the point's x and y co-ordinates. */
    void setXY (const float x,
                const float y) throw();

    /** Uses a transform to change the point's co-ordinates.

        @see AffineTransform::transformPoint
    */
    void applyTransform (const AffineTransform& transform) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    float x, y;
};


#endif   // __JUCE_POINT_JUCEHEADER__
