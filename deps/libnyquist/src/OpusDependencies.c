/*
Copyright (c) 2019, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// https://dxr.mozilla.org/mozilla-central/source/media/libopus

#if (_MSC_VER)
    #pragma warning (push)
    #pragma warning (disable: 181 111 4267 4996 4244 4701 4702 4133 4100 4127 4206 4312 4505 4365 4005 4013 4334 4703)
#endif
        
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconversion"
    #pragma clang diagnostic ignored "-Wshadow"
    #pragma clang diagnostic ignored "-Wdeprecated-register"
#endif

#undef HAVE_CONFIG_H

#define USE_ALLOCA 1
#define OPUS_BUILD 1

/* Enable SSE functions, if compiled with SSE/SSE2 (note that AMD64 implies SSE2) */
#if defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP >= 1))
#define __SSE__ 1
#endif

//////////
// CELT //
//////////

#include "opus/celt/arch.h"
#include "opus/celt/bands.h"
#include "opus/celt/celt.h"
#include "opus/celt/celt_lpc.h"
#include "opus/celt/cwrs.h"
#include "opus/celt/ecintrin.h"
#include "opus/celt/entcode.h"
#include "opus/celt/entdec.h"
#include "opus/celt/entenc.h"
#include "opus/celt/float_cast.h"
#include "opus/celt/kiss_fft.h"
#include "opus/celt/laplace.h"
#include "opus/celt/mathops.h"
#include "opus/celt/mdct.h"
#include "opus/celt/mfrngcod.h"
#include "opus/celt/modes.h"
#include "opus/celt/os_support.h"
#include "opus/celt/pitch.h"
#include "opus/celt/quant_bands.h"
#include "opus/celt/rate.h"
#include "opus/celt/stack_alloc.h"
#include "opus/celt/vq.h"
#include "opus/celt/_kiss_fft_guts.h"

#include "opus/celt/bands.c"
#include "opus/celt/celt.c"
#include "opus/celt/celt_lpc.c"
#include "opus/celt/cwrs.c"
#include "opus/celt/entcode.c"
#include "opus/celt/entdec.c"
#include "opus/celt/entenc.c"
#include "opus/celt/kiss_fft.c"
#include "opus/celt/laplace.c"
#include "opus/celt/mathops.c"
#include "opus/celt/mdct.c"
#include "opus/celt/modes.c"
#include "opus/celt/pitch.c"
#include "opus/celt/quant_bands.c"
#include "opus/celt/rate.c"
#include "opus/celt/vq.c"

//#define opus_custom_encoder_get_size opus_custom_encoder_get_size_alt
#include "opus/celt/celt_decoder.c"
#include "opus/celt/celt_encoder.c"

/////////////////
// SILK Common //
/////////////////

#include "opus_types.h"

#include "opus/silk/control.h"
#include "opus/silk/debug.h"
#include "opus/silk/define.h"
#include "opus/silk/errors.h"
#include "opus/silk/MacroCount.h"
#include "opus/silk/MacroDebug.h"
#include "opus/silk/macros.h"
#include "opus/silk/main.h"
#include "opus/silk/pitch_est_defines.h"
#include "opus/silk/PLC.h"
#include "opus/silk/resampler_private.h"
#include "opus/silk/resampler_rom.h"
#include "opus/silk/resampler_structs.h"
#include "opus/silk/API.h"
#include "opus/silk/SigProc_FIX.h"
#include "opus/silk/structs.h"
#include "opus/silk/tables.h"
#include "opus/silk/tuning_parameters.h"
#include "opus/silk/typedef.h"

