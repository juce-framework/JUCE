/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Rectangle.h"
#include "../../../text/juce_StringArray.h"


//==============================================================================
Rectangle::Rectangle() throw()
  : x (0),
    y (0),
    w (0),
    h (0)
{
}

Rectangle::Rectangle (const int x_, const int y_,
                      const int w_, const int h_) throw()
  : x (x_),
    y (y_),
    w (w_),
    h (h_)
{
}

Rectangle::Rectangle (const int w_, const int h_) throw()
  : x (0),
    y (0),
    w (w_),
    h (h_)
{
}

Rectangle::Rectangle (const Rectangle& other) throw()
  : x (other.x),
    y (other.y),
    w (other.w),
    h (other.h)
{
}

Rectangle::~Rectangle() throw()
{
}

//==============================================================================
bool Rectangle::isEmpty() const throw()
{
    return w <= 0 || h <= 0;
}

void Rectangle::setBounds (const int x_,
                           const int y_,
                           const int w_,
                           const int h_) throw()
{
    x = x_;
    y = y_;
    w = w_;
    h = h_;
}

void Rectangle::setPosition (const int x_,
                             const int y_) throw()
{
    x = x_;
    y = y_;
}

void Rectangle::setSize (const int w_,
                         const int h_) throw()
{
    w = w_;
    h = h_;
}

void Rectangle::setWidth (const int newWidth) throw()
{
    w = newWidth;
}

void Rectangle::setHeight (const int newHeight) throw()
{
    h = newHeight;
}

void Rectangle::setLeft (const int newLeft) throw()
{
    w = jmax (0, x + w - newLeft);
    x = newLeft;
}

void Rectangle::setTop (const int newTop) throw()
{
    h = jmax (0, y + h - newTop);
    y = newTop;
}

void Rectangle::setRight (const int newRight) throw()
{
    x = jmin (x, newRight);
    w = newRight - x;
}

void Rectangle::setBottom (const int newBottom) throw()
{
    y = jmin (y, newBottom);
    h = newBottom - y;
}

void Rectangle::translate (const int dx,
                           const int dy) throw()
{
    x += dx;
    y += dy;
}

const Rectangle Rectangle::translated (const int dx,
                                       const int dy) const throw()
{
    return Rectangle (x + dx, y + dy, w, h);
}

void Rectangle::expand (const int deltaX,
                        const int deltaY) throw()
{
    const int nw = jmax (0, w + deltaX + deltaX);
    const int nh = jmax (0, h + deltaY + deltaY);

    setBounds (x - deltaX,
               y - deltaY,
               nw, nh);
}

const Rectangle Rectangle::expanded (const int deltaX,
                                     const int deltaY) const throw()
{
    const int nw = jmax (0, w + deltaX + deltaX);
    const int nh = jmax (0, h + deltaY + deltaY);

    return Rectangle (x - deltaX,
                      y - deltaY,
                      nw, nh);
}

void Rectangle::reduce (const int deltaX,
                        const int deltaY) throw()
{
    expand (-deltaX, -deltaY);
}

const Rectangle Rectangle::reduced (const int deltaX,
                                    const int deltaY) const throw()
{
    return expanded (-deltaX, -deltaY);
}

bool Rectangle::operator== (const Rectangle& other) const throw()
{
    return x == other.x
        && y == other.y
        && w == other.w
        && h == other.h;
}

bool Rectangle::operator!= (const Rectangle& other) const throw()
{
    return x != other.x
        || y != other.y
        || w != other.w
        || h != other.h;
}

bool Rectangle::contains (const int px,
                          const int py) const throw()
{
    return px >= x
        && py >= y
        && px < x + w
        && py < y + h;
}

bool Rectangle::contains (const Rectangle& other) const throw()
{
    return x <= other.x
        && y <= other.y
        && x + w >= other.x + other.w
        && y + h >= other.y + other.h;
}

bool Rectangle::intersects (const Rectangle& other) const throw()
{
    return x + w > other.x
        && y + h > other.y
        && x < other.x + other.w
        && y < other.y + other.h
        && w > 0
        && h > 0;
}

