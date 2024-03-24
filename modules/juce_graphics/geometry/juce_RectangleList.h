/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Maintains a set of rectangles as a complex region.

    This class allows a set of rectangles to be treated as a solid shape, and can
    add and remove rectangular sections of it, and simplify overlapping or
    adjacent rectangles.

    @see Rectangle

    @tags{Graphics}
*/
template <typename ValueType>
class RectangleList  final
{
public:
    using RectangleType = Rectangle<ValueType>;

    //==============================================================================
    /** Creates an empty RectangleList */
    RectangleList() = default;

    /** Creates a copy of another list */
    RectangleList (const RectangleList& other)  : rects (other.rects)
    {
    }

    /** Creates a list containing just one rectangle. */
    RectangleList (RectangleType rect)
    {
        addWithoutMerging (rect);
    }

    /** Copies this list from another one. */
    RectangleList& operator= (const RectangleList& other)
    {
        rects = other.rects;
        return *this;
    }

    /** Move constructor */
    RectangleList (RectangleList&& other) noexcept
        : rects (std::move (other.rects))
    {
    }

    /** Move assignment operator */
    RectangleList& operator= (RectangleList&& other) noexcept
    {
        rects = std::move (other.rects);
        return *this;
    }

    //==============================================================================
    /** Returns true if the region is empty. */
    bool isEmpty() const noexcept                               { return rects.isEmpty(); }

    /** Returns the number of rectangles in the list. */
    int getNumRectangles() const noexcept                       { return rects.size(); }

    /** Returns one of the rectangles at a particular index.
        @returns  the rectangle at the index, or an empty rectangle if the index is out-of-range.
    */
    RectangleType getRectangle (int index) const noexcept       { return rects[index]; }

    //==============================================================================
    /** Removes all rectangles to leave an empty region. */
    void clear()
    {
        rects.clearQuick();
    }

    /** Merges a new rectangle into the list.

        The rectangle being added will first be clipped to remove any parts of it
        that overlap existing rectangles in the list, and adjacent rectangles will be
        merged into it.

        The rectangle can have any size and may be empty, but if it's floating point
        then it's expected to not contain any INF values.
    */
    void add (RectangleType rect)
    {
        jassert (rect.isFinite()); // You must provide a valid rectangle to this method!

        if (! rect.isEmpty())
        {
            if (isEmpty())
            {
                rects.add (rect);
            }
            else
            {
                bool anyOverlaps = false;

                for (int j = rects.size(); --j >= 0;)
                {
                    auto& ourRect = rects.getReference (j);

                    if (rect.intersects (ourRect))
                    {
                        if (rect.contains (ourRect))
                            rects.remove (j);
                        else if (! ourRect.reduceIfPartlyContainedIn (rect))
                            anyOverlaps = true;
                    }
                }

                if (anyOverlaps && ! isEmpty())
                {
                    RectangleList r (rect);

                    for (auto& ourRect : rects)
                    {
                        if (rect.intersects (ourRect))
                        {
                            r.subtract (ourRect);

                            if (r.isEmpty())
                                return;
                        }
                    }

                    rects.addArray (r.rects);
                }
                else
                {
                    rects.add (rect);
                }
            }
        }
    }

    /** Merges a new rectangle into the list.

        The rectangle being added will first be clipped to remove any parts of it
        that overlap existing rectangles in the list.
    */
    void add (ValueType x, ValueType y, ValueType width, ValueType height)
    {
        add (RectangleType (x, y, width, height));
    }

    /** Dumbly adds a rectangle to the list without checking for overlaps.

        This simply adds the rectangle to the end, it doesn't merge it or remove
        any overlapping bits.

        The rectangle can have any size and may be empty, but if it's floating point
        then it's expected to not contain any INF values.
    */
    void addWithoutMerging (RectangleType rect)
    {
        jassert (rect.isFinite()); // You must provide a valid rectangle to this method!

        if (! rect.isEmpty())
            rects.add (rect);
    }

    /** Merges another rectangle list into this one.

        Any overlaps between the two lists will be clipped, so that the result is
        the union of both lists.
    */
    void add (const RectangleList& other)
    {
        for (auto& r : other)
            add (r);
    }

