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
    static int JUCE_CALLTYPE length (const char* const s) throw();
    static int JUCE_CALLTYPE length (const juce_wchar* const s) throw();

    static void JUCE_CALLTYPE copy (char* dest, const char* src, const int maxChars) throw();
    static void JUCE_CALLTYPE copy (juce_wchar* dest, const juce_wchar* src, const int maxChars) throw();

    static void JUCE_CALLTYPE copy (juce_wchar* dest, const char* src, const int maxChars) throw();
    static void JUCE_CALLTYPE copy (char* dest, const juce_wchar* src, const int maxChars) throw();

    static void JUCE_CALLTYPE append (char* dest, const char* src) throw();
    static void JUCE_CALLTYPE append (juce_wchar* dest, const juce_wchar* src) throw();

    static int JUCE_CALLTYPE compare (const char* const s1, const char* const s2) throw();
    static int JUCE_CALLTYPE compare (const juce_wchar* s1, const juce_wchar* s2) throw();

    static int JUCE_CALLTYPE compare (const char* const s1, const char* const s2, const int maxChars) throw();
    static int JUCE_CALLTYPE compare (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw();

    static int JUCE_CALLTYPE compareIgnoreCase (const char* const s1, const char* const s2) throw();
    static int JUCE_CALLTYPE compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2) throw();

    static int JUCE_CALLTYPE compareIgnoreCase (const char* const s1, const char* const s2, const int maxChars) throw();
    static int JUCE_CALLTYPE compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw();

    static const char* JUCE_CALLTYPE find (const char* const haystack, const char* const needle) throw();
    static const juce_wchar* JUCE_CALLTYPE find (const juce_wchar* haystack, const juce_wchar* const needle) throw();

    static int JUCE_CALLTYPE indexOfChar (const char* const haystack, const char needle, const bool ignoreCase) throw();
    static int JUCE_CALLTYPE indexOfChar (const juce_wchar* const haystack, const juce_wchar needle, const bool ignoreCase) throw();

    static int JUCE_CALLTYPE indexOfCharFast (const char* const haystack, const char needle) throw();
    static int JUCE_CALLTYPE indexOfCharFast (const juce_wchar* const haystack, const juce_wchar needle) throw();

    static int JUCE_CALLTYPE getIntialSectionContainingOnly (const char* const text, const char* const allowedChars) throw();
    static int JUCE_CALLTYPE getIntialSectionContainingOnly (const juce_wchar* const text, const juce_wchar* const allowedChars) throw();

    static int JUCE_CALLTYPE ftime (char* const dest, const int maxChars, const char* const format, const struct tm* const tm) throw();
    static int JUCE_CALLTYPE ftime (juce_wchar* const dest, const int maxChars, const juce_wchar* const format, const struct tm* const tm) throw();

    static int JUCE_CALLTYPE getIntValue (const char* const s) throw();
    static int JUCE_CALLTYPE getIntValue (const juce_wchar* s) throw();

    static int64 JUCE_CALLTYPE getInt64Value (const char* s) throw();
    static int64 JUCE_CALLTYPE getInt64Value (const juce_wchar* s) throw();

    static double JUCE_CALLTYPE getDoubleValue (const char* const s) throw();
    static double JUCE_CALLTYPE getDoubleValue (const juce_wchar* const s) throw();

    //==============================================================================
    static char JUCE_CALLTYPE toUpperCase (const char character) throw();
    static juce_wchar JUCE_CALLTYPE toUpperCase (const juce_wchar character) throw();
    static void JUCE_CALLTYPE toUpperCase (char* s) throw();

    static void JUCE_CALLTYPE toUpperCase (juce_wchar* s) throw();
    static bool JUCE_CALLTYPE isUpperCase (const char character) throw();
    static bool JUCE_CALLTYPE isUpperCase (const juce_wchar character) throw();

    static char JUCE_CALLTYPE toLowerCase (const char character) throw();
    static juce_wchar JUCE_CALLTYPE toLowerCase (const juce_wchar character) throw();
    static void JUCE_CALLTYPE toLowerCase (char* s) throw();
    static void JUCE_CALLTYPE toLowerCase (juce_wchar* s) throw();
    static bool JUCE_CALLTYPE isLowerCase (const char character) throw();
    static bool JUCE_CALLTYPE isLowerCase (const juce_wchar character) throw();

    //==============================================================================
    static bool JUCE_CALLTYPE isWhitespace (const char character) throw();
    static bool JUCE_CALLTYPE isWhitespace (const juce_wchar character) throw();

    static bool JUCE_CALLTYPE isDigit (const char character) throw();
    static bool JUCE_CALLTYPE isDigit (const juce_wchar character) throw();

    static bool JUCE_CALLTYPE isLetter (const char character) throw();
    static bool JUCE_CALLTYPE isLetter (const juce_wchar character) throw();

    static bool JUCE_CALLTYPE isLetterOrDigit (const char character) throw();
    static bool JUCE_CALLTYPE isLetterOrDigit (const juce_wchar character) throw();

    /** Returns 0 to 16 for '0' to 'F", or -1 for characters that aren't a legel
        hex digit.
    */
    static int JUCE_CALLTYPE getHexDigitValue (const tchar digit) throw();

    //==============================================================================
    static int JUCE_CALLTYPE printf (char* const dest, const int maxLength, const char* const format, ...) throw();
    static int JUCE_CALLTYPE printf (juce_wchar* const dest, const int maxLength, const juce_wchar* const format, ...) throw();

    static int JUCE_CALLTYPE vprintf (char* const dest, const int maxLength, const char* const format, va_list& args) throw();
    static int JUCE_CALLTYPE vprintf (juce_wchar* const dest, const int maxLength, const juce_wchar* const format, va_list& args) throw();
};

#endif   // __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
