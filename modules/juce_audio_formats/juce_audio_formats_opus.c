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

#include "juce_audio_formats/codecs/opus/juce_opus_config.h"
#include <juce_core/system/juce_CompilerWarnings.h>

#if JUCE_USE_OPUS && (JUCE_INCLUDE_OPUS_CODE || ! defined (JUCE_INCLUDE_OPUS_CODE))

// GCC reports a "compiling without optimization" warning message when building in Debug mode,
// which this define suppresses.
#define OPUS_WILL_BE_SLOW

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-W#pragma-messages",
                                     "-Wcpp",
                                     "-Wcast-align",
                                     "-Wconditional-uninitialized",
                                     "-Wconversion",
                                     "-Wfloat-equal",
                                     "-Wlanguage-extension-token",
                                     "-Wmissing-prototypes",
                                     "-Wsign-conversion",
                                     "-Wimplicit-fallthrough")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4267 4127 4244 4100 4701 4702 4013 4133 4206 4305 4189 4706 4995 4365 4456 4457 4459 6297 6011 6001 6308 6255 6386 6385 6246 6387 6263 6262 28182 4310 28199 6326)

#include "juce_audio_formats/codecs/opus/opus/package_version.h"

// opus_custom.h only declares the opus_custom_* functions while these are
// defined, and its include guard makes the first inclusion the one that counts
#define CELT_ENCODER_C
#define CELT_DECODER_C

#include "juce_audio_formats/codecs/opus/opus/src/opus.c"
#include "juce_audio_formats/codecs/opus/opus/src/opus_decoder.c"
#include "juce_audio_formats/codecs/opus/opus/src/opus_encoder.c"
#include "juce_audio_formats/codecs/opus/opus/src/extensions.c"
#include "juce_audio_formats/codecs/opus/opus/src/opus_multistream.c"
#include "juce_audio_formats/codecs/opus/opus/src/opus_multistream_encoder.c"
#include "juce_audio_formats/codecs/opus/opus/src/opus_multistream_decoder.c"
#include "juce_audio_formats/codecs/opus/opus/src/repacketizer.c"
#include "juce_audio_formats/codecs/opus/opus/src/opus_projection_encoder.c"
#include "juce_audio_formats/codecs/opus/opus/src/opus_projection_decoder.c"
#include "juce_audio_formats/codecs/opus/opus/src/mapping_matrix.c"
#include "juce_audio_formats/codecs/opus/opus/src/analysis.c"
#include "juce_audio_formats/codecs/opus/opus/src/mlp.c"
#include "juce_audio_formats/codecs/opus/opus/src/mlp_data.c"
#include "juce_audio_formats/codecs/opus/opus/celt/bands.c"
#include "juce_audio_formats/codecs/opus/opus/celt/celt.c"
#include "juce_audio_formats/codecs/opus/opus/celt/celt_encoder.c"
#include "juce_audio_formats/codecs/opus/opus/celt/celt_decoder.c"
#include "juce_audio_formats/codecs/opus/opus/celt/cwrs.c"
#include "juce_audio_formats/codecs/opus/opus/celt/entcode.c"
#include "juce_audio_formats/codecs/opus/opus/celt/entdec.c"
#include "juce_audio_formats/codecs/opus/opus/celt/entenc.c"
#include "juce_audio_formats/codecs/opus/opus/celt/kiss_fft.c"
#include "juce_audio_formats/codecs/opus/opus/celt/laplace.c"
#include "juce_audio_formats/codecs/opus/opus/celt/mathops.c"
#include "juce_audio_formats/codecs/opus/opus/celt/mdct.c"
#include "juce_audio_formats/codecs/opus/opus/celt/modes.c"
#include "juce_audio_formats/codecs/opus/opus/celt/pitch.c"
#include "juce_audio_formats/codecs/opus/opus/celt/celt_lpc.c"
#include "juce_audio_formats/codecs/opus/opus/celt/quant_bands.c"
#include "juce_audio_formats/codecs/opus/opus/celt/rate.c"
#include "juce_audio_formats/codecs/opus/opus/celt/vq.c"
#include "juce_audio_formats/codecs/opus/opus/silk/CNG.c"
#include "juce_audio_formats/codecs/opus/opus/silk/code_signs.c"
#include "juce_audio_formats/codecs/opus/opus/silk/init_decoder.c"
#include "juce_audio_formats/codecs/opus/opus/silk/decode_core.c"
#include "juce_audio_formats/codecs/opus/opus/silk/decode_frame.c"
#include "juce_audio_formats/codecs/opus/opus/silk/decode_parameters.c"
#include "juce_audio_formats/codecs/opus/opus/silk/decode_indices.c"
#include "juce_audio_formats/codecs/opus/opus/silk/decode_pulses.c"
#include "juce_audio_formats/codecs/opus/opus/silk/decoder_set_fs.c"
#include "juce_audio_formats/codecs/opus/opus/silk/dec_API.c"

