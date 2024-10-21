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

WebInputStream::WebInputStream (const URL& url, const bool usePost)
    : pimpl (std::make_unique<Pimpl> (*this, url, usePost))
{
}

WebInputStream::~WebInputStream()
{
}

WebInputStream& WebInputStream::withExtraHeaders (const String& extra)         { pimpl->withExtraHeaders (extra);       return *this; }
WebInputStream& WebInputStream::withCustomRequestCommand (const String& cmd)   { pimpl->withCustomRequestCommand (cmd); return *this; }
WebInputStream& WebInputStream::withConnectionTimeout (int t)                  { pimpl->withConnectionTimeout (t);      return *this; }
WebInputStream& WebInputStream::withNumRedirectsToFollow (int num)             { pimpl->withNumRedirectsToFollow (num); return *this; }
StringPairArray WebInputStream::getRequestHeaders() const                      { return pimpl->getRequestHeaders(); }
StringPairArray WebInputStream::getResponseHeaders()                           { connect (nullptr); return pimpl->getResponseHeaders(); }
bool WebInputStream::isError() const                                           { return pimpl->isError(); }
void WebInputStream::cancel()                                                  { pimpl->cancel(); }
bool WebInputStream::isExhausted()                                             { return pimpl->isExhausted(); }
int64 WebInputStream::getPosition()                                            { return pimpl->getPosition(); }
int64 WebInputStream::getTotalLength()                                         { connect (nullptr); return pimpl->getTotalLength(); }
int WebInputStream::read (void* buffer, int bytes)                             { connect (nullptr); return pimpl->read (buffer, bytes); }
bool WebInputStream::setPosition (int64 pos)                                   { return pimpl->setPosition (pos); }
int WebInputStream::getStatusCode()                                            { connect (nullptr); return pimpl->getStatusCode(); }

bool WebInputStream::connect (Listener* listener)
{
    if (hasCalledConnect)
        return ! isError();

    hasCalledConnect = true;
    return pimpl->connect (listener);
}

StringPairArray WebInputStream::parseHttpHeaders (const String& headerData)
{
    StringPairArray headerPairs;
    auto headerLines = StringArray::fromLines (headerData);

    for (const auto& headersEntry : headerLines)
    {
        if (headersEntry.isNotEmpty())
        {
            const auto key = headersEntry.upToFirstOccurrenceOf (": ", false, false);

            auto value = [&headersEntry, &headerPairs, &key]
            {
                const auto currentValue = headersEntry.fromFirstOccurrenceOf (": ", false, false);
                const auto previousValue = headerPairs [key];

                if (previousValue.isNotEmpty())
                    return previousValue + "," + currentValue;

                return currentValue;
            }();

            headerPairs.set (key, value);
        }
    }

    return headerPairs;
}

void WebInputStream::createHeadersAndPostData (const URL& aURL,
                                               String& headers,
                                               MemoryBlock& data,
                                               bool addParametersToBody)
{
    aURL.createHeadersAndPostData (headers, data, addParametersToBody);
}

bool WebInputStream::Listener::postDataSendProgress ([[maybe_unused]] WebInputStream& request,
                                                     [[maybe_unused]] int bytesSent,
                                                     [[maybe_unused]] int totalBytes)
{
    return true;
}

} // namespace juce
