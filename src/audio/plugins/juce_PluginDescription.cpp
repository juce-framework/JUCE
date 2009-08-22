/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_PluginDescription.h"
#include "juce_AudioPluginFormat.h"


//==============================================================================
PluginDescription::PluginDescription() throw()
    : uid (0),
      isInstrument (false),
      numInputChannels (0),
      numOutputChannels (0)
{
}

PluginDescription::~PluginDescription() throw()
{
}

PluginDescription::PluginDescription (const PluginDescription& other) throw()
    : name (other.name),
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

const PluginDescription& PluginDescription::operator= (const PluginDescription& other) throw()
{
    name = other.name;
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

const String PluginDescription::createIdentifierString() const throw()
{
    return pluginFormatName
            + T("-") + name
            + T("-") + String::toHexString (fileOrIdentifier.hashCode())
            + T("-") + String::toHexString (uid);
}

XmlElement* PluginDescription::createXml() const
{
    XmlElement* const e = new XmlElement (T("PLUGIN"));
    e->setAttribute (T("name"), name);
    e->setAttribute (T("format"), pluginFormatName);
    e->setAttribute (T("category"), category);
    e->setAttribute (T("manufacturer"), manufacturerName);
    e->setAttribute (T("version"), version);
    e->setAttribute (T("file"), fileOrIdentifier);
    e->setAttribute (T("uid"), String::toHexString (uid));
    e->setAttribute (T("isInstrument"), isInstrument);
    e->setAttribute (T("fileTime"), String::toHexString (lastFileModTime.toMilliseconds()));
    e->setAttribute (T("numInputs"), numInputChannels);
    e->setAttribute (T("numOutputs"), numOutputChannels);

    return e;
}

bool PluginDescription::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName (T("PLUGIN")))
    {
        name = xml.getStringAttribute (T("name"));
        pluginFormatName = xml.getStringAttribute (T("format"));
        category = xml.getStringAttribute (T("category"));
        manufacturerName = xml.getStringAttribute (T("manufacturer"));
        version = xml.getStringAttribute (T("version"));
        fileOrIdentifier = xml.getStringAttribute (T("file"));
        uid = xml.getStringAttribute (T("uid")).getHexValue32();
        isInstrument = xml.getBoolAttribute (T("isInstrument"), false);
        lastFileModTime = Time (xml.getStringAttribute (T("fileTime")).getHexValue64());
        numInputChannels = xml.getIntAttribute (T("numInputs"));
        numOutputChannels = xml.getIntAttribute (T("numOutputs"));

        return true;
    }

    return false;
}

END_JUCE_NAMESPACE
