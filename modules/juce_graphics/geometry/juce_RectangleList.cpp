/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

RectangleList::RectangleList() noexcept
{
}

RectangleList::RectangleList (const Rectangle<int>& rect)
{
    addWithoutMerging (rect);
}

RectangleList::RectangleList (const RectangleList& other)
    : rects (other.rects)
{
}

RectangleList& RectangleList::operator= (const RectangleList& other)
{
    rects = other.rects;
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
RectangleList::RectangleList (RectangleList&& other) noexcept
    : rects (static_cast <Array <Rectangle<int> >&&> (other.rects))
{
}

RectangleList& RectangleList::operator= (RectangleList&& other) noexcept
{
    rects = static_cast <Array <Rectangle<int> >&&> (other.rects);
    return *this;
}
#endif

RectangleList::~RectangleList()
{
}

//==============================================================================
void RectangleList::clear()
{
    rects.clearQuick();
}

Rectangle<int> RectangleList::getRectangle (const int index) const noexcept
{
    if (isPositiveAndBelow (index, rects.size()))
        return rects.getReference (index);

    return Rectangle<int>();
}

bool RectangleList::isEmpty() const noexcept
{
    return rects.size() == 0;
}

//==============================================================================
RectangleList::Iterator::Iterator (const RectangleList& list) noexcept
    : current (nullptr),
      owner (list),
      index (list.rects.size())
{
}

RectangleList::Iterator::~Iterator()
{
}

bool RectangleList::Iterator::next() noexcept
{
    if (--index >= 0)
    {
        current = &(owner.rects.getReference (index));
        return true;
    }

    return false;
}


//==============================================================================
void RectangleList::add (const Rectangle<int>& rect)
{
    if (! rect.isEmpty())
    {
        if (rects.size() == 0)
        {
            rects.add (rect);
        }
        else
        {
            bool anyOverlaps = false;

            for (int j = rects.size(); --j >= 0;)
            {
                Rectangle<int>& ourRect = rects.getReference (j);

                if (rect.intersects (ourRect))
                {
                    if (rect.contains (ourRect))
                        rects.remove (j);
                    else if (! ourRect.reduceIfPartlyContainedIn (rect))
                        anyOverlaps = true;
                }
            }

            if (anyOverlaps && rects.size() > 0)
            {
                RectangleList r (rect);

                for (int i = rects.size(); --i >= 0;)
                {
                    const Rectangle<int>& ourRect = rects.getReference (i);

                    if (rect.intersects (ourRect))
                    {
                        r.subtract (ourRect);

                        if (r.rects.size() == 0)
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

void RectangleList::addWithoutMerging (const Rectangle<int>& rect)
{
    if (! rect.isEmpty())
        rects.add (rect);
}

void RectangleList::add (const int x, const int y, const int w, const int h)
{
    add (Rectangle<int> (x, y, w, h));
}

void RectangleList::add (const RectangleList& other)
{
    for (const Rectangle<int>* r = other.begin(), * const e = other.end(); r != e; ++r)
        add (*r);
}

void RectangleList::subtract (const Rectangle<int>& rect)
{
    const int originalNumRects = rects.size();

    if (originalNumRects > 0)
    {
        const int x1 = rect.pos.x;
        const int y1 = rect.pos.y;
        const int x2 = x1 + rect.w;
        const int y2 = y1 + rect.h;

        for (int i = getNumRectangles(); --i >= 0;)
        {
            Rectangle<int>& r = rects.getReference (i);

            const int rx1 = r.pos.x;
            const int ry1 = r.pos.y;
            const int rx2 = rx1 + r.w;
            const int ry2 = ry1 + r.h;

            if (! (x2 <= rx1 || x1 >= rx2 || y2 <= ry1 || y1 >= ry2))
            {
                if (x1 > rx1 && x1 < rx2)
                {
                    if (y1 <= ry1 && y2 >= ry2 && x2 >= rx2)
                    {
                        r.w = x1 - rx1;
                    }
                    else
                    {
                        r.pos.x = x1;
                        r.w = rx2 - x1;

                        rects.insert (++i, Rectangle<int> (rx1, ry1, x1 - rx1,  ry2 - ry1));
                        ++i;
                    }
                }
                else if (x2 > rx1 && x2 < rx2)
                {
                    r.pos.x = x2;
                    r.w = rx2 - x2;

                    if (y1 > ry1 || y2 < ry2 || x1 > rx1)
                    {
                        rects.insert (++i, Rectangle<int> (rx1, ry1, x2 - rx1,  ry2 - ry1));
                        ++i;
                    }
                }
                else if (y1 > ry1 && y1 < ry2)
                {
                    if (x1 <= rx1 && x2 >= rx2 && y2 >= ry2)
                    {
                        r.h = y1 - ry1;
                    }
                    else
                    {
                        r.pos.y = y1;
                        r.h = ry2 - y1;

                        rects.insert (++i, Rectangle<int> (rx1, ry1, rx2 - rx1, y1 - ry1));
                        ++i;
                    }
                }
                else if (y2 > ry1 && y2 < ry2)
                {
                    r.pos.y = y2;
                    r.h = ry2 - y2;

                    if (x1 > rx1 || x2 < rx2 || y1 > ry1)
                    {
                        rects.insert (++i, Rectangle<int> (rx1, ry1, rx2 - rx1, y2 - ry1));
                        ++i;
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

bool RectangleList::subtract (const RectangleList& otherList)
{
    for (int i = otherList.rects.size(); --i >= 0 && rects.size() > 0;)
        subtract (otherList.rects.getReference (i));

    return rects.size() > 0;
}

bool RectangleList::clipTo (const Rectangle<int>& rect)
{
    bool notEmpty = false;

    if (rect.isEmpty())
    {
        clear();
    }
    else
    {
        for (int i = rects.size(); --i >= 0;)
        {
            Rectangle<int>& r = rects.getReference (i);

            if (! rect.intersectRectangle (r.pos.x, r.pos.y, r.w, r.h))
                rects.remove (i);
            else
                notEmpty = true;
        }
    }

    return notEmpty;
}

bool RectangleList::clipTo (const RectangleList& other)
{
    if (rects.size() == 0)
        return false;

    RectangleList result;

    for (int j = 0; j < rects.size(); ++j)
    {
        const Rectangle<int>& rect = rects.getReference (j);

        for (int i = other.rects.size(); --i >= 0;)
        {
            Rectangle<int> r (other.rects.getReference (i));

            if (rect.intersectRectangle (r.pos.x, r.pos.y, r.w, r.h))
                result.rects.add (r);
        }
    }

    swapWith (result);

    return ! isEmpty();
}

bool RectangleList::getIntersectionWith (const Rectangle<int>& rect, RectangleList& destRegion) const
{
    destRegion.clear();

    if (! rect.isEmpty())
    {
        for (int i = rects.size(); --i >= 0;)
        {
            Rectangle<int> r (rects.getReference (i));

            if (rect.intersectRectangle (r.pos.x, r.pos.y, r.w, r.h))
                destRegion.rects.add (r);
        }
    }

    return destRegion.rects.size() > 0;
}

void RectangleList::swapWith (RectangleList& otherList) noexcept
{
    rects.swapWithArray (otherList.rects);
}


//==============================================================================
void RectangleList::consolidate()
{
    for (int i = 0; i < getNumRectangles() - 1; ++i)
    {
        Rectangle<int>& r = rects.getReference (i);
        const int rx1 = r.pos.x;
        const int ry1 = r.pos.y;
        const int rx2 = rx1 + r.w;
        const int ry2 = ry1 + r.h;

        for (int j = rects.size(); --j > i;)
        {
            Rectangle<int>& r2 = rects.getReference (j);
            const int jrx1 = r2.pos.x;
            const int jry1 = r2.pos.y;
            const int jrx2 = jrx1 + r2.w;
            const int jry2 = jry1 + r2.h;

            // if the vertical edges of any blocks are touching and their horizontals don't
            // line up, split them horizontally..
            if (jrx1 == rx2 || jrx2 == rx1)
            {
                if (jry1 > ry1 && jry1 < ry2)
                {
                    r.h = jry1 - ry1;
                    rects.add (Rectangle<int> (rx1, jry1, rx2 - rx1, ry2 - jry1));
                    i = -1;
                    break;
                }

                if (jry2 > ry1 && jry2 < ry2)
                {
                    r.h = jry2 - ry1;
                    rects.add (Rectangle<int> (rx1, jry2, rx2 - rx1, ry2 - jry2));
                    i = -1;
                    break;
                }
                else if (ry1 > jry1 && ry1 < jry2)
                {
                    r2.h = ry1 - jry1;
                    rects.add (Rectangle<int> (jrx1, ry1, jrx2 - jrx1, jry2 - ry1));
                    i = -1;
                    break;
                }
                else if (ry2 > jry1 && ry2 < jry2)
                {
                    r2.h = ry2 - jry1;
                    rects.add (Rectangle<int> (jrx1, ry2, jrx2 - jrx1, jry2 - ry2));
                    i = -1;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < rects.size() - 1; ++i)
    {
        Rectangle<int>& r = rects.getReference (i);

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

//==============================================================================
bool RectangleList::containsPoint (const int x, const int y) const noexcept
{
    for (const Rectangle<int>* r = rects.begin(), * const e = rects.end(); r != e; ++r)
        if (r->contains (x, y))
            return true;

    return false;
}

bool RectangleList::containsRectangle (const Rectangle<int>& rectangleToCheck) const
{
    if (rects.size() > 1)
    {
        RectangleList r (rectangleToCheck);

        for (int i = rects.size(); --i >= 0;)
        {
            r.subtract (rects.getReference (i));

            if (r.rects.size() == 0)
                return true;
        }
    }
    else if (rects.size() > 0)
    {
        return rects.getReference (0).contains (rectangleToCheck);
    }

    return false;
}

bool RectangleList::intersectsRectangle (const Rectangle<int>& rectangleToCheck) const noexcept
{
    for (const Rectangle<int>* r = rects.begin(), * const e = rects.end(); r != e; ++r)
        if (r->intersects (rectangleToCheck))
            return true;

    return false;
}

bool RectangleList::intersects (const RectangleList& other) const noexcept
{
    for (const Rectangle<int>* r = rects.begin(), * const e = rects.end(); r != e; ++r)
        if (other.intersectsRectangle (*r))
            return true;

    return false;
}

Rectangle<int> RectangleList::getBounds() const noexcept
{
    if (rects.size() <= 1)
    {
        if (rects.size() == 0)
            return Rectangle<int>();

        return rects.getReference (0);
    }
    else
    {
        const Rectangle<int>& r = rects.getReference (0);

        int minX = r.pos.x;
        int minY = r.pos.y;
        int maxX = minX + r.w;
        int maxY = minY + r.h;

        for (int i = rects.size(); --i > 0;)
        {
            const Rectangle<int>& r2 = rects.getReference (i);

            minX = jmin (minX, r2.pos.x);
            minY = jmin (minY, r2.pos.y);
            maxX = jmax (maxX, r2.getRight());
            maxY = jmax (maxY, r2.getBottom());
        }

        return Rectangle<int> (minX, minY, maxX - minX, maxY - minY);
    }
}

void RectangleList::offsetAll (const int dx, const int dy) noexcept
{
    for (Rectangle<int>* r = rects.begin(), * const e = rects.end(); r != e; ++r)
    {
        r->pos.x += dx;
        r->pos.y += dy;
    }
}

//==============================================================================
Path RectangleList::toPath() const
{
    Path p;

    for (int i = 0; i < rects.size(); ++i)
        p.addRectangle (rects.getReference (i));

    return p;
}
