/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_Headers.h"

//==============================================================================
enum ProjucerAnalyticsEvent
{
    appEvent,
    projectEvent,
    userEvent,
    exampleEvent,
    startPageEvent
};

//==============================================================================
class ProjucerAnalyticsDestination    : public ThreadedAnalyticsDestination
{
public:
    ProjucerAnalyticsDestination();
    ~ProjucerAnalyticsDestination();

    //==============================================================================
    bool logBatchedEvents (const Array<AnalyticsEvent>&) override;
    void stopLoggingEvents() override;

    int getMaximumBatchSize() override    { return 20; }

private:
    void saveUnloggedEvents (const std::deque<AnalyticsEvent>&) override;
    void restoreUnloggedEvents (std::deque<AnalyticsEvent>&) override;

    //==============================================================================
    String apiKey;

    const int initialPeriodMs = 1000;
    int periodMs = initialPeriodMs;

    CriticalSection webStreamCreation;
    bool shouldExit = false;
    std::unique_ptr<WebInputStream> webStream;

    File savedEventsFile;
};
