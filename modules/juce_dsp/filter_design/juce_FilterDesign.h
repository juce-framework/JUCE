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

namespace juce::dsp
{

/**
    This class provides a set of functions which generates FIR::Coefficients
    and IIR::Coefficients, of high-order low-pass filters. They can be used
    for processing directly audio as an equalizer, in resampling algorithms etc.

    see FIRFilter::Coefficients, FIRFilter, WindowingFunction, IIRFilter::Coefficients, IIRFilter

    @tags{DSP}
*/
template <typename FloatType>
struct FilterDesign
{
    using FIRCoefficientsPtr = typename FIR::Coefficients<FloatType>::Ptr;
    using IIRCoefficients    = typename IIR::Coefficients<FloatType>;

    using WindowingMethod    = typename WindowingFunction<FloatType>::WindowingMethod;

    //==============================================================================
    /** This method generates a FIR::Coefficients for a low-pass filter, using
        the windowing design method, applied to a sinc impulse response. It is one
        of the simplest method used to generate a high order low-pass filter, which
        has the downside of needing more coefficients than more complex method to
        perform a given attenuation in the stop band.

        It generates linear phase filters coefficients.

        Note: The flatTop WindowingMethod generates an impulse response with a
        maximum amplitude higher than one, and might be normalised if necessary
        depending on the applications.

        @param frequency            the cutoff frequency of the low-pass filter
        @param sampleRate           the sample rate being used in the filter design
        @param order                the order of the filter
        @param type                 the type, must be a WindowingFunction::WindowingType
        @param beta                 an optional additional parameter useful for the Kaiser windowing function
    */

    static FIRCoefficientsPtr designFIRLowpassWindowMethod (FloatType frequency, double sampleRate,
                                                            size_t order, WindowingMethod type,
                                                            FloatType beta = static_cast<FloatType> (2));

    /** This a variant of the function designFIRLowpassWindowMethod, which allows the
        user to specify a transition width and a negative gain in dB,
        to get a low-pass filter using the Kaiser windowing function, with calculated
        values of the filter order and of the beta parameter, to satisfy the constraints.

        It generates linear phase filters coefficients.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param amplitudedB                  the maximum amplitude in dB expected in the stop band (must be negative)
    */

    static FIRCoefficientsPtr designFIRLowpassKaiserMethod (FloatType frequency, double sampleRate,
                                                            FloatType normalisedTransitionWidth,
                                                            FloatType amplitudedB);


    /** This method is also a variant of the function designFIRLowpassWindowMethod, using
        a rectangular window as a basis, and a spline transition between the pass band and
        the stop band, to reduce the Gibbs phenomenon.

        It generates linear phase filters coefficients.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param order                        the order of the filter
        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param spline                       between 1.0 and 4.0, indicates how much the transition
                                            is curved, with 1.0 meaning a straight line
    */
    static FIRCoefficientsPtr designFIRLowpassTransitionMethod (FloatType frequency, double sampleRate,
                                                                size_t order,
                                                                FloatType normalisedTransitionWidth,
                                                                FloatType spline);

    /** This method generates a FIR::Coefficients for a low-pass filter, by
        minimizing the average error between the generated filter and an ideal one
        using the least squares error criterion and matrices operations.

        It generates linear phase filters coefficients.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param order                        the order of the filter
        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param stopBandWeight               between 1.0 and 100.0, indicates how much we want
                                            attenuation in the stop band, against some oscillation
                                            in the pass band
    */
    static FIRCoefficientsPtr designFIRLowpassLeastSquaresMethod (FloatType frequency, double sampleRate, size_t order,
                                                                  FloatType normalisedTransitionWidth,
                                                                  FloatType stopBandWeight);

    /** This method generates a FIR::Coefficients for a low-pass filter, with
        a cutoff frequency at half band, using an algorithm described in the article
        "Design of Half-Band FIR Filters for Signal Compression" from Pavel
        Zahradnik, to get an equiripple like high order FIR filter, without the need
        of an iterative method and convergence failure risks.

        It generates linear phase filters coefficients.

        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param amplitudedB                  the maximum amplitude in dB expected in the stop band (must be negative)
    */
    static FIRCoefficientsPtr designFIRLowpassHalfBandEquirippleMethod (FloatType normalisedTransitionWidth,
                                                                        FloatType amplitudedB);

    //==============================================================================
    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIRFilters, providing a minimum phase low-pass filter without any
        ripple in the pass band and in the stop band.

        The algorithms are based on "Lecture Notes on Elliptic Filter Design" by
        Sophocles J. Orfanidis.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param passbandAmplitudedB          the highest gain in dB expected in the pass band (must be negative)
        @param stopbandAmplitudedB          the gain in dB expected in the stop band (must be negative)
    */

    static ReferenceCountedArray<IIRCoefficients> designIIRLowpassHighOrderButterworthMethod (FloatType frequency, double sampleRate,
                                                                                              FloatType normalisedTransitionWidth,
                                                                                              FloatType passbandAmplitudedB,
                                                                                              FloatType stopbandAmplitudedB);

