/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_LocalisedStrings.h"


//==============================================================================
LocalisedStrings::LocalisedStrings (const String& fileContents) throw()
{
    loadFromText (fileContents);
}

LocalisedStrings::LocalisedStrings (const File& fileToLoad) throw()
{
    loadFromText (fileToLoad.loadFileAsString());
}

LocalisedStrings::~LocalisedStrings() throw()
{
}

//==============================================================================
const String LocalisedStrings::translate (const String& text) const throw()
{
    return translations.getValue (text, text);
}

static int findCloseQuote (const String& text, int startPos) throw()
{
    tchar lastChar = 0;

    for (;;)
    {
        const tchar c = text [startPos];

        if (c == 0 || (c == T('"') && lastChar != T('\\')))
            break;

        lastChar = c;
        ++startPos;
    }

    return startPos;
}

static const String unescapeString (const String& s) throw()
{
    return s.replace (T("\\\""), T("\""))
            .replace (T("\\\'"), T("\'"))
            .replace (T("\\t"), T("\t"))
            .replace (T("\\r"), T("\r"))
            .replace (T("\\n"), T("\n"));
}

void LocalisedStrings::loadFromText (const String& fileContents) throw()
{
    StringArray lines;
    lines.addLines (fileContents);

    for (int i = 0; i < lines.size(); ++i)
    {
        String line (lines[i].trim());

        if (line.startsWithChar (T('"')))
        {
            int closeQuote = findCloseQuote (line, 1);

            const String originalText (unescapeString (line.substring (1, closeQuote)));

            if (originalText.isNotEmpty())
            {
                const int openingQuote = findCloseQuote (line, closeQuote + 1);
                closeQuote = findCloseQuote (line, openingQuote + 1);

                const String newText (unescapeString (line.substring (openingQuote + 1, closeQuote)));

                if (newText.isNotEmpty())
                    translations.set (originalText, newText);
            }
        }
        else if (line.startsWithIgnoreCase (T("language:")))
        {
            languageName = line.substring (9).trim();
        }
        else if (line.startsWithIgnoreCase (T("countries:")))
        {
            countryCodes.addTokens (line.substring (10).trim(), true);
            countryCodes.trim();
            countryCodes.removeEmptyStrings();
        }
    }
}

//==============================================================================
static CriticalSection currentMappingsLock;
static LocalisedStrings* currentMappings = 0;

void LocalisedStrings::setCurrentMappings (LocalisedStrings* newTranslations) throw()
{
    const ScopedLock sl (currentMappingsLock);

    delete currentMappings;
    currentMappings = newTranslations;
}

LocalisedStrings* LocalisedStrings::getCurrentMappings() throw()
{
    return currentMappings;
}

const String LocalisedStrings::translateWithCurrentMappings (const String& text) throw()
{
    const ScopedLock sl (currentMappingsLock);

    if (currentMappings != 0)
        return currentMappings->translate (text);

    return text;
}

const String LocalisedStrings::translateWithCurrentMappings (const char* text) throw()
{
    return translateWithCurrentMappings (String (text));
}


END_JUCE_NAMESPACE
