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
    A singleton class to manage analytics data.

    Use an Analytics object to manage sending analytics data to one or more
    AnalyticsDestinations.

    @see AnalyticsDestination, ThreadedAnalyticsDestination,
         AnalyticsDestination::AnalyticsEvent

    @tags{Analytics}
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

    /** Returns the array of AnalyticsDestinations managed by this class.

        If you have added any subclasses of ThreadedAnalyticsDestination to
        this class then you can remove them from this list to force them to
        flush any pending events.
    */
    OwnedArray<AnalyticsDestination>& getDestinations();

    /** Sets a user ID that will be added to all AnalyticsEvents sent to
        AnalyticsDestinations.

        @param newUserId            the userId to add to AnalyticsEvents
    */
    void setUserId (String newUserId);

    /** Sets some user properties that will be added to all AnalyticsEvents sent
        to AnalyticsDestinations.

        @param properties           the userProperties to add to AnalyticsEvents
    */
    void setUserProperties (StringPairArray properties);

    /** Sends an AnalyticsEvent to all AnalyticsDestinations.

        The AnalyticsEvent will be timestamped, and will have the userId and
        userProperties populated by values previously set by calls to
        setUserId and setUserProperties. The name, parameters and type will be
        populated by the arguments supplied to this function.

        @param eventName            the event name
        @param parameters           the event parameters
        @param eventType            (optional) an integer to indicate the event
                                    type, which will be set to 0 if not supplied.
    */
    void logEvent (const String& eventName, const StringPairArray& parameters, int eventType = 0);

    /** Suspends analytics submissions to AnalyticsDestinations.

        @param shouldBeSuspended    if event submission should be suspended
    */
    void setSuspended (bool shouldBeSuspended);

   #ifndef DOXYGEN
    JUCE_DECLARE_SINGLETON_INLINE (Analytics, false)
   #endif

private:
    //==============================================================================
    Analytics() = default;
    ~Analytics() override  { clearSingletonInstance(); }

    String userId;
    StringPairArray userProperties;

    bool isSuspended = false;

    OwnedArray<AnalyticsDestination> destinations;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analytics)
};

} // namespace juce
