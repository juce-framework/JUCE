/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_AUDIOFILTERSTREAMER_JUCEHEADER__
#define __JUCE_AUDIOFILTERSTREAMER_JUCEHEADER__

#include "../juce_PluginHeaders.h"


//==============================================================================
/**
    A class that wraps an AudioProcessor as an AudioIODeviceCallback, so its
    output can be streamed directly to/from some audio and midi inputs and outputs.

    To use it, just create an instance of this for your filter, and register it
    as the callback with an AudioIODevice or AudioDeviceManager object.

    To receive midi input in your filter, you should also register it as a
    MidiInputCallback with a suitable MidiInput or an AudioDeviceManager.

    And for an even easier way of doing a standalone plugin, see the
    AudioFilterStreamingDeviceManager class...
*/
class AudioFilterStreamer   : public AudioIODeviceCallback,
                              public MidiInputCallback,
                              public AudioPlayHead
{
public:
    //==============================================================================
    AudioFilterStreamer (AudioProcessor& filterToUse);
    ~AudioFilterStreamer();


    //==============================================================================
    void audioDeviceIOCallback (const float** inputChannelData,
                                int numInputChannels,
                                float** outputChannelData,
                                int numOutputChannels,
                                int numSamples);

    void audioDeviceAboutToStart (AudioIODevice* device);
    void audioDeviceStopped();

    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message);

    bool getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info);

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    AudioProcessor& filter;
    bool isPlaying;
    double sampleRate;
    MidiMessageCollector midiCollector;

    float* outChans [128];
    float* inChans [128];
    AudioSampleBuffer emptyBuffer;
};

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


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioFilterStreamer* streamer;
};


#endif   // __JUCE_AUDIOFILTERSTREAMER_JUCEHEADER__
