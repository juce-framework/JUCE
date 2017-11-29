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
    This class provides a set of functions which generates FIR::Coefficients
    and IIR::Coefficients, of high-order lowpass filters. They can be used
    for processing directly audio as an equalizer, in resampling algorithms etc.

    see FIRFilter::Coefficients, FIRFilter, WindowingFunction, IIRFilter::Coefficients, IIRFilter
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

        Note : the flatTop WindowingMethod generates an impulse response with a
        maximum amplitude higher than one, and might be normalized if necessary
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
        user to specify a transition width and an attenuation in dB,
        to get a low-pass filter using the Kaiser windowing function, with calculated
        values of the filter order and of the beta parameter, to satisfy the constraints.

        It generates linear phase filters coefficients.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param attenuationdB                the attenuation in dB expected in the stop band
    */

    static FIRCoefficientsPtr designFIRLowpassKaiserMethod (FloatType frequency, double sampleRate,
                                                            FloatType normalizedTransitionWidth,
                                                            FloatType attenuationdB);


    /** This method is also a variant of the function designFIRLowpassWindowMethod, using
        a rectangular window as a basis, and a spline transition between the pass band and
        the stop band, to reduce the Gibbs phenomenon.

        It generates linear phase filters coefficients.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param order                        the order of the filter
        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param spline                       between 1.0 and 4.0, indicates how much the transition
                                            is curved, with 1.0 meaning a straight line
    */
    static FIRCoefficientsPtr designFIRLowpassTransitionMethod (FloatType frequency, double sampleRate,
                                                                size_t order,
                                                                FloatType normalizedTransitionWidth,
                                                                FloatType spline);

    /** This method generates a FIR::Coefficients for a low-pass filter, by
        minimizing the average error between the generated filter and an ideal one
        using the least squares error criterion and matrices operations.

        It generates linear phase filters coefficients.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param order                        the order of the filter
        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param stopBandWeight               between 1.0 and 100.0, indicates how much we want
                                            attenuation in the stop band, against some oscillation
                                            in the pass band
    */
    static FIRCoefficientsPtr designFIRLowpassLeastSquaresMethod (FloatType frequency, double sampleRate, size_t order,
                                                                  FloatType normalizedTransitionWidth,
                                                                  FloatType stopBandWeight);

    /** This method generates a FIR::Coefficients for a low-pass filter, with
        a cutoff frequency at half band, using an algorithm described in the article
        "Design of Half-Band FIR Filters for Signal Compression" from Pavel
        Zahradnik, to get an equiripple like high order FIR filter, without the need
        of an iterative method and convergence failure risks.

        It generates linear phase filters coefficients.

        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param attenuationdB                the attenuation in dB expected in the stop band
    */
    static FIRCoefficientsPtr designFIRLowpassHalfBandEquirippleMethod (FloatType normalizedTransitionWidth,
                                                                        FloatType attenuationdB);

    //==============================================================================
    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIRFilters, providing a minimum phase lowpass filter without any
        ripple in the pass band and in the stop band.

        The algorithms are based on "Lecture Notes on Elliptic Filter Design" by
        Sophocles J. Orfanidis.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param passbandAttenuationdB        the lowest attenuation in dB expected in the pass band
        @param stopbandAttenuationdB        the attenuation in dB expected in the stop band
    */

    static Array<IIRCoefficients> designIIRLowpassHighOrderButterworthMethod (FloatType frequency, double sampleRate,
                                                                              FloatType normalizedTransitionWidth,
                                                                              FloatType passbandAttenuationdB,
                                                                              FloatType stopbandAttenuationdB);

    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIRFilters, providing a minimum phase lowpass filter without any
        ripple in the stop band only.

        The algorithms are based on "Lecture Notes on Elliptic Filter Design" by
        Sophocles J. Orfanidis.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param passbandAttenuationdB        the lowest attenuation in dB expected in the pass band
        @param stopbandAttenuationdB        the attenuation in dB expected in the stop band
    */
    static Array<IIRCoefficients> designIIRLowpassHighOrderChebyshev1Method (FloatType frequency, double sampleRate,
                                                                             FloatType normalizedTransitionWidth,
                                                                             FloatType passbandAttenuationdB,
                                                                             FloatType stopbandAttenuationdB);

    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIRFilters, providing a minimum phase lowpass filter without any
        ripple in the pass band only.

        The algorithms are based on "Lecture Notes on Elliptic Filter Design" by
        Sophocles J. Orfanidis.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param passbandAttenuationdB        the lowest attenuation in dB expected in the pass band
        @param stopbandAttenuationdB        the attenuation in dB expected in the stop band
    */
    static Array<IIRCoefficients> designIIRLowpassHighOrderChebyshev2Method (FloatType frequency, double sampleRate,
                                                                             FloatType normalizedTransitionWidth,
                                                                             FloatType passbandAttenuationdB,
                                                                             FloatType stopbandAttenuationdB);

    /** This method returns an array of IIR::Coefficients, made to be used in
        cascaded IIR::Filters, providing a minimum phase lowpass filter with ripples
        in both the pass band and in the stop band.

        The algorithms are based on "Lecture Notes on Elliptic Filter Design" by
        Sophocles J. Orfanidis.

        @param frequency                    the cutoff frequency of the low-pass filter
        @param sampleRate                   the sample rate being used in the filter design
        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param passbandAttenuationdB        the lowest attenuation in dB expected in the pass band
        @param stopbandAttenuationdB        the attenuation in dB expected in the stop band
    */
    static Array<IIRCoefficients> designIIRLowpassHighOrderEllipticMethod (FloatType frequency, double sampleRate,
                                                                           FloatType normalizedTransitionWidth,
                                                                           FloatType passbandAttenuationdB,
                                                                           FloatType stopbandAttenuationdB);

    /** The structure returned by the function designIIRLowpassHalfBandPolyphaseAllpassMethod.

        The two members of this structure directPath and delayedPath are arrays of
        IIR::Coefficients, made of polyphase second order allpass filters and an additional
        delay in the second array, that can be used in cascaded filters processed in two
        parallel paths, which must be summed at the end to get the high order efficient
        low-pass filtering.
    */
    struct IIRPolyphaseAllpassStructure { Array<IIRCoefficients> directPath, delayedPath; };

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

        @param normalizedTransitionWidth    the normalized size between 0 and 0.5 of the transition
                                            between the pass band and the stop band
        @param stopbandAttenuationdB        the attenuation in dB expected in the stop band
    */
    static IIRPolyphaseAllpassStructure designIIRLowpassHalfBandPolyphaseAllpassMethod (FloatType normalizedTransitionWidth,
                                                                                        FloatType stopbandAttenuationdB);

private:
    //==============================================================================
    static Array<double> getPartialImpulseResponseHn (int n, double kp);

    static Array<IIRCoefficients> designIIRLowpassHighOrderGeneralMethod (int type, FloatType frequency, double sampleRate,
                                                                          FloatType normalizedTransitionWidth,
                                                                          FloatType passbandAttenuationdB,
                                                                          FloatType stopbandAttenuationdB);
    FilterDesign() = delete;
};

} // namespace dsp
} // namespace juce
