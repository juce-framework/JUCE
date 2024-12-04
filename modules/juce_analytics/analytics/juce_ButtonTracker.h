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
    A class that automatically sends analytics events to the Analytics singleton
    when a button is clicked.

    @see Analytics, AnalyticsDestination::AnalyticsEvent

    @tags{Analytics}
*/
class JUCE_API  ButtonTracker   : private Button::Listener
{
public:
    //==============================================================================
    /**
        Creating one of these automatically sends analytics events to the Analytics
        singleton when the corresponding button is clicked.

        The name and parameters of the analytics event will be populated from the
        variables supplied here. If clicking changes the button's state then the
        parameters will have a {"ButtonState", "On"/"Off"} entry added.

        @param buttonToTrack               the button to track
        @param triggeredEventName          the name of the generated event
        @param triggeredEventParameters    the parameters to add to the generated
                                           event
        @param triggeredEventType          (optional) an integer to indicate the event
                                           type, which will be set to 0 if not supplied.

        @see Analytics, AnalyticsDestination::AnalyticsEvent
    */
    ButtonTracker (Button& buttonToTrack,
                   const String& triggeredEventName,
                   const StringPairArray& triggeredEventParameters = {},
                   int triggeredEventType = 0);

    /** Destructor. */
    ~ButtonTracker() override;

private:
    /** @internal */
    void buttonClicked (Button*) override;

    Button& button;
    const String eventName;
    const StringPairArray eventParameters;
    const int eventType;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonTracker)
};

} // namespace juce
