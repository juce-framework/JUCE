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

bool MidiRPNDetector::parseControllerMessage (int midiChannel,
                                              int controllerNumber,
                                              int controllerValue,
                                              MidiRPNMessage& result) noexcept
{
    auto parsed = tryParse (midiChannel, controllerNumber, controllerValue);

    if (! parsed.has_value())
        return false;

    result = *parsed;
    return true;
}

std::optional<MidiRPNMessage> MidiRPNDetector::tryParse (int midiChannel,
                                                         int controllerNumber,
                                                         int controllerValue)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    jassert (controllerNumber >= 0 && controllerNumber < 128);
    jassert (controllerValue >= 0 && controllerValue < 128);

    return states[midiChannel - 1].handleController (midiChannel, controllerNumber, controllerValue);
}

void MidiRPNDetector::reset() noexcept
{
    for (auto& state : states)
    {
        state.parameterMSB = 0xff;
        state.parameterLSB = 0xff;
        state.resetValue();
        state.isNRPN = false;
    }
}

//==============================================================================
std::optional<MidiRPNMessage> MidiRPNDetector::ChannelState::handleController (int channel,
                                                                               int controllerNumber,
                                                                               int value) noexcept
{
    switch (controllerNumber)
    {
        case 0x62:  parameterLSB = uint8 (value); resetValue(); isNRPN = true;  break;
        case 0x63:  parameterMSB = uint8 (value); resetValue(); isNRPN = true;  break;

        case 0x64:  parameterLSB = uint8 (value); resetValue(); isNRPN = false; break;
        case 0x65:  parameterMSB = uint8 (value); resetValue(); isNRPN = false; break;

        case 0x06:  valueMSB = uint8 (value); valueLSB = 0xff; return sendIfReady (channel);
        case 0x26:  valueLSB = uint8 (value);                  return sendIfReady (channel);
    }

    return {};
}

void MidiRPNDetector::ChannelState::resetValue() noexcept
{
    valueMSB = 0xff;
    valueLSB = 0xff;
}

//==============================================================================
std::optional<MidiRPNMessage> MidiRPNDetector::ChannelState::sendIfReady (int channel) noexcept
{
    if (parameterMSB >= 0x80 || parameterLSB >= 0x80 || valueMSB >= 0x80)
        return {};

    MidiRPNMessage result{};
    result.channel = channel;
    result.parameterNumber = (parameterMSB << 7) + parameterLSB;
    result.isNRPN = isNRPN;

    if (valueLSB < 0x80)
    {
        result.value = (valueMSB << 7) + valueLSB;
        result.is14BitValue = true;
    }
    else
    {
        result.value = valueMSB;
        result.is14BitValue = false;
    }

    return result;
}

//==============================================================================
MidiBuffer MidiRPNGenerator::generate (MidiRPNMessage message)
{
    return generate (message.channel,
                     message.parameterNumber,
                     message.value,
                     message.isNRPN,
                     message.is14BitValue);
}

