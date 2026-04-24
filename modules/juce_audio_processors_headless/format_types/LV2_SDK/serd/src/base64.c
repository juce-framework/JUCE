/*
  Copyright 2011-2020 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "base64.h"

#include "serd_internal.h"
#include "string_utils.h"

#include "serd/serd.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
   Base64 encoding table.

   @see <a href="http://tools.ietf.org/html/rfc3548#section-3">RFC3548 S3</a>.
*/
static const uint8_t b64_map[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
   Base64 decoding table.

   This is indexed by encoded characters and returns the numeric value used
   for decoding, shifted up by 47 to be in the range of printable ASCII.
   A '$' is a placeholder for characters not in the base64 alphabet.
*/
static const char b64_unmap[] =
  "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$m$$$ncdefghijkl$$$$$$"
  "$/0123456789:;<=>?@ABCDEFGH$$$$$$IJKLMNOPQRSTUVWXYZ[\\]^_`ab$$$$"
  "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"
  "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";

/** Encode 3 raw bytes to 4 base64 characters. */
static inline void
encode_chunk(uint8_t out[4], const uint8_t in[3], size_t n_in)
{
  out[0] = b64_map[in[0] >> 2];
  out[1] = b64_map[((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4)];

  out[2] = (n_in > 1) ? (b64_map[((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6)])
                      : (uint8_t)'=';

  out[3] = ((n_in > 2) ? b64_map[in[2] & 0x3F] : (uint8_t)'=');
}

size_t
serd_base64_get_length(const size_t size, const bool wrap_lines)
{
  return (size + 2) / 3 * 4 + (wrap_lines * ((size - 1) / 57));
}

bool
serd_base64_encode(uint8_t* const    str,
                   const void* const buf,
                   const size_t      size,
                   const bool        wrap_lines)
{
  bool has_newline = false;

  for (size_t i = 0, j = 0; i < size; i += 3, j += 4) {
    uint8_t in[4] = {0, 0, 0, 0};
    size_t  n_in  = MIN(3, size - i);
    memcpy(in, (const uint8_t*)buf + i, n_in);

    if (wrap_lines && i > 0 && (i % 57) == 0) {
      str[j++]    = '\n';
      has_newline = true;
    }

    encode_chunk(str + j, in, n_in);
  }

  return has_newline;
}

static inline uint8_t
unmap(const uint8_t in)
{
  return (uint8_t)(b64_unmap[in] - 47);
}

/** Decode 4 base64 characters to 3 raw bytes. */
static inline size_t
decode_chunk(const uint8_t in[4], uint8_t out[3])
{
  out[0] = (uint8_t)(((unmap(in[0]) << 2)) | unmap(in[1]) >> 4);
  out[1] = (uint8_t)(((unmap(in[1]) << 4) & 0xF0) | unmap(in[2]) >> 2);
  out[2] = (uint8_t)(((unmap(in[2]) << 6) & 0xC0) | unmap(in[3]));
  return 1 + (in[2] != '=') + ((in[2] != '=') && (in[3] != '='));
}

void*
serd_base64_decode(const uint8_t* str, size_t len, size_t* size)
{
  void* buf = malloc((len * 3) / 4 + 2);

  *size = 0;
  for (size_t i = 0, j = 0; i < len; j += 3) {
    uint8_t in[] = "====";
    size_t  n_in = 0;
    for (; i < len && n_in < 4; ++n_in) {
      for (; i < len && !is_base64(str[i]); ++i) {
        // Skip junk
      }

      in[n_in] = str[i++];
    }

    if (n_in > 1) {
      *size += decode_chunk(in, (uint8_t*)buf + j);
    }
  }

  return buf;
}