#include "opus/silk/A2NLSF.c"
#include "opus/silk/ana_filt_bank_1.c"
#include "opus/silk/biquad_alt.c"
#include "opus/silk/bwexpander.c"
#include "opus/silk/bwexpander_32.c"
#include "opus/silk/check_control_input.c"
#include "opus/silk/CNG.c"
#include "opus/silk/code_signs.c"
#include "opus/silk/control_audio_bandwidth.c"
#include "opus/silk/control_codec.c"
#include "opus/silk/control_SNR.c"
#include "opus/silk/debug.c"
#include "opus/silk/decoder_set_fs.c"
#include "opus/silk/decode_core.c"
#include "opus/silk/decode_frame.c"
#include "opus/silk/decode_indices.c"
#include "opus/silk/decode_parameters.c"
#include "opus/silk/decode_pitch.c"
#include "opus/silk/decode_pulses.c"
#include "opus/silk/dec_API.c"
#include "opus/silk/encode_indices.c"
#include "opus/silk/encode_pulses.c"
#include "opus/silk/enc_API.c"
#include "opus/silk/gain_quant.c"
#include "opus/silk/HP_variable_cutoff.c"
#include "opus/silk/init_decoder.c"
#include "opus/silk/init_encoder.c"
#include "opus/silk/inner_prod_aligned.c"
#include "opus/silk/interpolate.c"
#include "opus/silk/lin2log.c"
#include "opus/silk/log2lin.c"
#include "opus/silk/LPC_analysis_filter.c"
#include "opus/silk/LPC_inv_pred_gain.c"
#include "opus/silk/LP_variable_cutoff.c"
#include "opus/silk/NLSF2A.c"
#include "opus/silk/NLSF_decode.c"
#include "opus/silk/NLSF_del_dec_quant.c"
#include "opus/silk/NLSF_encode.c"
#include "opus/silk/NLSF_stabilize.c"
#include "opus/silk/NLSF_unpack.c"
#include "opus/silk/NLSF_VQ.c"
#include "opus/silk/NLSF_VQ_weights_laroia.c"
#include "opus/silk/NSQ.c"
#include "opus/silk/NSQ_del_dec.c"
#include "opus/silk/pitch_est_tables.c"
#include "opus/silk/PLC.c"
#include "opus/silk/process_NLSFs.c"
#include "opus/silk/quant_LTP_gains.c"
#include "opus/silk/resampler.c"
#include "opus/silk/resampler_down2.c"
#include "opus/silk/resampler_down2_3.c"
#include "opus/silk/resampler_private_AR2.c"
#include "opus/silk/resampler_private_down_FIR.c"
#include "opus/silk/resampler_private_IIR_FIR.c"
#include "opus/silk/resampler_private_up2_HQ.c"
#include "opus/silk/resampler_rom.c"
#include "opus/silk/shell_coder.c"
#include "opus/silk/sigm_Q15.c"
#include "opus/silk/sort.c"
#include "opus/silk/stereo_decode_pred.c"
#include "opus/silk/stereo_encode_pred.c"
#include "opus/silk/stereo_find_predictor.c"
#include "opus/silk/stereo_LR_to_MS.c"
#include "opus/silk/stereo_MS_to_LR.c"
#include "opus/silk/stereo_quant_pred.c"
#include "opus/silk/sum_sqr_shift.c"
#include "opus/silk/tables_gain.c"
#include "opus/silk/tables_LTP.c"
#include "opus/silk/tables_NLSF_CB_NB_MB.c"
#include "opus/silk/tables_NLSF_CB_WB.c"
#include "opus/silk/tables_other.c"
#include "opus/silk/tables_pitch_lag.c"
#include "opus/silk/tables_pulses_per_block.c"
#include "opus/silk/table_LSF_cos.c"
#include "opus/silk/VAD.c"
#include "opus/silk/VQ_WMat_EC.c"

////////////////
// SILK Float //
////////////////

#include "opus/silk/float/main_FLP.h"
#include "opus/silk/float/SigProc_FLP.h"
#include "opus/silk/float/structs_FLP.h"

#include "opus/silk/float/apply_sine_window_FLP.c"
#include "opus/silk/float/autocorrelation_FLP.c"
#include "opus/silk/float/burg_modified_FLP.c"
#include "opus/silk/float/bwexpander_FLP.c"
#include "opus/silk/float/corrMatrix_FLP.c"
#include "opus/silk/float/encode_frame_FLP.c"
#include "opus/silk/float/energy_FLP.c"
#include "opus/silk/float/find_LPC_FLP.c"
#include "opus/silk/float/find_LTP_FLP.c"
#include "opus/silk/float/find_pitch_lags_FLP.c"
#include "opus/silk/float/find_pred_coefs_FLP.c"
#include "opus/silk/float/inner_product_FLP.c"
#include "opus/silk/float/k2a_FLP.c"
#include "opus/silk/float/levinsondurbin_FLP.c"
#include "opus/silk/float/LPC_analysis_filter_FLP.c"
#include "opus/silk/float/LPC_inv_pred_gain_FLP.c"
#include "opus/silk/float/LTP_analysis_filter_FLP.c"
#include "opus/silk/float/LTP_scale_ctrl_FLP.c"
#include "opus/silk/float/noise_shape_analysis_FLP.c"
#include "opus/silk/float/pitch_analysis_core_FLP.c"
#include "opus/silk/float/prefilter_FLP.c"
#include "opus/silk/float/process_gains_FLP.c"
#include "opus/silk/float/regularize_correlations_FLP.c"
#include "opus/silk/float/residual_energy_FLP.c"
#include "opus/silk/float/scale_copy_vector_FLP.c"
#include "opus/silk/float/scale_vector_FLP.c"
#include "opus/silk/float/schur_FLP.c"
#include "opus/silk/float/solve_LS_FLP.c"
#include "opus/silk/float/sort_FLP.c"
#include "opus/silk/float/warped_autocorrelation_FLP.c"
#include "opus/silk/float/wrappers_FLP.c"

/////////////
// LibOpus //
/////////////

#include "opus/libopus/src/opus.c"
#include "opus/libopus/src/opus_decoder.c"
#include "opus/libopus/src/opus_encoder.c"
#include "opus/libopus/src/opus_multistream.c"
#include "opus/libopus/src/opus_multistream_decoder.c"
#include "opus/libopus/src/opus_multistream_encoder.c"
#include "opus/libopus/src/repacketizer.c"

#include "opus/libopus/src/analysis.c"
#include "opus/libopus/src/mlp.c"
#include "opus/libopus/src/mlp_data.c"

//////////////
// Opusfile //
//////////////

#include "opus/opusfile/src/http.c"
#include "opus/opusfile/src/info.c"
#include "opus/opusfile/src/internal.c"
#include "opus/opusfile/src/opusfile.c"
#include "opus/opusfile/src/stream.c"
#include "opus/opusfile/src/wincerts.c"

#ifdef __clang__
    #pragma clang diagnostic pop
#endif

#if (_MSC_VER)
    #pragma warning (pop)
#endif
