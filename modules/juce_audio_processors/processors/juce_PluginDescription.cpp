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

bool PluginDescription::isDuplicateOf (const PluginDescription& other) const noexcept
{
    if ((other.fileOrIdentifier == fileOrIdentifier) &&
        (other.deprecatedUid == deprecatedUid))
    {
        // Only compare uniqueId field if it is set in both descriptions
        if ((other.uniqueId > 0) && (uniqueId > 0))
        {
            return (other.uniqueId == uniqueId);
        }
        else
        {
            return true;
        }
    }

    return false;
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
    e->setAttribute ("hasARAExtension", hasARAExtension);

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
        hasARAExtension     = xml.getBoolAttribute ("hasARAExtension", false);

        deprecatedUid       = xml.getStringAttribute ("uid").getHexValue32();
        uniqueId            = xml.getStringAttribute ("uniqueId", "0").getHexValue32();

        return true;
    }

    return false;
}

} // namespace juce
