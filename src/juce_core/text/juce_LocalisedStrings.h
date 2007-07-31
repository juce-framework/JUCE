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

#ifndef __JUCE_LOCALISEDSTRINGS_JUCEHEADER__
#define __JUCE_LOCALISEDSTRINGS_JUCEHEADER__

#include "juce_StringPairArray.h"
#include "../basics/juce_Singleton.h"
#include "../io/files/juce_File.h"


//==============================================================================
/** Used in the same way as the T(text) macro, this will attempt to translate a
    string into a localised version using the LocalisedStrings class.

    @see LocalisedStrings
*/
#define TRANS(stringLiteral)     \
    LocalisedStrings::translateWithCurrentMappings (stringLiteral)



//==============================================================================
/**
    Used to convert strings to localised foreign-language versions.

    This is basically a look-up table of strings and their translated equivalents.
    It can be loaded from a text file, so that you can supply a set of localised
    versions of strings that you use in your app.

    To use it in your code, simply call the translate() method on each string that
    might have foreign versions, and if none is found, the method will just return
    the original string.

    The translation file should start with some lines specifying a description of
    the language it contains, and also a list of ISO country codes where it might
    be appropriate to use the file. After that, each line of the file should contain
    a pair of quoted strings with an '=' sign.

    E.g. for a french translation, the file might be:

    @code
    language: French
    countries: fr be mc ch lu

    "hello" = "bonjour"
    "goodbye" = "au revoir"
    @endcode

    If the strings need to contain a quote character, they can use '\"' instead, and
    if the first non-whitespace character on a line isn't a quote, then it's ignored,
    (you can use this to add comments).

    Note that this is a singleton class, so don't create or destroy the object directly.
    There's also a TRANS(text) macro defined to make it easy to use the this.

    E.g. @code
    printSomething (TRANS("hello"));
    @endcode

    This macro is used in the Juce classes themselves, so your application has a chance to
    intercept and translate any internal Juce text strings that might be shown. (You can easily
    get a list of all the messages by searching for the TRANS() macro in the Juce source
    code).
*/
class JUCE_API  LocalisedStrings
{
public:
    //==============================================================================
    /** Creates a set of translations from the text of a translation file.

        When you create one of these, you can call setCurrentMappings() to make it
        the set of mappings that the system's using.
    */
    LocalisedStrings (const String& fileContents) throw();

    /** Creates a set of translations from a file.

        When you create one of these, you can call setCurrentMappings() to make it
        the set of mappings that the system's using.
    */
    LocalisedStrings (const File& fileToLoad) throw();

    /** Destructor. */
    ~LocalisedStrings() throw();

    //==============================================================================
    /** Selects the current set of mappings to be used by the system.

        The object you pass in will be automatically deleted when no longer needed, so
        don't keep a pointer to it. You can also pass in zero to remove the current
        mappings.

        See also the TRANS() macro, which uses the current set to do its translation.

        @see translateWithCurrentMappings
    */
    static void setCurrentMappings (LocalisedStrings* newTranslations) throw();

    /** Returns the currently selected set of mappings.

        This is the object that was last passed to setCurrentMappings(). It may
        be 0 if none has been created.
    */
    static LocalisedStrings* getCurrentMappings() throw();

    /** Tries to translate a string using the currently selected set of mappings.

        If no mapping has been set, or if the mapping doesn't contain a translation
        for the string, this will just return the original string.

        See also the TRANS() macro, which uses this method to do its translation.

        @see setCurrentMappings, getCurrentMappings
    */
    static const String translateWithCurrentMappings (const String& text) throw();

    /** Tries to translate a string using the currently selected set of mappings.

        If no mapping has been set, or if the mapping doesn't contain a translation
        for the string, this will just return the original string.

        See also the TRANS() macro, which uses this method to do its translation.

        @see setCurrentMappings, getCurrentMappings
    */
    static const String translateWithCurrentMappings (const char* text) throw();

    //==============================================================================
    /** Attempts to look up a string and return its localised version.

        If the string isn't found in the list, the original string will be returned.
    */
    const String translate (const String& text) const throw();

    /** Returns the name of the language specified in the translation file.

        This is specified in the file using a line starting with "language:", e.g.
        @code
        language: german
        @endcode
    */
    const String getLanguageName() const throw()                { return languageName; }

    /** Returns the list of suitable country codes listed in the translation file.

        These is specified in the file using a line starting with "countries:", e.g.
        @code
        countries: fr be mc ch lu
        @endcode

        The country codes are supposed to be 2-character ISO complient codes.
    */
    const StringArray getCountryCodes() const throw()           { return countryCodes; }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String languageName;
    StringArray countryCodes;
    StringPairArray translations;

    void loadFromText (const String& fileContents) throw();
};


#endif   // __JUCE_LOCALISEDSTRINGS_JUCEHEADER__
