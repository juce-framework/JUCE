/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_dsp
  vendor:             juce
  version:            6.0.1
  name:               JUCE DSP classes
  description:        Classes for audio buffer manipulation, digital audio processing, filtering, oversampling, fast math functions etc.
  website:            http://www.juce.com/juce
  license:            GPL/Commercial
  minimumCppStandard: 14

  dependencies:       juce_audio_formats
  OSXFrameworks:      Accelerate
  iOSFrameworks:      Accelerate

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once

#define JUCE_DSP_H_INCLUDED

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

#if defined(_M_X64) || defined(__amd64__) || defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP == 2)

 #if defined(_M_X64) || defined(__amd64__)
  #ifndef __SSE2__
   #define __SSE2__
  #endif
 #endif

 #ifndef JUCE_USE_SIMD
  #define JUCE_USE_SIMD 1
 #endif

 #if JUCE_USE_SIMD
  #include <immintrin.h>
 #endif

#elif defined (__ARM_NEON__) || defined (__ARM_NEON) || defined (__arm64__) || defined (__aarch64__)

 #ifndef JUCE_USE_SIMD
  #define JUCE_USE_SIMD 1
 #endif

 #include <arm_neon.h>

#else

 // No SIMD Support
 #ifndef JUCE_USE_SIMD
  #define JUCE_USE_SIMD 0
 #endif

#endif

#ifndef JUCE_VECTOR_CALLTYPE
 // __vectorcall does not work on 64-bit due to internal compiler error in
 // release mode in both VS2015 and VS2017. Re-enable when Microsoft fixes this
 #if _MSC_VER && JUCE_USE_SIMD && ! (defined(_M_X64) || defined(__amd64__))
  #define JUCE_VECTOR_CALLTYPE __vectorcall
 #else
  #define JUCE_VECTOR_CALLTYPE
 #endif
#endif

#include <atomic>
#include <complex>
#include <cmath>
#include <array>


//==============================================================================
/** Config: JUCE_ASSERTION_FIRFILTER

    When this flag is enabled, an assertion will be generated during the
    execution of DEBUG configurations if you use a FIRFilter class to process
    FIRCoefficients with a size higher than 128, to tell you that's it would be
    more efficient to use the Convolution class instead. It is enabled by
    default, but you may want to disable it if you really want to process such
    a filter in the time domain.
*/
#ifndef JUCE_ASSERTION_FIRFILTER
 #define JUCE_ASSERTION_FIRFILTER 1
#endif

/** Config: JUCE_DSP_USE_INTEL_MKL

    If this flag is set, then JUCE will use Intel's MKL for JUCE's FFT and
    convolution classes.

    The folder containing the mkl_dfti.h header must be in your header
    search paths when using this flag. You also need to add all the necessary
    intel mkl libraries to the "External Libraries to Link" field in the
    Projucer.
*/
#ifndef JUCE_DSP_USE_INTEL_MKL
 #define JUCE_DSP_USE_INTEL_MKL 0
#endif

/** Config: JUCE_DSP_USE_SHARED_FFTW

    If this flag is set, then JUCE will search for the fftw shared libraries
    at runtime and use the library for JUCE's FFT and convolution classes.

    If the library is not found, then JUCE's fallback FFT routines will be used.

    This is especially useful on linux as fftw often comes pre-installed on
    popular linux distros.

    You must respect the FFTW license when enabling this option.
*/
 #ifndef JUCE_DSP_USE_SHARED_FFTW
 #define JUCE_DSP_USE_SHARED_FFTW 0
#endif

/** Config: JUCE_DSP_USE_STATIC_FFTW

    If this flag is set, then JUCE will use the statically linked fftw libraries
    for JUCE's FFT and convolution classes.

    You must add the fftw header/library folder to the extra header/library search
    paths of your JUCE project. You also need to add the fftw library itself
    to the extra libraries supplied to your JUCE project during linking.

    You must respect the FFTW license when enabling this option.
*/
#ifndef JUCE_DSP_USE_STATIC_FFTW
 #define JUCE_DSP_USE_STATIC_FFTW 0
#endif

