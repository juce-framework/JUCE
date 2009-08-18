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

#ifndef __JUCER_FONTPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCER_FONTPROPERTYCOMPONENT_JUCEHEADER__


//==============================================================================
/**
*/
class FontPropertyComponent    : public ChoicePropertyComponent
{
public:
    FontPropertyComponent (const String& name);
    ~FontPropertyComponent();

    //==============================================================================
    static const String defaultFont;
    static const String defaultSans;
    static const String defaultSerif;
    static const String defaultMono;

    static void preloadAllFonts();

    virtual void setTypefaceName (const String& newFontName) = 0;
    virtual const String getTypefaceName() const = 0;

    static const Font applyNameToFont (const String& typefaceName, const Font& font);
    static const String getTypefaceNameCode (const String& typefaceName);
    static const String getFontStyleCode (const Font& font);

    static const String getCompleteFontCode (const Font& font, const String& typefaceName);

    //==============================================================================
    juce_UseDebuggingNewOperator

    void setIndex (const int newIndex);
    int getIndex() const;
};




#endif   // __JUCER_FONTPROPERTYCOMPONENT_JUCEHEADER__
