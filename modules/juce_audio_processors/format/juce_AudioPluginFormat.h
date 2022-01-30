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

namespace juce
{

//==============================================================================
/**
    The base class for a type of plugin format, such as VST, AudioUnit, LADSPA, etc.

    @see AudioPluginFormatManager

    @tags{Audio}
*/
class JUCE_API  AudioPluginFormat  : private MessageListener
{
public:
    /** Destructor. */
    ~AudioPluginFormat() override;

    //==============================================================================
    /** Returns the format name.
        E.g. "VST", "AudioUnit", etc.
    */
    virtual String getName() const = 0;

    /** This tries to create descriptions for all the plugin types available in
        a binary module file.

        The file will be some kind of DLL or bundle.

        Normally there will only be one type returned, but some plugins
        (e.g. VST shells) can use a single DLL to create a set of different plugin
        subtypes, so in that case, each subtype is returned as a separate object.
    */
    virtual void findAllTypesForFile (OwnedArray<PluginDescription>& results,
                                      const String& fileOrIdentifier) = 0;

    /** Tries to recreate a type from a previously generated PluginDescription.
        @see AudioPluginFormatManager::createInstance
    */
    std::unique_ptr<AudioPluginInstance> createInstanceFromDescription (const PluginDescription&,
                                                                        double initialSampleRate,
                                                                        int initialBufferSize);

    /** Same as above but with the possibility of returning an error message.
        @see AudioPluginFormatManager::createInstance
    */
    std::unique_ptr<AudioPluginInstance> createInstanceFromDescription (const PluginDescription&,
                                                                        double initialSampleRate,
                                                                        int initialBufferSize,
                                                                        String& errorMessage);

    /** A callback lambda that is passed to createPluginInstanceAsync() */
    using PluginCreationCallback = std::function<void (std::unique_ptr<AudioPluginInstance>, const String&)>;

    /** Tries to recreate a type from a previously generated PluginDescription.
        When the plugin has been created, it will be passed to the caller via an
        asynchronous call to the PluginCreationCallback lambda that was provided.
        @see AudioPluginFormatManager::createPluginInstanceAsync
     */
    void createPluginInstanceAsync (const PluginDescription& description,
                                    double initialSampleRate,
                                    int initialBufferSize,
                                    PluginCreationCallback);

    /** Should do a quick check to see if this file or directory might be a plugin of
        this format.

        This is for searching for potential files, so it shouldn't actually try to
        load the plugin or do anything time-consuming.
    */
    virtual bool fileMightContainThisPluginType (const String& fileOrIdentifier) = 0;

    /** Returns a readable version of the name of the plugin that this identifier refers to. */
    virtual String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) = 0;

    /** Returns true if this plugin's version or date has changed and it should be re-checked. */
    virtual bool pluginNeedsRescanning (const PluginDescription&) = 0;

    /** Checks whether this plugin could possibly be loaded.
        It doesn't actually need to load it, just to check whether the file or component
        still exists.
    */
    virtual bool doesPluginStillExist (const PluginDescription&) = 0;

    /** Returns true if this format needs to run a scan to find its list of plugins. */
    virtual bool canScanForPlugins() const = 0;

    /** Should return true if this format is both safe and quick to scan - i.e. if a file
        can be scanned within a few milliseconds on a background thread, without actually
        needing to load an executable.
    */
    virtual bool isTrivialToScan() const = 0;

    /** Searches a suggested set of directories for any plugins in this format.
        The path might be ignored, e.g. by AUs, which are found by the OS rather
        than manually.

        @param directoriesToSearch   This specifies which directories shall be
                                     searched for plug-ins.
        @param recursive             Should the search recursively traverse folders.
        @param allowPluginsWhichRequireAsynchronousInstantiation
                                     If this is false then plug-ins which require
                                     asynchronous creation will be excluded.
    */
    virtual StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                               bool recursive,
                                               bool allowPluginsWhichRequireAsynchronousInstantiation = false) = 0;

    /** Returns the typical places to look for this kind of plugin.

        Note that if this returns no paths, it means that the format doesn't search in
        files or folders, e.g. AudioUnits.
    */
    virtual FileSearchPath getDefaultLocationsToSearch() = 0;

    /** Returns true if instantiation of this plugin type must be done from a non-message thread. */
    virtual bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const = 0;

protected:
    //==============================================================================
    friend class AudioPluginFormatManager;

    AudioPluginFormat();

    /** Implementors must override this function. This is guaranteed to be called on
        the message thread. You may call the callback on any thread.
    */
    virtual void createPluginInstance (const PluginDescription&, double initialSampleRate,
                                       int initialBufferSize, PluginCreationCallback) = 0;

private:
    struct AsyncCreateMessage;
    void handleMessage (const Message&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginFormat)
};

} // namespace juce
