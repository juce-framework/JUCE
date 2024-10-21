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
 #if JUCE_INTEL
  #ifdef __AVX2__
   #include "native/juce_SIMDNativeOps_avx.cpp"
  #else
   #include "native/juce_SIMDNativeOps_sse.cpp"
  #endif
 #elif JUCE_ARM
  #include "native/juce_SIMDNativeOps_neon.cpp"
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
 #include "frequency/juce_Convolution_test.cpp"
 #include "frequency/juce_FFT_test.cpp"
 #include "processors/juce_FIRFilter_test.cpp"
 #include "processors/juce_ProcessorChain_test.cpp"
#endif
