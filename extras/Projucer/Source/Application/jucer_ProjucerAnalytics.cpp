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

#include "jucer_ProjucerAnalytics.h"

//==============================================================================
ProjucerAnalyticsDestination::ProjucerAnalyticsDestination()
    : ThreadedAnalyticsDestination ("ProjucerAnalyticsThread")
{
    {
        MemoryOutputStream mo;
        if (Base64::convertFromBase64 (mo, BinaryData::nothingtoseehere_txt))
            apiKey = mo.toString();
    }

    auto dataDir = File::getSpecialLocation (File::userApplicationDataDirectory)
                        #if JUCE_MAC
                        .getChildFile ("Application Support")
                        #endif
                        .getChildFile ("Projucer")
                        .getChildFile ("Analytics");

    if (! dataDir.exists())
        dataDir.createDirectory();

    savedEventsFile = dataDir.getChildFile ("analytics_events.xml");

    startAnalyticsThread (initialPeriodMs);
}

ProjucerAnalyticsDestination::~ProjucerAnalyticsDestination()
{
    Thread::sleep (initialPeriodMs);

    stopAnalyticsThread (1000);
}

//==============================================================================
static void setData (const AnalyticsDestination::AnalyticsEvent& event, StringPairArray& data)
{
    data.set ("ea",  event.name);

    if (event.parameters.getAllKeys().contains ("label"))
        data.set ("el", event.parameters.getValue ("label", {}));

    data.addArray (event.userProperties);
}

bool ProjucerAnalyticsDestination::logBatchedEvents (const Array<AnalyticsEvent>& events)
{
    String appData ("v=1&aip=1&tid=" + apiKey);

    StringArray postData;

    for (auto& event : events)
    {
        StringPairArray data;

        data.set ("t", "event");
        data.set ("cid", event.userID);

        switch (event.eventType)
        {
            case ProjucerAnalyticsEvent::appEvent:
            {
                data.set ("ec", "App");
                setData (event, data);

                break;
            }

            case ProjucerAnalyticsEvent::projectEvent:
            {
                data.set ("ec", "Project");
                setData (event, data);

                break;
            }

            case ProjucerAnalyticsEvent::userEvent:
            {
                data.set ("ec", "User");
                setData (event, data);

                break;
            }

            case ProjucerAnalyticsEvent::exampleEvent:
            {
                data.set ("ec", "Example");
                setData (event, data);

                break;
            }

            case ProjucerAnalyticsEvent::startPageEvent:
            {
                data.set ("ec", "Start Page");
                setData (event, data);

                break;
            }

            default:
            {
                // unknown event type!
                jassertfalse;
                break;
            }
        }

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

void ProjucerAnalyticsDestination::stopLoggingEvents()
{
    const ScopedLock lock (webStreamCreation);

    shouldExit = true;

    if (webStream.get() != nullptr)
        webStream->cancel();
}

//==============================================================================
void ProjucerAnalyticsDestination::saveUnloggedEvents (const std::deque<AnalyticsEvent>& eventsToSave)
{
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

void ProjucerAnalyticsDestination::restoreUnloggedEvents (std::deque<AnalyticsEvent>& restoredEventQueue)
{
    XmlDocument savedEvents (savedEventsFile);
    std::unique_ptr<XmlElement> xml (savedEvents.getDocumentElement());

    if (xml.get() == nullptr || xml->getTagName() != "events")
        return;

    auto numEvents = xml->getNumChildElements();

    for (int iEvent = 0; iEvent < numEvents; ++iEvent)
    {
        auto* xmlEvent = xml->getChildElement (iEvent);

        StringPairArray parameters;
        auto* xmlParameters = xmlEvent->getChildByName ("parameters");
        auto numParameters = xmlParameters->getNumAttributes();

        for (int iParam = 0; iParam < numParameters; ++iParam)
            parameters.set (xmlParameters->getAttributeName (iParam),
                            xmlParameters->getAttributeValue (iParam));

        StringPairArray userProperties;
        auto* xmlUserProperties = xmlEvent->getChildByName ("user_properties");
        auto numUserProperties = xmlUserProperties->getNumAttributes();

        for (int iProp = 0; iProp < numUserProperties; ++iProp)
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
