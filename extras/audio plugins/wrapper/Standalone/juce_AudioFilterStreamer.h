/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_AUDIOFILTERSTREAMER_JUCEHEADER__
#define __JUCE_AUDIOFILTERSTREAMER_JUCEHEADER__

#include "../juce_PluginHeaders.h"


//==============================================================================
/**
    Wraps an AudioFilterStreamer in an AudioDeviceManager to make it easy to
    create a standalone filter.

    This simply acts as a singleton AudioDeviceManager, which continuously
    streams audio from the filter you give it with the setFilter() method.

    To use it, simply create an instance of it (or use getInstance() if you're
    using it as a singleton), initialise it like you would a normal
    AudioDeviceManager, and call setFilter() to start it running your plugin.

*/
class AudioFilterStreamingDeviceManager  : public AudioDeviceManager
{
public:
    //==============================================================================
    AudioFilterStreamingDeviceManager();
    ~AudioFilterStreamingDeviceManager();

    juce_DeclareSingleton (AudioFilterStreamingDeviceManager, true);

    //==============================================================================
    /** Tells the device which filter to stream audio through.

        Pass in 0 to deselect the current filter.
    */
    void setFilter (AudioProcessor* filterToStream);

private:
    //==============================================================================
    ScopedPointer <AudioProcessorPlayer> player;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilterStreamingDeviceManager);
};


#endif   // __JUCE_AUDIOFILTERSTREAMER_JUCEHEADER__