// PI is a double in celt/mathops.h and a float in silk/float/SigProc_FLP.h
#undef PI
#include "juce_audio_formats/codecs/opus/opus/silk/enc_API.c"
#include "juce_audio_formats/codecs/opus/opus/silk/encode_indices.c"
#include "juce_audio_formats/codecs/opus/opus/silk/encode_pulses.c"
#include "juce_audio_formats/codecs/opus/opus/silk/gain_quant.c"
#include "juce_audio_formats/codecs/opus/opus/silk/interpolate.c"
#include "juce_audio_formats/codecs/opus/opus/silk/LP_variable_cutoff.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NLSF_decode.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NSQ.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NSQ_del_dec.c"
#include "juce_audio_formats/codecs/opus/opus/silk/PLC.c"
#include "juce_audio_formats/codecs/opus/opus/silk/shell_coder.c"
#include "juce_audio_formats/codecs/opus/opus/silk/tables_gain.c"
#include "juce_audio_formats/codecs/opus/opus/silk/tables_LTP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/tables_NLSF_CB_NB_MB.c"
#include "juce_audio_formats/codecs/opus/opus/silk/tables_NLSF_CB_WB.c"
#include "juce_audio_formats/codecs/opus/opus/silk/tables_other.c"
#include "juce_audio_formats/codecs/opus/opus/silk/tables_pitch_lag.c"
#include "juce_audio_formats/codecs/opus/opus/silk/tables_pulses_per_block.c"
#include "juce_audio_formats/codecs/opus/opus/silk/VAD.c"
#include "juce_audio_formats/codecs/opus/opus/silk/control_audio_bandwidth.c"
#include "juce_audio_formats/codecs/opus/opus/silk/quant_LTP_gains.c"
#include "juce_audio_formats/codecs/opus/opus/silk/VQ_WMat_EC.c"
#include "juce_audio_formats/codecs/opus/opus/silk/HP_variable_cutoff.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NLSF_encode.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NLSF_VQ.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NLSF_unpack.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NLSF_del_dec_quant.c"
#include "juce_audio_formats/codecs/opus/opus/silk/process_NLSFs.c"
#include "juce_audio_formats/codecs/opus/opus/silk/stereo_LR_to_MS.c"
#include "juce_audio_formats/codecs/opus/opus/silk/stereo_MS_to_LR.c"
#include "juce_audio_formats/codecs/opus/opus/silk/check_control_input.c"
#include "juce_audio_formats/codecs/opus/opus/silk/control_SNR.c"
#include "juce_audio_formats/codecs/opus/opus/silk/init_encoder.c"
#include "juce_audio_formats/codecs/opus/opus/silk/control_codec.c"
#include "juce_audio_formats/codecs/opus/opus/silk/A2NLSF.c"
#include "juce_audio_formats/codecs/opus/opus/silk/ana_filt_bank_1.c"
#include "juce_audio_formats/codecs/opus/opus/silk/biquad_alt.c"
#include "juce_audio_formats/codecs/opus/opus/silk/bwexpander_32.c"
#include "juce_audio_formats/codecs/opus/opus/silk/bwexpander.c"
#include "juce_audio_formats/codecs/opus/opus/silk/debug.c"
#include "juce_audio_formats/codecs/opus/opus/silk/decode_pitch.c"
#include "juce_audio_formats/codecs/opus/opus/silk/inner_prod_aligned.c"
#include "juce_audio_formats/codecs/opus/opus/silk/lin2log.c"
#include "juce_audio_formats/codecs/opus/opus/silk/log2lin.c"
#include "juce_audio_formats/codecs/opus/opus/silk/LPC_analysis_filter.c"
#include "juce_audio_formats/codecs/opus/opus/silk/LPC_inv_pred_gain.c"
#include "juce_audio_formats/codecs/opus/opus/silk/table_LSF_cos.c"

