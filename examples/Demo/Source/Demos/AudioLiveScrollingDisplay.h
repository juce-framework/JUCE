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

#ifndef __AUDIOLIVESCROLLINGDISPLAY_H_4C3BD3A7__
#define __AUDIOLIVESCROLLINGDISPLAY_H_4C3BD3A7__

#include "../JuceDemoHeader.h"


//==============================================================================
/* This component scrolls a continuous waveform showing the audio that's
   coming into whatever audio inputs this object is connected to.
*/
class LiveScrollingAudioDisplay  : public AudioVisualiserComponent,
                                   public AudioIODeviceCallback
{
public:
    LiveScrollingAudioDisplay()  : AudioVisualiserComponent (1)
    {
        setSamplesPerBlock (256);
        setBufferSize (1024);
    }

    //==============================================================================
    void audioDeviceAboutToStart (AudioIODevice*) override
    {
        clear();
    }

    void audioDeviceStopped() override
    {
        clear();
    }

    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numberOfSamples) override
    {
        for (int i = 0; i < numberOfSamples; ++i)
        {
            float inputSample = 0;

            for (int chan = 0; chan < numInputChannels; ++chan)
                if (const float* inputChannel = inputChannelData[chan])
                    inputSample += inputChannel[i];  // find the sum of all the channels

            inputSample *= 10.0f; // boost the level to make it more easily visible.

            pushSample (&inputSample, 1);
        }

        // We need to clear the output buffers before returning, in case they're full of junk..
        for (int j = 0; j < numOutputChannels; ++j)
            if (float* outputChannel = outputChannelData[j])
                zeromem (outputChannel, sizeof (float) * (size_t) numberOfSamples);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveScrollingAudioDisplay)
};


#endif  // __AUDIOLIVESCROLLINGDISPLAY_H_4C3BD3A7__
