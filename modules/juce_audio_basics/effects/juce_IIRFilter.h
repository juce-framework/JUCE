/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_IIRFILTER_JUCEHEADER__
#define __JUCE_IIRFILTER_JUCEHEADER__


//==============================================================================
/**
    An IIR filter that can perform low, high, or band-pass filtering on an
    audio signal.

    @see IIRFilterAudioSource
*/
class JUCE_API  IIRFilter
{
public:
    //==============================================================================
    /** Creates a filter.

        Initially the filter is inactive, so will have no effect on samples that
        you process with it. Use the appropriate method to turn it into the type
        of filter needed.
    */
    IIRFilter();

    /** Creates a copy of another filter. */
    IIRFilter (const IIRFilter& other);

    /** Destructor. */
    ~IIRFilter();

    //==============================================================================
    /** Resets the filter's processing pipeline, ready to start a new stream of data.

        Note that this clears the processing state, but the type of filter and
        its coefficients aren't changed. To put a filter into an inactive state, use
        the makeInactive() method.
    */
    void reset() noexcept;

    /** Performs the filter operation on the given set of samples.
    */
    void processSamples (float* samples,
                         int numSamples) noexcept;

    /** Processes a single sample, without any locking or checking.

        Use this if you need fast processing of a single value, but be aware that
        this isn't thread-safe in the way that processSamples() is.
    */
    float processSingleSampleRaw (float sample) noexcept;

    //==============================================================================
    /** Sets the filter up to act as a low-pass filter.
    */
    void makeLowPass (double sampleRate,
                      double frequency) noexcept;

    /** Sets the filter up to act as a high-pass filter.
    */
    void makeHighPass (double sampleRate,
                       double frequency) noexcept;

    //==============================================================================
    /** Sets the filter up to act as a low-pass shelf filter with variable Q and gain.

        The gain is a scale factor that the low frequencies are multiplied by, so values
        greater than 1.0 will boost the low frequencies, values less than 1.0 will
        attenuate them.
    */
    void makeLowShelf (double sampleRate,
                       double cutOffFrequency,
                       double Q,
                       float gainFactor) noexcept;

    /** Sets the filter up to act as a high-pass shelf filter with variable Q and gain.

        The gain is a scale factor that the high frequencies are multiplied by, so values
        greater than 1.0 will boost the high frequencies, values less than 1.0 will
        attenuate them.
    */
    void makeHighShelf (double sampleRate,
                        double cutOffFrequency,
                        double Q,
                        float gainFactor) noexcept;

    /** Sets the filter up to act as a band pass filter centred around a
        frequency, with a variable Q and gain.

        The gain is a scale factor that the centre frequencies are multiplied by, so
        values greater than 1.0 will boost the centre frequencies, values less than
        1.0 will attenuate them.
    */
    void makeBandPass (double sampleRate,
                       double centreFrequency,
                       double Q,
                       float gainFactor) noexcept;

    /** Clears the filter's coefficients so that it becomes inactive.
    */
    void makeInactive() noexcept;

    //==============================================================================
    /** Makes this filter duplicate the set-up of another one.
    */
    void copyCoefficientsFrom (const IIRFilter& other) noexcept;


protected:
    //==============================================================================
    CriticalSection processLock;

    void setCoefficients (double c1, double c2, double c3,
                          double c4, double c5, double c6) noexcept;

    bool active;
    float coefficients[6];
    float x1, x2, y1, y2;

    // (use the copyCoefficientsFrom() method instead of this operator)
    IIRFilter& operator= (const IIRFilter&);
    JUCE_LEAK_DETECTOR (IIRFilter)
};


#endif   // __JUCE_IIRFILTER_JUCEHEADER__