    /** Removes a rectangular region from the list.

        Any rectangles in the list which overlap this will be clipped and subdivided
        if necessary.
    */
    void subtract (const RectangleType rect)
    {
        if (auto numRects = rects.size())
        {
            const auto x1 = rect.getX();
            const auto y1 = rect.getY();
            const auto x2 = x1 + rect.getWidth();
            const auto y2 = y1 + rect.getHeight();

            for (int i = numRects; --i >= 0;)
            {
                auto& r = rects.getReference (i);

                const auto rx1 = r.getX();
                const auto ry1 = r.getY();
                const auto rx2 = rx1 + r.getWidth();
                const auto ry2 = ry1 + r.getHeight();

                const auto isNotEqual = [&] (const RectangleType newRect)
                {
                    // When subtracting tiny slices from relatively large rectangles, the
                    // subtraction may have no effect (due to limited-precision floating point
                    // maths) and the original rectangle may remain unchanged.
                    // We check that any 'new' rectangle has different dimensions to the rectangle
                    // being tested before adding it to the rects array.
                    // Integer arithmetic is not susceptible to this problem, so there's no need
                    // for this additional equality check when working with integral rectangles.
                    if constexpr (std::is_floating_point_v<ValueType>)
                    {
                        return newRect != r;
                    }
                    else
                    {
                        ignoreUnused (newRect);
                        return true;
                    }
                };

                if (rx1 < x2 && x1 < rx2 && ry1 < y2 && y1 < ry2)
                {
                    if (rx1 < x1 && x1 < rx2)
                    {
                        if (y1 <= ry1 && ry2 <= y2 && rx2 <= x2)
                        {
                            r.setWidth (x1 - rx1);
                        }
                        else
                        {
                            if (const RectangleType newRect (rx1, ry1, x1 - rx1, ry2 - ry1); isNotEqual (newRect))
                            {
                                r.setX (x1);
                                r.setWidth (rx2 - x1);

                                rects.insert (++i, newRect);
                                ++i;
                            }
                        }
                    }
                    else if (rx1 < x2 && x2 < rx2)
                    {
                        r.setX (x2);
                        r.setWidth (rx2 - x2);

                        if (ry1 < y1 || y2 < ry2 || rx1 < x1)
                        {
                            if (const RectangleType newRect (rx1, ry1, x2 - rx1, ry2 - ry1); isNotEqual (newRect))
                            {
                                rects.insert (++i, newRect);
                                ++i;
                            }
                        }
                    }
                    else if (ry1 < y1 && y1 < ry2)
                    {
                        if (x1 <= rx1 && rx2 <= x2 && ry2 <= y2)
                        {
                            r.setHeight (y1 - ry1);
                        }
                        else
                        {
                            if (const RectangleType newRect (rx1, ry1, rx2 - rx1, y1 - ry1); isNotEqual (newRect))
                            {
                                r.setY (y1);
                                r.setHeight (ry2 - y1);

                                rects.insert (++i, newRect);
                                ++i;
                            }
                        }
                    }
                    else if (ry1 < y2 && y2 < ry2)
                    {
                        r.setY (y2);
                        r.setHeight (ry2 - y2);

                        if (rx1 < x1 || x2 < rx2 || ry1 < y1)
                        {
                            if (const RectangleType newRect (rx1, ry1, rx2 - rx1, y2 - ry1); isNotEqual (newRect))
                            {
                                rects.insert (++i, newRect);
                                ++i;
                            }
                        }
                    }
                    else
                    {
                        rects.remove (i);
                    }
                }
            }
        }
    }

    /** Removes all areas in another RectangleList from this one.

        Any rectangles in the list which overlap this will be clipped and subdivided
        if necessary.

        @returns true if the resulting list is non-empty.
    */
    bool subtract (const RectangleList& otherList)
    {
        for (auto& r : otherList)
        {
            if (isEmpty())
                return false;

            subtract (r);
        }

        return ! isEmpty();
    }

    /** Removes any areas of the region that lie outside a given rectangle.

        Any rectangles in the list which overlap this will be clipped and subdivided
        if necessary.

        Returns true if the resulting region is not empty, false if it is empty.

        @see getIntersectionWith
    */
    bool clipTo (RectangleType rect)
    {
        jassert (rect.isFinite()); // You must provide a valid rectangle to this method!

        bool notEmpty = false;

        if (rect.isEmpty())
        {
            clear();
        }
        else
        {
            for (int i = rects.size(); --i >= 0;)
            {
                auto& r = rects.getReference (i);

                if (! rect.intersectRectangle (r))
                    rects.remove (i);
                else
                    notEmpty = true;
            }
        }

        return notEmpty;
    }

