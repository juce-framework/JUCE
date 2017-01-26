/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

ReverbAudioSource::ReverbAudioSource (AudioSource* const inputSource, const bool deleteInputWhenDeleted)
   : input (inputSource, deleteInputWhenDeleted),
     bypass (false)
{
    jassert (inputSource != nullptr);
}

ReverbAudioSource::~ReverbAudioSource() {}

void ReverbAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    const ScopedLock sl (lock);
    input->prepareToPlay (samplesPerBlockExpected, sampleRate);
    reverb.setSampleRate (sampleRate);
}

void ReverbAudioSource::releaseResources() {}

void ReverbAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    const ScopedLock sl (lock);

    input->getNextAudioBlock (bufferToFill);

    if (! bypass)
    {
        float* const firstChannel = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);

        if (bufferToFill.buffer->getNumChannels() > 1)
        {
            reverb.processStereo (firstChannel,
                                  bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample),
                                  bufferToFill.numSamples);
        }
        else
        {
            reverb.processMono (firstChannel, bufferToFill.numSamples);
        }
    }
}

void ReverbAudioSource::setParameters (const Reverb::Parameters& newParams)
{
    const ScopedLock sl (lock);
    reverb.setParameters (newParams);
}

void ReverbAudioSource::setBypassed (bool b) noexcept
{
    if (bypass != b)
    {
        const ScopedLock sl (lock);
        bypass = b;
        reverb.reset();
    }
}
