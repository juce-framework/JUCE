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

//==============================================================================
/**
    A singleton class to manage analytics data.

    Use an Analytics object to manage sending analytics data to one or more
    AnalyticsDestinations.

    @see AnalyticsDestination, ThreadedAnalyticsDestination,
         AnalyticsDestination::AnalyticsEvent
*/
class JUCE_API  Analytics   : public DeletedAtShutdown
{
public:
    //==============================================================================
    /** Adds an AnalyticsDestination to the list of AnalyticsDestinations
        managed by this Analytics object.

        The Analytics class will take ownership of the AnalyticsDestination
        passed to this function.

        @param destination          the AnalyticsDestination to manage
    */
    void addDestination (AnalyticsDestination* destination);

    /** Sets a user ID that will be added to all AnalyticsEvents sent to
        AnalyticsDestinations.

        @param newUserId            the userId to add to AnalyticsEvents
    */
    void setUserId (const String& newUserId);

    /** Sets some user properties that will be added to all AnalyticsEvents sent
        to AnalyticsDestinations.

        @param properties           the userProperties to add to AnalyticsEvents
    */
    void setUserProperties (const StringPairArray& properties);

    /** Sends an AnalyticsEvent to all AnalyticsDestinations.

        The AnalyticsEvent will be timestamped, and will have the userId and
        userProperties populated by values previously set by calls to
        setUserId and setUserProperties. The name and parameters will be
        populated by the arguments supplied to this function.

        @param eventName            the event name
        @param parameters           the event parameters
    */
    void logEvent (const String& eventName, const StringPairArray& parameters);

    /** Suspends analytics submission to AnalyticsDestinations.

        @param shouldBeSuspended    if event submission should be suspended
    */
    void setSuspended (bool shouldBeSuspended);


    juce_DeclareSingleton (Analytics, true)

private:
    //==============================================================================
    Analytics() = default;

    String userId;
    StringPairArray userProperties;

    bool isSuspended = false;

    OwnedArray<AnalyticsDestination> destinations;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analytics)
};

} // namespace juce
