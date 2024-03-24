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
/** Represents a MIDI RPN (registered parameter number) or NRPN (non-registered
    parameter number) message.

    @tags{Audio}
*/
struct MidiRPNMessage
{
    /** Midi channel of the message, in the range 1 to 16. */
    int channel;

    /** The 14-bit parameter index, in the range 0 to 16383 (0x3fff). */
    int parameterNumber;

    /** The parameter value, in the range 0 to 16383 (0x3fff).
        If the message contains no value LSB, the value will be in the range
        0 to 127 (0x7f).
    */
    int value;

    /** True if this message is an NRPN; false if it is an RPN. */
    bool isNRPN;

    /** True if the value uses 14-bit resolution (LSB + MSB); false if
        the value is 7-bit (MSB only).
    */
    bool is14BitValue;
};

//==============================================================================
/**
    Parses a stream of MIDI data to assemble RPN and NRPN messages from their
    constituent MIDI CC messages.

    The detector uses the following parsing rules: the parameter number
    LSB/MSB can be sent/received in either order and must both come before the
    parameter value; for the parameter value, LSB always has to be sent/received
    before the value MSB, otherwise it will be treated as 7-bit (MSB only).

    @tags{Audio}
*/
class JUCE_API  MidiRPNDetector
{
public:
    /** Constructor. */
    MidiRPNDetector() noexcept = default;

    /** Destructor. */
    ~MidiRPNDetector() noexcept = default;

    /** Resets the RPN detector's internal state, so that it forgets about
        previously received MIDI CC messages.
    */
    void reset() noexcept;

    //==============================================================================
    /** @see tryParse() */
    [[deprecated ("Use tryParse() instead")]]
    bool parseControllerMessage (int midiChannel,
                                 int controllerNumber,
                                 int controllerValue,
                                 MidiRPNMessage& result) noexcept;

    /** Takes the next in a stream of incoming MIDI CC messages and returns
        a MidiRPNMessage if the current message produces a well-formed RPN or NRPN.

        Note that senders are expected to send the MSB before the LSB, but senders are
        not required to send a LSB at all. Therefore, tryParse() will return a non-null
        optional on all MSB messages (provided a parameter number has been set), and will
        also return a non-null optional for each LSB that follows the initial MSB.

        This behaviour allows senders to transmit a single MSB followed by multiple LSB
        messages to facilitate fine-tuning of parameters.

        The result of parsing a MSB will always be a 7-bit value.
        The result of parsing a LSB that follows an MSB will always be a 14-bit value.
    */
    std::optional<MidiRPNMessage> tryParse (int midiChannel,
                                            int controllerNumber,
                                            int controllerValue);

private:
    //==============================================================================
    struct ChannelState
    {
        std::optional<MidiRPNMessage> handleController (int channel, int controllerNumber,
                                                        int value) noexcept;
        void resetValue() noexcept;
        std::optional<MidiRPNMessage> sendIfReady (int channel) noexcept;

        uint8 parameterMSB = 0xff, parameterLSB = 0xff, valueMSB = 0xff, valueLSB = 0xff;
        bool isNRPN = false;
    };

    //==============================================================================
    ChannelState states[16];

    JUCE_LEAK_DETECTOR (MidiRPNDetector)
};

//==============================================================================
/**
    Generates an appropriate sequence of MIDI CC messages to represent an RPN
    or NRPN message.

    This sequence (as a MidiBuffer) can then be directly sent to a MidiOutput.

    @tags{Audio}
*/
class JUCE_API  MidiRPNGenerator
{
public:
    //==============================================================================
    /** Generates a MIDI sequence representing the given RPN or NRPN message. */
    static MidiBuffer generate (MidiRPNMessage message);

    //==============================================================================
    /** Generates a MIDI sequence representing an RPN or NRPN message with the
        given parameters.

        @param channel           The MIDI channel of the RPN/NRPN message.

        @param parameterNumber   The parameter number, in the range 0 to 16383.

        @param value             The parameter value, in the range 0 to 16383, or
                                 in the range 0 to 127 if sendAs14BitValue is false.

        @param isNRPN            Whether you need a MIDI RPN or NRPN sequence (RPN is default).

        @param use14BitValue     If true (default), the value will have 14-bit precision
                                 (two MIDI bytes). If false, instead the value will have
                                 7-bit precision (a single MIDI byte).
    */
    static MidiBuffer generate (int channel,
                                int parameterNumber,
                                int value,
                                bool isNRPN = false,
                                bool use14BitValue = true);
};

} // namespace juce
