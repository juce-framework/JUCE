/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

PluginDescription::PluginDescription()
    : uid (0),
      isInstrument (false),
      numInputChannels (0),
      numOutputChannels (0)
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
      uid (other.uid),
      isInstrument (other.isInstrument),
      numInputChannels (other.numInputChannels),
      numOutputChannels (other.numOutputChannels)
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
    numInputChannels = other.numInputChannels;
    numOutputChannels = other.numOutputChannels;

    return *this;
}

bool PluginDescription::isDuplicateOf (const PluginDescription& other) const
{
    return fileOrIdentifier == other.fileOrIdentifier
            && uid == other.uid;
}

String PluginDescription::createIdentifierString() const
{
    return pluginFormatName
            + "-" + name
            + "-" + String::toHexString (fileOrIdentifier.hashCode())
            + "-" + String::toHexString (uid);
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
    e->setAttribute ("numInputs", numInputChannels);
    e->setAttribute ("numOutputs", numOutputChannels);

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
        numInputChannels    = xml.getIntAttribute ("numInputs");
        numOutputChannels   = xml.getIntAttribute ("numOutputs");

        return true;
    }

    return false;
}
