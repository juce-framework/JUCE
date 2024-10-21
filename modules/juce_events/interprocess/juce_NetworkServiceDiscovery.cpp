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

#if JUCE_ANDROID
 extern void acquireMulticastLock();
 extern void releaseMulticastLock();
#endif

NetworkServiceDiscovery::Advertiser::Advertiser (const String& serviceTypeUID,
                                                 const String& serviceDescription,
                                                 int broadcastPortToUse, int connectionPort,
                                                 RelativeTime minTimeBetweenBroadcasts)
    : Thread (SystemStats::getJUCEVersion() + ": Discovery_broadcast"),
      message (serviceTypeUID), broadcastPort (broadcastPortToUse),
      minInterval (minTimeBetweenBroadcasts)
{
    message.setAttribute ("id", Uuid().toString());
    message.setAttribute ("name", serviceDescription);
    message.setAttribute ("address", String());
    message.setAttribute ("port", connectionPort);

    startThread (Priority::background);
}

NetworkServiceDiscovery::Advertiser::~Advertiser()
{
    stopThread (2000);
    socket.shutdown();
}

void NetworkServiceDiscovery::Advertiser::run()
{
    if (! socket.bindToPort (0))
    {
        jassertfalse;
        return;
    }

    while (! threadShouldExit())
    {
        sendBroadcast();
        wait ((int) minInterval.inMilliseconds());
    }
}

void NetworkServiceDiscovery::Advertiser::sendBroadcast()
{
    static IPAddress local = IPAddress::local();

    for (auto& address : IPAddress::getAllAddresses())
    {
        if (address == local)
            continue;

        message.setAttribute ("address", address.toString());

        auto broadcastAddress = IPAddress::getInterfaceBroadcastAddress (address);
        auto data = message.toString (XmlElement::TextFormat().singleLine().withoutHeader());

        socket.write (broadcastAddress.toString(), broadcastPort, data.toRawUTF8(), (int) data.getNumBytesAsUTF8());
    }
}

//==============================================================================
NetworkServiceDiscovery::AvailableServiceList::AvailableServiceList (const String& serviceType, int broadcastPort)
    : Thread (SystemStats::getJUCEVersion() + ": Discovery_listen"), serviceTypeUID (serviceType)
{
   #if JUCE_ANDROID
    acquireMulticastLock();
   #endif

    socket.bindToPort (broadcastPort);
    startThread (Priority::background);
}

NetworkServiceDiscovery::AvailableServiceList::~AvailableServiceList()
{
    socket.shutdown();
    stopThread (2000);

    #if JUCE_ANDROID
     releaseMulticastLock();
    #endif
}

void NetworkServiceDiscovery::AvailableServiceList::run()
{
    while (! threadShouldExit())
    {
        if (socket.waitUntilReady (true, 200) == 1)
        {
            char buffer[1024];
            auto bytesRead = socket.read (buffer, sizeof (buffer) - 1, false);

            if (bytesRead > 10)
                if (auto xml = parseXML (String (CharPointer_UTF8 (buffer),
                                                 CharPointer_UTF8 (buffer + bytesRead))))
                    if (xml->hasTagName (serviceTypeUID))
                        handleMessage (*xml);
        }

        removeTimedOutServices();
    }
}

std::vector<NetworkServiceDiscovery::Service> NetworkServiceDiscovery::AvailableServiceList::getServices() const
{
    const ScopedLock sl (listLock);
    auto listCopy = services;
    return listCopy;
}

void NetworkServiceDiscovery::AvailableServiceList::handleAsyncUpdate()
{
    NullCheckedInvocation::invoke (onChange);
}

void NetworkServiceDiscovery::AvailableServiceList::handleMessage (const XmlElement& xml)
{
    Service service;
    service.instanceID = xml.getStringAttribute ("id");

    if (service.instanceID.trim().isNotEmpty())
    {
        service.description = xml.getStringAttribute ("name");
        service.address = IPAddress (xml.getStringAttribute ("address"));
        service.port = xml.getIntAttribute ("port");
        service.lastSeen = Time::getCurrentTime();

        handleMessage (service);
    }
}

static void sortServiceList (std::vector<NetworkServiceDiscovery::Service>& services)
{
    auto compareServices = [] (const NetworkServiceDiscovery::Service& s1,
                               const NetworkServiceDiscovery::Service& s2)
    {
        return s1.instanceID < s2.instanceID;
    };

    std::sort (services.begin(), services.end(), compareServices);
}

void NetworkServiceDiscovery::AvailableServiceList::handleMessage (const Service& service)
{
    const ScopedLock sl (listLock);

    for (auto& s : services)
    {
        if (s.instanceID == service.instanceID)
        {
            if (s.description != service.description
                 || s.address != service.address
                 || s.port != service.port)
            {
                s = service;
                triggerAsyncUpdate();
            }

            s.lastSeen = service.lastSeen;
            return;
        }
    }

    services.push_back (service);
    sortServiceList (services);
    triggerAsyncUpdate();
}

void NetworkServiceDiscovery::AvailableServiceList::removeTimedOutServices()
{
    const double timeoutSeconds = 5.0;
    auto oldestAllowedTime = Time::getCurrentTime() - RelativeTime::seconds (timeoutSeconds);

    const ScopedLock sl (listLock);

    auto oldEnd = std::end (services);
    auto newEnd = std::remove_if (std::begin (services), oldEnd,
                                  [=] (const Service& s) { return s.lastSeen < oldestAllowedTime; });

    if (newEnd != oldEnd)
    {
        services.erase (newEnd, oldEnd);
        triggerAsyncUpdate();
    }
}

} // namespace juce
