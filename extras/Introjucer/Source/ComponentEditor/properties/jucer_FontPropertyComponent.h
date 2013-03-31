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

#ifndef __JUCER_FONTPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCER_FONTPROPERTYCOMPONENT_JUCEHEADER__


class FontPropertyComponent    : public ChoicePropertyComponent
{
public:
    FontPropertyComponent (const String& name)
        : ChoicePropertyComponent (name)
    {
        choices.add (getDefaultFont());
        choices.add (getDefaultSans());
        choices.add (getDefaultSans());
        choices.add (getDefaultMono());
        choices.add (String::empty);

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
        if (typefaceName == getDefaultSans())  return Font (Font::getDefaultSerifFontName(), font.getHeight(), font.getStyleFlags());
        if (typefaceName == getDefaultMono())  return Font (Font::getDefaultMonospacedFontName(), font.getHeight(), font.getStyleFlags());

        return Font (typefaceName, font.getHeight(), font.getStyleFlags());
    }

    static String getTypefaceNameCode (const String& typefaceName)
    {
        if (typefaceName == getDefaultFont())   return String::empty;
        if (typefaceName == getDefaultSans())   return "Font::getDefaultSansSerifFontName(), ";
        if (typefaceName == getDefaultSans())   return "Font::getDefaultSerifFontName(), ";
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
        return "Font ("
            + getTypefaceNameCode (typefaceName)
            + CodeHelpers::floatLiteral (font.getHeight(), 2)
            + ", "
            + getFontStyleCode (font)
            + ")";
    }
};


#endif   // __JUCER_FONTPROPERTYCOMPONENT_JUCEHEADER__