MidiBuffer MidiRPNGenerator::generate (int midiChannel,
                                       int parameterNumber,
                                       int value,
                                       bool isNRPN,
                                       bool use14BitValue)
{
    jassert (midiChannel > 0 && midiChannel <= 16);
    jassert (parameterNumber >= 0 && parameterNumber < 16384);
    jassert (value >= 0 && value < (use14BitValue ? 16384 : 128));

    auto parameterLSB = uint8 (parameterNumber & 0x0000007f);
    auto parameterMSB = uint8 (parameterNumber >> 7);

    uint8 valueLSB = use14BitValue ? uint8 (value & 0x0000007f) : 0x00;
    uint8 valueMSB = use14BitValue ? uint8 (value >> 7) : uint8 (value);

    auto channelByte = uint8 (0xb0 + midiChannel - 1);

    MidiBuffer buffer;

    buffer.addEvent (MidiMessage (channelByte, isNRPN ? 0x62 : 0x64, parameterLSB),  0);
    buffer.addEvent (MidiMessage (channelByte, isNRPN ? 0x63 : 0x65, parameterMSB),  0);

    buffer.addEvent (MidiMessage (channelByte, 0x06, valueMSB), 0);

    // According to the MIDI spec, whenever a MSB is received, the corresponding LSB will
    // be reset. Therefore, the LSB should be sent after the MSB.
    if (use14BitValue)
        buffer.addEvent (MidiMessage (channelByte, 0x26, valueLSB), 0);

    return buffer;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class MidiRPNDetectorTests final : public UnitTest
{
public:
    MidiRPNDetectorTests()
        : UnitTest ("MidiRPNDetector class", UnitTestCategories::midi)
    {}

    void runTest() override
    {
        // From the MIDI 1.0 spec:
        // If 128 steps of resolution is sufficient the second byte (LSB) of the data value can be
        // omitted. If both the MSB and LSB are sent initially, a subsequent fine adjustment only
        // requires the sending of the LSB. The MSB does not have to be retransmitted. If a
        // subsequent major adjustment is necessary the MSB must be transmitted again. When an MSB
        // is received, the receiver should set its concept of the LSB to zero.

        beginTest ("Individual MSB is parsed as 7-bit");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (2, 101, 0));
            expect (! detector.tryParse (2, 100, 7));

            auto parsed = detector.tryParse (2, 6, 42);
            expect (parsed.has_value());

            expectEquals (parsed->channel, 2);
            expectEquals (parsed->parameterNumber, 7);
            expectEquals (parsed->value, 42);
            expect (! parsed->isNRPN);
            expect (! parsed->is14BitValue);
        }

        beginTest ("LSB without preceding MSB is ignored");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (2, 101, 0));
            expect (! detector.tryParse (2, 100, 7));
            expect (! detector.tryParse (2, 38, 42));
        }

        beginTest ("LSB following MSB is parsed as 14-bit");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (1, 101, 2));
            expect (! detector.tryParse (1, 100, 44));

            expect (detector.tryParse (1, 6, 1).has_value());

            auto lsbParsed = detector.tryParse (1, 38, 94);
            expect (lsbParsed.has_value());

            expectEquals (lsbParsed->channel, 1);
            expectEquals (lsbParsed->parameterNumber, 300);
            expectEquals (lsbParsed->value, 222);
            expect (! lsbParsed->isNRPN);
            expect (lsbParsed->is14BitValue);
        }

        beginTest ("Multiple LSB following MSB re-use the MSB");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (1, 101, 2));
            expect (! detector.tryParse (1, 100, 43));

            expect (detector.tryParse (1, 6, 1).has_value());

            expect (detector.tryParse (1, 38, 94).has_value());
            expect (detector.tryParse (1, 38, 95).has_value());
            expect (detector.tryParse (1, 38, 96).has_value());

            auto lsbParsed = detector.tryParse (1, 38, 97);
            expect (lsbParsed.has_value());

            expectEquals (lsbParsed->channel, 1);
            expectEquals (lsbParsed->parameterNumber, 299);
            expectEquals (lsbParsed->value, 225);
            expect (! lsbParsed->isNRPN);
            expect (lsbParsed->is14BitValue);
        }

        beginTest ("Sending a new MSB resets the LSB");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (1, 101, 3));
            expect (! detector.tryParse (1, 100, 43));

            expect (detector.tryParse (1, 6, 1).has_value());
            expect (detector.tryParse (1, 38, 94).has_value());

            auto newMsb = detector.tryParse (1, 6, 2);
            expect (newMsb.has_value());

            expectEquals (newMsb->channel, 1);
            expectEquals (newMsb->parameterNumber, 427);
            expectEquals (newMsb->value, 2);
            expect (! newMsb->isNRPN);
            expect (! newMsb->is14BitValue);
        }

        beginTest ("RPNs on multiple channels simultaneously");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (1, 100, 44));
            expect (! detector.tryParse (2, 101, 0));
            expect (! detector.tryParse (1, 101, 2));
            expect (! detector.tryParse (2, 100, 7));
            expect (detector.tryParse   (1, 6,   1).has_value());

            auto channelTwo = detector.tryParse (2, 6, 42);
            expect (channelTwo.has_value());

            expectEquals (channelTwo->channel, 2);
            expectEquals (channelTwo->parameterNumber, 7);
            expectEquals (channelTwo->value, 42);
            expect (! channelTwo->isNRPN);
            expect (! channelTwo->is14BitValue);

            auto channelOne = detector.tryParse (1, 38,  94);
            expect (channelOne.has_value());

            expectEquals (channelOne->channel, 1);
            expectEquals (channelOne->parameterNumber, 300);
            expectEquals (channelOne->value, 222);
            expect (! channelOne->isNRPN);
            expect (channelOne->is14BitValue);
        }

        beginTest ("14-bit RPN with value within 7-bit range");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (16, 100, 0));
            expect (! detector.tryParse (16, 101, 0));
            expect (detector.tryParse   (16, 6,   0).has_value());

            auto parsed = detector.tryParse (16, 38,  3);
            expect (parsed.has_value());

            expectEquals (parsed->channel, 16);
            expectEquals (parsed->parameterNumber, 0);
            expectEquals (parsed->value, 3);
            expect (! parsed->isNRPN);
            expect (parsed->is14BitValue);
        }

        beginTest ("invalid RPN (wrong order)");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (2, 6,   42));
            expect (! detector.tryParse (2, 101, 0));
            expect (! detector.tryParse (2, 100, 7));
        }

        beginTest ("14-bit RPN interspersed with unrelated CC messages");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (16, 3,   80));
            expect (! detector.tryParse (16, 100, 0));
            expect (! detector.tryParse (16, 4,   81));
            expect (! detector.tryParse (16, 101, 0));
            expect (! detector.tryParse (16, 5,   82));
            expect (! detector.tryParse (16, 5,   83));
            expect (detector.tryParse   (16, 6,   0).has_value());
            expect (! detector.tryParse (16, 4,   84).has_value());
            expect (! detector.tryParse (16, 3,   85).has_value());

            auto parsed = detector.tryParse (16, 38,  3);
            expect (parsed.has_value());

            expectEquals (parsed->channel, 16);
            expectEquals (parsed->parameterNumber, 0);
            expectEquals (parsed->value, 3);
            expect (! parsed->isNRPN);
            expect (parsed->is14BitValue);
        }

        beginTest ("14-bit NRPN");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (1, 98,  44));
            expect (! detector.tryParse (1, 99 , 2));
            expect (detector.tryParse   (1, 6,   1).has_value());

            auto parsed = detector.tryParse (1, 38,  94);
            expect (parsed.has_value());

            expectEquals (parsed->channel, 1);
            expectEquals (parsed->parameterNumber, 300);
            expectEquals (parsed->value, 222);
            expect (parsed->isNRPN);
            expect (parsed->is14BitValue);
        }

        beginTest ("reset");
        {
            MidiRPNDetector detector;
            expect (! detector.tryParse (2, 101, 0));
            detector.reset();
            expect (! detector.tryParse (2, 100, 7));
            expect (! detector.tryParse (2, 6,   42));
        }
    }
};

