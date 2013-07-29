/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_PLUGINDESCRIPTION_H_INCLUDED
#define JUCE_PLUGINDESCRIPTION_H_INCLUDED


//==============================================================================
/**
    A small class to represent some facts about a particular type of plugin.

    This class is for storing and managing the details about a plugin without
    actually having to load an instance of it.

    A KnownPluginList contains a list of PluginDescription objects.

    @see KnownPluginList
*/
class JUCE_API  PluginDescription
{
public:
    //==============================================================================
    PluginDescription();
    PluginDescription (const PluginDescription& other);
    PluginDescription& operator= (const PluginDescription& other);
    ~PluginDescription();

    //==============================================================================
    /** The name of the plugin. */
    String name;

    /** A more descriptive name for the plugin.
        This may be the same as the 'name' field, but some plugins may provide an
        alternative name.
    */
    String descriptiveName;

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

    /** Either the file containing the plugin module, or some other unique way
        of identifying it.

        E.g. for an AU, this would be an ID string that the component manager
        could use to retrieve the plugin. For a VST, it's the file path.
    */
    String fileOrIdentifier;

    /** The last time the plugin file was changed.
        This is handy when scanning for new or changed plugins.
    */
    Time lastFileModTime;

    /** A unique ID for the plugin.

        Note that this might not be unique between formats, e.g. a VST and some
        other format might actually have the same id.

        @see createIdentifierString
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
    /** Returns a string that can be saved and used to uniquely identify the
        plugin again.

        This contains less info than the XML encoding, and is independent of the
        plugin's file location, so can be used to store a plugin ID for use
        across different machines.
    */
    String createIdentifierString() const;

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


private:
    //==============================================================================
    JUCE_LEAK_DETECTOR (PluginDescription)
};


#endif   // JUCE_PLUGINDESCRIPTION_H_INCLUDED