const Rectangle Rectangle::getIntersection (const Rectangle& other) const throw()
{
    const int nx = jmax (x, other.x);
    const int ny = jmax (y, other.y);
    const int nw = jmin (x + w, other.x + other.w) - nx;
    const int nh = jmin (y + h, other.y + other.h) - ny;

    if (nw >= 0 && nh >= 0)
        return Rectangle (nx, ny, nw, nh);
    else
        return Rectangle();
}

bool Rectangle::intersectRectangle (int& x1, int& y1, int& w1, int& h1) const throw()
{
    const int maxX = jmax (x1, x);
    w1 = jmin (x1 + w1, x + w) - maxX;

    if (w1 > 0)
    {
        const int maxY = jmax (y1, y);
        h1 = jmin (y1 + h1, y + h) - maxY;

        if (h1 > 0)
        {
            x1 = maxX;
            y1 = maxY;

            return true;
        }
    }

    return false;
}

bool Rectangle::intersectRectangles (int& x1, int& y1, int& w1, int& h1,
                                     int x2, int y2, int w2, int h2) throw()
{
    const int x = jmax (x1, x2);
    w1 = jmin (x1 + w1, x2 + w2) - x;

    if (w1 > 0)
    {
        const int y = jmax (y1, y2);
        h1 = jmin (y1 + h1, y2 + h2) - y;

        if (h1 > 0)
        {
            x1 = x;
            y1 = y;

            return true;
        }
    }

    return false;
}

const Rectangle Rectangle::getUnion (const Rectangle& other) const throw()
{
    const int newX = jmin (x, other.x);
    const int newY = jmin (y, other.y);

    return Rectangle (newX, newY,
                      jmax (x + w, other.x + other.w) - newX,
                      jmax (y + h, other.y + other.h) - newY);
}

bool Rectangle::enlargeIfAdjacent (const Rectangle& other) throw()
{
    if (x == other.x && getRight() == other.getRight()
        && (other.getBottom() >= y && other.y <= getBottom()))
    {
        const int newY = jmin (y, other.y);
        h = jmax (getBottom(), other.getBottom()) - newY;
        y = newY;
        return true;
    }
    else if (y == other.y && getBottom() == other.getBottom()
              && (other.getRight() >= x && other.x <= getRight()))
    {
        const int newX = jmin (x, other.x);
        w = jmax (getRight(), other.getRight()) - newX;
        x = newX;
        return true;
    }

    return false;
}

bool Rectangle::reduceIfPartlyContainedIn (const Rectangle& other) throw()
{
    int inside = 0;
    const int otherR = other.getRight();

    if (x >= other.x && x < otherR)
        inside = 1;

    const int otherB = other.getBottom();

    if (y >= other.y && y < otherB)
        inside |= 2;

    const int r = x + w;

    if (r >= other.x && r < otherR)
        inside |= 4;

    const int b = y + h;

    if (b >= other.y && b < otherB)
        inside |= 8;

    switch (inside)
    {
    case 1 + 2 + 8:
        w = r - otherR;
        x = otherR;
        return true;

    case 1 + 2 + 4:
        h = b - otherB;
        y = otherB;
        return true;

    case 2 + 4 + 8:
        w = other.x - x;
        return true;

    case 1 + 4 + 8:
        h = other.y - y;
        return true;
    }

    return false;
}

const String Rectangle::toString() const throw()
{
    String s;
    s.preallocateStorage (16);

    s << x << T(' ')
      << y << T(' ')
      << w << T(' ')
      << h;

    return s;
}

const Rectangle Rectangle::fromString (const String& stringVersion)
{
    StringArray toks;
    toks.addTokens (stringVersion.trim(), T(",; \t\r\n"), 0);

    return Rectangle (toks[0].trim().getIntValue(),
                      toks[1].trim().getIntValue(),
                      toks[2].trim().getIntValue(),
                      toks[3].trim().getIntValue());
}


END_JUCE_NAMESPACE
