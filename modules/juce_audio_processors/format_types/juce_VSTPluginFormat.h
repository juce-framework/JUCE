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
   #if JUCE_64BIT
    typedef int64 VstIntPtr;
   #else
    typedef int32 VstIntPtr;
   #endif

    /** This simply calls directly to the VST's AEffect::dispatcher() function. */
    static VstIntPtr JUCE_CALLTYPE dispatcher (AudioPluginInstance*, int32, int32, VstIntPtr, void*, float);

    //==============================================================================
    String getName() const override                { return "VST"; }
    void findAllTypesForFile (OwnedArray<PluginDescription>&, const String& fileOrIdentifier) override;
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription&, double, int) override;
    bool fileMightContainThisPluginType (const String& fileOrIdentifier) override;
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override;
    bool pluginNeedsRescanning (const PluginDescription&) override;
    StringArray searchPathsForPlugins (const FileSearchPath&, bool recursive) override;
    bool doesPluginStillExist (const PluginDescription&) override;
    FileSearchPath getDefaultLocationsToSearch() override;
    bool canScanForPlugins() const override        { return true; }

private:
    void recursiveFileSearch (StringArray&, const File&, bool recursive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTPluginFormat)
};


#endif
