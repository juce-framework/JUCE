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

#ifndef __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
#define __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__


//==============================================================================
/* The String class can either use wchar_t unicode characters, or 8-bit characters
   (in the default system encoding) as its internal representation.

   To use unicode, define the JUCE_STRINGS_ARE_UNICODE macro in juce_Config.h

   Be sure to use "tchar" for characters rather than "char", and always wrap string
   literals in the T("abcd") macro, so that it all works nicely either way round.
*/
#if JUCE_STRINGS_ARE_UNICODE

  #define JUCE_T(stringLiteral)     (L##stringLiteral)
  typedef juce_wchar                tchar;
  #define juce_tcharToWideChar(c)   (c)

#else

  #define JUCE_T(stringLiteral)     (stringLiteral)
  typedef char                      tchar;
  #define juce_tcharToWideChar(c)   ((juce_wchar) (unsigned char) (c))

#endif

#if ! JUCE_DONT_DEFINE_MACROS

/** The 'T' macro allows a literal string to be compiled using either 8-bit characters
    or unicode.

    If you write your string literals in the form T("xyz"), this will either be compiled
    as "xyz" for non-unicode builds, or L"xyz" for unicode builds, depending on whether the
    JUCE_STRINGS_ARE_UNICODE macro has been set in juce_Config.h

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
    static int length (const char* const s) throw();
    static int length (const juce_wchar* const s) throw();

    static void copy (char* dest, const char* src, const int maxBytes) throw();
    static void copy (juce_wchar* dest, const juce_wchar* src, const int maxChars) throw();

    static void copy (juce_wchar* dest, const char* src, const int maxChars) throw();
    static void copy (char* dest, const juce_wchar* src, const int maxBytes) throw();
    static int bytesRequiredForCopy (const juce_wchar* src) throw();

    static void append (char* dest, const char* src) throw();
    static void append (juce_wchar* dest, const juce_wchar* src) throw();

    static int compare (const char* const s1, const char* const s2) throw();
    static int compare (const juce_wchar* s1, const juce_wchar* s2) throw();

    static int compare (const char* const s1, const char* const s2, const int maxChars) throw();
    static int compare (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw();

    static int compareIgnoreCase (const char* const s1, const char* const s2) throw();
    static int compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2) throw();

    static int compareIgnoreCase (const char* const s1, const char* const s2, const int maxChars) throw();
    static int compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw();

    static const char* find (const char* const haystack, const char* const needle) throw();
    static const juce_wchar* find (const juce_wchar* haystack, const juce_wchar* const needle) throw();

    static int indexOfChar (const char* const haystack, const char needle, const bool ignoreCase) throw();
    static int indexOfChar (const juce_wchar* const haystack, const juce_wchar needle, const bool ignoreCase) throw();

    static int indexOfCharFast (const char* const haystack, const char needle) throw();
    static int indexOfCharFast (const juce_wchar* const haystack, const juce_wchar needle) throw();

    static int getIntialSectionContainingOnly (const char* const text, const char* const allowedChars) throw();
    static int getIntialSectionContainingOnly (const juce_wchar* const text, const juce_wchar* const allowedChars) throw();

    static int ftime (char* const dest, const int maxChars, const char* const format, const struct tm* const tm) throw();
    static int ftime (juce_wchar* const dest, const int maxChars, const juce_wchar* const format, const struct tm* const tm) throw();

    static int getIntValue (const char* const s) throw();
    static int getIntValue (const juce_wchar* s) throw();

    static int64 getInt64Value (const char* s) throw();
    static int64 getInt64Value (const juce_wchar* s) throw();

    static double getDoubleValue (const char* const s) throw();
    static double getDoubleValue (const juce_wchar* const s) throw();

    //==============================================================================
    static char toUpperCase (const char character) throw();
    static juce_wchar toUpperCase (const juce_wchar character) throw();
    static void toUpperCase (char* s) throw();

    static void toUpperCase (juce_wchar* s) throw();
    static bool isUpperCase (const char character) throw();
    static bool isUpperCase (const juce_wchar character) throw();

    static char toLowerCase (const char character) throw();
    static juce_wchar toLowerCase (const juce_wchar character) throw();
    static void toLowerCase (char* s) throw();
    static void toLowerCase (juce_wchar* s) throw();
    static bool isLowerCase (const char character) throw();
    static bool isLowerCase (const juce_wchar character) throw();

    //==============================================================================
    static bool isWhitespace (const char character) throw();
    static bool isWhitespace (const juce_wchar character) throw();

    static bool isDigit (const char character) throw();
    static bool isDigit (const juce_wchar character) throw();

    static bool isLetter (const char character) throw();
    static bool isLetter (const juce_wchar character) throw();

    static bool isLetterOrDigit (const char character) throw();
    static bool isLetterOrDigit (const juce_wchar character) throw();

    /** Returns 0 to 16 for '0' to 'F", or -1 for characters that aren't a legel
        hex digit.
    */
    static int getHexDigitValue (const tchar digit) throw();

    //==============================================================================
    static int printf (char* const dest, const int maxLength, const char* const format, ...) throw();
    static int printf (juce_wchar* const dest, const int maxLength, const juce_wchar* const format, ...) throw();

    static int vprintf (char* const dest, const int maxLength, const char* const format, va_list& args) throw();
    static int vprintf (juce_wchar* const dest, const int maxLength, const juce_wchar* const format, va_list& args) throw();
};

#endif   // __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
