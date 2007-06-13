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

#include <stdio.h>
#include <string.h> /* for strlen() */
#include "include/private/stream_encoder_framing.h"
#include "include/private/crc.h"
#include "../assert.h"

#ifdef max
#undef max
#endif
#define max(x,y) ((x)>(y)?(x):(y))

static FLAC__bool add_entropy_coding_method_(FLAC__BitBuffer *bb, const FLAC__EntropyCodingMethod *method);
static FLAC__bool add_residual_partitioned_rice_(FLAC__BitBuffer *bb, const FLAC__int32 residual[], const unsigned residual_samples, const unsigned predictor_order, const unsigned rice_parameters[], const unsigned raw_bits[], const unsigned partition_order);

FLAC__bool FLAC__add_metadata_block(const FLAC__StreamMetadata *metadata, FLAC__BitBuffer *bb)
{
	unsigned i, j;
	const unsigned vendor_string_length = (unsigned)strlen(FLAC__VENDOR_STRING);

	if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->is_last, FLAC__STREAM_METADATA_IS_LAST_LEN))
		return false;

	if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->type, FLAC__STREAM_METADATA_TYPE_LEN))
		return false;

	/*
	 * First, for VORBIS_COMMENTs, adjust the length to reflect our vendor string
	 */
	i = metadata->length;
	if(metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
		FLAC__ASSERT(metadata->data.vorbis_comment.vendor_string.length == 0 || 0 != metadata->data.vorbis_comment.vendor_string.entry);
		i -= metadata->data.vorbis_comment.vendor_string.length;
		i += vendor_string_length;
	}
	FLAC__ASSERT(i < (1u << FLAC__STREAM_METADATA_LENGTH_LEN));
	if(!FLAC__bitbuffer_write_raw_uint32(bb, i, FLAC__STREAM_METADATA_LENGTH_LEN))
		return false;

	switch(metadata->type) {
		case FLAC__METADATA_TYPE_STREAMINFO:
			FLAC__ASSERT(metadata->data.stream_info.min_blocksize < (1u << FLAC__STREAM_METADATA_STREAMINFO_MIN_BLOCK_SIZE_LEN));
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.stream_info.min_blocksize, FLAC__STREAM_METADATA_STREAMINFO_MIN_BLOCK_SIZE_LEN))
				return false;
			FLAC__ASSERT(metadata->data.stream_info.max_blocksize < (1u << FLAC__STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN));
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.stream_info.max_blocksize, FLAC__STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN))
				return false;
			FLAC__ASSERT(metadata->data.stream_info.min_framesize < (1u << FLAC__STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN));
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.stream_info.min_framesize, FLAC__STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN))
				return false;
			FLAC__ASSERT(metadata->data.stream_info.max_framesize < (1u << FLAC__STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN));
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.stream_info.max_framesize, FLAC__STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN))
				return false;
			FLAC__ASSERT(FLAC__format_sample_rate_is_valid(metadata->data.stream_info.sample_rate));
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.stream_info.sample_rate, FLAC__STREAM_METADATA_STREAMINFO_SAMPLE_RATE_LEN))
				return false;
			FLAC__ASSERT(metadata->data.stream_info.channels > 0);
			FLAC__ASSERT(metadata->data.stream_info.channels <= (1u << FLAC__STREAM_METADATA_STREAMINFO_CHANNELS_LEN));
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.stream_info.channels-1, FLAC__STREAM_METADATA_STREAMINFO_CHANNELS_LEN))
				return false;
			FLAC__ASSERT(metadata->data.stream_info.bits_per_sample > 0);
			FLAC__ASSERT(metadata->data.stream_info.bits_per_sample <= (1u << FLAC__STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN));
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.stream_info.bits_per_sample-1, FLAC__STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN))
				return false;
			if(!FLAC__bitbuffer_write_raw_uint64(bb, metadata->data.stream_info.total_samples, FLAC__STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN))
				return false;
			if(!FLAC__bitbuffer_write_byte_block(bb, metadata->data.stream_info.md5sum, 16))
				return false;
			break;
		case FLAC__METADATA_TYPE_PADDING:
			if(!FLAC__bitbuffer_write_zeroes(bb, metadata->length * 8))
				return false;
			break;
		case FLAC__METADATA_TYPE_APPLICATION:
			if(!FLAC__bitbuffer_write_byte_block(bb, metadata->data.application.id, FLAC__STREAM_METADATA_APPLICATION_ID_LEN / 8))
				return false;
			if(!FLAC__bitbuffer_write_byte_block(bb, metadata->data.application.data, metadata->length - (FLAC__STREAM_METADATA_APPLICATION_ID_LEN / 8)))
				return false;
			break;
		case FLAC__METADATA_TYPE_SEEKTABLE:
			for(i = 0; i < metadata->data.seek_table.num_points; i++) {
				if(!FLAC__bitbuffer_write_raw_uint64(bb, metadata->data.seek_table.points[i].sample_number, FLAC__STREAM_METADATA_SEEKPOINT_SAMPLE_NUMBER_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint64(bb, metadata->data.seek_table.points[i].stream_offset, FLAC__STREAM_METADATA_SEEKPOINT_STREAM_OFFSET_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.seek_table.points[i].frame_samples, FLAC__STREAM_METADATA_SEEKPOINT_FRAME_SAMPLES_LEN))
					return false;
			}
			break;
		case FLAC__METADATA_TYPE_VORBIS_COMMENT:
			if(!FLAC__bitbuffer_write_raw_uint32_little_endian(bb, vendor_string_length))
				return false;
			if(!FLAC__bitbuffer_write_byte_block(bb, (const FLAC__byte*)FLAC__VENDOR_STRING, vendor_string_length))
				return false;
			if(!FLAC__bitbuffer_write_raw_uint32_little_endian(bb, metadata->data.vorbis_comment.num_comments))
				return false;
			for(i = 0; i < metadata->data.vorbis_comment.num_comments; i++) {
				if(!FLAC__bitbuffer_write_raw_uint32_little_endian(bb, metadata->data.vorbis_comment.comments[i].length))
					return false;
				if(!FLAC__bitbuffer_write_byte_block(bb, metadata->data.vorbis_comment.comments[i].entry, metadata->data.vorbis_comment.comments[i].length))
					return false;
			}
			break;
		case FLAC__METADATA_TYPE_CUESHEET:
			FLAC__ASSERT(FLAC__STREAM_METADATA_CUESHEET_MEDIA_CATALOG_NUMBER_LEN % 8 == 0);
			if(!FLAC__bitbuffer_write_byte_block(bb, (const FLAC__byte*)metadata->data.cue_sheet.media_catalog_number, FLAC__STREAM_METADATA_CUESHEET_MEDIA_CATALOG_NUMBER_LEN/8))
				return false;
			if(!FLAC__bitbuffer_write_raw_uint64(bb, metadata->data.cue_sheet.lead_in, FLAC__STREAM_METADATA_CUESHEET_LEAD_IN_LEN))
				return false;
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.cue_sheet.is_cd? 1 : 0, FLAC__STREAM_METADATA_CUESHEET_IS_CD_LEN))
				return false;
			if(!FLAC__bitbuffer_write_zeroes(bb, FLAC__STREAM_METADATA_CUESHEET_RESERVED_LEN))
				return false;
			if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.cue_sheet.num_tracks, FLAC__STREAM_METADATA_CUESHEET_NUM_TRACKS_LEN))
				return false;
			for(i = 0; i < metadata->data.cue_sheet.num_tracks; i++) {
				const FLAC__StreamMetadata_CueSheet_Track *track = metadata->data.cue_sheet.tracks + i;

				if(!FLAC__bitbuffer_write_raw_uint64(bb, track->offset, FLAC__STREAM_METADATA_CUESHEET_TRACK_OFFSET_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, track->number, FLAC__STREAM_METADATA_CUESHEET_TRACK_NUMBER_LEN))
					return false;
				FLAC__ASSERT(FLAC__STREAM_METADATA_CUESHEET_TRACK_ISRC_LEN % 8 == 0);
				if(!FLAC__bitbuffer_write_byte_block(bb, (const FLAC__byte*)track->isrc, FLAC__STREAM_METADATA_CUESHEET_TRACK_ISRC_LEN/8))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, track->type, FLAC__STREAM_METADATA_CUESHEET_TRACK_TYPE_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, track->pre_emphasis, FLAC__STREAM_METADATA_CUESHEET_TRACK_PRE_EMPHASIS_LEN))
					return false;
				if(!FLAC__bitbuffer_write_zeroes(bb, FLAC__STREAM_METADATA_CUESHEET_TRACK_RESERVED_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, track->num_indices, FLAC__STREAM_METADATA_CUESHEET_TRACK_NUM_INDICES_LEN))
					return false;
				for(j = 0; j < track->num_indices; j++) {
					const FLAC__StreamMetadata_CueSheet_Index *index = track->indices + j;

					if(!FLAC__bitbuffer_write_raw_uint64(bb, index->offset, FLAC__STREAM_METADATA_CUESHEET_INDEX_OFFSET_LEN))
						return false;
					if(!FLAC__bitbuffer_write_raw_uint32(bb, index->number, FLAC__STREAM_METADATA_CUESHEET_INDEX_NUMBER_LEN))
						return false;
					if(!FLAC__bitbuffer_write_zeroes(bb, FLAC__STREAM_METADATA_CUESHEET_INDEX_RESERVED_LEN))
						return false;
				}
			}
			break;
		case FLAC__METADATA_TYPE_PICTURE:
			{
				size_t len;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.picture.type, FLAC__STREAM_METADATA_PICTURE_TYPE_LEN))
					return false;
				len = strlen(metadata->data.picture.mime_type);
				if(!FLAC__bitbuffer_write_raw_uint32(bb, len, FLAC__STREAM_METADATA_PICTURE_MIME_TYPE_LENGTH_LEN))
					return false;
				if(!FLAC__bitbuffer_write_byte_block(bb, (const FLAC__byte*)metadata->data.picture.mime_type, len))
					return false;
				len = strlen((const char *)metadata->data.picture.description);
				if(!FLAC__bitbuffer_write_raw_uint32(bb, len, FLAC__STREAM_METADATA_PICTURE_DESCRIPTION_LENGTH_LEN))
					return false;
				if(!FLAC__bitbuffer_write_byte_block(bb, metadata->data.picture.description, len))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.picture.width, FLAC__STREAM_METADATA_PICTURE_WIDTH_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.picture.height, FLAC__STREAM_METADATA_PICTURE_HEIGHT_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.picture.depth, FLAC__STREAM_METADATA_PICTURE_DEPTH_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.picture.colors, FLAC__STREAM_METADATA_PICTURE_COLORS_LEN))
					return false;
				if(!FLAC__bitbuffer_write_raw_uint32(bb, metadata->data.picture.data_length, FLAC__STREAM_METADATA_PICTURE_DATA_LENGTH_LEN))
					return false;
				if(!FLAC__bitbuffer_write_byte_block(bb, metadata->data.picture.data, metadata->data.picture.data_length))
					return false;
			}
			break;
		default:
			if(!FLAC__bitbuffer_write_byte_block(bb, metadata->data.unknown.data, metadata->length))
				return false;
			break;
	}

	FLAC__ASSERT(FLAC__bitbuffer_is_byte_aligned(bb));
	return true;
}

