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

#include "juce_BorderSize.h"


//==============================================================================
BorderSize::BorderSize() throw()
    : top (0),
      left (0),
      bottom (0),
      right (0)
{
}

BorderSize::BorderSize (const BorderSize& other) throw()
    : top (other.top),
      left (other.left),
      bottom (other.bottom),
      right (other.right)
{
}

BorderSize::BorderSize (const int topGap,
                        const int leftGap,
                        const int bottomGap,
                        const int rightGap) throw()
    : top (topGap),
      left (leftGap),
      bottom (bottomGap),
      right (rightGap)
{
}

BorderSize::BorderSize (const int allGaps) throw()
    : top (allGaps),
      left (allGaps),
      bottom (allGaps),
      right (allGaps)
{
}

BorderSize::~BorderSize() throw()
{
}

//==============================================================================
void BorderSize::setTop (const int newTopGap) throw()
{
    top = newTopGap;
}

void BorderSize::setLeft (const int newLeftGap) throw()
{
    left = newLeftGap;
}

void BorderSize::setBottom (const int newBottomGap) throw()
{
    bottom = newBottomGap;
}

void BorderSize::setRight (const int newRightGap) throw()
{
    right = newRightGap;
}

//==============================================================================
const Rectangle BorderSize::subtractedFrom (const Rectangle& r) const throw()
{
    return Rectangle (r.getX() + left,
                      r.getY() + top,
                      r.getWidth() - (left + right),
                      r.getHeight() - (top + bottom));
}

void BorderSize::subtractFrom (Rectangle& r) const throw()
{
    r.setBounds (r.getX() + left,
                 r.getY() + top,
                 r.getWidth() - (left + right),
                 r.getHeight() - (top + bottom));
}

const Rectangle BorderSize::addedTo (const Rectangle& r) const throw()
{
    return Rectangle (r.getX() - left,
                      r.getY() - top,
                      r.getWidth() + (left + right),
                      r.getHeight() + (top + bottom));
}

void BorderSize::addTo (Rectangle& r) const throw()
{
    r.setBounds (r.getX() - left,
                 r.getY() - top,
                 r.getWidth() + (left + right),
                 r.getHeight() + (top + bottom));
}

bool BorderSize::operator== (const BorderSize& other) const throw()
{
    return top == other.top
        && left == other.left
        && bottom == other.bottom
        && right == other.right;
}

bool BorderSize::operator!= (const BorderSize& other) const throw()
{
    return ! operator== (other);
}


END_JUCE_NAMESPACE
