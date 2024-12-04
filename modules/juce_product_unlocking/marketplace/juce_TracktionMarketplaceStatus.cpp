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