FLAC__bool FLAC__frame_add_header(const FLAC__FrameHeader *header, FLAC__BitBuffer *bb)
{
	unsigned u, blocksize_hint, sample_rate_hint;

	FLAC__ASSERT(FLAC__bitbuffer_is_byte_aligned(bb));

	if(!FLAC__bitbuffer_write_raw_uint32(bb, FLAC__FRAME_HEADER_SYNC, FLAC__FRAME_HEADER_SYNC_LEN))
		return false;

	if(!FLAC__bitbuffer_write_raw_uint32(bb, 0, FLAC__FRAME_HEADER_RESERVED_LEN))
		return false;

	FLAC__ASSERT(header->blocksize > 0 && header->blocksize <= FLAC__MAX_BLOCK_SIZE);
	/* when this assertion holds true, any legal blocksize can be expressed in the frame header */
	FLAC__ASSERT(FLAC__MAX_BLOCK_SIZE <= 65535u);
	blocksize_hint = 0;
	switch(header->blocksize) {
		case   192: u = 1; break;
		case   576: u = 2; break;
		case  1152: u = 3; break;
		case  2304: u = 4; break;
		case  4608: u = 5; break;
		case   256: u = 8; break;
		case   512: u = 9; break;
		case  1024: u = 10; break;
		case  2048: u = 11; break;
		case  4096: u = 12; break;
		case  8192: u = 13; break;
		case 16384: u = 14; break;
		case 32768: u = 15; break;
		default:
			if(header->blocksize <= 0x100)
				blocksize_hint = u = 6;
			else if(header->blocksize <= 0x10000)
				blocksize_hint = u = 7;
			else
				u = 0;
			break;
	}
	if(!FLAC__bitbuffer_write_raw_uint32(bb, u, FLAC__FRAME_HEADER_BLOCK_SIZE_LEN))
		return false;

	FLAC__ASSERT(FLAC__format_sample_rate_is_valid(header->sample_rate));
	sample_rate_hint = 0;
	switch(header->sample_rate) {
		case  8000: u = 4; break;
		case 16000: u = 5; break;
		case 22050: u = 6; break;
		case 24000: u = 7; break;
		case 32000: u = 8; break;
		case 44100: u = 9; break;
		case 48000: u = 10; break;
		case 96000: u = 11; break;
		default:
			if(header->sample_rate <= 255000 && header->sample_rate % 1000 == 0)
				sample_rate_hint = u = 12;
			else if(header->sample_rate % 10 == 0)
				sample_rate_hint = u = 14;
			else if(header->sample_rate <= 0xffff)
				sample_rate_hint = u = 13;
			else
				u = 0;
			break;
	}
	if(!FLAC__bitbuffer_write_raw_uint32(bb, u, FLAC__FRAME_HEADER_SAMPLE_RATE_LEN))
		return false;

	FLAC__ASSERT(header->channels > 0 && header->channels <= (1u << FLAC__STREAM_METADATA_STREAMINFO_CHANNELS_LEN) && header->channels <= FLAC__MAX_CHANNELS);
	switch(header->channel_assignment) {
		case FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT:
			u = header->channels - 1;
			break;
		case FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE:
			FLAC__ASSERT(header->channels == 2);
			u = 8;
			break;
		case FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE:
			FLAC__ASSERT(header->channels == 2);
			u = 9;
			break;
		case FLAC__CHANNEL_ASSIGNMENT_MID_SIDE:
			FLAC__ASSERT(header->channels == 2);
			u = 10;
			break;
		default:
			FLAC__ASSERT(0);
	}
	if(!FLAC__bitbuffer_write_raw_uint32(bb, u, FLAC__FRAME_HEADER_CHANNEL_ASSIGNMENT_LEN))
		return false;

	FLAC__ASSERT(header->bits_per_sample > 0 && header->bits_per_sample <= (1u << FLAC__STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN));
	switch(header->bits_per_sample) {
		case 8 : u = 1; break;
		case 12: u = 2; break;
		case 16: u = 4; break;
		case 20: u = 5; break;
		case 24: u = 6; break;
		default: u = 0; break;
	}
	if(!FLAC__bitbuffer_write_raw_uint32(bb, u, FLAC__FRAME_HEADER_BITS_PER_SAMPLE_LEN))
		return false;

	if(!FLAC__bitbuffer_write_raw_uint32(bb, 0, FLAC__FRAME_HEADER_ZERO_PAD_LEN))
		return false;

	FLAC__ASSERT(header->number_type == FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER);
	if(!FLAC__bitbuffer_write_utf8_uint32(bb, header->number.frame_number))
		return false;

	if(blocksize_hint)
		if(!FLAC__bitbuffer_write_raw_uint32(bb, header->blocksize-1, (blocksize_hint==6)? 8:16))
			return false;

	switch(sample_rate_hint) {
		case 12:
			if(!FLAC__bitbuffer_write_raw_uint32(bb, header->sample_rate / 1000, 8))
				return false;
			break;
		case 13:
			if(!FLAC__bitbuffer_write_raw_uint32(bb, header->sample_rate, 16))
				return false;
			break;
		case 14:
			if(!FLAC__bitbuffer_write_raw_uint32(bb, header->sample_rate / 10, 16))
				return false;
			break;
	}

	/* write the CRC */
	if(!FLAC__bitbuffer_write_raw_uint32(bb, FLAC__bitbuffer_get_write_crc8(bb), FLAC__FRAME_HEADER_CRC_LEN))
		return false;

	return true;
}

