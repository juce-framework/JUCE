/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
#define __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__


//==============================================================================
#define JUCE_T(stringLiteral)     (L##stringLiteral)
typedef juce_wchar                tchar;


#if ! JUCE_DONT_DEFINE_MACROS

/** The 'T' macro allows a literal string to be compiled as unicode.

    If you write your string literals in the form T("xyz"), it will be compiled as L"xyz"
    or "xyz", depending on which representation is best for the String class to work with.

    Because the 'T' symbol is occasionally used inside 3rd-party library headers which you
    may need to include after juce.h, you can use the juce_withoutMacros.h file (in
    the juce/src directory) to avoid defining this macro. See the comments in
    juce_withoutMacros.h for more info.
*/
#define T(stringLiteral)            JUCE_T(stringLiteral)

#endif

//==============================================================================
/**
    A set of methods for manipulating characters and character strings, with
    duplicate methods to handle 8-bit and unicode characters.

    These are defined as wrappers around the basic C string handlers, to provide
    a clean, cross-platform layer, (because various platforms differ in the
    range of C library calls that they provide).

    @see String
*/
class JUCE_API  CharacterFunctions
{
public:
    static int length (const char* s) throw();
    static int length (const juce_wchar* s) throw();

    static void copy (char* dest, const char* src, int maxBytes) throw();
    static void copy (juce_wchar* dest, const juce_wchar* src, int maxChars) throw();

    static void copy (juce_wchar* dest, const char* src, int maxChars) throw();
    static void copy (char* dest, const juce_wchar* src, int maxBytes) throw();
    static int bytesRequiredForCopy (const juce_wchar* src) throw();

    static void append (char* dest, const char* src) throw();
    static void append (juce_wchar* dest, const juce_wchar* src) throw();

    static int compare (const char* s1, const char* s2) throw();
    static int compare (const juce_wchar* s1, const juce_wchar* s2) throw();
    static int compare (const juce_wchar* s1, const char* s2) throw();
    static int compare (const char* s1, const juce_wchar* s2) throw();

    static int compare (const char* s1, const char* s2, int maxChars) throw();
    static int compare (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw();

    static int compareIgnoreCase (const char* s1, const char* s2) throw();
    static int compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2) throw();
    static int compareIgnoreCase (const juce_wchar* s1, const char* s2) throw();

    static int compareIgnoreCase (const char* s1, const char* s2, int maxChars) throw();
    static int compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw();

    static const char* find (const char* haystack, const char* needle) throw();
    static const juce_wchar* find (const juce_wchar* haystack, const juce_wchar* needle) throw();

    static int indexOfChar (const char* haystack, char needle, bool ignoreCase) throw();
    static int indexOfChar (const juce_wchar* haystack, juce_wchar needle, bool ignoreCase) throw();

    static int indexOfCharFast (const char* haystack, char needle) throw();
    static int indexOfCharFast (const juce_wchar* haystack, juce_wchar needle) throw();

    static int getIntialSectionContainingOnly (const char* text, const char* allowedChars) throw();
    static int getIntialSectionContainingOnly (const juce_wchar* text, const juce_wchar* allowedChars) throw();

    static int ftime (char* dest, int maxChars, const char* format, const struct tm* tm) throw();
    static int ftime (juce_wchar* dest, int maxChars, const juce_wchar* format, const struct tm* tm) throw();

    static int getIntValue (const char* s) throw();
    static int getIntValue (const juce_wchar* s) throw();

    static int64 getInt64Value (const char* s) throw();
    static int64 getInt64Value (const juce_wchar* s) throw();

    static double getDoubleValue (const char* s) throw();
    static double getDoubleValue (const juce_wchar* s) throw();

    //==============================================================================
    static char toUpperCase (char character) throw();
    static juce_wchar toUpperCase (juce_wchar character) throw();
    static void toUpperCase (char* s) throw();

    static void toUpperCase (juce_wchar* s) throw();
    static bool isUpperCase (char character) throw();
    static bool isUpperCase (juce_wchar character) throw();

    static char toLowerCase (char character) throw();
    static juce_wchar toLowerCase (juce_wchar character) throw();
    static void toLowerCase (char* s) throw();
    static void toLowerCase (juce_wchar* s) throw();
    static bool isLowerCase (char character) throw();
    static bool isLowerCase (juce_wchar character) throw();

    //==============================================================================
    static bool isWhitespace (char character) throw();
    static bool isWhitespace (juce_wchar character) throw();

    static bool isDigit (char character) throw();
    static bool isDigit (juce_wchar character) throw();

    static bool isLetter (char character) throw();
    static bool isLetter (juce_wchar character) throw();

    static bool isLetterOrDigit (char character) throw();
    static bool isLetterOrDigit (juce_wchar character) throw();

    /** Returns 0 to 16 for '0' to 'F", or -1 for characters that aren't a legel
        hex digit.
    */
    static int getHexDigitValue (juce_wchar digit) throw();
};

#endif   // __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
