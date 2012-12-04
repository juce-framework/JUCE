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

#ifndef __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
#define __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__

#include "../processors/juce_AudioProcessor.h"
#include "juce_PluginDescription.h"


//==============================================================================
/**
    Base class for an active instance of a plugin.

    This derives from the AudioProcessor class, and adds some extra functionality
    that helps when wrapping dynamically loaded plugins.

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginInstance)
};


#endif   // __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
