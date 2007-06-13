/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2000,2001,2002,2003,2004,2005,2006  Josh Coalson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "juce_FlacHeader.h"
#if JUCE_USE_FLAC


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <math.h>
#include "../assert.h"
#include "../format.h"
#include "include/private/bitmath.h"
#include "include/private/lpc.h"
#if defined DEBUG || defined FLAC__OVERFLOW_DETECT || defined FLAC__OVERFLOW_DETECT_VERBOSE
#include <stdio.h>
#endif

#ifndef FLAC__INTEGER_ONLY_LIBRARY

#ifndef M_LN2
/* math.h in VC++ doesn't seem to have this (how Microsoft is that?) */
#define M_LN2 0.69314718055994530942
#endif

void FLAC__lpc_window_data(const FLAC__real in[], const FLAC__real window[], FLAC__real out[], unsigned data_len)
{
	unsigned i;
	for(i = 0; i < data_len; i++)
		out[i] = in[i] * window[i];
}

void FLAC__lpc_compute_autocorrelation(const FLAC__real data[], unsigned data_len, unsigned lag, FLAC__real autoc[])
{
	/* a readable, but slower, version */
#if 0
	FLAC__real d;
	unsigned i;

	FLAC__ASSERT(lag > 0);
	FLAC__ASSERT(lag <= data_len);

	/*
	 * Technically we should subtract the mean first like so:
	 *   for(i = 0; i < data_len; i++)
	 *     data[i] -= mean;
	 * but it appears not to make enough of a difference to matter, and
	 * most signals are already closely centered around zero
	 */
	while(lag--) {
		for(i = lag, d = 0.0; i < data_len; i++)
			d += data[i] * data[i - lag];
		autoc[lag] = d;
	}
#endif

	/*
	 * this version tends to run faster because of better data locality
	 * ('data_len' is usually much larger than 'lag')
	 */
	FLAC__real d;
	unsigned sample, coeff;
	const unsigned limit = data_len - lag;

	FLAC__ASSERT(lag > 0);
	FLAC__ASSERT(lag <= data_len);

	for(coeff = 0; coeff < lag; coeff++)
		autoc[coeff] = 0.0;
	for(sample = 0; sample <= limit; sample++) {
		d = data[sample];
		for(coeff = 0; coeff < lag; coeff++)
			autoc[coeff] += d * data[sample+coeff];
	}
	for(; sample < data_len; sample++) {
		d = data[sample];
		for(coeff = 0; coeff < data_len - sample; coeff++)
			autoc[coeff] += d * data[sample+coeff];
	}
}

void FLAC__lpc_compute_lp_coefficients(const FLAC__real autoc[], unsigned *max_order, FLAC__real lp_coeff[][FLAC__MAX_LPC_ORDER], FLAC__double error[])
{
	unsigned i, j;
	FLAC__double r, err, ref[FLAC__MAX_LPC_ORDER], lpc[FLAC__MAX_LPC_ORDER];

	FLAC__ASSERT(0 != max_order);
	FLAC__ASSERT(0 < *max_order);
	FLAC__ASSERT(*max_order <= FLAC__MAX_LPC_ORDER);
	FLAC__ASSERT(autoc[0] != 0.0);

	err = autoc[0];

	for(i = 0; i < *max_order; i++) {
		/* Sum up this iteration's reflection coefficient. */
		r = -autoc[i+1];
		for(j = 0; j < i; j++)
			r -= lpc[j] * autoc[i-j];
		ref[i] = (r/=err);

		/* Update LPC coefficients and total error. */
		lpc[i]=r;
		for(j = 0; j < (i>>1); j++) {
			FLAC__double tmp = lpc[j];
			lpc[j] += r * lpc[i-1-j];
			lpc[i-1-j] += r * tmp;
		}
		if(i & 1)
			lpc[j] += lpc[j] * r;

		err *= (1.0 - r * r);

		/* save this order */
		for(j = 0; j <= i; j++)
			lp_coeff[i][j] = (FLAC__real)(-lpc[j]); /* negate FIR filter coeff to get predictor coeff */
		error[i] = err;

		/*@@@@@@ see SF bug #1601812 http://sourceforge.net/tracker/index.php?func=detail&aid=1601812&group_id=13478&atid=113478 */
		if(err == 0.0) {
			*max_order = i+1;
			return;
		}
	}
}

