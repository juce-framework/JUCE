/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

bool PluginDescription::isDuplicateOf (const PluginDescription& other) const noexcept
{
    const auto tie = [] (const PluginDescription& d)
    {
        return std::tie (d.fileOrIdentifier, d.deprecatedUid, d.uniqueId);
    };

    return tie (*this) == tie (other);
}

static String getPluginDescSuffix (const PluginDescription& d, int uid)
{
    return "-" + String::toHexString (d.fileOrIdentifier.hashCode())
         + "-" + String::toHexString (uid);
}

bool PluginDescription::matchesIdentifierString (const String& identifierString) const
{
    const auto matches = [&] (int uid)
    {
        return identifierString.endsWithIgnoreCase (getPluginDescSuffix (*this, uid));
    };

    return matches (uniqueId) || matches (deprecatedUid);
}

String PluginDescription::createIdentifierString() const
{
    const auto idToUse = uniqueId != 0 ? uniqueId : deprecatedUid;
    return pluginFormatName + "-" + name + getPluginDescSuffix (*this, idToUse);
}

std::unique_ptr<XmlElement> PluginDescription::createXml() const
{
    auto e = std::make_unique<XmlElement> ("PLUGIN");

    e->setAttribute ("name", name);

    if (descriptiveName != name)
        e->setAttribute ("descriptiveName", descriptiveName);

    e->setAttribute ("format", pluginFormatName);
    e->setAttribute ("category", category);
    e->setAttribute ("manufacturer", manufacturerName);
    e->setAttribute ("version", version);
    e->setAttribute ("file", fileOrIdentifier);
    e->setAttribute ("uniqueId", String::toHexString (uniqueId));
    e->setAttribute ("isInstrument", isInstrument);
    e->setAttribute ("fileTime", String::toHexString (lastFileModTime.toMilliseconds()));
    e->setAttribute ("infoUpdateTime", String::toHexString (lastInfoUpdateTime.toMilliseconds()));
    e->setAttribute ("numInputs", numInputChannels);
    e->setAttribute ("numOutputs", numOutputChannels);
    e->setAttribute ("isShell", hasSharedContainer);

    e->setAttribute ("uid", String::toHexString (deprecatedUid));

    return e;
}

bool PluginDescription::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName ("PLUGIN"))
    {
        name                = xml.getStringAttribute ("name");
        descriptiveName     = xml.getStringAttribute ("descriptiveName", name);
        pluginFormatName    = xml.getStringAttribute ("format");
        category            = xml.getStringAttribute ("category");
        manufacturerName    = xml.getStringAttribute ("manufacturer");
        version             = xml.getStringAttribute ("version");
        fileOrIdentifier    = xml.getStringAttribute ("file");
        isInstrument        = xml.getBoolAttribute ("isInstrument", false);
        lastFileModTime     = Time (xml.getStringAttribute ("fileTime").getHexValue64());
        lastInfoUpdateTime  = Time (xml.getStringAttribute ("infoUpdateTime").getHexValue64());
        numInputChannels    = xml.getIntAttribute ("numInputs");
        numOutputChannels   = xml.getIntAttribute ("numOutputs");
        hasSharedContainer  = xml.getBoolAttribute ("isShell", false);

        deprecatedUid       = xml.getStringAttribute ("uid").getHexValue32();
        uniqueId            = xml.getStringAttribute ("uniqueId", "0").getHexValue32();

        return true;
    }

    return false;
}

} // namespace juce
