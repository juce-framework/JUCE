/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             ConvolutionDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Convolution demo using the DSP module.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_dsp, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        ConvolutionDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/DSPDemos_Common.h"

using namespace dsp;

//==============================================================================
struct ConvolutionDemoDSP
{
    void prepare (const ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        convolution.prepare (spec);
        updateParameters();
    }

    void process (ProcessContextReplacing<float> context)
    {
        context.isBypassed = bypass;

        // Load a new IR if there's one available. Note that this doesn't lock or allocate!
        bufferTransfer.get ([this] (BufferWithSampleRate& buf)
        {
            convolution.loadImpulseResponse (std::move (buf.buffer),
                                             buf.sampleRate,
                                             Convolution::Stereo::yes,
                                             Convolution::Trim::yes,
                                             Convolution::Normalise::yes);
        });

        convolution.process (context);
    }

    void reset()
    {
        convolution.reset();
    }

    void updateParameters()
    {
        auto* cabinetTypeParameter = dynamic_cast<ChoiceParameter*> (parameters[0]);

        if (cabinetTypeParameter == nullptr)
        {
            jassertfalse;
            return;
        }

        if (cabinetTypeParameter->getCurrentSelectedID() == 1)
        {
            bypass = true;
        }
        else
        {
            bypass = false;

            auto selectedType = cabinetTypeParameter->getCurrentSelectedID();
            auto assetName = (selectedType == 2 ? "guitar_amp.wav" : "cassette_recorder.wav");

            auto assetInputStream = createAssetInputStream (assetName);

            if (assetInputStream == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioFormatManager manager;
            manager.registerBasicFormats();
            std::unique_ptr<AudioFormatReader> reader { manager.createReaderFor (std::move (assetInputStream)) };

            if (reader == nullptr)
            {
                jassertfalse;
                return;
            }

            AudioBuffer<float> buffer (static_cast<int> (reader->numChannels),
                                       static_cast<int> (reader->lengthInSamples));
            reader->read (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), 0, buffer.getNumSamples());

            bufferTransfer.set (BufferWithSampleRate { std::move (buffer), reader->sampleRate });
        }
    }

    //==============================================================================
    struct BufferWithSampleRate
    {
        BufferWithSampleRate() = default;

        BufferWithSampleRate (AudioBuffer<float>&& bufferIn, double sampleRateIn)
            : buffer (std::move (bufferIn)), sampleRate (sampleRateIn) {}

        AudioBuffer<float> buffer;
        double sampleRate = 0.0;
    };

    class BufferTransfer
    {
    public:
        void set (BufferWithSampleRate&& p)
        {
            const SpinLock::ScopedLockType lock (mutex);
            buffer = std::move (p);
            newBuffer = true;
        }

        // Call `fn` passing the new buffer, if there's one available
        template <typename Fn>
        void get (Fn&& fn)
        {
            const SpinLock::ScopedTryLockType lock (mutex);

            if (lock.isLocked() && newBuffer)
            {
                fn (buffer);
                newBuffer = false;
            }
        }

    private:
        BufferWithSampleRate buffer;
        bool newBuffer = false;
        SpinLock mutex;
    };

    double sampleRate = 0.0;
    bool bypass = false;

    MemoryBlock currentCabinetData;
    Convolution convolution;

    BufferTransfer bufferTransfer;

    ChoiceParameter cabinetParam { { "Bypass", "Guitar amplifier 8''", "Cassette recorder" }, 1, "Cabinet Type" };

    std::vector<DSPDemoParameterBase*> parameters { &cabinetParam };
};

struct ConvolutionDemo final : public Component
{
    ConvolutionDemo()
    {
        addAndMakeVisible (fileReaderComponent);
        setSize (750, 500);
    }

    void resized() override
    {
        fileReaderComponent.setBounds (getLocalBounds());
    }

    AudioFileReaderComponent<ConvolutionDemoDSP> fileReaderComponent;
};