FLAC__bool FLAC__subframe_add_constant(const FLAC__Subframe_Constant *subframe, unsigned subframe_bps, unsigned wasted_bits, FLAC__BitBuffer *bb)
{
	FLAC__bool ok;

	ok =
		FLAC__bitbuffer_write_raw_uint32(bb, FLAC__SUBFRAME_TYPE_CONSTANT_BYTE_ALIGNED_MASK | (wasted_bits? 1:0), FLAC__SUBFRAME_ZERO_PAD_LEN + FLAC__SUBFRAME_TYPE_LEN + FLAC__SUBFRAME_WASTED_BITS_FLAG_LEN) &&
		(wasted_bits? FLAC__bitbuffer_write_unary_unsigned(bb, wasted_bits-1) : true) &&
		FLAC__bitbuffer_write_raw_int32(bb, subframe->value, subframe_bps)
	;

	return ok;
}

FLAC__bool FLAC__subframe_add_fixed(const FLAC__Subframe_Fixed *subframe, unsigned residual_samples, unsigned subframe_bps, unsigned wasted_bits, FLAC__BitBuffer *bb)
{
	unsigned i;

	if(!FLAC__bitbuffer_write_raw_uint32(bb, FLAC__SUBFRAME_TYPE_FIXED_BYTE_ALIGNED_MASK | (subframe->order<<1) | (wasted_bits? 1:0), FLAC__SUBFRAME_ZERO_PAD_LEN + FLAC__SUBFRAME_TYPE_LEN + FLAC__SUBFRAME_WASTED_BITS_FLAG_LEN))
		return false;
	if(wasted_bits)
		if(!FLAC__bitbuffer_write_unary_unsigned(bb, wasted_bits-1))
			return false;

	for(i = 0; i < subframe->order; i++)
		if(!FLAC__bitbuffer_write_raw_int32(bb, subframe->warmup[i], subframe_bps))
			return false;

	if(!add_entropy_coding_method_(bb, &subframe->entropy_coding_method))
		return false;
	switch(subframe->entropy_coding_method.type) {
		case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
			if(!add_residual_partitioned_rice_(bb, subframe->residual, residual_samples, subframe->order, subframe->entropy_coding_method.data.partitioned_rice.contents->parameters, subframe->entropy_coding_method.data.partitioned_rice.contents->raw_bits, subframe->entropy_coding_method.data.partitioned_rice.order))
				return false;
			break;
		default:
			FLAC__ASSERT(0);
	}

	return true;
}

