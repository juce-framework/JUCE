/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_OSCTYPE_H_INCLUDED
#define JUCE_OSCTYPE_H_INCLUDED

//==============================================================================

/** The type used for OSC type tags. */
typedef char OSCType;


/** The type used for OSC type tag strings. */
typedef Array<OSCType> OSCTypeList;

//==============================================================================

/** The definitions of supported OSC types and their associated OSC type tags,
    as defined in the OpenSoundControl 1.0 specification.

    Note: this implementation does not support any additional type tags that
    are not part of the specification.
*/
class JUCE_API  OSCTypes
{
public:
    static const OSCType int32;
    static const OSCType float32;
    static const OSCType string;
    static const OSCType blob;

    static bool isSupportedType (OSCType type) noexcept
    {
        return type == OSCTypes::int32
            || type == OSCTypes::float32
            || type == OSCTypes::string
            || type == OSCTypes::blob;
    }
};

//==============================================================================
/** Base class for exceptions that can be thrown by methods in the OSC module.
*/
struct OSCException  : public std::exception
{
    OSCException (const String& desc)
        : description (desc)
    {
       #if ! JUCE_UNIT_TESTS
        DBG ("OSCFormatError: " + description);
       #endif
    }

    String description;
};

//==============================================================================
/** Exception type thrown when the OSC module fails to parse something because
    of a data format not compatible with the OpenSoundControl 1.0 specification.
*/
struct OSCFormatError : public OSCException
{
    OSCFormatError (const String& desc) : OSCException (desc) {}
};

//==============================================================================
/** Exception type thrown in cases of unexpected errors in the OSC module.

    Note: this should never happen, and all the places where this is thrown
    should have a preceding jassertfalse to facilitate debugging.
*/
struct OSCInternalError : public OSCException
{
    OSCInternalError (const String& desc) : OSCException (desc) {}
};


#endif // JUCE_OSCTYPE_H_INCLUDED