int FLAC__lpc_quantize_coefficients(const FLAC__real lp_coeff[], unsigned order, unsigned precision, FLAC__int32 qlp_coeff[], int *shift)
{
	unsigned i;
	FLAC__double d, cmax = -1e32;
	FLAC__int32 qmax, qmin;
	const int max_shiftlimit = (1 << (FLAC__SUBFRAME_LPC_QLP_SHIFT_LEN-1)) - 1;
	const int min_shiftlimit = -max_shiftlimit - 1;

	FLAC__ASSERT(precision > 0);
	FLAC__ASSERT(precision >= FLAC__MIN_QLP_COEFF_PRECISION);

	/* drop one bit for the sign; from here on out we consider only |lp_coeff[i]| */
	precision--;
	qmax = 1 << precision;
	qmin = -qmax;
	qmax--;

	for(i = 0; i < order; i++) {
		if(lp_coeff[i] == 0.0)
			continue;
		d = fabs(lp_coeff[i]);
		if(d > cmax)
			cmax = d;
	}
redo_it:
	if(cmax <= 0.0) {
		/* => coefficients are all 0, which means our constant-detect didn't work */
		return 2;
	}
	else {
		int log2cmax;

		(void)frexp(cmax, &log2cmax);
		log2cmax--;
		*shift = (int)precision - log2cmax - 1;

		if(*shift < min_shiftlimit || *shift > max_shiftlimit) {
#if 0
			/*@@@ this does not seem to help at all, but was not extensively tested either: */
			if(*shift > max_shiftlimit)
				*shift = max_shiftlimit;
			else
#endif
				return 1;
		}
	}

	if(*shift >= 0) {
		for(i = 0; i < order; i++) {
			qlp_coeff[i] = (FLAC__int32)floor((FLAC__double)lp_coeff[i] * (FLAC__double)(1 << *shift));

			/* double-check the result */
			if(qlp_coeff[i] > qmax || qlp_coeff[i] < qmin) {
#ifdef FLAC__OVERFLOW_DETECT
				fprintf(stderr,"FLAC__lpc_quantize_coefficients: compensating for overflow, qlp_coeff[%u]=%d, lp_coeff[%u]=%f, cmax=%f, precision=%u, shift=%d, q=%f, f(q)=%f\n", i, qlp_coeff[i], i, lp_coeff[i], cmax, precision, *shift, (FLAC__double)lp_coeff[i] * (FLAC__double)(1 << *shift), floor((FLAC__double)lp_coeff[i] * (FLAC__double)(1 << *shift)));
#endif
				cmax *= 2.0;
				goto redo_it;
			}
		}
	}
	else { /* (*shift < 0) */
		const int nshift = -(*shift);
#ifdef DEBUG
		fprintf(stderr,"FLAC__lpc_quantize_coefficients: negative shift = %d\n", *shift);
#endif
		for(i = 0; i < order; i++) {
			qlp_coeff[i] = (FLAC__int32)floor((FLAC__double)lp_coeff[i] / (FLAC__double)(1 << nshift));

			/* double-check the result */
			if(qlp_coeff[i] > qmax || qlp_coeff[i] < qmin) {
#ifdef FLAC__OVERFLOW_DETECT
				fprintf(stderr,"FLAC__lpc_quantize_coefficients: compensating for overflow, qlp_coeff[%u]=%d, lp_coeff[%u]=%f, cmax=%f, precision=%u, shift=%d, q=%f, f(q)=%f\n", i, qlp_coeff[i], i, lp_coeff[i], cmax, precision, *shift, (FLAC__double)lp_coeff[i] / (FLAC__double)(1 << nshift), floor((FLAC__double)lp_coeff[i] / (FLAC__double)(1 << nshift)));
#endif
				cmax *= 2.0;
				goto redo_it;
			}
		}
	}

	return 0;
}

