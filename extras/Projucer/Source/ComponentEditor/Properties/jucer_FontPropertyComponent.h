/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class FontPropertyComponent    : public ChoicePropertyComponent
{
public:
    FontPropertyComponent (const String& name)
        : ChoicePropertyComponent (name)
    {
        choices.add (getDefaultFont());
        choices.add (getDefaultSans());
        choices.add (getDefaultSerif());
        choices.add (getDefaultMono());
        choices.add (String());

        static StringArray fontNames;

        if (fontNames.size() == 0)
        {
            Array<Font> fonts;
            Font::findFonts (fonts);

            for (int i = 0; i < fonts.size(); ++i)
                fontNames.add (fonts[i].getTypefaceName());
        }

        choices.addArray (fontNames);
    }

    static String getDefaultFont()  { return "Default font"; }
    static String getDefaultSans()  { return "Default sans-serif font"; }
    static String getDefaultSerif() { return "Default serif font"; }
    static String getDefaultMono()  { return "Default monospaced font"; }

    //==============================================================================
    virtual void setTypefaceName (const String& newFontName) = 0;
    virtual String getTypefaceName() const = 0;

    //==============================================================================
    void setIndex (int newIndex)
    {
        String type (choices [newIndex]);

        if (type.isEmpty())
            type = getDefaultFont();

        if (getTypefaceName() != type)
            setTypefaceName (type);
    }

    int getIndex() const
    {
        return choices.indexOf (getTypefaceName());
    }

    static Font applyNameToFont (const String& typefaceName, const Font& font)
    {
        if (typefaceName == getDefaultFont())  return Font (font.getHeight(), font.getStyleFlags());
        if (typefaceName == getDefaultSans())  return Font (Font::getDefaultSansSerifFontName(), font.getHeight(), font.getStyleFlags());
        if (typefaceName == getDefaultSerif()) return Font (Font::getDefaultSerifFontName(), font.getHeight(), font.getStyleFlags());
        if (typefaceName == getDefaultMono())  return Font (Font::getDefaultMonospacedFontName(), font.getHeight(), font.getStyleFlags());

        auto f = Font (typefaceName, font.getHeight(), font.getStyleFlags()).withExtraKerningFactor (font.getExtraKerningFactor());
        if (f.getAvailableStyles().contains (font.getTypefaceStyle()))
        {
            f.setTypefaceStyle (font.getTypefaceStyle());
        }
        return f;
    }

    static String getTypefaceNameCode (const String& typefaceName)
    {
        if (typefaceName == getDefaultFont())   return {};
        if (typefaceName == getDefaultSans())   return "Font::getDefaultSansSerifFontName(), ";
        if (typefaceName == getDefaultSerif())  return "Font::getDefaultSerifFontName(), ";
        if (typefaceName == getDefaultMono())   return "Font::getDefaultMonospacedFontName(), ";

        return "\"" + typefaceName + "\", ";
    }

    static String getFontStyleCode (const Font& font)
    {
        if (font.isBold() && font.isItalic())   return "Font::bold | Font::italic";
        if (font.isBold())                      return "Font::bold";
        if (font.isItalic())                    return "Font::italic";

        return "Font::plain";
    }

    static String getCompleteFontCode (const Font& font, const String& typefaceName)
    {
        String s;

        s << "Font ("
          << getTypefaceNameCode (typefaceName)
          << CodeHelpers::floatLiteral (font.getHeight(), 2)
          << ", ";

        if (font.getAvailableStyles().contains(font.getTypefaceStyle()))
            s << "Font::plain).withTypefaceStyle ("
              << CodeHelpers::stringLiteral (font.getTypefaceStyle())
              << ")";
        else
            s << getFontStyleCode (font)
              << ")";

        if (font.getExtraKerningFactor() != 0.0f)
            s << ".withExtraKerningFactor ("
              << CodeHelpers::floatLiteral (font.getExtraKerningFactor(), 3)
              << ")";

        return s;
    }
};
