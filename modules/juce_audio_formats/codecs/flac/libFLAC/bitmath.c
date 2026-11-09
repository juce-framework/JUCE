/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2001-2009  Josh Coalson
 * Copyright (C) 2011-2025  Xiph.Org Foundation
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "include/private/bitmath.h"

/* An example of what FLAC__bitmath_silog2() computes:
 *
 * silog2(-10) = 5
 * silog2(- 9) = 5
 * silog2(- 8) = 4
 * silog2(- 7) = 4
 * silog2(- 6) = 4
 * silog2(- 5) = 4
 * silog2(- 4) = 3
 * silog2(- 3) = 3
 * silog2(- 2) = 2
 * silog2(- 1) = 2
 * silog2(  0) = 0
 * silog2(  1) = 2
 * silog2(  2) = 3
 * silog2(  3) = 3
 * silog2(  4) = 4
 * silog2(  5) = 4
 * silog2(  6) = 4
 * silog2(  7) = 4
 * silog2(  8) = 5
 * silog2(  9) = 5
 * silog2( 10) = 5
 */
uint32_t FLAC__bitmath_silog2(FLAC__int64 v)
{
	if(v == 0)
		return 0;

	if(v == -1)
		return 2;

	v = (v < 0) ? (-(v+1)) : v;
	return FLAC__bitmath_ilog2_wide(v)+2;
}

/* An example of what FLAC__bitmath_extra_mulbits_unsigned() computes:
 *
 * extra_mulbits_unsigned( 0) = 0
 * extra_mulbits_unsigned( 1) = 0
 * extra_mulbits_unsigned( 2) = 1
 * extra_mulbits_unsigned( 3) = 2
 * extra_mulbits_unsigned( 4) = 2
 * extra_mulbits_unsigned( 5) = 3
 * extra_mulbits_unsigned( 6) = 3
 * extra_mulbits_unsigned( 7) = 3
 * extra_mulbits_unsigned( 8) = 3
 * extra_mulbits_unsigned( 9) = 4
 * extra_mulbits_unsigned(10) = 4
 * extra_mulbits_unsigned(11) = 4
 * extra_mulbits_unsigned(12) = 4
 * extra_mulbits_unsigned(13) = 4
 * extra_mulbits_unsigned(14) = 4
 * extra_mulbits_unsigned(15) = 4
 * extra_mulbits_unsigned(16) = 4
 * extra_mulbits_unsigned(17) = 5
 * extra_mulbits_unsigned(18) = 5
 *
 * The intent of this is to calculate how many extra bits multiplication
 * by a certain number requires. So, if a signal fits in a certain number
 * of bits (for example 16) than multiplying by a number (for example 1024)
 * grows that storage requirement (to 26 in this example). In effect this is
 * is the log2 rounded up.
 */
uint32_t FLAC__bitmath_extra_mulbits_unsigned(FLAC__uint32 v)
{
	uint32_t ilog2;
	if(v == 0)
		return 0;
	ilog2 = FLAC__bitmath_ilog2(v);
	if(((v >> ilog2) << ilog2) == v)
		/* v is power of 2 */
		return ilog2;
	else
		/* v is not a power of 2, return one higher */
		return ilog2 + 1;
}
