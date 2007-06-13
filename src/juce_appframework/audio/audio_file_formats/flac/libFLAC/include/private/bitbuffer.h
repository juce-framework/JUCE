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

#ifndef FLAC__PRIVATE__BITBUFFER_H
#define FLAC__PRIVATE__BITBUFFER_H

#include <stdio.h> /* for FILE */
#include "../../../ordinals.h"

/* @@@ This should be configurable. Valid values are currently 8 and 32. */
/* @@@ WATCHOUT!  do not use 32 with a little endian system yet. */
#define FLAC__BITS_PER_BLURB 8

#if FLAC__BITS_PER_BLURB == 8
typedef FLAC__byte FLAC__blurb;
#elif FLAC__BITS_PER_BLURB == 32
typedef FLAC__uint32 FLAC__blurb;
#else
/* ERROR, only sizes of 8 and 32 are supported */
#endif

/*
 * opaque structure definition
 */
struct FLAC__BitBuffer;
typedef struct FLAC__BitBuffer FLAC__BitBuffer;

/*
 * construction, deletion, initialization, cloning functions
 */
FLAC__BitBuffer *FLAC__bitbuffer_new();
void FLAC__bitbuffer_delete(FLAC__BitBuffer *bb);
FLAC__bool FLAC__bitbuffer_init(FLAC__BitBuffer *bb);
FLAC__bool FLAC__bitbuffer_init_from(FLAC__BitBuffer *bb, const FLAC__byte buffer[], unsigned bytes);
FLAC__bool FLAC__bitbuffer_concatenate_aligned(FLAC__BitBuffer *dest, const FLAC__BitBuffer *src);
void FLAC__bitbuffer_free(FLAC__BitBuffer *bb); /* does not 'free(buffer)' */
FLAC__bool FLAC__bitbuffer_clear(FLAC__BitBuffer *bb);
FLAC__bool FLAC__bitbuffer_clone(FLAC__BitBuffer *dest, const FLAC__BitBuffer *src);

/*
 * CRC functions
 */
void FLAC__bitbuffer_reset_read_crc16(FLAC__BitBuffer *bb, FLAC__uint16 seed);
FLAC__uint16 FLAC__bitbuffer_get_read_crc16(FLAC__BitBuffer *bb);
FLAC__uint16 FLAC__bitbuffer_get_write_crc16(const FLAC__BitBuffer *bb);
FLAC__byte FLAC__bitbuffer_get_write_crc8(const FLAC__BitBuffer *bb);

/*
 * info functions
 */
FLAC__bool FLAC__bitbuffer_is_byte_aligned(const FLAC__BitBuffer *bb);
FLAC__bool FLAC__bitbuffer_is_consumed_byte_aligned(const FLAC__BitBuffer *bb);
unsigned FLAC__bitbuffer_bits_left_for_byte_alignment(const FLAC__BitBuffer *bb);
unsigned FLAC__bitbuffer_get_input_bytes_unconsumed(const FLAC__BitBuffer *bb); /* do not call unless byte-aligned */

/*
 * direct buffer access
 */
void FLAC__bitbuffer_get_buffer(FLAC__BitBuffer *bb, const FLAC__byte **buffer, size_t *bytes);
void FLAC__bitbuffer_release_buffer(FLAC__BitBuffer *bb);

/*
 * write functions
 */
FLAC__bool FLAC__bitbuffer_write_zeroes(FLAC__BitBuffer *bb, unsigned bits);
FLAC__bool FLAC__bitbuffer_write_raw_uint32(FLAC__BitBuffer *bb, FLAC__uint32 val, unsigned bits);
FLAC__bool FLAC__bitbuffer_write_raw_int32(FLAC__BitBuffer *bb, FLAC__int32 val, unsigned bits);
FLAC__bool FLAC__bitbuffer_write_raw_uint64(FLAC__BitBuffer *bb, FLAC__uint64 val, unsigned bits);
#if 0 /* UNUSED */
FLAC__bool FLAC__bitbuffer_write_raw_int64(FLAC__BitBuffer *bb, FLAC__int64 val, unsigned bits);
#endif
FLAC__bool FLAC__bitbuffer_write_raw_uint32_little_endian(FLAC__BitBuffer *bb, FLAC__uint32 val); /*only for bits=32*/
FLAC__bool FLAC__bitbuffer_write_byte_block(FLAC__BitBuffer *bb, const FLAC__byte vals[], unsigned nvals);
FLAC__bool FLAC__bitbuffer_write_unary_unsigned(FLAC__BitBuffer *bb, unsigned val);
unsigned FLAC__bitbuffer_rice_bits(int val, unsigned parameter);
#if 0 /* UNUSED */
unsigned FLAC__bitbuffer_golomb_bits_signed(int val, unsigned parameter);
unsigned FLAC__bitbuffer_golomb_bits_unsigned(unsigned val, unsigned parameter);
#endif
FLAC__bool FLAC__bitbuffer_write_rice_signed(FLAC__BitBuffer *bb, int val, unsigned parameter);
#if 0 /* UNUSED */
FLAC__bool FLAC__bitbuffer_write_rice_signed_guarded(FLAC__BitBuffer *bb, int val, unsigned parameter, unsigned max_bits, FLAC__bool *overflow);
#endif
#if 0 /* UNUSED */
FLAC__bool FLAC__bitbuffer_write_golomb_signed(FLAC__BitBuffer *bb, int val, unsigned parameter);
FLAC__bool FLAC__bitbuffer_write_golomb_signed_guarded(FLAC__BitBuffer *bb, int val, unsigned parameter, unsigned max_bits, FLAC__bool *overflow);
FLAC__bool FLAC__bitbuffer_write_golomb_unsigned(FLAC__BitBuffer *bb, unsigned val, unsigned parameter);
FLAC__bool FLAC__bitbuffer_write_golomb_unsigned_guarded(FLAC__BitBuffer *bb, unsigned val, unsigned parameter, unsigned max_bits, FLAC__bool *overflow);
#endif
FLAC__bool FLAC__bitbuffer_write_utf8_uint32(FLAC__BitBuffer *bb, FLAC__uint32 val);
FLAC__bool FLAC__bitbuffer_write_utf8_uint64(FLAC__BitBuffer *bb, FLAC__uint64 val);
FLAC__bool FLAC__bitbuffer_zero_pad_to_byte_boundary(FLAC__BitBuffer *bb);

