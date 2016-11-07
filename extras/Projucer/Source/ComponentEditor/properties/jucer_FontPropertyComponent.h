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

#ifndef JUCER_FONTPROPERTYCOMPONENT_H_INCLUDED
#define JUCER_FONTPROPERTYCOMPONENT_H_INCLUDED


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

        return Font (typefaceName, font.getHeight(), font.getStyleFlags());
    }

    static String getTypefaceNameCode (const String& typefaceName)
    {
        if (typefaceName == getDefaultFont())   return String();
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
        return "Font ("
            + getTypefaceNameCode (typefaceName)
            + CodeHelpers::floatLiteral (font.getHeight(), 2)
            + ", "
            + getFontStyleCode (font)
            + ")";
    }
};


#endif   // JUCER_FONTPROPERTYCOMPONENT_H_INCLUDED
