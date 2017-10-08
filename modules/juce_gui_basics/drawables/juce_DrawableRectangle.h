/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

} // namespace juce
