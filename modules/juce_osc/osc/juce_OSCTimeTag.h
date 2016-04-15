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

#ifndef JUCE_OSCTIMETAG_H_INCLUDED
#define JUCE_OSCTIMETAG_H_INCLUDED


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

    /** Constructs an OSCTimeTag object from a Juce Time object. */
    OSCTimeTag (Time time) noexcept;

    /** Returns a Juce Time object representing the same time as the OSCTimeTag.

        If the OSCTimeTag has the special value representing "immediately", the
        resulting Juce Time object will represent an arbitrary point of time (but
        guaranteed to be in the past), since Juce Time does not have such a special value.
    */
    Time toTime() const noexcept;

    /** Returns true if the OSCTimeTag object has the special value representing "immedately". */
    bool isImmediately() const noexcept;

    /** Returns the raw binary OSC time tag representation. */
    uint64 getRawTimeTag() const noexcept               { return rawTimeTag; }

    /** The special value representing "immediately". */
    static const OSCTimeTag immediately;

private:
    //==============================================================================
    uint64 rawTimeTag;
};


#endif // JUCE_OSCTIMETAG_H_INCLUDED
