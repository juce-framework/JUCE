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

#ifndef SERD_BASE64_H
#define SERD_BASE64_H

#include "serd/serd.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
   Return the number of bytes required to encode `size` bytes in base64.

   @param size The number of input (binary) bytes to encode.
   @param wrap_lines Wrap lines at 76 characters to conform to RFC 2045.
   @return The length of the base64 encoding, excluding null terminator.
*/
SERD_CONST_FUNC size_t
serd_base64_get_length(size_t size, bool wrap_lines);

/**
   Encode `size` bytes of `buf` into `str`, which must be large enough.

   @param str Output string buffer.
   @param buf Input binary data.
   @param size Number of bytes to encode from `buf`.
   @param wrap_lines Wrap lines at 76 characters to conform to RFC 2045.
   @return True iff `str` contains newlines.
*/
bool
serd_base64_encode(uint8_t* str, const void* buf, size_t size, bool wrap_lines);

#endif // SERD_BASE64_H
