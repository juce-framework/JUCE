#include "../JuceLibraryCode/JuceHeader.h"

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

        String appData ("v=1&tid=" + apiKey + "&t=event&");

        StringArray postData;

        for (auto& event : events)
        {
            StringPairArray data;

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
            else
            {
                continue;
            }

            data.set ("cid", event.userID);

            StringArray eventData;

            for (auto& key : data.getAllKeys())
                eventData.add (key + "=" + URL::addEscapeChars (data[key], true));

            postData.add (appData + eventData.joinIntoString ("&"));
        }

        auto url = URL ("https://www.google-analytics.com/batch")
                       .withPOSTData (postData.joinIntoString ("\n"));

        {
            const ScopedLock lock (webStreamCreation);

            if (shouldExit)
                return false;

            webStream = new WebInputStream (url, true);
        }

        const auto success = webStream->connect (nullptr);

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

        if (webStream != nullptr)
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
        ScopedPointer<XmlElement> xml = previouslySavedEvents.getDocumentElement();

        if (xml == nullptr || xml->getTagName() != "events")
            xml = new XmlElement ("events");

        for (auto& event : eventsToSave)
        {
            auto* xmlEvent = new XmlElement ("google_analytics_event");
            xmlEvent->setAttribute ("name", event.name);
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
        ScopedPointer<XmlElement> xml = savedEvents.getDocumentElement();

        if (xml == nullptr || xml->getTagName() != "events")
            return;

        const auto numEvents = xml->getNumChildElements();

        for (auto iEvent = 0; iEvent < numEvents; ++iEvent)
        {
            const auto* xmlEvent = xml->getChildElement (iEvent);

            StringPairArray parameters;
            const auto* xmlParameters = xmlEvent->getChildByName ("parameters");
            const auto numParameters = xmlParameters->getNumAttributes();

            for (auto iParam = 0; iParam < numParameters; ++iParam)
                parameters.set (xmlParameters->getAttributeName (iParam),
                                xmlParameters->getAttributeValue (iParam));

            StringPairArray userProperties;
            const auto* xmlUserProperties = xmlEvent->getChildByName ("user_properties");
            const auto numUserProperties = xmlUserProperties->getNumAttributes();

            for (auto iProp = 0; iProp < numUserProperties; ++iProp)
                userProperties.set (xmlUserProperties->getAttributeName (iProp),
                                    xmlUserProperties->getAttributeValue (iProp));

            restoredEventQueue.push_back ({
                xmlEvent->getStringAttribute ("name"),
                (uint32) xmlEvent->getIntAttribute ("timestamp"),
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
    ScopedPointer<WebInputStream> webStream;

    String apiKey;

    File savedEventsFile;
};
