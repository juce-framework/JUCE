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

#include "../../jucer_Headers.h"
#include "jucer_StrokeType.h"


//==============================================================================
StrokeType::StrokeType()
    : stroke (1.0f)
{
    reset();
}

StrokeType::~StrokeType()
{
}

void StrokeType::reset()
{
    stroke = PathStrokeType (5.0f);
    fill = FillType();
    fill.colour = Colours::black;
}

const String StrokeType::getPathStrokeCode() const
{
    PathStrokeType defaultStroke (1.0f);

    String s;

    s << "PathStrokeType (" << valueToFloat (stroke.getStrokeThickness());

    if (stroke.getJointStyle() != defaultStroke.getJointStyle()
        || stroke.getEndStyle() != defaultStroke.getEndStyle())
    {
        s << ", ";

        switch (stroke.getJointStyle())
        {
        case PathStrokeType::mitered:
            s << "PathStrokeType::mitered";
            break;

        case PathStrokeType::curved:
            s << "PathStrokeType::curved";
            break;
        case PathStrokeType::beveled:
            s << "PathStrokeType::beveled";
            break;

        default:
            jassertfalse
            break;
        }

        if (stroke.getEndStyle() != defaultStroke.getEndStyle())
        {
            s << ", ";

            switch (stroke.getEndStyle())
            {
            case PathStrokeType::butt:
                s << "PathStrokeType::butt";
                break;

            case PathStrokeType::square:
                s << "PathStrokeType::square";
                break;
            case PathStrokeType::rounded:
                s << "PathStrokeType::rounded";
                break;

            default:
                jassertfalse
                break;
            }
        }
    }

    s << ")";
    return s;
}

const String StrokeType::toString() const
{
    String s;
    s << stroke.getStrokeThickness();

    switch (stroke.getJointStyle())
    {
    case PathStrokeType::mitered:
        s << ", mitered";
        break;

    case PathStrokeType::curved:
        s << ", curved";
        break;
    case PathStrokeType::beveled:
        s << ", beveled";
        break;

    default:
        jassertfalse
        break;
    }

    switch (stroke.getEndStyle())
    {
    case PathStrokeType::butt:
        s << ", butt";
        break;

    case PathStrokeType::square:
        s << ", square";
        break;
    case PathStrokeType::rounded:
        s << ", rounded";
        break;

    default:
        jassertfalse
        break;
    }

    return s;
}

void StrokeType::restoreFromString (const String& s)
{
    reset();

    if (s.isNotEmpty())
    {
        const float thickness = (float) s.upToFirstOccurrenceOf (T(","), false, false).getDoubleValue();

        PathStrokeType::JointStyle joint = stroke.getJointStyle();

        if (s.containsIgnoreCase (T("miter")))
            joint = PathStrokeType::mitered;
        else if (s.containsIgnoreCase (T("curve")))
            joint = PathStrokeType::curved;
        else if (s.containsIgnoreCase (T("bevel")))
            joint = PathStrokeType::beveled;

        PathStrokeType::EndCapStyle end = stroke.getEndStyle();

        if (s.containsIgnoreCase (T("butt")))
            end = PathStrokeType::butt;
        else if (s.containsIgnoreCase (T("square")))
            end = PathStrokeType::square;
        else if (s.containsIgnoreCase (T("round")))
            end = PathStrokeType::rounded;

        stroke = PathStrokeType (thickness, joint, end);
    }
}

bool StrokeType::isOpaque() const
{
    return fill.isOpaque();
}

bool StrokeType::isInvisible() const
{
    return fill.isInvisible() || stroke.getStrokeThickness() <= 0.0f;
}

bool StrokeType::operator== (const StrokeType& other) const throw()
{
    return stroke == other.stroke
            && fill == other.fill;
}

bool StrokeType::operator!= (const StrokeType& other) const throw()
{
    return ! operator== (other);
}
