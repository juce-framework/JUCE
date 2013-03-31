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

#ifndef JUCER_UTILS_H
#define JUCER_UTILS_H

inline String quotedString (const String& s)
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
                result << quotedString (s1) << " + ";

            result << code;

            if (s2.isNotEmpty())
                result << " + " << quotedString (s2);

            return result;
        }
    }

    return CodeHelpers::stringLiteral (s);
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
                                 const bool isMouseOver)
{
    g.setColour (Colours::orange.withAlpha (isMouseOver ? 0.4f : 0.3f));

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
    RectangleList r (Rectangle<int> (0, 0, w, h));
    r.subtract (Rectangle<int> (1, 1, w - 2, h - 2));

    const int size = jmin (w / 3, h / 3, 12);
    r.subtract (Rectangle<int> (size, 0, w - size - size, h));
    r.subtract (Rectangle<int> (0, size, w, h - size - size));

    g.setColour (Colours::darkgrey);

    for (int i = r.getNumRectangles(); --i >= 0;)
        g.fillRect (r.getRectangle (i));
}

#endif
