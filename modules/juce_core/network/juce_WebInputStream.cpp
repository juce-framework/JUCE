/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

WebInputStream::WebInputStream (const URL& url, const bool usePost)
    : pimpl (std::make_unique<Pimpl> (*this, url, usePost))
{
}

WebInputStream::~WebInputStream()
{
}

WebInputStream& WebInputStream::withExtraHeaders (const String& extra)         { pimpl->withExtraHeaders (extra);       return *this; }
WebInputStream& WebInputStream::withCustomRequestCommand (const String& cmd)   { pimpl->withCustomRequestCommand(cmd);  return *this; }
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

    // ignore the first line as this is the status line
    for (int i = 1; i < headerLines.size(); ++i)
    {
        const auto& headersEntry = headerLines[i];

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
