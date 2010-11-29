/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_DRAWABLERECTANGLE_JUCEHEADER__
#define __JUCE_DRAWABLERECTANGLE_JUCEHEADER__

#include "juce_DrawableShape.h"


//==============================================================================
/**
    A Drawable object which draws a rectangle.

    For details on how to change the fill and stroke, see the DrawableShape class.

    @see Drawable, DrawableShape
*/
class JUCE_API  DrawableRectangle  : public DrawableShape
{
public:
    //==============================================================================
    DrawableRectangle();
    DrawableRectangle (const DrawableRectangle& other);

    /** Destructor. */
    ~DrawableRectangle();

    //==============================================================================
    /** Sets the rectangle's bounds. */
    void setRectangle (const RelativeParallelogram& newBounds);

    /** Returns the rectangle's bounds. */
    const RelativeParallelogram& getRectangle() const throw()           { return bounds; }

    /** Returns the corner size to be used. */
    const RelativePoint getCornerSize() const                           { return cornerSize; }

    /** Sets a new corner size for the rectangle */
    void setCornerSize (const RelativePoint& newSize);

    //==============================================================================
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const    { return valueTreeType; }

    //==============================================================================
    /** Internally-used class for wrapping a DrawableRectangle's state into a ValueTree. */
    class ValueTreeWrapper   : public DrawableShape::FillAndStrokeState
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        const RelativeParallelogram getRectangle() const;
        void setRectangle (const RelativeParallelogram& newBounds, UndoManager* undoManager);

        void setCornerSize (const RelativePoint& cornerSize, UndoManager* undoManager);
        const RelativePoint getCornerSize() const;
        Value getCornerSizeValue (UndoManager* undoManager) const;

        static const Identifier topLeft, topRight, bottomLeft, cornerSize;
    };


protected:
    /** @internal */
    bool rebuildPath (Path& path) const;

private:
    RelativeParallelogram bounds;
    RelativePoint cornerSize;

    const AffineTransform calculateTransform() const;

    DrawableRectangle& operator= (const DrawableRectangle&);
    JUCE_LEAK_DETECTOR (DrawableRectangle);
};


#endif   // __JUCE_DRAWABLERECTANGLE_JUCEHEADER__
