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

namespace juce::universal_midi_packets
{

/** Directional properties of a MIDI endpoint. */
enum class IOKind : uint8_t
{
    src,    ///< A source of MIDI events
    dst,    ///< A destination for MIDI events
};

/** All possible MIDI directions. */
constexpr IOKind ioKinds[] { IOKind::src, IOKind::dst };

//==============================================================================
/**
    Identifies a MIDI endpoint. This is intended to be an opaque type that can only be compared with
    other instances.

    For backwards compatibility, we need to ensure that port identifier strings that used to work
    with MidiInput and MidiOutput continue to function in the same way. However, the old identifiers
    weren't necessarily unique between inputs and outputs (a MIDI 1.0 input and output could have
    the same ID), which means that a single id string isn't enough to uniquely identify an input
    or output port.

    @tags{Audio}
*/
class EndpointId
{
    auto tie() const { return std::tuple (src, dst); }

public:
    /** @internal */
    class Impl;

    EndpointId() = default;

    bool operator== (const EndpointId& x) const { return tie() == x.tie(); }
    bool operator!= (const EndpointId& x) const { return tie() != x.tie(); }
    bool operator<  (const EndpointId& x) const { return tie() <  x.tie(); }
    bool operator<= (const EndpointId& x) const { return tie() <= x.tie(); }
    bool operator>  (const EndpointId& x) const { return tie() >  x.tie(); }
    bool operator>= (const EndpointId& x) const { return tie() >= x.tie(); }

    String get (IOKind k) const { return k == IOKind::src ? src : dst; }

    static EndpointId make (IOKind dir, const String& id)
    {
        return dir == IOKind::src ? makeSrcDst (id, {}) : makeSrcDst ({}, id);
    }

    static EndpointId makeSrcDst (const String& s, const String& d)
    {
        return { s, d };
    }

    String src, dst;

private:
    EndpointId (const String& s, const String& d) : src (s), dst (d) {}
};

}
