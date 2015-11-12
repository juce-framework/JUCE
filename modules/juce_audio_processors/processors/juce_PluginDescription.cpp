/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

PluginDescription::PluginDescription()
    : uid (0),
      isInstrument (false),
      numInputChannels (0),
      numOutputChannels (0),
      hasSharedContainer (false)
{
}

PluginDescription::~PluginDescription()
{
}

PluginDescription::PluginDescription (const PluginDescription& other)
    : name (other.name),
      descriptiveName (other.descriptiveName),
      pluginFormatName (other.pluginFormatName),
      category (other.category),
      manufacturerName (other.manufacturerName),
      version (other.version),
      fileOrIdentifier (other.fileOrIdentifier),
      lastFileModTime (other.lastFileModTime),
      lastInfoUpdateTime (other.lastInfoUpdateTime),
      uid (other.uid),
      isInstrument (other.isInstrument),
      numInputChannels (other.numInputChannels),
      numOutputChannels (other.numOutputChannels),
      hasSharedContainer (other.hasSharedContainer)
{
}

PluginDescription& PluginDescription::operator= (const PluginDescription& other)
{
    name = other.name;
    descriptiveName = other.descriptiveName;
    pluginFormatName = other.pluginFormatName;
    category = other.category;
    manufacturerName = other.manufacturerName;
    version = other.version;
    fileOrIdentifier = other.fileOrIdentifier;
    uid = other.uid;
    isInstrument = other.isInstrument;
    lastFileModTime = other.lastFileModTime;
    lastInfoUpdateTime = other.lastInfoUpdateTime;
    numInputChannels = other.numInputChannels;
    numOutputChannels = other.numOutputChannels;
    hasSharedContainer = other.hasSharedContainer;

    return *this;
}

bool PluginDescription::isDuplicateOf (const PluginDescription& other) const noexcept
{
    return fileOrIdentifier == other.fileOrIdentifier
            && uid == other.uid;
}

static String getPluginDescSuffix (const PluginDescription& d)
{
    return "-" + String::toHexString (d.fileOrIdentifier.hashCode())
         + "-" + String::toHexString (d.uid);
}

bool PluginDescription::matchesIdentifierString (const String& identifierString) const
{
    return identifierString.endsWithIgnoreCase (getPluginDescSuffix (*this));
}

String PluginDescription::createIdentifierString() const
{
    return pluginFormatName + "-" + name + getPluginDescSuffix (*this);
}

XmlElement* PluginDescription::createXml() const
{
    XmlElement* const e = new XmlElement ("PLUGIN");
    e->setAttribute ("name", name);
    if (descriptiveName != name)
        e->setAttribute ("descriptiveName", descriptiveName);

    e->setAttribute ("format", pluginFormatName);
    e->setAttribute ("category", category);
    e->setAttribute ("manufacturer", manufacturerName);
    e->setAttribute ("version", version);
    e->setAttribute ("file", fileOrIdentifier);
    e->setAttribute ("uid", String::toHexString (uid));
    e->setAttribute ("isInstrument", isInstrument);
    e->setAttribute ("fileTime", String::toHexString (lastFileModTime.toMilliseconds()));
    e->setAttribute ("infoUpdateTime", String::toHexString (lastInfoUpdateTime.toMilliseconds()));
    e->setAttribute ("numInputs", numInputChannels);
    e->setAttribute ("numOutputs", numOutputChannels);
    e->setAttribute ("isShell", hasSharedContainer);

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
        uid                 = xml.getStringAttribute ("uid").getHexValue32();
        isInstrument        = xml.getBoolAttribute ("isInstrument", false);
        lastFileModTime     = Time (xml.getStringAttribute ("fileTime").getHexValue64());
        lastInfoUpdateTime  = Time (xml.getStringAttribute ("infoUpdateTime").getHexValue64());
        numInputChannels    = xml.getIntAttribute ("numInputs");
        numOutputChannels   = xml.getIntAttribute ("numOutputs");
        hasSharedContainer  = xml.getBoolAttribute ("isShell", false);

        return true;
    }

    return false;
}
