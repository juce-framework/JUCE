/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_IDENTIFIER_JUCEHEADER__
#define __JUCE_IDENTIFIER_JUCEHEADER__

class StringPool;


//==============================================================================
/**
    Represents a string identifier, designed for accessing properties by name.

    Identifier objects are very light and fast to copy, but slower to initialise
    from a string, so it's much faster to keep a static identifier object to refer
    to frequently-used names, rather than constructing them each time you need it.

    @see NamedPropertySet, ValueTree
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

    /** Creates a copy of another identifier. */
    Identifier (const Identifier& other) noexcept;

    /** Creates a copy of another identifier. */
    Identifier& operator= (const Identifier& other) noexcept;

    /** Destructor */
    ~Identifier();

    /** Compares two identifiers. This is a very fast operation. */
    inline bool operator== (const Identifier& other) const noexcept     { return name == other.name; }

    /** Compares two identifiers. This is a very fast operation. */
    inline bool operator!= (const Identifier& other) const noexcept     { return name != other.name; }

    /** Returns this identifier as a string. */
    String toString() const                                             { return name; }

    /** Returns this identifier's raw string pointer. */
    operator const String::CharPointerType() const noexcept             { return name; }

    /** Returns this identifier's raw string pointer. */
    const String::CharPointerType getCharPointer() const noexcept       { return name; }

    /** Returns true if this Identifier is not null */
    bool isValid() const noexcept                                       { return name.getAddress() != nullptr; }

    /** Returns true if this Identifier is null */
    bool isNull() const noexcept                                        { return name.getAddress() == nullptr; }

    /** A null identifier. */
    static Identifier null;

    /** Checks a given string for characters that might not be valid in an Identifier.
        Since Identifiers are used as a script variables and XML attributes, they should only contain
        alphanumeric characters, underscores, or the '-' and ':' characters.
    */
    static bool isValidIdentifier (const String& possibleIdentifier) noexcept;


private:
    //==============================================================================
    String::CharPointerType name;

    static StringPool& getPool();
};


#endif   // __JUCE_IDENTIFIER_JUCEHEADER__
