/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_IDENTIFIER_H_INCLUDED
#define JUCE_IDENTIFIER_H_INCLUDED


//==============================================================================
/**
    Represents a string identifier, designed for accessing properties by name.

    Comparing two Identifier objects is very fast (an O(1) operation), but creating
    them can be slower than just using a String directly, so the optimal way to use them
    is to keep some static Identifier objects for the things you use often.

    @see NamedValueSet, ValueTree
*/
class JUCE_API  Identifier
{
public:
    /** Creates a null identifier. */
    Identifier() noexcept;

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (const char* name);

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (const String& name);

    /** Creates an identifier with a specified name.
        Because this name may need to be used in contexts such as script variables or XML
        tags, it must only contain ascii letters and digits, or the underscore character.
    */
    Identifier (String::CharPointerType nameStart, String::CharPointerType nameEnd);

    /** Creates a copy of another identifier. */
    Identifier (const Identifier& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier& operator= (const Identifier& other) noexcept;

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    /** Creates a copy of another identifier. */
    Identifier (Identifier&& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier& operator= (Identifier&& other) noexcept;
   #endif

    /** Destructor */
    ~Identifier() noexcept;

    /** Compares two identifiers. This is a very fast operation. */
    inline bool operator== (const Identifier& other) const noexcept     { return name.getCharPointer() == other.name.getCharPointer(); }

    /** Compares two identifiers. This is a very fast operation. */
    inline bool operator!= (const Identifier& other) const noexcept     { return name.getCharPointer() != other.name.getCharPointer(); }

    /** Compares the identifier with a string. */
    inline bool operator== (StringRef other) const noexcept             { return name == other; }

    /** Compares the identifier with a string. */
    inline bool operator!= (StringRef other) const noexcept             { return name != other; }

    /** Returns this identifier as a string. */
    const String& toString() const noexcept                             { return name; }

    /** Returns this identifier's raw string pointer. */
    operator String::CharPointerType() const noexcept                   { return name.getCharPointer(); }

    /** Returns this identifier's raw string pointer. */
    String::CharPointerType getCharPointer() const noexcept             { return name.getCharPointer(); }

    /** Returns this identifier as a StringRef. */
    operator StringRef() const noexcept                                 { return name; }

    /** Returns true if this Identifier is not null */
    bool isValid() const noexcept                                       { return name.isNotEmpty(); }

    /** Returns true if this Identifier is null */
    bool isNull() const noexcept                                        { return name.isEmpty(); }

    /** A null identifier. */
    static Identifier null;

    /** Checks a given string for characters that might not be valid in an Identifier.
        Since Identifiers are used as a script variables and XML attributes, they should only contain
        alphanumeric characters, underscores, or the '-' and ':' characters.
    */
    static bool isValidIdentifier (const String& possibleIdentifier) noexcept;

private:
    String name;
};


#endif   // JUCE_IDENTIFIER_H_INCLUDED
