/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if (JUCE_PLUGINHOST_VST || DOXYGEN)

namespace juce
{

//==============================================================================
/**
    Implements a plugin format manager for VSTs.

    @tags{Audio}
*/
class JUCE_API  VSTPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    VSTPluginFormat();
    ~VSTPluginFormat() override;

    //==============================================================================
    /** Attempts to retrieve the VSTXML data from a plugin.
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

    /** Given a suitable function pointer to a VSTPluginMain function, this will attempt to
        instantiate and return a plugin for it.
    */
    static AudioPluginInstance* createCustomVSTFromMainCall (void* entryPointFunction,
                                                             double initialSampleRate,
                                                             int initialBufferSize);

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
    /** This simply calls directly to the VST's AEffect::dispatcher() function. */
    static pointer_sized_int JUCE_CALLTYPE dispatcher (AudioPluginInstance*, int32, int32, pointer_sized_int, void*, float);

    /** Given a VstEffectInterface* (aka vst::AEffect*), this will return the juce AudioPluginInstance
        that is being used to wrap it
    */
    static AudioPluginInstance* getPluginInstanceFromVstEffectInterface (void* aEffect);

    //==============================================================================
    static String getFormatName()                   { return "VST"; }
    String getName() const override                 { return getFormatName(); }
    bool canScanForPlugins() const override         { return true; }
    bool isTrivialToScan() const override           { return false; }

    void findAllTypesForFile (OwnedArray<PluginDescription>&, const String& fileOrIdentifier) override;
    bool fileMightContainThisPluginType (const String& fileOrIdentifier) override;
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override;
    bool pluginNeedsRescanning (const PluginDescription&) override;
    StringArray searchPathsForPlugins (const FileSearchPath&, bool recursive, bool) override;
    bool doesPluginStillExist (const PluginDescription&) override;
    FileSearchPath getDefaultLocationsToSearch() override;

    /** Can be overridden to receive a callback when each member of a shell plugin is about to be
        tested during a call to findAllTypesForFile().
        Only the name and uid members of the PluginDescription are guaranteed to be valid when
        this is called.
    */
    virtual void aboutToScanVSTShellPlugin (const PluginDescription&);

private:
    //==============================================================================
    void createPluginInstance (const PluginDescription&, double initialSampleRate,
                               int initialBufferSize, PluginCreationCallback) override;
    bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;
    void recursiveFileSearch (StringArray&, const File&, bool recursive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTPluginFormat)
};

} // namespace juce

#endif