static MidiRPNDetectorTests MidiRPNDetectorUnitTests;

//==============================================================================
class MidiRPNGeneratorTests final : public UnitTest
{
public:
    MidiRPNGeneratorTests()
        : UnitTest ("MidiRPNGenerator class", UnitTestCategories::midi)
    {}

    void runTest() override
    {
        beginTest ("generating RPN/NRPN");
        {
            {
                MidiBuffer buffer = MidiRPNGenerator::generate (1, 23, 1337, true, true);
                expectContainsRPN (buffer, 1, 23, 1337, true, true);
            }
            {
                MidiBuffer buffer = MidiRPNGenerator::generate (16, 101, 34, false, false);
                expectContainsRPN (buffer, 16, 101, 34, false, false);
            }
            {
                MidiRPNMessage message = { 16, 101, 34, false, false };
                MidiBuffer buffer = MidiRPNGenerator::generate (message);
                expectContainsRPN (buffer, message);
            }
        }
    }

private:
    //==============================================================================
    void expectContainsRPN (const MidiBuffer& midiBuffer,
                            int channel,
                            int parameterNumber,
                            int value,
                            bool isNRPN,
                            bool is14BitValue)
    {
        MidiRPNMessage expected = { channel, parameterNumber, value, isNRPN, is14BitValue };
        expectContainsRPN (midiBuffer, expected);
    }

    //==============================================================================
    void expectContainsRPN (const MidiBuffer& midiBuffer, MidiRPNMessage expected)
    {
        std::optional<MidiRPNMessage> result;
        MidiRPNDetector detector;

        for (const auto metadata : midiBuffer)
        {
            const auto midiMessage = metadata.getMessage();

            result = detector.tryParse (midiMessage.getChannel(),
                                        midiMessage.getControllerNumber(),
                                        midiMessage.getControllerValue());
        }

        expect (result.has_value());
        expectEquals (result->channel, expected.channel);
        expectEquals (result->parameterNumber, expected.parameterNumber);
        expectEquals (result->value, expected.value);
        expect (result->isNRPN == expected.isNRPN);
        expect (result->is14BitValue == expected.is14BitValue);
    }
};

static MidiRPNGeneratorTests MidiRPNGeneratorUnitTests;

#endif

} // namespace juce