FLAC__bool FLAC__subframe_add_lpc(const FLAC__Subframe_LPC *subframe, unsigned residual_samples, unsigned subframe_bps, unsigned wasted_bits, FLAC__BitBuffer *bb)
{
	unsigned i;

	if(!FLAC__bitbuffer_write_raw_uint32(bb, FLAC__SUBFRAME_TYPE_LPC_BYTE_ALIGNED_MASK | ((subframe->order-1)<<1) | (wasted_bits? 1:0), FLAC__SUBFRAME_ZERO_PAD_LEN + FLAC__SUBFRAME_TYPE_LEN + FLAC__SUBFRAME_WASTED_BITS_FLAG_LEN))
		return false;
	if(wasted_bits)
		if(!FLAC__bitbuffer_write_unary_unsigned(bb, wasted_bits-1))
			return false;

	for(i = 0; i < subframe->order; i++)
		if(!FLAC__bitbuffer_write_raw_int32(bb, subframe->warmup[i], subframe_bps))
			return false;

	if(!FLAC__bitbuffer_write_raw_uint32(bb, subframe->qlp_coeff_precision-1, FLAC__SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN))
		return false;
	if(!FLAC__bitbuffer_write_raw_int32(bb, subframe->quantization_level, FLAC__SUBFRAME_LPC_QLP_SHIFT_LEN))
		return false;
	for(i = 0; i < subframe->order; i++)
		if(!FLAC__bitbuffer_write_raw_int32(bb, subframe->qlp_coeff[i], subframe->qlp_coeff_precision))
			return false;

	if(!add_entropy_coding_method_(bb, &subframe->entropy_coding_method))
		return false;
	switch(subframe->entropy_coding_method.type) {
		case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
			if(!add_residual_partitioned_rice_(bb, subframe->residual, residual_samples, subframe->order, subframe->entropy_coding_method.data.partitioned_rice.contents->parameters, subframe->entropy_coding_method.data.partitioned_rice.contents->raw_bits, subframe->entropy_coding_method.data.partitioned_rice.order))
				return false;
			break;
		default:
			FLAC__ASSERT(0);
	}

	return true;
}

