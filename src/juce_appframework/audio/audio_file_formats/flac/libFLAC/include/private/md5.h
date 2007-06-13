/*
 * This is the header file for the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * Changed so as no longer to depend on Colin Plumb's `usual.h'
 * header definitions; now uses stuff from dpkg's config.h
 *  - Ian Jackson <ijackson@nyx.cs.du.edu>.
 * Still in the public domain.
 *
 * Josh Coalson: made some changes to integrate with libFLAC.
 * Still in the public domain.
 */

#ifndef FLAC__PRIVATE__MD5_H
#define FLAC__PRIVATE__MD5_H

#define md5byte unsigned char

#include "../../../ordinals.h"

struct FLAC__MD5Context {
	FLAC__uint32 buf[4];
	FLAC__uint32 bytes[2];
	FLAC__uint32 in[16];
	FLAC__byte *internal_buf;
	unsigned capacity;
};

void FLAC__MD5Init(struct FLAC__MD5Context *context);
void FLAC__MD5Update(struct FLAC__MD5Context *context, md5byte const *buf, unsigned len);
void FLAC__MD5Final(md5byte digest[16], struct FLAC__MD5Context *context);
void FLAC__MD5Transform(FLAC__uint32 buf[4], FLAC__uint32 const in[16]);

FLAC__bool FLAC__MD5Accumulate(struct FLAC__MD5Context *ctx, const FLAC__int32 * const signal[], unsigned channels, unsigned samples, unsigned bytes_per_sample);

#endif /* !MD5_H */
