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
    */
    AudioPluginInstance* createPluginInstance (const PluginDescription& description,
                                               double initialSampleRate,
                                               int initialBufferSize,
                                               String& errorMessage) const;

    /** Checks that the file or component for this plugin actually still exists.

        (This won't try to load the plugin)
    */
    bool doesPluginStillExist (const PluginDescription& description) const;

private:
    //==============================================================================
    OwnedArray<AudioPluginFormat> formats;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginFormatManager)
};



#endif   // JUCE_AUDIOPLUGINFORMATMANAGER_H_INCLUDED
