/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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

    @tags{GUI}
*/
class JUCE_API  DrawableRectangle  : public DrawableShape
{
public:
    //==============================================================================
    DrawableRectangle();
    DrawableRectangle (const DrawableRectangle&);

    /** Destructor. */
    ~DrawableRectangle() override;

    //==============================================================================
    /** Sets the rectangle's bounds. */
    void setRectangle (Parallelogram<float> newBounds);

    /** Returns the rectangle's bounds. */
    Parallelogram<float> getRectangle() const noexcept              { return bounds; }

    /** Returns the corner size to be used. */
    Point<float> getCornerSize() const noexcept                     { return cornerSize; }

    /** Sets a new corner size for the rectangle */
    void setCornerSize (Point<float> newSize);

    //==============================================================================
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;

private:
    Parallelogram<float> bounds;
    Point<float> cornerSize;

    void rebuildPath();

    DrawableRectangle& operator= (const DrawableRectangle&);
    JUCE_LEAK_DETECTOR (DrawableRectangle)
};

} // namespace juce
