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

TracktionMarketplaceStatus::TracktionMarketplaceStatus() {}

URL TracktionMarketplaceStatus::getServerAuthenticationURL()
{
    return URL ("https://www.tracktion.com/marketplace/authenticate.php");
}

String TracktionMarketplaceStatus::getWebsiteName()
{
    return "tracktion.com";
}

bool TracktionMarketplaceStatus::doesProductIDMatch (const String& returnedIDFromServer)
{
    return getProductID() == returnedIDFromServer;
}

String TracktionMarketplaceStatus::readReplyFromWebserver (const String& email, const String& password)
{
    URL url (getServerAuthenticationURL()
                .withParameter ("product", getProductID())
                .withParameter ("email", email)
                .withParameter ("pw", password)
                .withParameter ("os", SystemStats::getOperatingSystemName())
                .withParameter ("mach", getLocalMachineIDs()[0]));

    DBG ("Trying to unlock via URL: " << url.toString (true));

    {
        ScopedLock lock (streamCreationLock);
        stream.reset (new WebInputStream (url, true));
    }

    if (stream->connect (nullptr))
    {
        auto thread = Thread::getCurrentThread();
        MemoryOutputStream result;

        while (! (stream->isExhausted() || stream->isError()
                    || (thread != nullptr && thread->threadShouldExit())))
        {
            auto bytesRead = result.writeFromInputStream (*stream, 8192);

            if (bytesRead < 0)
                break;
        }

        return result.toString();
    }

    return {};
}

void TracktionMarketplaceStatus::userCancelled()
{
    ScopedLock lock (streamCreationLock);

    if (stream != nullptr)
        stream->cancel();
}

} // namespace juce
