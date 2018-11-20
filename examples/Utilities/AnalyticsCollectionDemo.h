/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             AnalyticsCollectionDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Collects analytics data.

 dependencies:     juce_analytics, juce_core, juce_data_structures, juce_events,
                   juce_graphics, juce_gui_basics
 exporters:        xcode_mac, vs2017, linux_make, xcode_iphone, androidstudio

 type:             Component
 mainClass:        AnalyticsCollectionDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
enum DemoAnalyticsEventTypes
{
    event,
    sessionStart,
    sessionEnd,
    screenView,
    exception
};

//==============================================================================
class GoogleAnalyticsDestination  : public ThreadedAnalyticsDestination
{
public:
    GoogleAnalyticsDestination()
        : ThreadedAnalyticsDestination ("GoogleAnalyticsThread")
    {
        {
            // Choose where to save any unsent events.

            auto appDataDir = File::getSpecialLocation (File::userApplicationDataDirectory)
                                   .getChildFile (JUCEApplication::getInstance()->getApplicationName());

            if (! appDataDir.exists())
                appDataDir.createDirectory();

            savedEventsFile = appDataDir.getChildFile ("analytics_events.xml");
        }

        {
            // It's often a good idea to construct any analytics service API keys
            // at runtime, so they're not searchable in the binary distribution of
            // your application (but we've not done this here). You should replace
            // the following key with your own to get this example application
            // fully working.

            apiKey = "UA-XXXXXXXXX-1";
        }

        startAnalyticsThread (initialPeriodMs);
    }

    ~GoogleAnalyticsDestination()
    {
        // Here we sleep so that our background thread has a chance to send the
        // last lot of batched events. Be careful - if your app takes too long to
        // shut down then some operating systems will kill it forcibly!
        Thread::sleep (initialPeriodMs);

        stopAnalyticsThread (1000);
    }

    int getMaximumBatchSize() override   { return 20; }

    bool logBatchedEvents (const Array<AnalyticsEvent>& events) override
    {
        // Send events to Google Analytics.

        String appData ("v=1&aip=1&tid=" + apiKey);

        StringArray postData;

        for (auto& event : events)
        {
            StringPairArray data;

            switch (event.eventType)
            {
                case (DemoAnalyticsEventTypes::event):
                {
                    data.set ("t", "event");

                    if (event.name == "startup")
                    {
                        data.set ("ec",  "info");
                        data.set ("ea",  "appStarted");
                    }
                    else if (event.name == "shutdown")
                    {
                        data.set ("ec",  "info");
                        data.set ("ea",  "appStopped");
                    }
                    else if (event.name == "button_press")
                    {
                        data.set ("ec",  "button_press");
                        data.set ("ea",  event.parameters["id"]);
                    }
                    else if (event.name == "crash")
                    {
                        data.set ("ec",  "crash");
                        data.set ("ea",  "crash");
                    }
                    else
                    {
                        jassertfalse;
                        continue;
                    }

                    break;
                }

                default:
                {
                    // Unknown event type! In this demo app we're just using a
                    // single event type, but in a real app you probably want to
                    // handle multiple ones.
                    jassertfalse;
                    break;
                }
            }

            data.set ("cid", event.userID);

            StringArray eventData;

            for (auto& key : data.getAllKeys())
                eventData.add (key + "=" + URL::addEscapeChars (data[key], true));

            postData.add (appData + "&" + eventData.joinIntoString ("&"));
        }

        auto url = URL ("https://www.google-analytics.com/batch")
                       .withPOSTData (postData.joinIntoString ("\n"));

        {
            const ScopedLock lock (webStreamCreation);

            if (shouldExit)
                return false;

            webStream.reset (new WebInputStream (url, true));
        }

        auto success = webStream->connect (nullptr);

        // Do an exponential backoff if we failed to connect.
        if (success)
            periodMs = initialPeriodMs;
        else
            periodMs *= 2;

        setBatchPeriod (periodMs);

        return success;
    }

    void stopLoggingEvents() override
    {
        const ScopedLock lock (webStreamCreation);

        shouldExit = true;

        if (webStream.get() != nullptr)
            webStream->cancel();
    }

private:
    void saveUnloggedEvents (const std::deque<AnalyticsEvent>& eventsToSave) override
    {
        // Save unsent events to disk. Here we use XML as a serialisation format, but
        // you can use anything else as long as the restoreUnloggedEvents method can
        // restore events from disk. If you're saving very large numbers of events then
        // a binary format may be more suitable if it is faster - remember that this
        // method is called on app shutdown so it needs to complete quickly!

        XmlDocument previouslySavedEvents (savedEventsFile);
        std::unique_ptr<XmlElement> xml (previouslySavedEvents.getDocumentElement());

        if (xml.get() == nullptr || xml->getTagName() != "events")
            xml.reset (new XmlElement ("events"));

        for (auto& event : eventsToSave)
        {
            auto* xmlEvent = new XmlElement ("google_analytics_event");
            xmlEvent->setAttribute ("name", event.name);
            xmlEvent->setAttribute ("type", event.eventType);
            xmlEvent->setAttribute ("timestamp", (int) event.timestamp);
            xmlEvent->setAttribute ("user_id", event.userID);

            auto* parameters = new XmlElement ("parameters");

            for (auto& key : event.parameters.getAllKeys())
                parameters->setAttribute (key, event.parameters[key]);

            xmlEvent->addChildElement (parameters);

            auto* userProperties = new XmlElement ("user_properties");

            for (auto& key : event.userProperties.getAllKeys())
                userProperties->setAttribute (key, event.userProperties[key]);

            xmlEvent->addChildElement (userProperties);

            xml->addChildElement (xmlEvent);
        }

        xml->writeToFile (savedEventsFile, {});
    }

