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

#ifndef JUCE_AUDIOPLUGINFORMATMANAGER_H_INCLUDED
#define JUCE_AUDIOPLUGINFORMATMANAGER_H_INCLUDED


//==============================================================================
/**
    This maintains a list of known AudioPluginFormats.

    @see AudioPluginFormat
*/
class JUCE_API  AudioPluginFormatManager
{
public:
    //==============================================================================
    AudioPluginFormatManager();

    /** Destructor. */
    ~AudioPluginFormatManager();

    //==============================================================================
    /** Adds any formats that it knows about, e.g. VST.
    */
    void addDefaultFormats();

    //==============================================================================
    /** Returns the number of types of format that are available.

        Use getFormat() to get one of them.
    */
    int getNumFormats();

    /** Returns one of the available formats.

        @see getNumFormats
    */
    AudioPluginFormat* getFormat (int index);

    //==============================================================================
    /** Adds a format to the list.

        The object passed in will be owned and deleted by the manager.
    */
    void addFormat (AudioPluginFormat* format);


    //==============================================================================
    /** Tries to load the type for this description, by trying all the formats
        that this manager knows about.

        The caller is responsible for deleting the object that is returned.

        If it can't load the plugin, it returns nullptr and leaves a message in the
        errorMessage string.

        If you intend to instantiate a AudioUnit v3 plug-in then you must either
        use the non-blocking asynchrous version below - or call this method from a
        thread other than the message thread and without blocking the message
        thread.
    */
    AudioPluginInstance* createPluginInstance (const PluginDescription& description,
                                               double initialSampleRate,
                                               int initialBufferSize,
                                               String& errorMessage) const;

    /** Tries to asynchronously load the type for this description, by trying
        all the formats that this manager knows about.

        The caller must supply a callback object which will be called when
        the instantantiation has completed.

        If it can't load the plugin then the callback function will be called
        passing a nullptr as the instance argument along with an error message.

        The callback function will be called on the message thread so the caller
        must not block the message thread.

        The callback object will be deleted automatically after it has been
        invoked.

        The caller is responsible for deleting the instance that is passed to
        the callback function.

        If you intend to instantiate a AudioUnit v3 plug-in then you must use
        this non-blocking asynchrous version - or call the synchrous method
        from an auxiliary thread.
    */
    void createPluginInstanceAsync (const PluginDescription& description,
                                    double initialSampleRate,
                                    int initialBufferSize,
                                    AudioPluginFormat::InstantiationCompletionCallback* callback);

   #if JUCE_COMPILER_SUPPORTS_LAMBDAS
    void createPluginInstanceAsync (const PluginDescription& description,
                                    double initialSampleRate,
                                    int initialBufferSize,
                                    std::function<void (AudioPluginInstance*, const String&)> completionCallback);
   #endif

    /** Checks that the file or component for this plugin actually still exists.

        (This won't try to load the plugin)
    */
    bool doesPluginStillExist (const PluginDescription& description) const;

private:
    //==============================================================================
    //@internal
    AudioPluginFormat* findFormatForDescription (const PluginDescription&, String& errorMessage) const;

    OwnedArray<AudioPluginFormat> formats;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginFormatManager)
};



#endif   // JUCE_AUDIOPLUGINFORMATMANAGER_H_INCLUDED
