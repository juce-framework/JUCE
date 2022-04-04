/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
