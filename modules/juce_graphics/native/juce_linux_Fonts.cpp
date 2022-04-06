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

namespace juce
{

static std::unique_ptr<XmlElement> findFontsConfFile()
{
    static const char* pathsToSearch[] = { "/etc/fonts/fonts.conf",
                                           "/usr/share/fonts/fonts.conf",
                                           "/usr/local/etc/fonts/fonts.conf" };

    for (auto* path : pathsToSearch)
        if (auto xml = parseXML (File (path)))
            return xml;

    return {};
}

StringArray FTTypefaceList::getDefaultFontDirectories()
{
    StringArray fontDirs;

    fontDirs.addTokens (String (CharPointer_UTF8 (getenv ("JUCE_FONT_PATH"))), ";,", "");
    fontDirs.removeEmptyStrings (true);

    if (fontDirs.isEmpty())
    {
        if (auto fontsInfo = findFontsConfFile())
        {
            for (auto* e : fontsInfo->getChildWithTagNameIterator ("dir"))
            {
                auto fontPath = e->getAllSubText().trim();

                if (fontPath.isNotEmpty())
                {
                    if (e->getStringAttribute ("prefix") == "xdg")
                    {
                        auto xdgDataHome = SystemStats::getEnvironmentVariable ("XDG_DATA_HOME", {});

                        if (xdgDataHome.trimStart().isEmpty())
                            xdgDataHome = "~/.local/share";

                        fontPath = File (xdgDataHome).getChildFile (fontPath).getFullPathName();
                    }

                    fontDirs.add (fontPath);
                }
            }
        }
    }

    if (fontDirs.isEmpty())
        fontDirs.add ("/usr/X11R6/lib/X11/fonts");

    fontDirs.removeDuplicates (false);
    return fontDirs;
}

Typeface::Ptr Typeface::createSystemTypefaceFor (const Font& font)
{
    return new FreeTypeTypeface (font);
}

Typeface::Ptr Typeface::createSystemTypefaceFor (const void* data, size_t dataSize)
{
    return new FreeTypeTypeface (data, dataSize);
}

void Typeface::scanFolderForFonts (const File& folder)
{
    FTTypefaceList::getInstance()->scanFontPaths (StringArray (folder.getFullPathName()));
}

StringArray Font::findAllTypefaceNames()
{
    return FTTypefaceList::getInstance()->findAllFamilyNames();
}

StringArray Font::findAllTypefaceStyles (const String& family)
{
    return FTTypefaceList::getInstance()->findAllTypefaceStyles (family);
}

bool TextLayout::createNativeLayout (const AttributedString&)
{
    return false;
}

//==============================================================================
struct DefaultFontInfo
{
    struct Characteristics
    {
        String name, style;
    };

    DefaultFontInfo()
        : defaultSans  (getDefaultSansSerifFontCharacteristics()),
          defaultSerif (getDefaultSerifFontCharacteristics()),
          defaultFixed (getDefaultMonospacedFontCharacteristics())
    {
    }

    Characteristics getRealFontCharacteristics (const String& faceName) const
    {
        if (faceName == Font::getDefaultSansSerifFontName())    return defaultSans;
        if (faceName == Font::getDefaultSerifFontName())        return defaultSerif;
        if (faceName == Font::getDefaultMonospacedFontName())   return defaultFixed;

        return { faceName };
    }

    Characteristics defaultSans, defaultSerif, defaultFixed;

private:
    template <typename Range>
    static Characteristics pickBestFont (const StringArray& names, Range&& choicesArray)
    {
        for (auto& choice : choicesArray)
            if (names.contains (choice.name, true))
                return choice;

        for (auto& choice : choicesArray)
            for (auto& name : names)
                if (name.startsWithIgnoreCase (choice.name))
                    return { name, choice.style };

        for (auto& choice : choicesArray)
            for (auto& name : names)
                if (name.containsIgnoreCase (choice.name))
                    return { name, choice.style };

        return { *names.begin() };
    }

    static Characteristics getDefaultSansSerifFontCharacteristics()
    {
        StringArray allFonts;
        FTTypefaceList::getInstance()->getSansSerifNames (allFonts);

        static const Characteristics targets[] { { "Verdana" },
                                                 { "Bitstream Vera Sans", "Roman" },
                                                 { "Luxi Sans" },
                                                 { "Liberation Sans" },
                                                 { "DejaVu Sans" },
                                                 { "Sans" } };
        return pickBestFont (allFonts, targets);
    }

    static Characteristics getDefaultSerifFontCharacteristics()
    {
        StringArray allFonts;
        FTTypefaceList::getInstance()->getSerifNames (allFonts);

        static const Characteristics targets[] { { "Bitstream Vera Serif", "Roman" },
                                                 { "Times" },
                                                 { "Nimbus Roman" },
                                                 { "Liberation Serif" },
                                                 { "DejaVu Serif" },
                                                 { "Serif" } };
        return pickBestFont (allFonts, targets);
    }

    static Characteristics getDefaultMonospacedFontCharacteristics()
    {
        StringArray allFonts;
        FTTypefaceList::getInstance()->getMonospacedNames (allFonts);

        static const Characteristics targets[] { { "DejaVu Sans Mono" },
                                                 { "Bitstream Vera Sans Mono", "Roman" },
                                                 { "Sans Mono" },
                                                 { "Liberation Mono" },
                                                 { "Courier" },
                                                 { "DejaVu Mono" },
                                                 { "Mono" } };
        return pickBestFont (allFonts, targets);
    }

    JUCE_DECLARE_NON_COPYABLE (DefaultFontInfo)
};

Typeface::Ptr Font::getDefaultTypefaceForFont (const Font& font)
{
    static const DefaultFontInfo defaultInfo;

    Font f (font);

    const auto characteristics = defaultInfo.getRealFontCharacteristics (font.getTypefaceName());
    f.setTypefaceName (characteristics.name);
    f.setTypefaceStyle (characteristics.style);

    return Typeface::createSystemTypefaceFor (f);
}

} // namespace juce
