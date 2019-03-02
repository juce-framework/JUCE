/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/**
    Firmware below 0.2.5 does not report its version over the Blocks API.
    This class can make requests and process responses to retreive the master Block version.
*/
class DepreciatedVersionReader :  private MIDIDeviceConnection::Listener,
                                  private Timer
{
public:
    //==============================================================================
    DepreciatedVersionReader (MIDIDeviceConnection& deviceConnectionToUse)
        : deviceConnection (deviceConnectionToUse)
    {
        deviceConnection.addListener (this);
        startTimer (10);
    }

    //==============================================================================
    ~DepreciatedVersionReader()
    {
        deviceConnection.removeListener (this);
    }

    //==============================================================================
    BlocksProtocol::VersionNumber getVersionNumber()
    {
        if (! allRequestsComplete())
            return {};

        auto highestVersion = result[0];

        for (size_t i = 1; i < numFirmwareApps; ++i)
        {
            const BlocksVersion highest { highestVersion.asString() };
            const BlocksVersion test { result[i].asString() };

            if (highest < test)
                highestVersion = result[i];
        }

        return highestVersion;
    }

private:
    //==============================================================================
    static constexpr size_t numFirmwareApps = 3;
    BlocksProtocol::VersionNumber result[numFirmwareApps];
    MIDIDeviceConnection& deviceConnection;
    Atomic<size_t> currentRequest = 0;

    //==============================================================================
    bool allRequestsComplete() const { return currentRequest.get() >= numFirmwareApps; }

    //==============================================================================
    void makeNextRequest()
    {
        static constexpr size_t requestSize { 8 };
        static constexpr uint8 requests[][requestSize] = {{ 0xf0, 0x00, 0x21, 0x10, 0x47, 0x03, 0x00, 0xf7 },  // Main App
                                                          { 0xf0, 0x00, 0x21, 0x10, 0x47, 0x03, 0x03, 0xf7 },  // Bootloader
                                                          { 0xf0, 0x00, 0x21, 0x10, 0x47, 0x03, 0x01, 0xf7 }}; // Stm32

        static const BlocksVersion depreciatedVersion ("0.3.0");

        if (currentRequest.get() == numFirmwareApps - 1
            && (BlocksVersion (result[0].asString()) >= depreciatedVersion
                || BlocksVersion (result[1].asString()) >= depreciatedVersion))
        {
            stopTimer();
        }
        else
        {
            deviceConnection.sendMessageToDevice (&requests[currentRequest.get()][0], requestSize);
        }
    }

    //==============================================================================
    void processVersionMessage (const uint8* data, const size_t size)
    {
        if (currentRequest.get() >= numFirmwareApps || size < 1 || size - 1 > VersionNumber::maxLength)
            return;

        result[currentRequest.get()].length = uint8 (size - 1);
        memcpy (result[currentRequest.get()].data, data, result[currentRequest.get()].length);

        ++currentRequest;

        allRequestsComplete() ? stopTimer() : startTimer (10);
    }

    //==============================================================================
    void handleIncomingMidiMessage (const MidiMessage& message) override
    {
        const uint8 roliVersionHeader[] = { 0xf0, 0x00, 0x21, 0x10, 0x47, 0x03 };
        const auto headerSize = sizeof (roliVersionHeader);
        const auto data = message.getRawData();
        const auto size = size_t (message.getRawDataSize());

        if (memcmp (data, roliVersionHeader, headerSize) == 0)
            processVersionMessage (data + headerSize, size - headerSize);
    }

    void connectionBeingDeleted (const MIDIDeviceConnection&) override {}

    //==============================================================================
    void timerCallback() override
    {
        startTimer (200);
        makeNextRequest();
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DepreciatedVersionReader)
};

} // namespace juce
