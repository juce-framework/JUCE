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


//==============================================================================
inline String quotedString (const String& s, bool wrapInTransMacro)
{
    const int embeddedIndex = s.indexOfIgnoreCase ("%%");

    if (embeddedIndex >= 0)
    {
        String s1 (s.substring (0, embeddedIndex));
        String s2 (s.substring (embeddedIndex + 2));
        String code;

        const int closeIndex = s2.indexOf ("%%");

        if (closeIndex > 0)
        {
            code = s2.substring (0, closeIndex).trim();
            s2 = s2.substring (closeIndex + 2);
        }

        if (code.isNotEmpty())
        {
            String result;

            if (s1.isNotEmpty())
                result << quotedString (s1, wrapInTransMacro) << " + ";

            result << code;

            if (s2.isNotEmpty())
                result << " + " << quotedString (s2, wrapInTransMacro);

            return result;
        }
    }

    String lit (CodeHelpers::stringLiteral (s));

    if (wrapInTransMacro && lit.startsWithChar ('"'))
        return "TRANS (" + lit + ")";

    return lit;
}

inline String castToFloat (const String& expression)
{
    if (expression.containsOnly ("0123456789.f"))
    {
        String s (expression.getFloatValue());

        if (s.containsChar ('.'))
            return s + "f";

        return s + ".0f";
    }

    return "static_cast<float> (" + expression + ")";
}

inline void drawResizableBorder (Graphics& g, int w, int h,
                                 const BorderSize<int> borderSize,
                                 const bool isMouseOver,
                                 Colour borderColour)
{
    ignoreUnused (isMouseOver);
    g.setColour (borderColour);

    g.fillRect (0, 0, w, borderSize.getTop());
    g.fillRect (0, 0, borderSize.getLeft(), h);
    g.fillRect (0, h - borderSize.getBottom(), w, borderSize.getBottom());
    g.fillRect (w - borderSize.getRight(), 0, borderSize.getRight(), h);

    g.drawRect (borderSize.getLeft() - 1, borderSize.getTop() - 1,
                w - borderSize.getRight() - borderSize.getLeft() + 2,
                h - borderSize.getTop() - borderSize.getBottom() + 2);
}

inline void drawMouseOverCorners (Graphics& g, int w, int h)
{
    RectangleList<int> r (Rectangle<int> (0, 0, w, h));
    r.subtract (Rectangle<int> (1, 1, w - 2, h - 2));

    const int size = jmin (w / 3, h / 3, 12);
    r.subtract (Rectangle<int> (size, 0, w - size - size, h));
    r.subtract (Rectangle<int> (0, size, w, h - size - size));

    g.setColour (Colours::black);

    for (int i = r.getNumRectangles(); --i >= 0;)
        g.fillRect (r.getRectangle (i));
}
