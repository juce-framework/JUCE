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
    This topology source manages the topology of the physical Blocks devices
    that are currently connected. It maintains a list of them and tells
    listeners when physical devices are added or removed.

    @tags{Blocks}
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
    std::unique_ptr<DetectorHolder> detector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhysicalTopologySource)
};

} // namespace juce