/*
 * read functions
 */
typedef FLAC__bool (*FLAC__BitbufferReadCallback)(FLAC__byte buffer[], size_t *bytes, void *client_data);

FLAC__bool FLAC__bitbuffer_peek_bit(FLAC__BitBuffer *bb, unsigned *val, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_bit(FLAC__BitBuffer *bb, unsigned *val, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_bit_to_uint32(FLAC__BitBuffer *bb, FLAC__uint32 *val, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_bit_to_uint64(FLAC__BitBuffer *bb, FLAC__uint64 *val, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_raw_uint32(FLAC__BitBuffer *bb, FLAC__uint32 *val, const unsigned bits, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_raw_int32(FLAC__BitBuffer *bb, FLAC__int32 *val, const unsigned bits, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_raw_uint64(FLAC__BitBuffer *bb, FLAC__uint64 *val, const unsigned bits, FLAC__BitbufferReadCallback read_callback, void *client_data);
#if 0 /* UNUSED */
FLAC__bool FLAC__bitbuffer_read_raw_int64(FLAC__BitBuffer *bb, FLAC__int64 *val, const unsigned bits, FLAC__BitbufferReadCallback read_callback, void *client_data);
#endif
FLAC__bool FLAC__bitbuffer_read_raw_uint32_little_endian(FLAC__BitBuffer *bb, FLAC__uint32 *val, FLAC__BitbufferReadCallback read_callback, void *client_data); /*only for bits=32*/
FLAC__bool FLAC__bitbuffer_skip_bits_no_crc(FLAC__BitBuffer *bb, unsigned bits, FLAC__BitbufferReadCallback read_callback, void *client_data); /* WATCHOUT: does not CRC the skipped data! */ /*@@@@ add to unit tests */
FLAC__bool FLAC__bitbuffer_read_byte_block_aligned_no_crc(FLAC__BitBuffer *bb, FLAC__byte *val, unsigned nvals, FLAC__BitbufferReadCallback read_callback, void *client_data); /* val may be 0 to skip bytes instead of reading them */ /* WATCHOUT: does not CRC the read data! */
FLAC__bool FLAC__bitbuffer_read_unary_unsigned(FLAC__BitBuffer *bb, unsigned *val, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_rice_signed(FLAC__BitBuffer *bb, int *val, unsigned parameter, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_rice_signed_block(FLAC__BitBuffer *bb, int vals[], unsigned nvals, unsigned parameter, FLAC__BitbufferReadCallback read_callback, void *client_data);
#if 0 /* UNUSED */
FLAC__bool FLAC__bitbuffer_read_golomb_signed(FLAC__BitBuffer *bb, int *val, unsigned parameter, FLAC__BitbufferReadCallback read_callback, void *client_data);
FLAC__bool FLAC__bitbuffer_read_golomb_unsigned(FLAC__BitBuffer *bb, unsigned *val, unsigned parameter, FLAC__BitbufferReadCallback read_callback, void *client_data);
#endif
FLAC__bool FLAC__bitbuffer_read_utf8_uint32(FLAC__BitBuffer *bb, FLAC__uint32 *val, FLAC__BitbufferReadCallback read_callback, void *client_data, FLAC__byte *raw, unsigned *rawlen);
FLAC__bool FLAC__bitbuffer_read_utf8_uint64(FLAC__BitBuffer *bb, FLAC__uint64 *val, FLAC__BitbufferReadCallback read_callback, void *client_data, FLAC__byte *raw, unsigned *rawlen);
void FLAC__bitbuffer_dump(const FLAC__BitBuffer *bb, FILE *out);

#endif
