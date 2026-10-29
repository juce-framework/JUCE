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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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
        if (rects.isEmpty())
            return;

        modifyWithExactEdges ([&] (auto& rectList) { subtract (rectList, rect); });
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

    /** Simplifies the list by splitting and merging adjacent rectangles.

        This will try to combine adjacent rectangles into larger ones where possible,
        to simplify lists that might have been fragmented by repeated add/subtract
        calls.
    */
    void consolidate()
    {
        if (rects.size() < 2)
            return;

        modifyWithExactEdges ([&] (auto& rectList) { consolidate (rectList); });
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
    using PointType = Point<ValueType>;

    struct AABB
    {
        AABB() = default;
        AABB (const RectangleType& r) : tl (r.getTopLeft()), br (r.getBottomRight()) {}
        AABB (PointType a, PointType b) : tl (a), br (b) {}
        operator RectangleType() const { return RectangleType { tl, br }; }

        bool completelyOutside (const AABB& other) const
        {
            return other.br.x <= tl.x || br.x <= other.tl.x || other.br.y <= tl.y || br.y <= other.tl.y;
        }

        PointType tl, br;
    };

    // Selects the axis that a single pass over the list modifies (major) and the one it
    // leaves unchanged (minor), so the same code can be run once per axis.
    struct Fields
    {
        ValueType PointType::* major;
        ValueType PointType::* minor;
    };

    // Floating-point lists are edited as pairs of corners, so that moving an edge copies a
    // coordinate rather than computing a width or height that may round to a different edge.
    template <typename Fn>
    void modifyWithExactEdges (Fn&& fn)
    {
        if constexpr (std::is_floating_point_v<ValueType>)
        {
            Array<AABB> alternativeRepresentation;
            alternativeRepresentation.resize (rects.size());
            std::copy (rects.begin(), rects.end(), alternativeRepresentation.begin());

            fn (alternativeRepresentation);

            rects.resize (alternativeRepresentation.size());
            std::copy (alternativeRepresentation.begin(), alternativeRepresentation.end(), rects.begin());
        }
        else
        {
            fn (rects);
        }
    }

    static PointType getTL (const AABB& aabb)
    {
        return aabb.tl;
    }

    static PointType getBR (const AABB& aabb)
    {
        return aabb.br;
    }

    static PointType getTL (const RectangleType& r)
    {
        return r.getTopLeft();
    }

    static PointType getBR (const RectangleType& r)
    {
        return r.getBottomRight();
    }

    template <typename Rect>
    static void subtract (Array<Rect>& rectList, RectangleType rect)
    {
        jassert (! rectList.isEmpty());

        const AABB rAABB { getTL (rect), getBR (rect) };

        for (int i = rectList.size(); --i >= 0;)
        {
            auto& iRef = rectList.getReference (i);
            const AABB iAABB { getTL (iRef), getBR (iRef) };

            if (iAABB.completelyOutside (rAABB))
                continue;

            std::invoke ([&]
            {
                // For each rectangle in the list, we check for an overlap with the parameter rect
                // and subdivide the list rectangles as necessary.
                // The checks for each list rectangle happen in two stages, treating first the X
                // then the Y axis as the 'major' axis. The terms 'major' and 'minor' are somewhat
                // arbitrary. Essentially, in each pass, the rectangles in the list are only
                // modified and/or subdivided on the major axis, without changing any minor axis
                // coordinates. Subdividing an overlapping rect in this way may produce new rects
                // that still overlap with the parameter rect. However, those will get cleaned up
                // on the subsequent pass, where the major/minor axes are swapped.

                for (const auto& fields : { Fields { &PointType::x, &PointType::y },
                                            Fields { &PointType::y, &PointType::x } })
                {
                    const auto replaceMajor = [&] (auto modified, const auto& replacement)
                    {
                        modified.*(fields.major) = replacement.*(fields.major);
                        return modified;
                    };

                    const auto insert = [&] (const PointType& rAABBpt)
                    {
                        rectList.insert (++i, { iAABB.tl, replaceMajor (iAABB.br, rAABBpt) });
                        ++i;
                    };

                    if (   iAABB.tl.*(fields.major) < rAABB.tl.*(fields.major)
                        && rAABB.tl.*(fields.major) < iAABB.br.*(fields.major))
                    {
                        if (   rAABB.tl.*(fields.minor) <= iAABB.tl.*(fields.minor)
                            && iAABB.br.*(fields.minor) <= rAABB.br.*(fields.minor)
                            && iAABB.br.*(fields.major) <= rAABB.br.*(fields.major))
                        {
                            iRef = { iAABB.tl, replaceMajor (iAABB.br, rAABB.tl) };
                        }
                        else
                        {
                            iRef = { replaceMajor (iAABB.tl, rAABB.tl), iAABB.br };
                            insert (rAABB.tl);
                        }

                        return;
                    }

                    if (   iAABB.tl.*(fields.major) < rAABB.br.*(fields.major)
                        && rAABB.br.*(fields.major) < iAABB.br.*(fields.major))
                    {
                        iRef = { replaceMajor (iAABB.tl, rAABB.br), iAABB.br };

                        if (   iAABB.tl.*(fields.minor) < rAABB.tl.*(fields.minor)
                            || rAABB.br.*(fields.minor) < iAABB.br.*(fields.minor)
                            || iAABB.tl.*(fields.major) < rAABB.tl.*(fields.major))
                        {
                            insert (rAABB.br);
                        }

                        return;
                    }
                }

                rectList.remove (i);
            });
        }
    }

    // The equivalent of Rectangle::enlargeIfAdjacent, operating on corners rather than sizes.
    static std::optional<AABB> mergeIfAdjacent (const AABB& a, const AABB& b)
    {
        for (const auto& fields : { Fields { &PointType::x, &PointType::y },
                                    Fields { &PointType::y, &PointType::x } })
        {
            if (   exactlyEqual (a.tl.*(fields.minor), b.tl.*(fields.minor))
                && exactlyEqual (a.br.*(fields.minor), b.br.*(fields.minor))
                && a.tl.*(fields.major) <= b.br.*(fields.major)
                && b.tl.*(fields.major) <= a.br.*(fields.major))
            {
                auto merged = a;
                merged.tl.*(fields.major) = jmin (a.tl.*(fields.major), b.tl.*(fields.major));
                merged.br.*(fields.major) = jmax (a.br.*(fields.major), b.br.*(fields.major));
                return merged;
            }
        }

        return {};
    }

    template <typename Rect>
    static void consolidate (Array<Rect>& rectList)
    {
        // Where the vertical edges of two rectangles touch but their horizontal edges don't line
        // up, split them horizontally so that the merging pass below can combine them.
        for (int i = 0; i < rectList.size() - 1; ++i)
        {
            const AABB r { getTL (rectList.getReference (i)), getBR (rectList.getReference (i)) };

            for (int j = rectList.size(); --j > i;)
            {
                const AABB r2 { getTL (rectList.getReference (j)), getBR (rectList.getReference (j)) };

                if (! (exactlyEqual (r2.tl.x, r.br.x) || exactlyEqual (r2.br.x, r.tl.x)))
                    continue;

                const auto splitIfInside = [&] (int index, const AABB& aabb, ValueType y)
                {
                    if (! (aabb.tl.y < y && y < aabb.br.y))
                        return false;

                    rectList.getReference (index) = { aabb.tl, PointType { aabb.br.x, y } };
                    rectList.add ({ PointType { aabb.tl.x, y }, aabb.br });
                    return true;
                };

                if (   splitIfInside (i, r, r2.tl.y) || splitIfInside (i, r, r2.br.y)
                    || splitIfInside (j, r2, r.tl.y) || splitIfInside (j, r2, r.br.y))
                {
                    i = -1;
                    break;
                }
            }
        }

        for (int i = 0; i < rectList.size() - 1; ++i)
        {
            auto& r = rectList.getReference (i);

            for (int j = rectList.size(); --j > i;)
            {
                const auto& r2 = rectList.getReference (j);

                if (const auto merged = mergeIfAdjacent ({ getTL (r), getBR (r) }, { getTL (r2), getBR (r2) }))
                {
                    r = *merged;
                    rectList.remove (j);
                    i = -1;
                    break;
                }
            }
        }
    }

    //==============================================================================
    Array<RectangleType> rects;
};

} // namespace juce
