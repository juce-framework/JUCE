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

#pragma once

#include "jucer_FillType.h"

//==============================================================================
class StrokeType
{
public:
    StrokeType()  : stroke (1.0f)
    {
        reset();
    }

    String getPathStrokeCode() const
    {
        PathStrokeType defaultStroke (1.0f);

        String s;

        s << "juce::PathStrokeType (" << CodeHelpers::floatLiteral (stroke.getStrokeThickness(), 3);

        if (stroke.getJointStyle() != defaultStroke.getJointStyle()
            || stroke.getEndStyle() != defaultStroke.getEndStyle())
        {
            s << ", ";

            switch (stroke.getJointStyle())
            {
                case PathStrokeType::mitered:   s << "juce::PathStrokeType::mitered"; break;
                case PathStrokeType::curved:    s << "juce::PathStrokeType::curved"; break;
                case PathStrokeType::beveled:   s << "juce::PathStrokeType::beveled"; break;
                default:                        jassertfalse; break;
            }

            if (stroke.getEndStyle() != defaultStroke.getEndStyle())
            {
                s << ", ";

                switch (stroke.getEndStyle())
                {
                    case PathStrokeType::butt:      s << "juce::PathStrokeType::butt"; break;
                    case PathStrokeType::square:    s << "juce::PathStrokeType::square"; break;
                    case PathStrokeType::rounded:   s << "juce::PathStrokeType::rounded"; break;
                    default:                        jassertfalse; break;
                }
            }
        }

        s << ")";
        return s;
    }

    String toString() const
    {
        String s;
        s << stroke.getStrokeThickness();

        switch (stroke.getJointStyle())
        {
            case PathStrokeType::mitered:   s << ", mitered"; break;
            case PathStrokeType::curved:    s << ", curved"; break;
            case PathStrokeType::beveled:   s << ", beveled"; break;
            default:                        jassertfalse; break;
        }

        switch (stroke.getEndStyle())
        {
            case PathStrokeType::butt:      s << ", butt"; break;
            case PathStrokeType::square:    s << ", square"; break;
            case PathStrokeType::rounded:   s << ", rounded"; break;
            default:                        jassertfalse; break;
        }

        return s;
    }

    void restoreFromString (const String& s)
    {
        reset();

        if (s.isNotEmpty())
        {
            const float thickness = (float) s.upToFirstOccurrenceOf (",", false, false).getDoubleValue();

            PathStrokeType::JointStyle joint = stroke.getJointStyle();

            if (s.containsIgnoreCase ("miter"))         joint = PathStrokeType::mitered;
            else if (s.containsIgnoreCase ("curve"))    joint = PathStrokeType::curved;
            else if (s.containsIgnoreCase ("bevel"))    joint = PathStrokeType::beveled;

            PathStrokeType::EndCapStyle end = stroke.getEndStyle();

            if (s.containsIgnoreCase ("butt"))          end = PathStrokeType::butt;
            else if (s.containsIgnoreCase ("square"))   end = PathStrokeType::square;
            else if (s.containsIgnoreCase ("round"))    end = PathStrokeType::rounded;

            stroke = PathStrokeType (thickness, joint, end);
        }
    }

    bool isOpaque() const
    {
        return fill.isOpaque();
    }

    bool isInvisible() const
    {
        return fill.isInvisible() || stroke.getStrokeThickness() <= 0.0f;
    }

    bool operator== (const StrokeType& other) const noexcept
    {
        return stroke == other.stroke && fill == other.fill;
    }

    bool operator!= (const StrokeType& other) const noexcept
    {
        return ! operator== (other);
    }

    //==============================================================================
    PathStrokeType stroke;
    JucerFillType fill;

private:
    void reset()
    {
        stroke = PathStrokeType (5.0f);
        fill = JucerFillType();
        fill.colour = Colours::black;
    }
};
