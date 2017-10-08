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

namespace juce
{

//==============================================================================
/**
    A small class to represent some facts about a particular type of plug-in.

    This class is for storing and managing the details about a plug-in without
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
    /** The name of the plug-in. */
    String name;

    /** A more descriptive name for the plug-in.
        This may be the same as the 'name' field, but some plug-ins may provide an
        alternative name.
    */
    String descriptiveName;

    /** The plug-in format, e.g. "VST", "AudioUnit", etc. */
    String pluginFormatName;

    /** A category, such as "Dynamics", "Reverbs", etc. */
    String category;

    /** The manufacturer. */
    String manufacturerName;

    /** The version. This string doesn't have any particular format. */
    String version;

    /** Either the file containing the plug-in module, or some other unique way
        of identifying it.

        E.g. for an AU, this would be an ID string that the component manager
        could use to retrieve the plug-in. For a VST, it's the file path.
    */
    String fileOrIdentifier;

    /** The last time the plug-in file was changed.
        This is handy when scanning for new or changed plug-ins.
    */
    Time lastFileModTime;

    /** The last time that this information was updated. This would typically have
        been during a scan when this plugin was first tested or found to have changed.
    */
    Time lastInfoUpdateTime;

    /** A unique ID for the plug-in.

        Note that this might not be unique between formats, e.g. a VST and some
        other format might actually have the same id.

        @see createIdentifierString
    */
    int uid;

    /** True if the plug-in identifies itself as a synthesiser. */
    bool isInstrument;

    /** The number of inputs. */
    int numInputChannels;

    /** The number of outputs. */
    int numOutputChannels;

    /** True if the plug-in is part of a multi-type container, e.g. a VST Shell. */
    bool hasSharedContainer;

    /** Returns true if the two descriptions refer to the same plug-in.

        This isn't quite as simple as them just having the same file (because of
        shell plug-ins).
    */
    bool isDuplicateOf (const PluginDescription& other) const noexcept;

    /** Return true if this description is equivalent to another one which created the
        given identifier string.

        Note that this isn't quite as simple as them just calling createIdentifierString()
        and comparing the strings, because the identifiers can differ (thanks to shell plug-ins).
    */
    bool matchesIdentifierString (const String& identifierString) const;

    //==============================================================================
    /** Returns a string that can be saved and used to uniquely identify the
        plugin again.

        This contains less info than the XML encoding, and is independent of the
        plug-in's file location, so can be used to store a plug-in ID for use
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

        Returns true if the XML was a valid plug-in description.
    */
    bool loadFromXml (const XmlElement& xml);


private:
    //==============================================================================
    JUCE_LEAK_DETECTOR (PluginDescription)
};

} // namespace juce
