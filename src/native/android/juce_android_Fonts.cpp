/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
const StringArray Font::findAllTypefaceNames()
{
    StringArray results;

    // TODO

    return results;
}

void Font::getPlatformDefaultFontNames (String& defaultSans, String& defaultSerif, String& defaultFixed, String& defaultFallback)
{
    // TODO
    defaultSans     = "Verdana";
    defaultSerif    = "Times";
    defaultFixed    = "Lucida Console";
    defaultFallback = "Tahoma";
}

//==============================================================================
class AndroidTypeface   : public Typeface
{
public:
    AndroidTypeface (const Font& font)
        : Typeface (font.getTypefaceName())
    {
        // TODO
    }

    float getAscent() const
    {
        return 0;  // TODO
    }

    float getDescent() const
    {
        return 0;  // TODO
    }

    float getStringWidth (const String& text)
    {
        // TODO
        return 0;
    }

    void getGlyphPositions (const String& text, Array<int>& glyphs, Array<float>& xOffsets)
    {
        // TODO
    }

    bool getOutlineForGlyph (int glyphNumber, Path& destPath)
    {
        // TODO
        return false;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidTypeface);
};

//==============================================================================
const Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new AndroidTypeface (font);
}


#endif
