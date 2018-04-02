/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

LocalizedStrings::LocalizedStrings (const String& fileContents, bool ignoreCase)
{
    loadFromText (fileContents, ignoreCase);
}

LocalizedStrings::LocalizedStrings (const File& fileToLoad, bool ignoreCase)
{
    loadFromText (fileToLoad.loadFileAsString(), ignoreCase);
}

LocalizedStrings::LocalizedStrings (const LocalizedStrings& other)
    : languageName (other.languageName), countryCodes (other.countryCodes),
      translations (other.translations), fallback (createCopyIfNotNull (other.fallback.get()))
{
}

LocalizedStrings& LocalizedStrings::operator= (const LocalizedStrings& other)
{
    languageName = other.languageName;
    countryCodes = other.countryCodes;
    translations = other.translations;
    fallback.reset (createCopyIfNotNull (other.fallback.get()));
    return *this;
}

LocalizedStrings::~LocalizedStrings()
{
}

//==============================================================================
String LocalizedStrings::translate (const String& text) const
{
    if (fallback != nullptr && ! translations.containsKey (text))
        return fallback->translate (text);

    return translations.getValue (text, text);
}

String LocalizedStrings::translate (const String& text, const String& resultIfNotFound) const
{
    if (fallback != nullptr && ! translations.containsKey (text))
        return fallback->translate (text, resultIfNotFound);

    return translations.getValue (text, resultIfNotFound);
}

namespace
{
   #if JUCE_CHECK_MEMORY_LEAKS
    // By using this object to force a LocalizedStrings object to be created
    // before the currentMappings object, we can force the static order-of-destruction to
    // delete the currentMappings object first, which avoids a bogus leak warning.
    // (Oddly, just creating a LocalizedStrings on the stack doesn't work in gcc, it
    // has to be created with 'new' for this to work..)
    struct LeakAvoidanceTrick
    {
        LeakAvoidanceTrick()
        {
            const ScopedPointer<LocalizedStrings> dummy (new LocalizedStrings (String(), false));
        }
    };

    LeakAvoidanceTrick leakAvoidanceTrick;
   #endif

    SpinLock currentMappingsLock;
    ScopedPointer<LocalizedStrings> currentMappings;

    static int findCloseQuote (const String& text, int startPos)
    {
        juce_wchar lastChar = 0;
        auto t = text.getCharPointer() + startPos;

        for (;;)
        {
            auto c = t.getAndAdvance();

            if (c == 0 || (c == '"' && lastChar != '\\'))
                break;

            lastChar = c;
            ++startPos;
        }

        return startPos;
    }

    static String unescapeString (const String& s)
    {
        return s.replace ("\\\"", "\"")
                .replace ("\\\'", "\'")
                .replace ("\\t", "\t")
                .replace ("\\r", "\r")
                .replace ("\\n", "\n");
    }
}

void LocalizedStrings::loadFromText (const String& fileContents, bool ignoreCase)
{
    translations.setIgnoresCase (ignoreCase);

    StringArray lines;
    lines.addLines (fileContents);

    for (auto& l : lines)
    {
        auto line = l.trim();

        if (line.startsWithChar ('"'))
        {
            auto closeQuote = findCloseQuote (line, 1);
            auto originalText = unescapeString (line.substring (1, closeQuote));

            if (originalText.isNotEmpty())
            {
                auto openingQuote = findCloseQuote (line, closeQuote + 1);
                closeQuote = findCloseQuote (line, openingQuote + 1);
                auto newText = unescapeString (line.substring (openingQuote + 1, closeQuote));

                if (newText.isNotEmpty())
                    translations.set (originalText, newText);
            }
        }
        else if (line.startsWithIgnoreCase ("language:"))
        {
            languageName = line.substring (9).trim();
        }
        else if (line.startsWithIgnoreCase ("countries:"))
        {
            countryCodes.addTokens (line.substring (10).trim(), true);
            countryCodes.trim();
            countryCodes.removeEmptyStrings();
        }
    }

    translations.minimizeStorageOverheads();
}

void LocalizedStrings::addStrings (const LocalizedStrings& other)
{
    jassert (languageName == other.languageName);
    jassert (countryCodes == other.countryCodes);

    translations.addArray (other.translations);
}

void LocalizedStrings::setFallback (LocalizedStrings* f)
{
    fallback.reset (f);
}

//==============================================================================
void LocalizedStrings::setCurrentMappings (LocalizedStrings* newTranslations)
{
    const SpinLock::ScopedLockType sl (currentMappingsLock);
    currentMappings.reset (newTranslations);
}

LocalizedStrings* LocalizedStrings::getCurrentMappings()
{
    return currentMappings.get();
}

String LocalizedStrings::translateWithCurrentMappings (const String& text)  { return juce::translate (text); }
String LocalizedStrings::translateWithCurrentMappings (const char* text)    { return juce::translate (text); }

JUCE_API String translate (const String& text)       { return juce::translate (text, text); }
JUCE_API String translate (const char* text)         { return juce::translate (String (text)); }
JUCE_API String translate (CharPointer_UTF8 text)    { return juce::translate (String (text)); }

JUCE_API String translate (const String& text, const String& resultIfNotFound)
{
    const SpinLock::ScopedLockType sl (currentMappingsLock);

    if (auto* mappings = LocalizedStrings::getCurrentMappings())
        return mappings->translate (text, resultIfNotFound);

    return resultIfNotFound;
}

} // namespace juce
