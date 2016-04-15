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

#ifndef JUCE_OSCADDRESS_H_INCLUDED
#define JUCE_OSCADDRESS_H_INCLUDED


//==============================================================================
/**
    An OSC address.

    This address always starts with a forward slash and has a format similar
    to an URL, with several address parts separated by slashes.

    Only a subset of ASCII characters are allowed in OSC addresses; see
    OpenSoundControl 1.0 specification for details.

    OSC addresses can be used to register OSCMessageListener objects to an
    OSCReceiver if you wish them to only listen to certain messages with
    matching OSC address patterns.

    @see OSCMessageListener, OSCAddressPattern, OSCMessage
*/
class JUCE_API  OSCAddress
{
public:
    //==============================================================================
    /** Constructs a new OSCAddress from a String.
        @throw OSCFormatError if the string is not a valid OSC address.
    */
    OSCAddress (const String& address);

    /** Constructs a new OSCAddress from a C string.
        @throw OSCFormatError of the string is not a valid OSC address.
    */
    OSCAddress (const char* address);

    /** Compares two OSCAddress objects.
        @returns true if they contain the same address, false otherwise.
    */
    bool operator== (const OSCAddress& other) const noexcept;

    /** Compares two OSCAddress objects.
        @returns false if they contain the same address, true otherwise.
    */
    bool operator!= (const OSCAddress& other) const noexcept;

    /** Converts the OSCAddress to a String.
        Note: trailing slashes are always removed automatically.

        @returns a String object that represents the OSC address.
    */
    String toString() const noexcept;

private:
    //==============================================================================
    StringArray oscSymbols;
    String asString;
    friend class OSCAddressPattern;
};

//==============================================================================
/**
    An OSC address pattern.

    Extends an OSC address by additionally allowing the following wildcards:
    ?, *, [], {}

    OSC messages always have an OSC address pattern to specify the destination(s)
    of the message.

    @see OSCMessage, OSCAddress, OSCMessageListener
*/
class JUCE_API  OSCAddressPattern
{
public:
    //==============================================================================
    /** Constructs a new OSCAddressPattern from a String.
        @throw OSCFormatError if the string is not a valid OSC address pattern.
    */
    OSCAddressPattern (const String& address);

    /** Constructs a new OSCAddressPattern from a C string.
        @throw OSCFormatError of the string is not a valid OSC address pattern.
    */
    OSCAddressPattern (const char* address);

    /** Compares two OSCAddressPattern objects.
        @returns true if they contain the same address pattern, false otherwise.
    */
    bool operator== (const OSCAddressPattern& other) const noexcept;

    /** Compares two OSCAddressPattern objects.
        @returns false if they contain the same address pattern, true otherwise.
    */
    bool operator!= (const OSCAddressPattern& other) const noexcept;

    /** Checks if the OSCAddressPattern matches an OSC address with the wildcard
        rules defined by the OpenSoundControl 1.0 specification.

        @returns true if the OSCAddressPattern matches the given OSC address,
                 false otherwise.
    */
    bool matches (const OSCAddress& address) const noexcept;

    /** Checks whether the OSCAddressPattern contains any of the allowed OSC
        address patttern wildcards: ?, *, [], {}

        @returns true if the OSCAddressPattern contains OSC wildcards, false otherwise.
    */
    bool containsWildcards() const noexcept     { return wasInitialisedWithWildcards; }

    /** Converts the OSCAddressPattern to a String.
        Note: trailing slashes are always removed automatically.

        @returns a String object that represents the OSC address pattern.
    */
    String toString() const noexcept;


private:
    //==============================================================================
    StringArray oscSymbols;
    String asString;
    bool wasInitialisedWithWildcards;
};


#endif // JUCE_OSCADDRESS_H_INCLUDED