void FLAC__lpc_compute_residual_from_qlp_coefficients(const FLAC__int32 *data, unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 residual[])
{
#ifdef FLAC__OVERFLOW_DETECT
	FLAC__int64 sumo;
#endif
	unsigned i, j;
	FLAC__int32 sum;
	const FLAC__int32 *history;

#ifdef FLAC__OVERFLOW_DETECT_VERBOSE
	fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients: data_len=%d, order=%u, lpq=%d",data_len,order,lp_quantization);
	for(i=0;i<order;i++)
		fprintf(stderr,", q[%u]=%d",i,qlp_coeff[i]);
	fprintf(stderr,"\n");
#endif
	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
#ifdef FLAC__OVERFLOW_DETECT
		sumo = 0;
#endif
		sum = 0;
		history = data;
		for(j = 0; j < order; j++) {
			sum += qlp_coeff[j] * (*(--history));
#ifdef FLAC__OVERFLOW_DETECT
			sumo += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*history);
#if defined _MSC_VER
			if(sumo > 2147483647I64 || sumo < -2147483648I64)
				fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients: OVERFLOW, i=%u, j=%u, c=%d, d=%d, sumo=%I64d\n",i,j,qlp_coeff[j],*history,sumo);
#else
			if(sumo > 2147483647ll || sumo < -2147483648ll)
				fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients: OVERFLOW, i=%u, j=%u, c=%d, d=%d, sumo=%lld\n",i,j,qlp_coeff[j],*history,(long long)sumo);
#endif
#endif
		}
		*(residual++) = *(data++) - (sum >> lp_quantization);
	}

	/* Here's a slower but clearer version:
	for(i = 0; i < data_len; i++) {
		sum = 0;
		for(j = 0; j < order; j++)
			sum += qlp_coeff[j] * data[i-j-1];
		residual[i] = data[i] - (sum >> lp_quantization);
	}
	*/
}

void FLAC__lpc_compute_residual_from_qlp_coefficients_wide(const FLAC__int32 *data, unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 residual[])
{
	unsigned i, j;
	FLAC__int64 sum;
	const FLAC__int32 *history;

#ifdef FLAC__OVERFLOW_DETECT_VERBOSE
	fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients_wide: data_len=%d, order=%u, lpq=%d",data_len,order,lp_quantization);
	for(i=0;i<order;i++)
		fprintf(stderr,", q[%u]=%d",i,qlp_coeff[i]);
	fprintf(stderr,"\n");
#endif
	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
		sum = 0;
		history = data;
		for(j = 0; j < order; j++)
			sum += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*(--history));
#ifdef FLAC__OVERFLOW_DETECT
		if(FLAC__bitmath_silog2_wide(sum >> lp_quantization) > 32) {
			fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients_wide: OVERFLOW, i=%u, sum=%lld\n", i, (long long)(sum >> lp_quantization));
			break;
		}
		if(FLAC__bitmath_silog2_wide((FLAC__int64)(*data) - (sum >> lp_quantization)) > 32) {
			fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients_wide: OVERFLOW, i=%u, data=%d, sum=%lld, residual=%lld\n", i, *data, (long long)(sum >> lp_quantization), (long long)((FLAC__int64)(*data) - (sum >> lp_quantization)));
			break;
		}
#endif
		*(residual++) = *(data++) - (FLAC__int32)(sum >> lp_quantization);
	}
}

#endif /* !defined FLAC__INTEGER_ONLY_LIBRARY */

void FLAC__lpc_restore_signal(const FLAC__int32 residual[], unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 data[])
{
#ifdef FLAC__OVERFLOW_DETECT
	FLAC__int64 sumo;
#endif
	unsigned i, j;
	FLAC__int32 sum;
	const FLAC__int32 *r = residual, *history;

#ifdef FLAC__OVERFLOW_DETECT_VERBOSE
	fprintf(stderr,"FLAC__lpc_restore_signal: data_len=%d, order=%u, lpq=%d",data_len,order,lp_quantization);
	for(i=0;i<order;i++)
		fprintf(stderr,", q[%u]=%d",i,qlp_coeff[i]);
	fprintf(stderr,"\n");
#endif
	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
#ifdef FLAC__OVERFLOW_DETECT
		sumo = 0;
#endif
		sum = 0;
		history = data;
		for(j = 0; j < order; j++) {
			sum += qlp_coeff[j] * (*(--history));
#ifdef FLAC__OVERFLOW_DETECT
			sumo += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*history);
#if defined _MSC_VER
			if(sumo > 2147483647I64 || sumo < -2147483648I64)
				fprintf(stderr,"FLAC__lpc_restore_signal: OVERFLOW, i=%u, j=%u, c=%d, d=%d, sumo=%I64d\n",i,j,qlp_coeff[j],*history,sumo);
#else
			if(sumo > 2147483647ll || sumo < -2147483648ll)
				fprintf(stderr,"FLAC__lpc_restore_signal: OVERFLOW, i=%u, j=%u, c=%d, d=%d, sumo=%lld\n",i,j,qlp_coeff[j],*history,(long long)sumo);
#endif
#endif
		}
		*(data++) = *(r++) + (sum >> lp_quantization);
	}

	/* Here's a slower but clearer version:
	for(i = 0; i < data_len; i++) {
		sum = 0;
		for(j = 0; j < order; j++)
			sum += qlp_coeff[j] * data[i-j-1];
		data[i] = residual[i] + (sum >> lp_quantization);
	}
	*/
}

