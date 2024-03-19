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

            for (const auto& font : fonts)
                fontNames.add (font.getTypefaceName());
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
    void setIndex (int newIndex) override
    {
        String type (choices [newIndex]);

        if (type.isEmpty())
            type = getDefaultFont();

        if (getTypefaceName() != type)
            setTypefaceName (type);
    }

    int getIndex() const override
    {
        return choices.indexOf (getTypefaceName());
    }

    static Font applyNameToFont (const String& typefaceName, const Font& font)
    {
        const auto extraKerning = font.getExtraKerningFactor();
        const auto kerned = [extraKerning] (Font f) { return f.withExtraKerningFactor (extraKerning); };

        if (typefaceName == getDefaultFont())  return kerned (FontOptions (font.getHeight(), font.getStyleFlags()));
        if (typefaceName == getDefaultSans())  return kerned (FontOptions (Font::getDefaultSansSerifFontName(), font.getHeight(), font.getStyleFlags()));
        if (typefaceName == getDefaultSerif()) return kerned (FontOptions (Font::getDefaultSerifFontName(), font.getHeight(), font.getStyleFlags()));
        if (typefaceName == getDefaultMono())  return kerned (FontOptions (Font::getDefaultMonospacedFontName(), font.getHeight(), font.getStyleFlags()));

        auto f = kerned (FontOptions { typefaceName, font.getHeight(), font.getStyleFlags() });

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

        if (font.getAvailableStyles().contains (font.getTypefaceStyle()))
            s << "juce::Font::plain).withTypefaceStyle ("
              << CodeHelpers::stringLiteral (font.getTypefaceStyle())
              << ")";
        else
            s << getFontStyleCode (font)
              << ")";

        if (! approximatelyEqual (font.getExtraKerningFactor(), 0.0f))
            s << ".withExtraKerningFactor ("
              << CodeHelpers::floatLiteral (font.getExtraKerningFactor(), 3)
              << ")";

        return s;
    }
};