    //==============================================================================
    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIRFilters, providing a minimum phase low-pass filter without any
        ripple in the pass band and in the stop band.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param order                        the order of the resulting IIR filter, providing
                                            an attenuation of -6 dB times order / octave
    */

    static ReferenceCountedArray<IIRCoefficients> designIIRLowpassHighOrderButterworthMethod (FloatType frequency, double sampleRate,
                                                                                              int order);

    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIRFilters, providing a minimum phase high-pass filter without any
        ripple in the pass band and in the stop band.

        @param frequency                    the cutoff frequency of the high-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param order                        the order of the resulting IIR filter, providing
                                            an attenuation of -6 dB times order / octave
    */

    static ReferenceCountedArray<IIRCoefficients> designIIRHighpassHighOrderButterworthMethod (FloatType frequency, double sampleRate,
                                                                                               int order);

    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIRFilters, providing a minimum phase low-pass filter without any
        ripple in the stop band only.

        The algorithms are based on "Lecture Notes on Elliptic Filter Design" by
        Sophocles J. Orfanidis.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param passbandAmplitudedB          the highest amplitude in dB expected in the pass band (must be negative)
        @param stopbandAmplitudedB          the lowest amplitude in dB expected in the stop band (must be negative)
    */
    static ReferenceCountedArray<IIRCoefficients> designIIRLowpassHighOrderChebyshev1Method (FloatType frequency, double sampleRate,
                                                                                             FloatType normalisedTransitionWidth,
                                                                                             FloatType passbandAmplitudedB,
                                                                                             FloatType stopbandAmplitudedB);

    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIRFilters, providing a minimum phase low-pass filter without any
        ripple in the pass band only.

        The algorithms are based on "Lecture Notes on Elliptic Filter Design" by
        Sophocles J. Orfanidis.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param passbandAmplitudedB          the highest amplitude in dB expected in the pass band (must be negative)
        @param stopbandAmplitudedB          the lowest amplitude in dB expected in the stop band (must be negative)
    */
    static ReferenceCountedArray<IIRCoefficients> designIIRLowpassHighOrderChebyshev2Method (FloatType frequency, double sampleRate,
                                                                                             FloatType normalisedTransitionWidth,
                                                                                             FloatType passbandAmplitudedB,
                                                                                             FloatType stopbandAmplitudedB);

    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIR::Filters, providing a minimum phase low-pass filter with ripples
        in both the pass band and in the stop band.

        The algorithms are based on "Lecture Notes on Elliptic Filter Design" by
        Sophocles J. Orfanidis.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param passbandAmplitudedB          the highest amplitude in dB expected in the pass band (must be negative)
        @param stopbandAmplitudedB          the lowest amplitude in dB expected in the stop band (must be negative)
    */
    static ReferenceCountedArray<IIRCoefficients> designIIRLowpassHighOrderEllipticMethod (FloatType frequency, double sampleRate,
                                                                                           FloatType normalisedTransitionWidth,
                                                                                           FloatType passbandAmplitudedB,
                                                                                           FloatType stopbandAmplitudedB);

    /** The structure returned by the function designIIRLowpassHalfBandPolyphaseAllpassMethod.

        The two first members of this structure directPath and delayedPath are arrays of
        IIR::Coefficients, made of polyphase second order allpass filters and an additional
        delay in the second array, that can be used in cascaded filters processed in two
        parallel paths, which must be summed at the end to get the high order efficient
        low-pass filtering. The last member is an array with the useful parameters for
        simulating the structure using any custom processing function.
    */
    struct IIRPolyphaseAllpassStructure
    {
        ReferenceCountedArray<IIRCoefficients> directPath, delayedPath;
        Array<double> alpha;
    };

    /** This method generates arrays of IIR::Coefficients for a low-pass filter, with
        a cutoff frequency at half band, using an algorithm described in the article
        "Digital Signal Processing Schemes for efficient interpolation and decimation" from
        Pavel Valenzuela and Constantinides.

        The result is a IIRPolyphaseAllpassStructure object.

        The two members of this structure directPath and delayedPath are arrays of
        IIR::Coefficients, made of polyphase second order allpass filters and an additional
        delay in the second array, that can be used in cascaded filters processed in two
        parallel paths, which must be summed at the end to get the high order efficient
        low-pass filtering.

        The gain of the resulting pass-band is 6 dB, so don't forget to compensate it if you
        want to use that method for something else than two times oversampling.

        @param normalisedTransitionWidth    the normalised size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param stopbandAmplitudedB          the maximum amplitude in dB expected in the stop band (must be negative)
    */
    static IIRPolyphaseAllpassStructure designIIRLowpassHalfBandPolyphaseAllpassMethod (FloatType normalisedTransitionWidth,
                                                                                        FloatType stopbandAmplitudedB);

private:
    //==============================================================================
    static Array<double> getPartialImpulseResponseHn (int n, double kp);

    static ReferenceCountedArray<IIRCoefficients> designIIRLowpassHighOrderGeneralMethod (int type, FloatType frequency, double sampleRate,
                                                                                          FloatType normalisedTransitionWidth,
                                                                                          FloatType passbandAmplitudedB,
                                                                                          FloatType stopbandAmplitudedB);
    FilterDesign() = delete;
};

} // namespace juce::dsp