void FLAC__lpc_restore_signal_wide(const FLAC__int32 residual[], unsigned data_len, const FLAC__int32 qlp_coeff[], unsigned order, int lp_quantization, FLAC__int32 data[])
{
	unsigned i, j;
	FLAC__int64 sum;
	const FLAC__int32 *r = residual, *history;

#ifdef FLAC__OVERFLOW_DETECT_VERBOSE
	fprintf(stderr,"FLAC__lpc_restore_signal_wide: data_len=%d, order=%u, lpq=%d",data_len,order,lp_quantization);
	for(i=0;i<order;i++)
		fprintf(stderr,", q[%u]=%d",i,qlp_coeff[i]);
	fprintf(stderr,"\n");
#endif
	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
		sum = 0;
		history = data;
		for(j = 0; j < order; j++)
			sum += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*(--history));
#ifdef FLAC__OVERFLOW_DETECT
		if(FLAC__bitmath_silog2_wide(sum >> lp_quantization) > 32) {
			fprintf(stderr,"FLAC__lpc_restore_signal_wide: OVERFLOW, i=%u, sum=%lld\n", i, (long long)(sum >> lp_quantization));
			break;
		}
		if(FLAC__bitmath_silog2_wide((FLAC__int64)(*r) + (sum >> lp_quantization)) > 32) {
			fprintf(stderr,"FLAC__lpc_restore_signal_wide: OVERFLOW, i=%u, residual=%d, sum=%lld, data=%lld\n", i, *r, (long long)(sum >> lp_quantization), (long long)((FLAC__int64)(*r) + (sum >> lp_quantization)));
			break;
		}
#endif
		*(data++) = *(r++) + (FLAC__int32)(sum >> lp_quantization);
	}
}

#ifndef FLAC__INTEGER_ONLY_LIBRARY

FLAC__double FLAC__lpc_compute_expected_bits_per_residual_sample(FLAC__double lpc_error, unsigned total_samples)
{
	FLAC__double error_scale;

	FLAC__ASSERT(total_samples > 0);

	error_scale = 0.5 * M_LN2 * M_LN2 / (FLAC__double)total_samples;

	return FLAC__lpc_compute_expected_bits_per_residual_sample_with_error_scale(lpc_error, error_scale);
}

FLAC__double FLAC__lpc_compute_expected_bits_per_residual_sample_with_error_scale(FLAC__double lpc_error, FLAC__double error_scale)
{
	if(lpc_error > 0.0) {
		FLAC__double bps = (FLAC__double)0.5 * log(error_scale * lpc_error) / M_LN2;
		if(bps >= 0.0)
			return bps;
		else
			return 0.0;
	}
	else if(lpc_error < 0.0) { /* error should not be negative but can happen due to inadequate floating-point resolution */
		return 1e32;
	}
	else {
		return 0.0;
	}
}

unsigned FLAC__lpc_compute_best_order(const FLAC__double lpc_error[], unsigned max_order, unsigned total_samples, unsigned overhead_bits_per_order)
{
	unsigned order, index, best_index; /* 'index' the index into lpc_error; index==order-1 since lpc_error[0] is for order==1, lpc_error[1] is for order==2, etc */
	FLAC__double bits, best_bits, error_scale;

	FLAC__ASSERT(max_order > 0);
	FLAC__ASSERT(total_samples > 0);

	error_scale = 0.5 * M_LN2 * M_LN2 / (FLAC__double)total_samples;

	best_index = 0;
	best_bits = (unsigned)(-1);

	for(index = 0, order = 1; index < max_order; index++, order++) {
		bits = FLAC__lpc_compute_expected_bits_per_residual_sample_with_error_scale(lpc_error[index], error_scale) * (FLAC__double)(total_samples - order) + (FLAC__double)(order * overhead_bits_per_order);
		if(bits < best_bits) {
			best_index = index;
			best_bits = bits;
		}
	}

	return best_index+1; /* +1 since index of lpc_error[] is order-1 */
}

#endif /* !defined FLAC__INTEGER_ONLY_LIBRARY */


#endif