    /** Removes any areas of the region that lie outside a given rectangle list.

        Any rectangles in this object which overlap the specified list will be clipped
        and subdivided if necessary.

        Returns true if the resulting region is not empty, false if it is empty.

        @see getIntersectionWith
    */
    template <typename OtherValueType>
    bool clipTo (const RectangleList<OtherValueType>& other)
    {
        if (isEmpty())
            return false;

        RectangleList result;

        for (auto& rect : rects)
        {
            for (auto& r : other)
            {
                auto clipped = r.template toType<ValueType>();

                if (rect.intersectRectangle (clipped))
                    result.rects.add (clipped);
            }
        }

        swapWith (result);
        return ! isEmpty();
    }

    /** Creates a region which is the result of clipping this one to a given rectangle.

        Unlike the other clipTo method, this one doesn't affect this object - it puts the
        resulting region into the list whose reference is passed-in.

        Returns true if the resulting region is not empty, false if it is empty.

        @see clipTo
    */
    bool getIntersectionWith (RectangleType rect, RectangleList& destRegion) const
    {
        jassert (rect.isFinite()); // You must provide a valid rectangle to this method!

        destRegion.clear();

        if (! rect.isEmpty())
            for (auto r : rects)
                if (rect.intersectRectangle (r))
                    destRegion.rects.add (r);

        return ! destRegion.isEmpty();
    }

    /** Swaps the contents of this and another list.

        This swaps their internal pointers, so is hugely faster than using copy-by-value
        to swap them.
    */
    void swapWith (RectangleList& otherList) noexcept
    {
        rects.swapWith (otherList.rects);
    }

    //==============================================================================
    /** Checks whether the region contains a given point.
        @returns true if the point lies within one of the rectangles in the list
    */
    bool containsPoint (Point<ValueType> point) const noexcept
    {
        for (auto& r : rects)
            if (r.contains (point))
                return true;

        return false;
    }

    /** Checks whether the region contains a given point.
        @returns true if the point lies within one of the rectangles in the list
    */
    bool containsPoint (ValueType x, ValueType y) const noexcept
    {
        return containsPoint (Point<ValueType> (x, y));
    }

    /** Checks whether the region contains the whole of a given rectangle.

        @returns    true all parts of the rectangle passed in lie within the region
                    defined by this object
        @see intersectsRectangle, containsPoint
    */
    bool containsRectangle (RectangleType rectangleToCheck) const
    {
        if (rects.size() > 1)
        {
            RectangleList r (rectangleToCheck);

            for (auto& rect : rects)
            {
                r.subtract (rect);

                if (r.isEmpty())
                    return true;
            }
        }
        else if (! isEmpty())
        {
            return rects.getReference (0).contains (rectangleToCheck);
        }

        return false;
    }

    /** Checks whether the region contains any part of a given rectangle.

        @returns    true if any part of the rectangle passed in lies within the region
                    defined by this object
        @see containsRectangle
    */
    bool intersectsRectangle (RectangleType rectangleToCheck) const noexcept
    {
        for (auto& r : rects)
            if (r.intersects (rectangleToCheck))
                return true;

        return false;
    }

    /** Checks whether this region intersects any part of another one.
        @see intersectsRectangle
    */
    bool intersects (const RectangleList& other) const noexcept
    {
        for (auto& r : rects)
            if (other.intersectsRectangle (r))
                return true;

        return false;
    }

    //==============================================================================
    /** Returns the smallest rectangle that can enclose the whole of this region. */
    RectangleType getBounds() const noexcept
    {
        if (isEmpty())
            return {};

        auto& r = rects.getReference (0);

        if (rects.size() == 1)
            return r;

        auto minX = r.getX();
        auto minY = r.getY();
        auto maxX = minX + r.getWidth();
        auto maxY = minY + r.getHeight();

        for (int i = rects.size(); --i > 0;)
        {
            auto& r2 = rects.getReference (i);

            minX = jmin (minX, r2.getX());
            minY = jmin (minY, r2.getY());
            maxX = jmax (maxX, r2.getRight());
            maxY = jmax (maxY, r2.getBottom());
        }

        return { minX, minY, maxX - minX, maxY - minY };
    }

