/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An interface for handling analytics events collected by an Analytics object.

    For basic analytics logging you can implement this interface and add your
    class to an Analytics object.

    For more advanced logging you may want to subclass
    ThreadedAnalyticsDestination instead, which is more suitable for interacting
    with web servers and other time consuming destinations.

    @see Analytics, ThreadedAnalyticsDestination

    @tags{Analytics}
*/
struct JUCE_API  AnalyticsDestination
{
    /** Contains information about an event to be logged. */
    struct AnalyticsEvent
    {
        /** The name of the event. */
        String name;

        /** An optional integer representing the type of the event. You can use
            this to indicate if the event was a screenview, session start,
            exception, etc.
        */
        int eventType;

        /**
            The timestamp of the event.

            Timestamps are automatically applied by an Analytics object and are
            derived from Time::getMillisecondCounter(). As such these timestamps
            do not represent absolute times, but relative timings of events for
            each user in each session will be accurate.
        */
        uint32 timestamp;

        /** The parameters of the event. */
        StringPairArray parameters;

        /** The user ID associated with the event. */
        String userID;

        /** Properties associated with the user. */
        StringPairArray userProperties;
    };

    /** Constructor. */
    AnalyticsDestination() = default;

    /** Destructor. */
    virtual ~AnalyticsDestination() = default;

    /**
        When an AnalyticsDestination is added to an Analytics object this method
        is called when an analytics event is triggered from the Analytics
        object.

        Override this method to log the event information somewhere useful.
    */
    virtual void logEvent (const AnalyticsEvent& event) = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyticsDestination)
};


} // namespace juce