/** Config: JUCE_DSP_ENABLE_SNAP_TO_ZERO

    Enables code in the dsp module to avoid floating point denormals during the
    processing of some of the dsp module's filters.

    Enabling this will add a slight performance overhead to the DSP module's
    filters and algorithms. If your audio app already disables denormals altogether
    (for example, by using the ScopedNoDenormals class or the
    FloatVectorOperations::disableDenormalisedNumberSupport method), then you
    can safely disable this flag to shave off a few cpu cycles from the DSP module's
    filters and algorithms.
*/
#ifndef JUCE_DSP_ENABLE_SNAP_TO_ZERO
 #define JUCE_DSP_ENABLE_SNAP_TO_ZERO 1
#endif


//==============================================================================
#undef Complex  // apparently some C libraries actually define these symbols (!)
#undef Factor
#undef check

namespace juce
{
    namespace dsp
    {
        template <typename Type>
        using Complex = std::complex<Type>;

        //==============================================================================
        namespace util
        {
            /** Use this function to prevent denormals on intel CPUs.
                This function will work with both primitives and simple containers.
            */
          #if JUCE_DSP_ENABLE_SNAP_TO_ZERO
            inline void snapToZero (float&       x) noexcept            { JUCE_SNAP_TO_ZERO (x); }
           #ifndef DOXYGEN
            inline void snapToZero (double&      x) noexcept            { JUCE_SNAP_TO_ZERO (x); }
            inline void snapToZero (long double& x) noexcept            { JUCE_SNAP_TO_ZERO (x); }
           #endif
          #else
            inline void snapToZero (float&       x) noexcept            { ignoreUnused (x); }
           #ifndef DOXYGEN
            inline void snapToZero (double&      x) noexcept            { ignoreUnused (x); }
            inline void snapToZero (long double& x) noexcept            { ignoreUnused (x); }
           #endif
          #endif
        }
    }
}

//==============================================================================
#if JUCE_USE_SIMD
 #include "native/juce_fallback_SIMDNativeOps.h"

 // include the correct native file for this build target CPU
 #if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
  #ifdef __AVX2__
   #include "native/juce_avx_SIMDNativeOps.h"
  #else
   #include "native/juce_sse_SIMDNativeOps.h"
  #endif
 #elif defined(__arm__) || defined(_M_ARM) || defined (__arm64__) || defined (__aarch64__)
  #include "native/juce_neon_SIMDNativeOps.h"
 #else
  #error "SIMD register support not implemented for this platform"
 #endif

 #include "containers/juce_SIMDRegister.h"
#endif

#include "maths/juce_SpecialFunctions.h"
#include "maths/juce_Matrix.h"
#include "maths/juce_Phase.h"
#include "maths/juce_Polynomial.h"
#include "maths/juce_FastMathApproximations.h"
#include "maths/juce_LookupTable.h"
#include "maths/juce_LogRampedValue.h"
#include "containers/juce_AudioBlock.h"
#include "processors/juce_ProcessContext.h"
#include "processors/juce_ProcessorWrapper.h"
#include "processors/juce_ProcessorChain.h"
#include "processors/juce_ProcessorDuplicator.h"
#include "processors/juce_IIRFilter.h"
#include "processors/juce_FIRFilter.h"
#include "processors/juce_StateVariableFilter.h"
#include "processors/juce_FirstOrderTPTFilter.h"
#include "processors/juce_Panner.h"
#include "processors/juce_DelayLine.h"
#include "processors/juce_Oversampling.h"
#include "processors/juce_BallisticsFilter.h"
#include "processors/juce_LinkwitzRileyFilter.h"
#include "processors/juce_DryWetMixer.h"
#include "processors/juce_StateVariableTPTFilter.h"
#include "frequency/juce_FFT.h"
#include "frequency/juce_Convolution.h"
#include "frequency/juce_Windowing.h"
#include "filter_design/juce_FilterDesign.h"
#include "widgets/juce_Reverb.h"
#include "widgets/juce_Bias.h"
#include "widgets/juce_Gain.h"
#include "widgets/juce_WaveShaper.h"
#include "widgets/juce_Oscillator.h"
#include "widgets/juce_LadderFilter.h"
#include "widgets/juce_Compressor.h"
#include "widgets/juce_NoiseGate.h"
#include "widgets/juce_Limiter.h"
#include "widgets/juce_Phaser.h"
#include "widgets/juce_Chorus.h"