    /** Optimises the list into a minimum number of constituent rectangles.

        This will try to combine any adjacent rectangles into larger ones where
        possible, to simplify lists that might have been fragmented by repeated
        add/subtract calls.
    */
    void consolidate()
    {
        for (int i = 0; i < rects.size() - 1; ++i)
        {
            auto& r = rects.getReference (i);
            auto rx1 = r.getX();
            auto ry1 = r.getY();
            auto rx2 = rx1 + r.getWidth();
            auto ry2 = ry1 + r.getHeight();

            for (int j = rects.size(); --j > i;)
            {
                auto& r2 = rects.getReference (j);
                auto jrx1 = r2.getX();
                auto jry1 = r2.getY();
                auto jrx2 = jrx1 + r2.getWidth();
                auto jry2 = jry1 + r2.getHeight();

                // if the vertical edges of any blocks are touching and their horizontals don't
                // line up, split them horizontally..
                if (jrx1 == rx2 || jrx2 == rx1)
                {
                    if (jry1 > ry1 && jry1 < ry2)
                    {
                        r.setHeight (jry1 - ry1);
                        rects.add (RectangleType (rx1, jry1, rx2 - rx1, ry2 - jry1));
                        i = -1;
                        break;
                    }

                    if (jry2 > ry1 && jry2 < ry2)
                    {
                        r.setHeight (jry2 - ry1);
                        rects.add (RectangleType (rx1, jry2, rx2 - rx1, ry2 - jry2));
                        i = -1;
                        break;
                    }
                    else if (ry1 > jry1 && ry1 < jry2)
                    {
                        r2.setHeight (ry1 - jry1);
                        rects.add (RectangleType (jrx1, ry1, jrx2 - jrx1, jry2 - ry1));
                        i = -1;
                        break;
                    }
                    else if (ry2 > jry1 && ry2 < jry2)
                    {
                        r2.setHeight (ry2 - jry1);
                        rects.add (RectangleType (jrx1, ry2, jrx2 - jrx1, jry2 - ry2));
                        i = -1;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < rects.size() - 1; ++i)
        {
            auto& r = rects.getReference (i);

            for (int j = rects.size(); --j > i;)
            {
                if (r.enlargeIfAdjacent (rects.getReference (j)))
                {
                    rects.remove (j);
                    i = -1;
                    break;
                }
            }
        }
    }

    /** Adds an x and y value to all the coordinates. */
    void offsetAll (Point<ValueType> offset) noexcept
    {
        for (auto& r : rects)
            r += offset;
    }

    /** Adds an x and y value to all the coordinates. */
    void offsetAll (ValueType dx, ValueType dy) noexcept
    {
        offsetAll (Point<ValueType> (dx, dy));
    }

    /** Scales all the coordinates. */
    template <typename ScaleType>
    void scaleAll (ScaleType scaleFactor) noexcept
    {
        for (auto& r : rects)
            r *= scaleFactor;
    }

    /** Applies a transform to all the rectangles.
        Obviously this will create a mess if the transform involves any
        rotation or skewing.
    */
    void transformAll (const AffineTransform& transform) noexcept
    {
        for (auto& r : rects)
            r = r.transformedBy (transform);
    }

    //==============================================================================
    /** Creates a Path object to represent this region. */
    Path toPath() const
    {
        Path p;

        for (auto& r : rects)
            p.addRectangle (r);

        return p;
    }

    //==============================================================================
    /** Standard method for iterating the rectangles in the list. */
    const RectangleType* begin() const noexcept     { return rects.begin(); }
    /** Standard method for iterating the rectangles in the list. */
    const RectangleType* end() const noexcept       { return rects.end(); }

    /** Increases the internal storage to hold a minimum number of rectangles.
        Calling this before adding a large number of rectangles means that
        the array won't have to keep dynamically resizing itself as the elements
        are added, and it'll therefore be more efficient.
        @see Array::ensureStorageAllocated
    */
    void ensureStorageAllocated (int minNumRectangles)
    {
        rects.ensureStorageAllocated (minNumRectangles);
    }

private:
    //==============================================================================
    Array<RectangleType> rects;
};

} // namespace juce
