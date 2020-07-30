/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An OSC time tag.

    OSC time tags are part of OSCBundle objects.

    In accordance with the OSC 1.0 specification, the internal timestamp stored in
    OSCTimeTag uses the same binary format as NTP timestamps. The representation
    is by a 64 bit fixed point number. The first 32 bits specify the number of
    seconds since midnight on January 1, 1900, and the last 32 bits specify
    fractional parts of a second to a precision of about 200 picoseconds.

    The time tag value consisting of 63 zero bits followed by a one in the least
    significant bit is a special case meaning "immediately".

    For a more user-friendly time format, convert OSCTimeTag to a juce::Time object
    using toTime().

    @tags{OSC}
*/
class JUCE_API  OSCTimeTag
{
public:
    //==============================================================================
    /** Default constructor.
        Constructs an OSCTimeTag object with the special value representing "immediately".
    */
    OSCTimeTag() noexcept;

    /** Constructs an OSCTimeTag object from a raw binary OSC time tag. */
    OSCTimeTag (uint64 rawTimeTag) noexcept;

    /** Constructs an OSCTimeTag object from a juce::Time object. */
    OSCTimeTag (Time time) noexcept;

    /** Returns a juce::Time object representing the same time as the OSCTimeTag.

        If the OSCTimeTag has the special value representing "immediately", the
        resulting juce::Time object will represent an arbitrary point of time (but
        guaranteed to be in the past), since juce::Time does not have such a special value.
    */
    Time toTime() const noexcept;

    /** Returns true if the OSCTimeTag object has the special value representing "immediately". */
    bool isImmediately() const noexcept;

    /** Returns the raw binary OSC time tag representation. */
    uint64 getRawTimeTag() const noexcept               { return rawTimeTag; }

    /** The special value representing "immediately". */
    static const OSCTimeTag immediately;

private:
    //==============================================================================
    uint64 rawTimeTag;
};

} // namespace juce
