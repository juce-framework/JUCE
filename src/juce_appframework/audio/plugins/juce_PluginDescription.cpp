/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../juce_core/basics/juce_StandardHeader.h"

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
      file (other.file),
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
    file = other.file;
    uid = other.uid;
    isInstrument = other.isInstrument;
    lastFileModTime = other.lastFileModTime;
    numInputChannels = other.numInputChannels;
    numOutputChannels = other.numOutputChannels;

    return *this;
}

bool PluginDescription::isDuplicateOf (const PluginDescription& other) const
{
    return file == other.file
            && uid == other.uid;
}

const String PluginDescription::createIdentifierString() const throw()
{
    return pluginFormatName
            + T("-") + name
            + T("-") + String::toHexString (file.getFileName().hashCode())
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
    e->setAttribute (T("file"), file.getFullPathName());
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
        file = File (xml.getStringAttribute (T("file")));
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
