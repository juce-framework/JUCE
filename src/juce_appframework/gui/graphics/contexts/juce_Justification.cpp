/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_Justification.h"


//==============================================================================
Justification::Justification (const Justification& other) throw()
    : flags (other.flags)
{
}

const Justification& Justification::operator= (const Justification& other) throw()
{
    flags = other.flags;
    return *this;
}

int Justification::getOnlyVerticalFlags() const throw()
{
    return flags & (top | bottom | verticallyCentred);
}

int Justification::getOnlyHorizontalFlags() const throw()
{
    return flags & (left | right | horizontallyCentred | horizontallyJustified);
}

void Justification::applyToRectangle (int& x, int& y,
                                      const int w, const int h,
                                      const int spaceX, const int spaceY,
                                      const int spaceW, const int spaceH) const throw()
{
    if ((flags & horizontallyCentred) != 0)
    {
        x = spaceX + ((spaceW - w) >> 1);
    }
    else if ((flags & right) != 0)
    {
        x = spaceX + spaceW - w;
    }
    else
    {
        x = spaceX;
    }

    if ((flags & verticallyCentred) != 0)
    {
        y = spaceY + ((spaceH - h) >> 1);
    }
    else if ((flags & bottom) != 0)
    {
        y = spaceY + spaceH - h;
    }
    else
    {
        y = spaceY;
    }
}

END_JUCE_NAMESPACE
