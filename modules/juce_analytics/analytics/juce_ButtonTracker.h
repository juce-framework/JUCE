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
    A class that automatically sends analytics events to the Analytics singleton
    when a button is clicked.

    @see Analytics, AnalyticsDestination::AnalyticsEvent
*/
class JUCE_API  ButtonTracker   : private Button::Listener
{
public:
    //==============================================================================
    /**
        Creating one of these automatically sends analytics events to the Analytics
        singeton when the corresponding button is clicked.

        The name and parameters of the analytics event will be populated from the
        variables supplied here. If clicking changes the button's state then the
        parameters will have a {"ButtonState", "On"/"Off"} entry added.

        @param buttonToTrack               the button to track
        @param triggeredEventName          the name of the generated event
        @param triggeredEventParameters    the parameters to add to the generated
                                           event

        @see Analytics, AnalyticsDestination::AnalyticsEvent
    */
    ButtonTracker (Button& buttonToTrack,
                   const String& triggeredEventName,
                   const StringPairArray& triggeredEventParameters = {});

    /** Destructor. */
    ~ButtonTracker();

private:
    /** @internal */
    void buttonClicked (Button*) override;

    Button& button;
    const String eventName;
    const StringPairArray eventParameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonTracker)
};

} // namespace juce
