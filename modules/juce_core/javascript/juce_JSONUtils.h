/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

/**
    A mini namespace to hold utility functions for working with juce::vars.

    @tags{Core}
*/
struct JSONUtils
{
    /** No constructor. */
    JSONUtils() = delete;

    /** Given a JSON array/object 'v', a string representing a JSON pointer,
        and a new property value 'newValue', returns a copy of 'v' where the
        property or array index referenced by the pointer has been set to 'newValue'.

        If the pointer cannot be followed, due to referencing missing array indices
        or fields, then this returns nullopt.

        For more details, check the JSON Pointer RFC 6901:
        https://datatracker.ietf.org/doc/html/rfc6901
    */
    static std::optional<var> setPointer (const var& v, String pointer, const var& newValue);

    /** Converts the provided key/value pairs into a JSON object. */
    static var makeObject (const std::map<Identifier, var>& source);

    /** Converts the provided key/value pairs into a JSON object with the provided
        key at the first position in the object.

        This is useful because the MIDI-CI spec requires that certain fields (e.g.
        status) should be placed at the beginning of a MIDI-CI header.
    */
    static var makeObjectWithKeyFirst (const std::map<Identifier, var>& source, Identifier key);

    /** Returns true if and only if the contents of a match the contents of b.

        Unlike var::operator==, this will recursively check that contained DynamicObject and Array
        instances compare equal.
    */
    static bool deepEqual (const var& a, const var& b);
};

} // namespace juce
