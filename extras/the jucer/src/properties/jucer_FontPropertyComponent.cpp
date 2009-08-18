/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucer_Headers.h"
#include "jucer_FontPropertyComponent.h"


//==============================================================================
class FontList  : public DeletedAtShutdown
{
public:
    FontList()
    {
        OwnedArray<Font> fonts;
        Font::findFonts (fonts);

        for (int i = 0; i < fonts.size(); ++i)
            fontNames.add (fonts[i]->getTypefaceName());
    }

    ~FontList()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (FontList)

    StringArray fontNames;
};

juce_ImplementSingleton_SingleThreaded (FontList)

void FontPropertyComponent::preloadAllFonts()
{
    FontList::getInstance();
}

//==============================================================================
const String FontPropertyComponent::defaultFont (T("Default font"));
const String FontPropertyComponent::defaultSans (T("Default sans-serif font"));
const String FontPropertyComponent::defaultSerif (T("Default serif font"));
const String FontPropertyComponent::defaultMono (T("Default monospaced font"));

FontPropertyComponent::FontPropertyComponent (const String& name)
    : ChoicePropertyComponent (name)
{
    choices.add (defaultFont);
    choices.add (defaultSans);
    choices.add (defaultSerif);
    choices.add (defaultMono);
    choices.add (String::empty);

    choices.addArray (FontList::getInstance()->fontNames);
}

FontPropertyComponent::~FontPropertyComponent()
{
}

//==============================================================================
void FontPropertyComponent::setIndex (const int newIndex)
{
    String type (choices [newIndex]);

    if (type.isEmpty())
        type = defaultFont;

    if (getTypefaceName() != type)
        setTypefaceName (type);
}

int FontPropertyComponent::getIndex() const
{
    return choices.indexOf (getTypefaceName());
}

const Font FontPropertyComponent::applyNameToFont (const String& typefaceName, const Font& font)
{
    if (typefaceName == defaultFont)
        return Font (font.getHeight(), font.getStyleFlags());
    else if (typefaceName == defaultSans)
        return Font (Font::getDefaultSansSerifFontName(), font.getHeight(), font.getStyleFlags());
    else if (typefaceName == defaultSerif)
        return Font (Font::getDefaultSerifFontName(), font.getHeight(), font.getStyleFlags());
    else if (typefaceName == defaultMono)
        return Font (Font::getDefaultMonospacedFontName(), font.getHeight(), font.getStyleFlags());

    return Font (typefaceName, font.getHeight(), font.getStyleFlags());
}

const String FontPropertyComponent::getTypefaceNameCode (const String& typefaceName)
{
    if (typefaceName == defaultFont)
        return String::empty;
    else if (typefaceName == defaultSans)
        return T("Font::getDefaultSansSerifFontName(), ");
    else if (typefaceName == defaultSerif)
        return T("Font::getDefaultSerifFontName(), ");
    else if (typefaceName == defaultMono)
        return T("Font::getDefaultMonospacedFontName(), ");

    return T("T(\"") + typefaceName + T("\"), ");
}

const String FontPropertyComponent::getFontStyleCode (const Font& font)
{
    if (font.isBold() && font.isItalic())
        return "Font::bold | Font::italic";
    else if (font.isBold())
        return "Font::bold";
    else if (font.isItalic())
        return "Font::italic";

    return "Font::plain";
}

const String FontPropertyComponent::getCompleteFontCode (const Font& font, const String& typefaceName)
{
    return T("Font (")
        + getTypefaceNameCode (typefaceName)
        + valueToFloat (font.getHeight())
        + T(", ")
        + getFontStyleCode (font)
        + T(")");
}
