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

//==============================================================================
/**
    A type of AudioSource that takes an input source and changes its sample rate.

    @see AudioSource, LagrangeInterpolator, CatmullRomInterpolator

    @tags{Audio}
*/
class JUCE_API  ResamplingAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a ResamplingAudioSource for a given input source.

        @param inputSource              the input source to read from
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
        @param numChannels              the number of channels to process
    */
    ResamplingAudioSource (AudioSource* inputSource,
                           bool deleteInputWhenDeleted,
                           int numChannels = 2);

    /** Destructor. */
    ~ResamplingAudioSource() override;

    /** Changes the resampling ratio.

        (This value can be changed at any time, even while the source is running).

        @param samplesInPerOutputSample     if set to 1.0, the input is passed through; higher
                                            values will speed it up; lower values will slow it
                                            down. The ratio must be greater than 0
    */
    void setResamplingRatio (double samplesInPerOutputSample);

    /** Returns the current resampling ratio.

        This is the value that was set by setResamplingRatio().
    */
    double getResamplingRatio() const noexcept                  { return ratio; }

    /** Clears any buffers and filters that the resampler is using. */
    void flushBuffers();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;

private:
    //==============================================================================
    OptionalScopedPointer<AudioSource> input;
    double ratio = 1.0, lastRatio = 1.0;
    AudioBuffer<float> buffer;
    int bufferPos = 0, sampsInBuffer = 0;
    double subSampleOffset = 0.0;
    double coefficients[6];
    SpinLock ratioLock;
    CriticalSection callbackLock;
    const int numChannels;
    HeapBlock<float*> destBuffers;
    HeapBlock<const float*> srcBuffers;

    void setFilterCoefficients (double c1, double c2, double c3, double c4, double c5, double c6);
    void createLowPass (double proportionalRate);

    struct FilterState
    {
        double x1, x2, y1, y2;
    };

    HeapBlock<FilterState> filterStates;
    void resetFilters();

    void applyFilter (float* samples, int num, FilterState& fs);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResamplingAudioSource)
};

} // namespace juce
