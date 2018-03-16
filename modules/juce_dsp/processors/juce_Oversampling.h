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

#ifndef DOXYGEN
template <typename NumericType>
class OversamplingEngine;
#endif

//===============================================================================
/**
    A processing class performing multi-channel oversampling.

    It can be configured to do 2 times, 4 times, 8 times or 16 times oversampling
    using a multi-stage approach, either polyphase allpass IIR filters or FIR
    filters for the filtering, and reports successfully the latency added by the
    filter stages.

    The principle of oversampling is to increase the sample rate of a given
    non-linear process, to prevent it from creating aliasing. Oversampling works
    by upsampling N times the input signal, processing the upsampling signal
    with the increased internal sample rate, and downsample the result to get
    back the original processing sample rate.

    Choose between FIR or IIR filtering depending on your needs in term of
    latency and phase distortion. With FIR filters, the phase is linear but the
    latency is maximum. With IIR filtering, the phase is compromised around the
    Nyquist frequency but the latency is minimum.

    @see FilterDesign.

    @tags{DSP}
*/
template <typename SampleType>
class JUCE_API  Oversampling
{
public:
    /** The type of filter that can be used for the oversampling processing. */
    enum FilterType
    {
        filterHalfBandFIREquiripple = 0,
        filterHalfBandPolyphaseIIR,
        numFilterTypes
    };

    //===============================================================================
    /**
        Constructor of the oversampling class. All the processing parameters must be
        provided at the creation of the oversampling object.

        Note : you might want to create a class heriting from Oversampling with a
        different constructor if you need more control on what happens in the process.

        @param numChannels      the number of channels to process with this object
        @param factor           the processing will perform 2 ^ factor times oversampling
        @param type             the type of filter design employed for filtering during
                                oversampling
        @param isMaxQuality     if the oversampling is done using the maximum quality,
                                the filters will be more efficient, but the CPU load will
                                increase as well
    */
    Oversampling (size_t numChannels, size_t factor, FilterType type, bool isMaxQuality = true);

    /** Destructor. */
    ~Oversampling();

    //===============================================================================
    /** Returns the latency in samples of the whole processing. Use this information
        in your main processor to compensate the additional latency involved with
        the oversampling, for example with a dry / wet functionality, and to report
        the latency to the DAW.

        Note : the latency might not be integer, so you might need to round its value
        or to compensate it properly in your processing code.
    */
    SampleType getLatencyInSamples() noexcept;

    /** Returns the current oversampling factor. */
    size_t getOversamplingFactor() noexcept;

    //===============================================================================
    /** Must be called before any processing, to set the buffer sizes of the internal
        buffers of the oversampling processing.
    */
    void initProcessing (size_t maximumNumberOfSamplesBeforeOversampling);

    /** Resets the processing pipeline, ready to oversample a new stream of data. */
    void reset() noexcept;

    /** Must be called to perform the upsampling, prior to any oversampled processing.

        Returns an AudioBlock referencing the oversampled input signal, which must be
        used to perform the non-linear processing which needs the higher sample rate.
        Don't forget to set the sample rate of that processing to N times the original
        sample rate.
    */
    dsp::AudioBlock<SampleType> processSamplesUp (const dsp::AudioBlock<SampleType> &inputBlock) noexcept;

    /** Must be called to perform the downsampling, after the upsampling and the
        non-linear processing. The output signal is probably delayed by the internal
        latency of the whole oversampling behaviour, so don't forget to take this
        into account.
    */
    void processSamplesDown (dsp::AudioBlock<SampleType> &outputBlock) noexcept;

private:
    //===============================================================================
    bool isMaximumQuality;
    size_t factorOversampling, numStages;
    FilterType type;
    size_t numChannels;

    //===============================================================================
    bool isReady = false;

    OwnedArray<OversamplingEngine<SampleType>> engines;

    //===============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oversampling)
};

} // namespace dsp
} // namespace juce
