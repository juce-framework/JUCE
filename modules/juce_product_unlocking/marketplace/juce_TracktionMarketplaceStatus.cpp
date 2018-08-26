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
        auto* thread = Thread::getCurrentThread();

        if (thread->threadShouldExit() || stream->isError())
            return {};

        auto contentLength = stream->getTotalLength();
        auto downloaded    = 0;

        const size_t bufferSize = 0x8000;
        HeapBlock<char> buffer (bufferSize);

        while (! (stream->isExhausted() || stream->isError() || thread->threadShouldExit()))
        {
            auto max = jmin ((int) bufferSize, contentLength < 0 ? std::numeric_limits<int>::max()
                                                                 : static_cast<int> (contentLength - downloaded));

            auto actualBytesRead = stream->read (buffer.get() + downloaded, max - downloaded);

            if (actualBytesRead < 0 || thread->threadShouldExit() || stream->isError())
                break;

            downloaded += actualBytesRead;

            if (downloaded == contentLength)
                break;
        }

        if (thread->threadShouldExit() || stream->isError() || (contentLength > 0 && downloaded < contentLength))
            return {};

        return { CharPointer_UTF8 (buffer.get()), static_cast<size_t> (downloaded) };
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
