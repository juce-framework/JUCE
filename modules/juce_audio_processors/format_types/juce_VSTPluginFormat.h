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

#ifndef __JUCE_VSTPLUGINFORMAT_JUCEHEADER__
#define __JUCE_VSTPLUGINFORMAT_JUCEHEADER__

#include "../format/juce_AudioPluginFormat.h"

#if JUCE_PLUGINHOST_VST || DOXYGEN

//==============================================================================
/**
    Implements a plugin format manager for VSTs.
*/
class JUCE_API  VSTPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    VSTPluginFormat();
    ~VSTPluginFormat();

    //==============================================================================
    /** Attempts to retreive the VSTXML data from a plugin.
        Will return nullptr if the plugin isn't a VST, or if it doesn't have any VSTXML.
    */
    static const XmlElement* getVSTXML (AudioPluginInstance* plugin);

    /** Attempts to reload a VST plugin's state from some FXB or FXP data. */
    static bool loadFromFXBFile (AudioPluginInstance* plugin, const void* data, size_t dataSize);

    /** Attempts to save a VST's state to some FXP or FXB data. */
    static bool saveToFXBFile (AudioPluginInstance* plugin, MemoryBlock& result, bool asFXB);

    /** Attempts to get a VST's state as a chunk of memory. */
    static bool getChunkData (AudioPluginInstance* plugin, MemoryBlock& result, bool isPreset);

    /** Attempts to set a VST's state from a chunk of memory. */
    static bool setChunkData (AudioPluginInstance* plugin, const void* data, int size, bool isPreset);

    //==============================================================================
    /** Base class for some extra functions that can be attached to a VST plugin instance. */
    class ExtraFunctions
    {
    public:
        virtual ~ExtraFunctions() {}

        /** This should return 10000 * the BPM at this position in the current edit. */
        virtual int64 getTempoAt (int64 samplePos) = 0;

        /** This should return the host's automation state.
            @returns 0 = not supported, 1 = off, 2 = read, 3 = write, 4 = read/write
        */
        virtual int getAutomationState() = 0;
    };

    /** Provides an ExtraFunctions callback object for a plugin to use.
        The plugin will take ownership of the object and will delete it automatically.
    */
    static void setExtraFunctions (AudioPluginInstance* plugin, ExtraFunctions* functions);

    //==============================================================================
    String getName() const                { return "VST"; }
    void findAllTypesForFile (OwnedArray <PluginDescription>&, const String& fileOrIdentifier);
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription&);
    bool fileMightContainThisPluginType (const String& fileOrIdentifier);
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier);
    StringArray searchPathsForPlugins (const FileSearchPath&, bool recursive);
    bool doesPluginStillExist (const PluginDescription&);
    FileSearchPath getDefaultLocationsToSearch();
    bool canScanForPlugins() const        { return true; }

private:
    void recursiveFileSearch (StringArray&, const File&, bool recursive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTPluginFormat)
};


#endif
#endif   // __JUCE_VSTPLUGINFORMAT_JUCEHEADER__
