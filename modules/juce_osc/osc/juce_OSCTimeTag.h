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
