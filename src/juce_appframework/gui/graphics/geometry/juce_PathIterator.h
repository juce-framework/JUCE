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

#ifndef __JUCE_PATHITERATOR_JUCEHEADER__
#define __JUCE_PATHITERATOR_JUCEHEADER__

#include "juce_Path.h"


//==============================================================================
/**
    Flattens a Path object into a series of straight-line sections.

    Use one of these to iterate through a Path object, and it will convert
    all the curves into line sections so it's easy to render or perform
    geometric operations on.

    @see Path
*/
class JUCE_API  PathFlatteningIterator
{
public:
    //==============================================================================
    /** Creates a PathFlatteningIterator.

        After creation, use the next() method to initialise the fields in the
        object with the first line's position.

        @param path         the path to iterate along
        @param transform    a transform to apply to each point in the path being iterated
        @param tolerence    the amount by which the curves are allowed to deviate from the
                            lines into which they are being broken down - a higher tolerence
                            is a bit faster, but less smooth.
    */
    PathFlatteningIterator (const Path& path,
                            const AffineTransform& transform = AffineTransform::identity,
                            float tolerence = 9.0f) throw();

    /** Destructor. */
    ~PathFlatteningIterator() throw();

    //==============================================================================
    /** Fetches the next line segment from the path.

        This will update the member variables x1, y1, x2, y2, subPathIndex and closesSubPath
        so that they describe the new line segment.

        @returns false when there are no more lines to fetch.
    */
    bool next() throw();

    /** The x position of the start of the current line segment. */
    float x1;
    /** The y position of the start of the current line segment. */
    float y1;
    /** The x position of the end of the current line segment. */
    float x2;
    /** The y position of the end of the current line segment. */
    float y2;

    /** Indicates whether the current line segment is closing a sub-path.

        If the current line is the one that connects the end of a sub-path
        back to the start again, this will be true.
    */
    bool closesSubPath;

    /** The index of the current line within the current sub-path.

        E.g. you can use this to see whether the line is the first one in the
        subpath by seeing if it's 0.
    */
    int subPathIndex;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    const Path& path;
    const AffineTransform transform;
    float* points;
    float tolerence, subPathCloseX, subPathCloseY;
    bool isIdentityTransform;

    float* stackBase;
    float* stackPos;
    int index, stackSize;

    PathFlatteningIterator (const PathFlatteningIterator&);
    const PathFlatteningIterator& operator= (const PathFlatteningIterator&);
};


#endif   // __JUCE_PATHITERATOR_JUCEHEADER__