FLAC__bool FLAC__subframe_add_verbatim(const FLAC__Subframe_Verbatim *subframe, unsigned samples, unsigned subframe_bps, unsigned wasted_bits, FLAC__BitBuffer *bb)
{
	unsigned i;
	const FLAC__int32 *signal = subframe->data;

	if(!FLAC__bitbuffer_write_raw_uint32(bb, FLAC__SUBFRAME_TYPE_VERBATIM_BYTE_ALIGNED_MASK | (wasted_bits? 1:0), FLAC__SUBFRAME_ZERO_PAD_LEN + FLAC__SUBFRAME_TYPE_LEN + FLAC__SUBFRAME_WASTED_BITS_FLAG_LEN))
		return false;
	if(wasted_bits)
		if(!FLAC__bitbuffer_write_unary_unsigned(bb, wasted_bits-1))
			return false;

	for(i = 0; i < samples; i++)
		if(!FLAC__bitbuffer_write_raw_int32(bb, signal[i], subframe_bps))
			return false;

	return true;
}

FLAC__bool add_entropy_coding_method_(FLAC__BitBuffer *bb, const FLAC__EntropyCodingMethod *method)
{
	if(!FLAC__bitbuffer_write_raw_uint32(bb, method->type, FLAC__ENTROPY_CODING_METHOD_TYPE_LEN))
		return false;
	switch(method->type) {
		case FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE:
			if(!FLAC__bitbuffer_write_raw_uint32(bb, method->data.partitioned_rice.order, FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN))
				return false;
			break;
		default:
			FLAC__ASSERT(0);
	}
	return true;
}

