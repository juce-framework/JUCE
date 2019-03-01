/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

/**
    Performs stereo uniform-partitioned convolution of an input signal with an
    impulse response in the frequency domain, using the juce FFT class.

    It provides some thread-safe functions to load impulse responses as well,
    from audio files or memory on the fly without any noticeable artefacts,
    performing resampling and trimming if necessary.

    The processing is equivalent to the time domain convolution done in the
    class FIRFilter, with a FIRFilter::Coefficients object having as
    coefficients the samples of the impulse response. However, it is more
    efficient in general to do frequency domain convolution when the size of
    the impulse response is higher than 64 samples.

    @see FIRFilter, FIRFilter::Coefficients, FFT

    @tags{DSP}
*/
class JUCE_API  Convolution
{
public:
    //==============================================================================
    /** Initialises an object for performing convolution in the frequency domain. */
    Convolution();

    /** Destructor. */
    ~Convolution();

    //==============================================================================
    /** Must be called before loading any impulse response, to provide to the
        convolution the maximumBufferSize to handle, and the sample rate useful for
        optional resampling.
    */
    void prepare (const ProcessSpec&);

    /** Resets the processing pipeline, ready to start a new stream of data. */
    void reset() noexcept;

    /** Performs the filter operation on the given set of samples, with optional
        stereo processing.
    */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        static_assert (std::is_same<typename ProcessContext::SampleType, float>::value,
                       "Convolution engine only supports single precision floating point data");

        processSamples (context.getInputBlock(), context.getOutputBlock(), context.isBypassed);
    }

    //==============================================================================
    /** This function loads an impulse response audio file from memory, added in a
        JUCE project with the Projucer as binary data. It can load any of the audio
        formats registered in JUCE, and performs some resampling and pre-processing
        as well if needed.

        Note: Obviously, don't try to use this function on float samples, since the
        data is supposed to be an audio file in its binary format, and be sure that
        the original data is not going to move at all its memory location during the
        process !!

        @param sourceData               the block of data to use as the stream's source
        @param sourceDataSize           the number of bytes in the source data block
        @param wantsStereo              requests to process both stereo channels or only one mono channel
        @param wantsTrimming            requests to trim the start and the end of the impulse response
        @param size                     the expected size for the impulse response after loading, can be
                                        set to 0 for requesting maximum original impulse response size
        @param wantsNormalisation       requests to normalise the impulse response amplitude
    */
    void loadImpulseResponse (const void* sourceData, size_t sourceDataSize,
                              bool wantsStereo, bool wantsTrimming, size_t size,
                              bool wantsNormalisation = true);

    /** This function loads an impulse response from an audio file on any drive. It
        can load any of the audio formats registered in JUCE, and performs some
        resampling and pre-processing as well if needed.

        @param fileImpulseResponse      the location of the audio file
        @param wantsStereo              requests to process both stereo channels or only one mono channel
        @param wantsTrimming            requests to trim the start and the end of the impulse response
        @param size                     the expected size for the impulse response after loading, can be
                                        set to 0 for requesting maximum original impulse response size
        @param wantsNormalisation       requests to normalise the impulse response amplitude
    */
    void loadImpulseResponse (const File& fileImpulseResponse,
                              bool wantsStereo, bool wantsTrimming, size_t size,
                              bool wantsNormalisation = true);

    /** This function loads an impulse response from an audio buffer, which is
        copied before doing anything else. Performs some resampling and
        pre-processing as well if needed.

        @param buffer                   the AudioBuffer to use
        @param bufferSampleRate         the sampleRate of the data in the AudioBuffer
        @param wantsStereo              requests to process both stereo channels or only one mono channel
        @param wantsTrimming            requests to trim the start and the end of the impulse response
        @param wantsNormalisation       requests to normalise the impulse response amplitude
        @param size                     the expected size for the impulse response after loading, can be
                                        set to 0 for requesting maximum original impulse response size
    */
    void copyAndLoadImpulseResponseFromBuffer (AudioBuffer<float>& buffer, double bufferSampleRate,
                                               bool wantsStereo, bool wantsTrimming, bool wantsNormalisation,
                                               size_t size);

    /** This function loads an impulse response from an audio block, which is
        copied before doing anything else. Performs some resampling and
        pre-processing as well if needed.

        @param block                    the AudioBlock to use
        @param bufferSampleRate         the sampleRate of the data in the AudioBuffer
        @param wantsStereo              requests to process both stereo channels or only one channel
        @param wantsTrimming            requests to trim the start and the end of the impulse response
        @param wantsNormalisation       requests to normalise the impulse response amplitude
        @param size                     the expected size for the impulse response after loading,
                                        -1 for maximum length
    */
    void copyAndLoadImpulseResponseFromBlock (AudioBlock<float> block, double bufferSampleRate,
                                              bool wantsStereo, bool wantsTrimming, bool wantsNormalisation,
                                              size_t size);


private:
    //==============================================================================
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    //==============================================================================
    void processSamples (const AudioBlock<float>&, AudioBlock<float>&, bool isBypassed) noexcept;

    //==============================================================================
    double sampleRate;
    bool currentIsBypassed = false;
    bool isActive = false;
    SmoothedValue<float> volumeDry[2], volumeWet[2];
    AudioBlock<float> dryBuffer;
    HeapBlock<char> dryBufferStorage;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Convolution)
};

} // namespace dsp
} // namespace juce
