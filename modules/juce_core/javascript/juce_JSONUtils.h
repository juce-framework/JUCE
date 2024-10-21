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
