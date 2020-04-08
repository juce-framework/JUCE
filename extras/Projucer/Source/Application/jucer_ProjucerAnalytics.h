/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2017 - ROLI Ltd.

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
    ~ProjucerAnalyticsDestination() override;

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
