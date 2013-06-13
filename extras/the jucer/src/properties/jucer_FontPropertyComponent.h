/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
    virtual String getTypefaceName() const = 0;

    static const Font applyNameToFont (const String& typefaceName, const Font& font);
    static const String getTypefaceNameCode (const String& typefaceName);
    static const String getFontStyleCode (const Font& font);

    static const String getCompleteFontCode (const Font& font, const String& typefaceName);

    //==============================================================================
    void setIndex (int newIndex);
    int getIndex() const;
};




#endif   // __JUCER_FONTPROPERTYCOMPONENT_JUCEHEADER__
