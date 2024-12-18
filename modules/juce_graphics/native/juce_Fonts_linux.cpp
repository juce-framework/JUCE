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

namespace juce
{

static std::unique_ptr<XmlElement> findFontsConfFile()
{
    static const char* pathsToSearch[] = { "/etc/fonts/fonts.conf",
                                           "/usr/share/fonts/fonts.conf",
                                           "/usr/local/etc/fonts/fonts.conf",
                                           "/usr/share/defaults/fonts/fonts.conf" };

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

//==============================================================================
struct DefaultFontInfo
{
    DefaultFontInfo()
        : defaultSans  (getDefaultSansSerifFontName()),
          defaultSerif (getDefaultSerifFontName()),
          defaultFixed (getDefaultMonospacedFontName())
    {
    }

    String getRealFontName (const String& faceName) const
    {
        if (faceName == Font::getDefaultSansSerifFontName())    return defaultSans;
        if (faceName == Font::getDefaultSerifFontName())        return defaultSerif;
        if (faceName == Font::getDefaultMonospacedFontName())   return defaultFixed;

        return faceName;
    }

    String defaultSans, defaultSerif, defaultFixed;

private:
    template <typename Range>
    static String pickBestFont (const StringArray& names, Range&& choicesArray)
    {
        for (auto& choice : choicesArray)
            if (names.contains (choice, true))
                return choice;

        for (auto& choice : choicesArray)
            for (auto& name : names)
                if (name.startsWithIgnoreCase (choice))
                    return name;

        for (auto& choice : choicesArray)
            for (auto& name : names)
                if (name.containsIgnoreCase (choice))
                    return name;

        for (auto& name : names)
            if (name.isNotEmpty())
                return name;

        jassertfalse;
        return {};
    }

    static String getDefaultSansSerifFontName()
    {
        StringArray allFonts;
        FTTypefaceList::getInstance()->getSansSerifNames (allFonts);

        static constexpr const char* targets[] { "Verdana",
                                                 "Bitstream Vera Sans",
                                                 "Luxi Sans",
                                                 "Liberation Sans",
                                                 "DejaVu Sans",
                                                 "Sans" };
        return pickBestFont (allFonts, targets);
    }

    static String getDefaultSerifFontName()
    {
        StringArray allFonts;
        FTTypefaceList::getInstance()->getSerifNames (allFonts);

        static constexpr const char* targets[] { "Bitstream Vera Serif",
                                                 "Times",
                                                 "Nimbus Roman",
                                                 "Liberation Serif",
                                                 "DejaVu Serif",
                                                 "Serif" };
        return pickBestFont (allFonts, targets);
    }

    static String getDefaultMonospacedFontName()
    {
        StringArray allFonts;
        FTTypefaceList::getInstance()->getMonospacedNames (allFonts);

        static constexpr const char* targets[] { "DejaVu Sans Mono",
                                                 "Bitstream Vera Sans Mono",
                                                 "Sans Mono",
                                                 "Liberation Mono",
                                                 "Courier",
                                                 "DejaVu Mono",
                                                 "Mono" };
        return pickBestFont (allFonts, targets);
    }

    JUCE_DECLARE_NON_COPYABLE (DefaultFontInfo)
};

Typeface::Ptr Font::Native::getDefaultPlatformTypefaceForFont (const Font& font)
{
    static const DefaultFontInfo defaultInfo;

    Font f (font);

    const auto name = font.getTypefaceName();
    const auto realName = defaultInfo.getRealFontName (name);

    if (realName.isEmpty())
        return nullptr;

    f.setTypefaceName (realName);
    return Typeface::createSystemTypefaceFor (f);
}

} // namespace juce
