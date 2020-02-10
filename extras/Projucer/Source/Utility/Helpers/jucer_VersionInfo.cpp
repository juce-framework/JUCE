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

#include "../../Application/jucer_Headers.h"
#include "jucer_VersionInfo.h"

std::unique_ptr<VersionInfo> VersionInfo::fetchFromUpdateServer (const String& versionString)
{
    return fetch ("tags/" + versionString);
}

std::unique_ptr<VersionInfo> VersionInfo::fetchLatestFromUpdateServer()
{
    return fetch ("latest");
}

std::unique_ptr<InputStream> VersionInfo::createInputStreamForAsset (const Asset& asset, int& statusCode)
{
    URL downloadUrl (asset.url);
    StringPairArray responseHeaders;

    return std::unique_ptr<InputStream> (downloadUrl.createInputStream (false, nullptr, nullptr,
                                                                        "Accept: application/octet-stream",
                                                                        0, &responseHeaders, &statusCode, 1));
}

bool VersionInfo::isNewerVersionThanCurrent()
{
    jassert (versionString.isNotEmpty());

    auto currentTokens = StringArray::fromTokens (ProjectInfo::versionString, ".", {});
    auto thisTokens    = StringArray::fromTokens (versionString, ".", {});

    jassert (thisTokens.size() == 3 && thisTokens.size() == 3);

    if (currentTokens[0].getIntValue() == thisTokens[0].getIntValue())
    {
        if (currentTokens[1].getIntValue() == thisTokens[1].getIntValue())
            return currentTokens[2].getIntValue() < thisTokens[2].getIntValue();

        return currentTokens[1].getIntValue() < thisTokens[1].getIntValue();
    }

    return currentTokens[0].getIntValue() < thisTokens[0].getIntValue();
}

std::unique_ptr<VersionInfo> VersionInfo::fetch (const String& endpoint)
{
    URL latestVersionURL ("https://api.github.com/repos/WeAreROLI/JUCE/releases/" + endpoint);
    std::unique_ptr<InputStream> inStream (latestVersionURL.createInputStream (false));

    if (inStream == nullptr)
        return nullptr;

    auto content = inStream->readEntireStreamAsString();
    auto latestReleaseDetails = JSON::parse (content);

    auto* json = latestReleaseDetails.getDynamicObject();

    if (json == nullptr)
        return nullptr;

    auto versionString = json->getProperty ("tag_name").toString();

    if (versionString.isEmpty())
        return nullptr;

    auto* assets = json->getProperty ("assets").getArray();

    if (assets == nullptr)
        return nullptr;

    auto releaseNotes = json->getProperty ("body").toString();
    std::vector<VersionInfo::Asset> parsedAssets;

    for (auto& asset : *assets)
    {
        if (auto* assetJson = asset.getDynamicObject())
        {
            parsedAssets.push_back ({ assetJson->getProperty ("name").toString(),
                                      assetJson->getProperty ("url").toString() });
            jassert (parsedAssets.back().name.isNotEmpty());
            jassert (parsedAssets.back().url.isNotEmpty());
        }
        else
        {
            jassertfalse;
        }
    }

    return std::unique_ptr<VersionInfo> (new VersionInfo ({ versionString, releaseNotes, std::move (parsedAssets) }));
}
