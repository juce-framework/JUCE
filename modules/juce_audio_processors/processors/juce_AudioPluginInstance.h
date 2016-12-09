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

#ifndef JUCE_AUDIOPLUGININSTANCE_H_INCLUDED
#define JUCE_AUDIOPLUGININSTANCE_H_INCLUDED


//==============================================================================
/**
    Base class for an active instance of a plugin.

    This derives from the AudioProcessor class, and adds some extra functionality
    that helps when wrapping dynamically loaded plugins.

    This class is not needed when writing plugins, and you should never need to derive
    your own sub-classes from it. The plugin hosting classes use it internally and will
    return AudioPluginInstance objects which wrap external plugins.

    @see AudioProcessor, AudioPluginFormat
*/
class JUCE_API  AudioPluginInstance   : public AudioProcessor
{
public:
    //==============================================================================
    /** Destructor.

        Make sure that you delete any UI components that belong to this plugin before
        deleting the plugin.
    */
    virtual ~AudioPluginInstance() {}

    //==============================================================================
    /** Fills-in the appropriate parts of this plugin description object. */
    virtual void fillInPluginDescription (PluginDescription& description) const = 0;

    /** Returns a PluginDescription for this plugin.
        This is just a convenience method to avoid calling fillInPluginDescription.
    */
    PluginDescription getPluginDescription() const
    {
        PluginDescription desc;
        fillInPluginDescription (desc);
        return desc;
    }

    /** Returns a pointer to some kind of platform-specific data about the plugin.
        E.g. For a VST, this value can be cast to an AEffect*. For an AudioUnit, it can be
        cast to an AudioUnit handle.
    */
    virtual void* getPlatformSpecificData()                 { return nullptr; }

    /** For some formats (currently AudioUnit), this forces a reload of the list of
        available parameters.
    */
    virtual void refreshParameterList() {}

protected:
    //==============================================================================
    AudioPluginInstance() {}
    AudioPluginInstance (const BusesProperties& ioLayouts) : AudioProcessor (ioLayouts) {}
    template <int numLayouts>
    AudioPluginInstance (const short channelLayoutList[numLayouts][2]) : AudioProcessor (channelLayoutList) {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginInstance)
};


#endif   // JUCE_AUDIOPLUGININSTANCE_H_INCLUDED
