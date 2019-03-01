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

struct PhysicalTopologySource::DetectorHolder  : private Timer
{
    DetectorHolder (PhysicalTopologySource& pts)
        : topologySource (pts),
          detector (Detector::getDefaultDetector())
    {
        startTimerHz (30);
    }

    DetectorHolder (PhysicalTopologySource& pts, DeviceDetector& dd)
        : topologySource (pts),
          detector (new Detector (dd))
    {
        startTimerHz (30);
    }

    void timerCallback() override
    {
        if (! topologySource.hasOwnServiceTimer())
            handleTimerTick();
    }

    void handleTimerTick()
    {
        for (auto& b : detector->currentTopology.blocks)
            if (auto bi = BlockImplementation<Detector>::getFrom (*b))
                bi->handleTimerTick();
    }

    PhysicalTopologySource& topologySource;
    Detector::Ptr detector;
};

} // namespace juce
