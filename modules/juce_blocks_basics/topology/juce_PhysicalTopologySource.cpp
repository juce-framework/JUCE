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

//==============================================================================
/** These can be useful when debugging the topology. */
#define LOG_BLOCKS_CONNECTIVITY 0
#define LOG_BLOCKS_PINGS 0
#define DUMP_BANDWIDTH_STATS 0
#define DUMP_TOPOLOGY 0

#define TOPOLOGY_LOG(text) \
 JUCE_BLOCK_WITH_FORCED_SEMICOLON (juce::String buf ("Topology Src:   "); \
 juce::Logger::outputDebugString (buf << text);)

#if LOG_BLOCKS_CONNECTIVITY
 #define LOG_CONNECTIVITY(text) TOPOLOGY_LOG(text)
#else
 #define LOG_CONNECTIVITY(text)
#endif

#if LOG_BLOCKS_PINGS
 #define LOG_PING(text) TOPOLOGY_LOG(text)
#else
 #define LOG_PING(text)
#endif

#if DUMP_BANDWIDTH_STATS
 #include "internal/juce_BandwidthStatsLogger.cpp"
#endif

namespace
{
    /** Helper function to create juce::String from BlockStringData */
    template <size_t MaxSize>
    juce::String asString (juce::BlocksProtocol::BlockStringData<MaxSize> blockString)
    {
        return { reinterpret_cast<const char*> (blockString.data),
                 juce::jmin (sizeof (blockString.data), static_cast<size_t> (blockString.length))};
    }
}

#include "internal/juce_MidiDeviceConnection.cpp"
#include "internal/juce_MIDIDeviceDetector.cpp"
#include "internal/juce_DeviceInfo.cpp"
#include "internal/juce_DepreciatedVersionReader.cpp"
#include "internal/juce_ConnectedDeviceGroup.cpp"
#include "internal/juce_BlockImplementation.cpp"
#include "internal/juce_Detector.cpp"
#include "internal/juce_DetectorHolder.cpp"

namespace juce
{

//==============================================================================
PhysicalTopologySource::PhysicalTopologySource (bool startDetached)
{
    if (! startDetached)
        setActive (true);
}

PhysicalTopologySource::PhysicalTopologySource (DeviceDetector& detectorToUse, bool startDetached)
    : customDetector (&detectorToUse)
{
    if (! startDetached)
        setActive (true);
}

PhysicalTopologySource::~PhysicalTopologySource()
{
    setActive (false);
}

void PhysicalTopologySource::setActive (bool shouldBeActive)
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (isActive() == shouldBeActive)
        return;

    if (shouldBeActive)
    {
        if (customDetector == nullptr)
            detector = std::make_unique<DetectorHolder>(*this);
        else
            detector = std::make_unique<DetectorHolder>(*this, *customDetector);

        detector->detector->activeTopologySources.add (this);
    }
    else
    {
        detector->detector->detach (this);
        detector.reset();
    }

    listeners.call ([](TopologySource::Listener& l){ l.topologyChanged(); });
}

bool PhysicalTopologySource::isActive() const
{
    return detector != nullptr;
}

bool PhysicalTopologySource::isLockedFromOutside() const
{
    if (detector != nullptr && detector->detector != nullptr)
        return detector->detector->deviceDetector.isLockedFromOutside();

    return false;
}

BlockTopology PhysicalTopologySource::getCurrentTopology() const
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED // This method must only be called from the message thread!

    if (detector != nullptr)
        return detector->detector->currentTopology;

    return {};
}

void PhysicalTopologySource::cancelAllActiveTouches() noexcept
{
    if (detector != nullptr)
        detector->detector->cancelAllActiveTouches();
}

bool PhysicalTopologySource::hasOwnServiceTimer() const     { return false; }
void PhysicalTopologySource::handleTimerTick()
{
    if (detector != nullptr)
        detector->handleTimerTick();
}

PhysicalTopologySource::DeviceConnection::DeviceConnection() {}
PhysicalTopologySource::DeviceConnection::~DeviceConnection() {}

PhysicalTopologySource::DeviceDetector::DeviceDetector() {}
PhysicalTopologySource::DeviceDetector::~DeviceDetector() {}

const char* const* PhysicalTopologySource::getStandardLittleFootFunctions() noexcept
{
    return BlocksProtocol::ledProgramLittleFootFunctions;
}

template <typename ListType>
static bool collectionsMatch (const ListType& list1, const ListType& list2) noexcept
{
    if (list1.size() != list2.size())
        return false;

    for (auto&& b : list1)
        if (! list2.contains (b))
            return false;

    return true;
}

bool BlockTopology::operator== (const BlockTopology& other) const noexcept
{
    return collectionsMatch (connections, other.connections) && collectionsMatch (blocks, other.blocks);
}

bool BlockTopology::operator!= (const BlockTopology& other) const noexcept
{
    return ! operator== (other);
}

bool BlockDeviceConnection::operator== (const BlockDeviceConnection& other) const noexcept
{
    return (device1 == other.device1 && device2 == other.device2
             && connectionPortOnDevice1 == other.connectionPortOnDevice1
             && connectionPortOnDevice2 == other.connectionPortOnDevice2)
        || (device1 == other.device2 && device2 == other.device1
             && connectionPortOnDevice1 == other.connectionPortOnDevice2
             && connectionPortOnDevice2 == other.connectionPortOnDevice1);
}

bool BlockDeviceConnection::operator!= (const BlockDeviceConnection& other) const noexcept
{
    return ! operator== (other);
}

} // namespace juce
