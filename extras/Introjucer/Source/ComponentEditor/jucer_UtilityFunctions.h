/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_UTILITYFUNCTIONS_H_INCLUDED
#define JUCER_UTILITYFUNCTIONS_H_INCLUDED

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
        return "TRANS(" + lit + ")";

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
    RectangleList<int> r (Rectangle<int> (0, 0, w, h));
    r.subtract (Rectangle<int> (1, 1, w - 2, h - 2));

    const int size = jmin (w / 3, h / 3, 12);
    r.subtract (Rectangle<int> (size, 0, w - size - size, h));
    r.subtract (Rectangle<int> (0, size, w, h - size - size));

    g.setColour (Colours::darkgrey);

    for (int i = r.getNumRectangles(); --i >= 0;)
        g.fillRect (r.getRectangle (i));
}

#endif   // JUCER_UTILITYFUNCTIONS_H_INCLUDED
