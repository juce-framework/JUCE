/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_DRAWABLERECTANGLE_H_INCLUDED
#define JUCE_DRAWABLERECTANGLE_H_INCLUDED


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
    DrawableRectangle (const DrawableRectangle&);

    /** Destructor. */
    ~DrawableRectangle();

    //==============================================================================
    /** Sets the rectangle's bounds. */
    void setRectangle (const RelativeParallelogram& newBounds);

    /** Returns the rectangle's bounds. */
    const RelativeParallelogram& getRectangle() const noexcept          { return bounds; }

    /** Returns the corner size to be used. */
    const RelativePoint& getCornerSize() const noexcept                 { return cornerSize; }

    /** Sets a new corner size for the rectangle */
    void setCornerSize (const RelativePoint& newSize);

    //==============================================================================
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder);
    /** @internal */
    ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;

    //==============================================================================
    /** Internally-used class for wrapping a DrawableRectangle's state into a ValueTree. */
    class ValueTreeWrapper   : public DrawableShape::FillAndStrokeState
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        RelativeParallelogram getRectangle() const;
        void setRectangle (const RelativeParallelogram& newBounds, UndoManager*);

        void setCornerSize (const RelativePoint& cornerSize, UndoManager*);
        RelativePoint getCornerSize() const;
        Value getCornerSizeValue (UndoManager*);

        static const Identifier topLeft, topRight, bottomLeft, cornerSize;
    };


private:
    friend class Drawable::Positioner<DrawableRectangle>;

    RelativeParallelogram bounds;
    RelativePoint cornerSize;

    void rebuildPath();
    bool registerCoordinates (RelativeCoordinatePositionerBase&);
    void recalculateCoordinates (Expression::Scope*);

    DrawableRectangle& operator= (const DrawableRectangle&);
    JUCE_LEAK_DETECTOR (DrawableRectangle)
};


#endif   // JUCE_DRAWABLERECTANGLE_H_INCLUDED