    void restoreUnloggedEvents (std::deque<AnalyticsEvent>& restoredEventQueue) override
    {
        XmlDocument savedEvents (savedEventsFile);
        std::unique_ptr<XmlElement> xml (savedEvents.getDocumentElement());

        if (xml.get() == nullptr || xml->getTagName() != "events")
            return;

        auto numEvents = xml->getNumChildElements();

        for (auto iEvent = 0; iEvent < numEvents; ++iEvent)
        {
            auto* xmlEvent = xml->getChildElement (iEvent);

            StringPairArray parameters;
            auto* xmlParameters = xmlEvent->getChildByName ("parameters");
            auto numParameters = xmlParameters->getNumAttributes();

            for (auto iParam = 0; iParam < numParameters; ++iParam)
                parameters.set (xmlParameters->getAttributeName (iParam),
                                xmlParameters->getAttributeValue (iParam));

            StringPairArray userProperties;
            auto* xmlUserProperties = xmlEvent->getChildByName ("user_properties");
            auto numUserProperties = xmlUserProperties->getNumAttributes();

            for (auto iProp = 0; iProp < numUserProperties; ++iProp)
                userProperties.set (xmlUserProperties->getAttributeName (iProp),
                                    xmlUserProperties->getAttributeValue (iProp));

            restoredEventQueue.push_back ({
                xmlEvent->getStringAttribute ("name"),
                xmlEvent->getIntAttribute ("type"),
                static_cast<uint32> (xmlEvent->getIntAttribute ("timestamp")),
                parameters,
                xmlEvent->getStringAttribute ("user_id"),
                userProperties
            });
        }

        savedEventsFile.deleteFile();
    }

    const int initialPeriodMs = 1000;
    int periodMs = initialPeriodMs;

    CriticalSection webStreamCreation;
    bool shouldExit = false;
    std::unique_ptr<WebInputStream> webStream;

    String apiKey;

    File savedEventsFile;
};

//==============================================================================
class AnalyticsCollectionDemo   : public Component
{
public:
    //==============================================================================
    AnalyticsCollectionDemo()
    {
        // Add an analytics identifier for the user. Make sure you don't accidentally
        // collect identifiable information if you haven't asked for permission!
        Analytics::getInstance()->setUserId ("AnonUser1234");

        // Add any other constant user information.
        StringPairArray userData;
        userData.set ("group", "beta");
        Analytics::getInstance()->setUserProperties (userData);

        // Add any analytics destinations we want to use to the Analytics singleton.
        Analytics::getInstance()->addDestination (new GoogleAnalyticsDestination());

        // The event type here should probably be DemoAnalyticsEventTypes::sessionStart
        // in a more advanced app.
        Analytics::getInstance()->logEvent ("startup", {}, DemoAnalyticsEventTypes::event);

        crashButton.onClick = [this] { sendCrash(); };

        addAndMakeVisible (eventButton);
        addAndMakeVisible (crashButton);

        setSize (300, 200);

        StringPairArray logButtonPressParameters;
        logButtonPressParameters.set ("id", "a");
        logEventButtonPress.reset (new ButtonTracker (eventButton, "button_press", logButtonPressParameters));
    }

    ~AnalyticsCollectionDemo()
    {
        // The event type here should probably be DemoAnalyticsEventTypes::sessionEnd
        // in a more advanced app.
        Analytics::getInstance()->logEvent ("shutdown", {}, DemoAnalyticsEventTypes::event);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        eventButton.centreWithSize (100, 40);
        eventButton.setBounds (eventButton.getBounds().translated (0, 25));
        crashButton.setBounds (eventButton.getBounds().translated (0, -50));
    }

private:
    //==============================================================================
    void sendCrash()
    {
        // In a more advanced application you would probably use a different event
        // type here.
        Analytics::getInstance()->logEvent ("crash", {}, DemoAnalyticsEventTypes::event);
        Analytics::getInstance()->getDestinations().clear();
        JUCEApplication::getInstance()->shutdown();
    }

    TextButton eventButton { "Press me!" }, crashButton { "Simulate crash!" };
    std::unique_ptr<ButtonTracker> logEventButtonPress;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalyticsCollectionDemo)
};
