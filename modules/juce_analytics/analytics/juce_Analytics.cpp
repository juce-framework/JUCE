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

void Analytics::addDestination (AnalyticsDestination* destination)
{
    destinations.add (destination);
}

OwnedArray<AnalyticsDestination>& Analytics::getDestinations()
{
    return destinations;
}

void Analytics::setUserId (const String& newUserId)
{
    userId = newUserId;
}

void Analytics::setUserProperties (const StringPairArray& properties)
{
    userProperties = properties;
}

void Analytics::logEvent (const String& eventName,
                          const StringPairArray& parameters,
                          int eventType)
{
    if (! isSuspended)
    {
        AnalyticsDestination::AnalyticsEvent event
        {
            eventName,
            eventType,
            Time::getMillisecondCounter(),
            parameters,
            userId,
            userProperties
        };

        for (auto* destination : destinations)
            destination->logEvent (event);
    }
}

void Analytics::setSuspended (bool shouldBeSuspended)
{
    isSuspended = shouldBeSuspended;
}

JUCE_IMPLEMENT_SINGLETON (Analytics)

}
