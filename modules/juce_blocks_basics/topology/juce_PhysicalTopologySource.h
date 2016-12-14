/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/


/**
    This topology source manages the topology of the physical Blocks devices
    that are currently connected. It maintains a list of them and tells
    listeners when physical devices are added or removed.
*/
class PhysicalTopologySource  : public TopologySource
{
public:
    /** Constructor. */
    PhysicalTopologySource();

    /** Destructor. */
    ~PhysicalTopologySource();

    /** Returns the current physical topology. */
    BlockTopology getCurrentTopology() const override;

    /** Reset all touches */
    void cancelAllActiveTouches() noexcept override;


    //==========================================================================
    /** For custom transport systems, this represents a connected device */
    struct DeviceConnection
    {
        DeviceConnection();
        virtual ~DeviceConnection();

        virtual bool sendMessageToDevice (const void* data, size_t dataSize) = 0;
        std::function<void (const void* data, size_t dataSize)> handleMessageFromDevice;
    };

    /** For custom transport systems, this represents a connected device */
    struct DeviceDetector
    {
        DeviceDetector();
        virtual ~DeviceDetector();

        virtual juce::StringArray scanForDevices() = 0;
        virtual DeviceConnection* openDevice (int index) = 0;
    };

    /** Constructor for custom transport systems. */
    PhysicalTopologySource (DeviceDetector& detectorToUse);

    static const char* const* getStandardLittleFootFunctions() noexcept;

protected:
    virtual bool hasOwnServiceTimer() const;
    virtual void handleTimerTick();

private:
    //==========================================================================
    struct Internal;
    struct DetectorHolder;
    juce::ScopedPointer<DetectorHolder> detector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhysicalTopologySource)
};
