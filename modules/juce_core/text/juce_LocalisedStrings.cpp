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

LocalisedStrings::LocalisedStrings (const String& fileContents, bool ignoreCase)
{
    loadFromText (fileContents, ignoreCase);
}

LocalisedStrings::LocalisedStrings (const File& fileToLoad, bool ignoreCase)
{
    loadFromText (fileToLoad.loadFileAsString(), ignoreCase);
}

LocalisedStrings::LocalisedStrings (const LocalisedStrings& other)
    : languageName (other.languageName), countryCodes (other.countryCodes),
      translations (other.translations), fallback (createCopyIfNotNull (other.fallback.get()))
{
}

LocalisedStrings& LocalisedStrings::operator= (const LocalisedStrings& other)
{
    languageName = other.languageName;
    countryCodes = other.countryCodes;
    translations = other.translations;
    fallback.reset (createCopyIfNotNull (other.fallback.get()));
    return *this;
}

//==============================================================================
String LocalisedStrings::translate (const String& text) const
{
    if (fallback != nullptr && ! translations.containsKey (text))
        return fallback->translate (text);

    return translations.getValue (text, text);
}

String LocalisedStrings::translate (const String& text, const String& resultIfNotFound) const
{
    if (fallback != nullptr && ! translations.containsKey (text))
        return fallback->translate (text, resultIfNotFound);

    return translations.getValue (text, resultIfNotFound);
}

namespace
{
   #if JUCE_CHECK_MEMORY_LEAKS
    // By using this object to force a LocalisedStrings object to be created
    // before the currentMappings object, we can force the static order-of-destruction to
    // delete the currentMappings object first, which avoids a bogus leak warning.
    // (Oddly, just creating a LocalisedStrings on the stack doesn't work in gcc, it
    // has to be created with 'new' for this to work..)
    struct LeakAvoidanceTrick
    {
        LeakAvoidanceTrick()
        {
            const std::unique_ptr<LocalisedStrings> dummy (new LocalisedStrings (String(), false));
        }
    };

    LeakAvoidanceTrick leakAvoidanceTrick;
   #endif

    SpinLock currentMappingsLock;
    std::unique_ptr<LocalisedStrings> currentMappings;

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

void LocalisedStrings::loadFromText (const String& fileContents, bool ignoreCase)
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

    translations.minimiseStorageOverheads();
}

void LocalisedStrings::addStrings (const LocalisedStrings& other)
{
    jassert (languageName == other.languageName);
    jassert (countryCodes == other.countryCodes);

    translations.addArray (other.translations);
}

void LocalisedStrings::setFallback (LocalisedStrings* f)
{
    fallback.reset (f);
}

//==============================================================================
void LocalisedStrings::setCurrentMappings (LocalisedStrings* newTranslations)
{
    const SpinLock::ScopedLockType sl (currentMappingsLock);
    currentMappings.reset (newTranslations);
}

LocalisedStrings* LocalisedStrings::getCurrentMappings()
{
    return currentMappings.get();
}

String LocalisedStrings::translateWithCurrentMappings (const String& text)  { return juce::translate (text); }
String LocalisedStrings::translateWithCurrentMappings (const char* text)    { return juce::translate (text); }

JUCE_API String translate (const String& text)       { return juce::translate (text, text); }
JUCE_API String translate (const char* text)         { return juce::translate (String (text)); }
JUCE_API String translate (CharPointer_UTF8 text)    { return juce::translate (String (text)); }

JUCE_API String translate (const String& text, const String& resultIfNotFound)
{
    const SpinLock::ScopedLockType sl (currentMappingsLock);

    if (auto* mappings = LocalisedStrings::getCurrentMappings())
        return mappings->translate (text, resultIfNotFound);

    return resultIfNotFound;
}

} // namespace juce
