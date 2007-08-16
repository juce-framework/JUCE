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

#ifndef __JUCE_PLUGINDESCRIPTION_JUCEHEADER__
#define __JUCE_PLUGINDESCRIPTION_JUCEHEADER__

#include "juce_AudioPluginInstance.h"


//==============================================================================
/**
    A small class to represent some facts about a particular type of plugin.

    This class is for storing and managing the details about a plugin without
    actually having to load an instance of it.

    A KnownPluginList contains a list of PluginDescription objects.

    @see KnownPluginList
*/
class PluginDescription
{
public:
    //==============================================================================
    PluginDescription() throw();
    PluginDescription (const PluginDescription& other) throw();
    const PluginDescription& operator= (const PluginDescription& other) throw();
    ~PluginDescription() throw();

    //==============================================================================
    /** The name of the plugin. */
    String name;

    /** The plugin format, e.g. "VST", "AudioUnit", etc.
    */
    String pluginFormatName;

    /** A category, such as "Dynamics", "Reverbs", etc.
    */
    String category;

    /** The manufacturer. */
    String manufacturerName;

    /** The version. This string doesn't have any particular format. */
    String version;

    /** The binary module file containing the plugin. */
    File file;

    /** The last time the plugin file was changed.
        This is handy when scanning for new or changed plugins.
    */
    Time lastFileModTime;

    /** A unique ID for the plugin.

        Note that this might not be unique between formats, e.g. a VST and some
        other format might actually have the same id.
    */
    int uid;

    /** True if the plugin identifies itself as a synthesiser. */
    bool isInstrument;

    /** The number of inputs. */
    int numInputChannels;

    /** The number of outputs. */
    int numOutputChannels;

    /** Returns true if the two descriptions refer the the same plugin.

        This isn't quite as simple as them just having the same file (because of
        shell plugins).
    */
    bool isDuplicateOf (const PluginDescription& other) const;

    //==============================================================================
    /** Fills in this description based on the given plugin.

        Returns true if it worked, false if it couldn't get the info.
    */
    void fillInFromInstance (AudioPluginInstance& instance) throw();

    //==============================================================================
    /** Tries to load the type for this description, by trying all the formats 
        available in the AudioPluginFormat.

        The caller is responsible for deleting the object that is returned.

        If it can't load the plugin, it returns 0 and leaves a message in the 
        errorMessage string.
    */
    AudioPluginInstance* createInstance (String& errorMessage) const;

    //==============================================================================
    /** Creates an XML object containing these details.

        @see loadFromXml
    */
    XmlElement* createXml() const;

    /** Reloads the info in this structure from an XML record that was previously
        saved with createXML().

        Returns true if the XML was a valid plugin description.
    */
    bool loadFromXml (const XmlElement& xml);


    //==============================================================================
    juce_UseDebuggingNewOperator
};


#endif   // __JUCE_PLUGINDESCRIPTION_JUCEHEADER__