FLAC__bool add_residual_partitioned_rice_(FLAC__BitBuffer *bb, const FLAC__int32 residual[], const unsigned residual_samples, const unsigned predictor_order, const unsigned rice_parameters[], const unsigned raw_bits[], const unsigned partition_order)
{
	if(partition_order == 0) {
		unsigned i;

		if(!FLAC__bitbuffer_write_raw_uint32(bb, rice_parameters[0], FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN))
			return false;
		if(rice_parameters[0] < FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER) {
			for(i = 0; i < residual_samples; i++) {
				if(!FLAC__bitbuffer_write_rice_signed(bb, residual[i], rice_parameters[0]))
					return false;
			}
		}
		else {
			if(!FLAC__bitbuffer_write_raw_uint32(bb, raw_bits[0], FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN))
				return false;
			for(i = 0; i < residual_samples; i++) {
				if(!FLAC__bitbuffer_write_raw_int32(bb, residual[i], raw_bits[0]))
					return false;
			}
		}
		return true;
	}
	else {
		unsigned i, j, k = 0, k_last = 0;
		unsigned partition_samples;
		const unsigned default_partition_samples = (residual_samples+predictor_order) >> partition_order;
		for(i = 0; i < (1u<<partition_order); i++) {
			if(!FLAC__bitbuffer_write_raw_uint32(bb, rice_parameters[i], FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN))
				return false;
			partition_samples = default_partition_samples;
			if(i == 0)
				partition_samples -= predictor_order;
			k += partition_samples;
			if(rice_parameters[i] < FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER) {
				for(j = k_last; j < k; j++) {
					if(!FLAC__bitbuffer_write_rice_signed(bb, residual[j], rice_parameters[i]))
						return false;
				}
			}
			else {
				if(!FLAC__bitbuffer_write_raw_uint32(bb, raw_bits[i], FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN))
					return false;
				for(j = k_last; j < k; j++) {
					if(!FLAC__bitbuffer_write_raw_int32(bb, residual[j], raw_bits[i]))
						return false;
				}
			}
			k_last = k;
		}
		return true;
	}
}


#endif
