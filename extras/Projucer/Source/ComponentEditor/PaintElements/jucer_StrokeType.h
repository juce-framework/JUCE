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
