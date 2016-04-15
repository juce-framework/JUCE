/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_AUDIOPLUGINFORMAT_H_INCLUDED
#define JUCE_AUDIOPLUGINFORMAT_H_INCLUDED


//==============================================================================
/**
    The base class for a type of plugin format, such as VST, AudioUnit, LADSPA, etc.

    @see AudioPluginFormatManager
*/
class JUCE_API  AudioPluginFormat
{
public:
    //==============================================================================
    struct JUCE_API  InstantiationCompletionCallback
    {
        virtual ~InstantiationCompletionCallback() {}
        virtual void completionCallback (AudioPluginInstance* instance, const String& error) = 0;

        JUCE_LEAK_DETECTOR (InstantiationCompletionCallback)
    };

    //==============================================================================
    /** Destructor. */
    virtual ~AudioPluginFormat();

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
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription&,
                                                        double initialSampleRate,
                                                        int initialBufferSize);

    /** Same as above but with the possibility of returning an error message.

        @see AudioPluginFormatManager::createInstance
    */
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription&,
                                                        double initialSampleRate,
                                                        int initialBufferSize,
                                                        String& errorMessage);

    /** Tries to recreate a type from a previously generated PluginDescription.

        @see AudioPluginFormatManager::createInstanceAsync
     */
    void createPluginInstanceAsync (const PluginDescription& description,
                                    double initialSampleRate,
                                    int initialBufferSize,
                                    InstantiationCompletionCallback* completionCallback);

   #if JUCE_COMPILER_SUPPORTS_LAMBDAS
    void createPluginInstanceAsync (const PluginDescription& description,
                                    double initialSampleRate,
                                    int initialBufferSize,
                                    std::function<void (AudioPluginInstance*, const String&)> completionCallback);
   #endif

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

protected:
    //==============================================================================
    friend class AudioPluginFormatManager;

    AudioPluginFormat() noexcept;

    /** Implementors must override this function. This is guaranteed to be called on
        the message thread. You may call the callback on any thread.
    */
    virtual void createPluginInstance (const PluginDescription&, double initialSampleRate,
                                       int initialBufferSize, void* userData,
                                       void (*callback) (void*, AudioPluginInstance*, const String&)) = 0;

    virtual bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const noexcept = 0;

private:
    /** @internal */
    void createPluginInstanceOnMessageThread (const PluginDescription&, double rate, int size,
                                              AudioPluginFormat::InstantiationCompletionCallback*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginFormat)
};


#endif   // JUCE_AUDIOPLUGINFORMAT_H_INCLUDED
