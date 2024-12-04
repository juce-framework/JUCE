/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

} // namespace juce
