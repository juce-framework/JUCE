/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::midi_ci
{

//==============================================================================
/**
    An interface that will receive a callback every time a Device wishes to send a new MIDI-CI
    message.

    @tags{Audio}
*/
struct DeviceMessageHandler
{
    DeviceMessageHandler() = default;
    virtual ~DeviceMessageHandler() = default;
    DeviceMessageHandler (const DeviceMessageHandler&) = default;
    DeviceMessageHandler (DeviceMessageHandler&&) = default;
    DeviceMessageHandler& operator= (const DeviceMessageHandler&) = default;
    DeviceMessageHandler& operator= (DeviceMessageHandler&&) = default;

    /** Called with the bytes of a MIDI-CI message, along with the message's group.

        To send the message on, format the message appropriately (either into bytestream sysex
        or into multiple UMP sysex packets).
    */
    virtual void processMessage (ump::BytesOnGroup) = 0;
};

} // namespace juce::midi_ci
