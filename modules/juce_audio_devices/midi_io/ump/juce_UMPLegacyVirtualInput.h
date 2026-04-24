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

/**
    Represents a virtual MIDI 1.0 input port.

    @tags{Audio}
*/
class LegacyVirtualInput
{
public:
    /** Creates an invalid virtual port that doesn't correspond to any virtual device. */
    LegacyVirtualInput();
    ~LegacyVirtualInput();

    LegacyVirtualInput (LegacyVirtualInput&&) noexcept;
    LegacyVirtualInput& operator= (LegacyVirtualInput&&) noexcept;

    LegacyVirtualInput (const LegacyVirtualInput&) = delete;
    LegacyVirtualInput& operator= (const LegacyVirtualInput&) = delete;

    /** Retrieves the unique id of this input.

        You can pass this ID to Session::connectInput() in order to receive messages sent to this
        input.

        Note that this ID is *not* guaranteed to be stable - creating the 'same' virtual device
        across several program invocations may produce a different ID each time.

        To fetch the current details of this device, you can pass this ID to Endpoints::getEndpoint().
    */
    EndpointId getId() const;

    bool isAlive() const;

    explicit operator bool() const { return isAlive(); }

    /** @internal */
    class Impl;

private:
    explicit LegacyVirtualInput (std::unique_ptr<Impl>);

    std::unique_ptr<Impl> impl;
};

}
