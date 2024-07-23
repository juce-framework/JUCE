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
