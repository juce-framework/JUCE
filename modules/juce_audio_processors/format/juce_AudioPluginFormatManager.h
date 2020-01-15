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
    This maintains a list of known AudioPluginFormats.

    @see AudioPluginFormat

    @tags{Audio}
*/
class JUCE_API  AudioPluginFormatManager
{
public:
    //==============================================================================
    AudioPluginFormatManager();

    /** Destructor. */
    ~AudioPluginFormatManager();

    //==============================================================================
    /** Adds the set of available standard formats, e.g. VST. */
    void addDefaultFormats();

    //==============================================================================
    /** Returns the number of types of format that are available.
        Use getFormat() to get one of them.
    */
    int getNumFormats() const;

    /** Returns one of the available formats.
        @see getNumFormats
    */
    AudioPluginFormat* getFormat (int index) const;

    /** Returns a list of all the registered formats. */
    Array<AudioPluginFormat*> getFormats() const;

    //==============================================================================
    /** Adds a format to the list.
        The object passed in will be owned and deleted by the manager.
    */
    void addFormat (AudioPluginFormat*);

    //==============================================================================
    /** Tries to load the type for this description, by trying all the formats
        that this manager knows about.

        The caller is responsible for deleting the object that is returned.

        If it can't load the plugin, it returns nullptr and leaves a message in the
        errorMessage string.

        If you intend to instantiate a AudioUnit v3 plug-in then you must either
        use the non-blocking asynchronous version below - or call this method from a
        thread other than the message thread and without blocking the message
        thread.
    */
    std::unique_ptr<AudioPluginInstance> createPluginInstance (const PluginDescription& description,
                                                               double initialSampleRate, int initialBufferSize,
                                                               String& errorMessage) const;

    /** Tries to asynchronously load the type for this description, by trying
        all the formats that this manager knows about.

        The caller must supply a callback object which will be called when
        the instantiation has completed.

        If it can't load the plugin then the callback function will be called
        passing a nullptr as the instance argument along with an error message.

        The callback function will be called on the message thread so the caller
        must not block the message thread.

        The callback object will be deleted automatically after it has been
        invoked.

        The caller is responsible for deleting the instance that is passed to
        the callback function.

        If you intend to instantiate a AudioUnit v3 plug-in then you must use
        this non-blocking asynchronous version - or call the synchronous method
        from an auxiliary thread.
    */
    void createPluginInstanceAsync (const PluginDescription& description,
                                    double initialSampleRate, int initialBufferSize,
                                    AudioPluginFormat::PluginCreationCallback callback);

    /** Checks that the file or component for this plugin actually still exists.
        (This won't try to load the plugin)
    */
    bool doesPluginStillExist (const PluginDescription&) const;

private:
    //==============================================================================
    AudioPluginFormat* findFormatForDescription (const PluginDescription&, String& errorMessage) const;

    OwnedArray<AudioPluginFormat> formats;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginFormatManager)
};

} // namespace juce
