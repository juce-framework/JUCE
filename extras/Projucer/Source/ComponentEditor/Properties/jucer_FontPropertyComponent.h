/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
        auto extraKerning = font.getExtraKerningFactor();

        if (typefaceName == getDefaultFont())  return Font (font.getHeight(), font.getStyleFlags()).withExtraKerningFactor (extraKerning);
        if (typefaceName == getDefaultSans())  return Font (Font::getDefaultSansSerifFontName(), font.getHeight(), font.getStyleFlags()).withExtraKerningFactor (extraKerning);
        if (typefaceName == getDefaultSerif()) return Font (Font::getDefaultSerifFontName(), font.getHeight(), font.getStyleFlags()).withExtraKerningFactor (extraKerning);
        if (typefaceName == getDefaultMono())  return Font (Font::getDefaultMonospacedFontName(), font.getHeight(), font.getStyleFlags()).withExtraKerningFactor (extraKerning);

        auto f = Font (typefaceName, font.getHeight(), font.getStyleFlags()).withExtraKerningFactor (extraKerning);

        if (f.getAvailableStyles().contains (font.getTypefaceStyle()))
            f.setTypefaceStyle (font.getTypefaceStyle());

        return f;
    }

    static String getTypefaceNameCode (const String& typefaceName)
    {
        if (typefaceName == getDefaultFont())   return {};
        if (typefaceName == getDefaultSans())   return "juce::Font::getDefaultSansSerifFontName(), ";
        if (typefaceName == getDefaultSerif())  return "juce::Font::getDefaultSerifFontName(), ";
        if (typefaceName == getDefaultMono())   return "juce::Font::getDefaultMonospacedFontName(), ";

        return "\"" + typefaceName + "\", ";
    }

    static String getFontStyleCode (const Font& font)
    {
        if (font.isBold() && font.isItalic())   return "juce::Font::bold | juce::Font::italic";
        if (font.isBold())                      return "juce::Font::bold";
        if (font.isItalic())                    return "juce::Font::italic";

        return "juce::Font::plain";
    }

    static String getCompleteFontCode (const Font& font, const String& typefaceName)
    {
        String s;

        s << "juce::Font ("
          << getTypefaceNameCode (typefaceName)
          << CodeHelpers::floatLiteral (font.getHeight(), 2)
          << ", ";

        if (font.getAvailableStyles().contains(font.getTypefaceStyle()))
            s << "juce::Font::plain).withTypefaceStyle ("
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
