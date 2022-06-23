/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifdef JUCE_DSP_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "juce_dsp.h"

#ifndef JUCE_USE_VDSP_FRAMEWORK
 #define JUCE_USE_VDSP_FRAMEWORK 1
#endif

#if (JUCE_MAC || JUCE_IOS) && JUCE_USE_VDSP_FRAMEWORK
 #include <Accelerate/Accelerate.h>
#else
 #undef JUCE_USE_VDSP_FRAMEWORK
#endif

#if JUCE_DSP_USE_INTEL_MKL
 #include <mkl_dfti.h>
#endif

#if _IPP_SEQUENTIAL_STATIC || _IPP_SEQUENTIAL_DYNAMIC || _IPP_PARALLEL_STATIC || _IPP_PARALLEL_DYNAMIC
 #include <ippcore.h>
 #include <ipps.h>
 #define JUCE_IPP_AVAILABLE 1
#endif

#include "processors/juce_FIRFilter.cpp"
#include "processors/juce_IIRFilter.cpp"
#include "processors/juce_FirstOrderTPTFilter.cpp"
#include "processors/juce_Panner.cpp"
#include "processors/juce_Oversampling.cpp"
#include "processors/juce_BallisticsFilter.cpp"
#include "processors/juce_LinkwitzRileyFilter.cpp"
#include "processors/juce_DelayLine.cpp"
#include "processors/juce_DryWetMixer.cpp"
#include "processors/juce_StateVariableTPTFilter.cpp"
#include "maths/juce_SpecialFunctions.cpp"
#include "maths/juce_Matrix.cpp"
#include "maths/juce_LookupTable.cpp"
#include "frequency/juce_FFT.cpp"
#include "frequency/juce_Convolution.cpp"
#include "frequency/juce_Windowing.cpp"
#include "filter_design/juce_FilterDesign.cpp"
#include "widgets/juce_LadderFilter.cpp"
#include "widgets/juce_Compressor.cpp"
#include "widgets/juce_NoiseGate.cpp"
#include "widgets/juce_Limiter.cpp"
#include "widgets/juce_Phaser.cpp"
#include "widgets/juce_Chorus.cpp"

#if JUCE_USE_SIMD
 #if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
  #ifdef __AVX2__
   #include "native/juce_avx_SIMDNativeOps.cpp"
  #else
   #include "native/juce_sse_SIMDNativeOps.cpp"
  #endif
 #elif defined(__arm__) || defined(_M_ARM) || defined (__arm64__) || defined (__aarch64__)
  #include "native/juce_neon_SIMDNativeOps.cpp"
 #else
  #error "SIMD register support not implemented for this platform"
 #endif
#endif

#if JUCE_UNIT_TESTS
 #include "maths/juce_Matrix_test.cpp"
 #include "maths/juce_LogRampedValue_test.cpp"

 #if JUCE_USE_SIMD
  #include "containers/juce_SIMDRegister_test.cpp"
 #endif

 #include "containers/juce_AudioBlock_test.cpp"
 #include "containers/juce_FixedSizeFunction_test.cpp"
 #include "frequency/juce_Convolution_test.cpp"
 #include "frequency/juce_FFT_test.cpp"
 #include "processors/juce_FIRFilter_test.cpp"
 #include "processors/juce_ProcessorChain_test.cpp"
#endif
