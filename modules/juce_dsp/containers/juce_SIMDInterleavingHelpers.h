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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

namespace juce::dsp
{

#if JUCE_USE_SIMD

/**
    Utility functions for interleaving and deinterleaving between
    AudioBlock<float> and AudioBlock<SIMDRegister<float>>.

    @tags{DSP}
*/
struct SIMDInterleavingHelpers
{
private:
    using Format = AudioData::Format<AudioData::Float32, AudioData::NativeEndian>;

    static constexpr auto registerSize = SIMDRegister<float>::size();

    template <typename T>
    static T* toBasePointer (SIMDRegister<T>* r) noexcept
    {
        return reinterpret_cast<T*> (r);
    }

public:
    /** Converts a deinterleaved AudioBlock<float> into an interleaved AudioBlock<SIMDRegister<float>>.

        'destination' must have at least as many samples as 'source', and must have enough room
        for all channels in 'source' (at least ceil(source.getNumChannels() / SIMDRegister<float>::size())).
        The 'zeroChannel' argument is used to populate any channels in 'destination' that do not
        have a corresponding channel in 'source'.

        @param source           the original audio block of non-interleaved data
        @param destination      the audio block that will hold the transformed result
        @param zeroChannel      a buffer of zeros that has the same length in samples as source
    */
    static void interleave (AudioBlock<const float> source,
                            AudioBlock<SIMDRegister<float>> destination,
                            Span<const float> zeroChannel)
    {
        // There must be enough room in the destination buffer for all source channels
        jassert (source.getNumChannels() <= destination.getNumChannels() * registerSize);
        // The destination buffer length must be at least the source buffer length
        jassert (source.getNumSamples() <= destination.getNumSamples());
        // The zero buffer length must be at least the source buffer length
        jassert (source.getNumSamples() <= zeroChannel.size());

        for (size_t i = 0; i < destination.getNumChannels(); ++i)
        {
            const float* channelPointerBuffer[registerSize]{};

            for (size_t j = 0; j < registerSize; ++j)
            {
                const auto offset = (i * registerSize) + j;
                channelPointerBuffer[j] = offset < source.getNumChannels() ? source.getChannelPointer (offset) : zeroChannel.data();
            }

            AudioData::interleaveSamples (AudioData::NonInterleavedSource<Format> { channelPointerBuffer, registerSize },
                                          AudioData::InterleavedDest<Format> { toBasePointer (destination.getChannelPointer (i)), registerSize },
                                          (int) source.getNumSamples());
        }
    }

    /** Converts an interleaved AudioBlock<SIMDRegister<float>> into a non-interleaved AudioBlock<float>.

        'source' must have at least as many samples as 'destination', and must have at least as many
        channels as 'destination' (at least ceil(destination.getNumChannels() / SIMDRegister<float>::size())).
        The 'scratchChannel' argument is used to store intermediate results, and its content will be
        left in an unspecified state after this function returns.

        @param source           the original audio block of interleaved data
        @param destination      the audio block that will hold the transformed result
        @param scratchChannel   a buffer of floats that has the same length in samples as source
    */
    static void deinterleave (AudioBlock<SIMDRegister<float>> source,
                              AudioBlock<float> destination,
                              Span<float> scratchChannel)
    {
        // The source buffer must have room for at least as many channels as the destination buffer
        jassert (destination.getNumChannels() <= source.getNumChannels() * registerSize);
        // The source buffer length must be at least the destination buffer length
        jassert (destination.getNumSamples() <= source.getNumSamples());
        // The zero buffer length must be at least the destination buffer length
        jassert (destination.getNumSamples() <= scratchChannel.size());

        for (size_t i = 0; i < destination.getNumChannels(); i += registerSize)
        {
            const auto sourceChannel = i / registerSize;
            float* channelPointerBuffer[registerSize]{};

            for (size_t j = 0; j < registerSize; ++j)
            {
                const auto offset = (sourceChannel * registerSize) + j;
                channelPointerBuffer[j] = offset < destination.getNumChannels() ? destination.getChannelPointer (offset) : scratchChannel.data();
            }

            AudioData::deinterleaveSamples (AudioData::InterleavedSource<Format> { toBasePointer (source.getChannelPointer (sourceChannel)), registerSize },
                                            AudioData::NonInterleavedDest<Format> { channelPointerBuffer, registerSize },
                                            (int) destination.getNumSamples());
        }
    }
};

#endif

} // namespace juce::dsp