// QA is 24 in silk/LPC_inv_pred_gain.c and 16 in silk/NLSF2A.c
#undef QA
#include "juce_audio_formats/codecs/opus/opus/silk/NLSF2A.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NLSF_stabilize.c"
#include "juce_audio_formats/codecs/opus/opus/silk/NLSF_VQ_weights_laroia.c"
#include "juce_audio_formats/codecs/opus/opus/silk/pitch_est_tables.c"
#include "juce_audio_formats/codecs/opus/opus/silk/resampler.c"
#include "juce_audio_formats/codecs/opus/opus/silk/resampler_down2_3.c"
#include "juce_audio_formats/codecs/opus/opus/silk/resampler_down2.c"
#include "juce_audio_formats/codecs/opus/opus/silk/resampler_private_AR2.c"
#include "juce_audio_formats/codecs/opus/opus/silk/resampler_private_down_FIR.c"
#include "juce_audio_formats/codecs/opus/opus/silk/resampler_private_IIR_FIR.c"
#include "juce_audio_formats/codecs/opus/opus/silk/resampler_private_up2_HQ.c"
#include "juce_audio_formats/codecs/opus/opus/silk/resampler_rom.c"
#include "juce_audio_formats/codecs/opus/opus/silk/sigm_Q15.c"
#include "juce_audio_formats/codecs/opus/opus/silk/sort.c"
#include "juce_audio_formats/codecs/opus/opus/silk/sum_sqr_shift.c"
#include "juce_audio_formats/codecs/opus/opus/silk/stereo_decode_pred.c"
#include "juce_audio_formats/codecs/opus/opus/silk/stereo_encode_pred.c"
#include "juce_audio_formats/codecs/opus/opus/silk/stereo_find_predictor.c"
#include "juce_audio_formats/codecs/opus/opus/silk/stereo_quant_pred.c"
#include "juce_audio_formats/codecs/opus/opus/silk/LPC_fit.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/apply_sine_window_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/corrMatrix_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/encode_frame_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/find_LPC_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/find_LTP_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/find_pitch_lags_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/find_pred_coefs_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/LPC_analysis_filter_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/LTP_analysis_filter_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/LTP_scale_ctrl_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/noise_shape_analysis_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/process_gains_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/regularize_correlations_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/residual_energy_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/warped_autocorrelation_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/wrappers_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/autocorrelation_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/burg_modified_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/bwexpander_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/energy_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/inner_product_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/k2a_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/LPC_inv_pred_gain_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/pitch_analysis_core_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/scale_copy_vector_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/scale_vector_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/schur_FLP.c"
#include "juce_audio_formats/codecs/opus/opus/silk/float/sort_FLP.c"

#if OPUS_ARM_PRESUME_NEON_INTR
 #include "juce_audio_formats/codecs/opus/opus/celt/arm/celt_neon_intr.c"
 #include "juce_audio_formats/codecs/opus/opus/celt/arm/pitch_neon_intr.c"
 #include "juce_audio_formats/codecs/opus/opus/silk/arm/biquad_alt_neon_intr.c"
 #undef QA
 #include "juce_audio_formats/codecs/opus/opus/silk/arm/LPC_inv_pred_gain_neon_intr.c"
 #include "juce_audio_formats/codecs/opus/opus/silk/arm/NSQ_del_dec_neon_intr.c"
 #include "juce_audio_formats/codecs/opus/opus/silk/arm/NSQ_neon.c"
#endif

#if OPUS_X86_PRESUME_SSE2
 #include "juce_audio_formats/codecs/opus/opus/celt/x86/pitch_sse.c"
 #include "juce_audio_formats/codecs/opus/opus/celt/x86/pitch_sse2.c"
 #include "juce_audio_formats/codecs/opus/opus/celt/x86/vq_sse2.c"
#endif

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#else
// To get around 'empty translation unit' errors and 'has no symbols' warnings emitted by libtool
void juce_audioFormatsOpusPlaceholder (void);
void juce_audioFormatsOpusPlaceholder (void) {}
#endif
