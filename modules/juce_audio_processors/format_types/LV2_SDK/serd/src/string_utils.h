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

#ifndef SERD_STRING_UTILS_H
#define SERD_STRING_UTILS_H

#include "serd/serd.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Unicode replacement character in UTF-8 */
static const uint8_t replacement_char[] = {0xEF, 0xBF, 0xBD};

/** Return true if `c` lies within [`min`...`max`] (inclusive) */
static inline bool
in_range(const int c, const int min, const int max)
{
  return (c >= min && c <= max);
}

/** RFC2234: ALPHA ::= %x41-5A / %x61-7A  ; A-Z / a-z */
static inline bool
is_alpha(const int c)
{
  return in_range(c, 'A', 'Z') || in_range(c, 'a', 'z');
}

/** RFC2234: DIGIT ::= %x30-39  ; 0-9 */
static inline bool
is_digit(const int c)
{
  return in_range(c, '0', '9');
}

/* RFC2234: HEXDIG ::= DIGIT / "A" / "B" / "C" / "D" / "E" / "F" */
static inline bool
is_hexdig(const int c)
{
  return is_digit(c) || in_range(c, 'A', 'F');
}

/* Turtle / JSON / C: XDIGIT ::= DIGIT / A-F / a-f */
static inline bool
is_xdigit(const int c)
{
  return is_hexdig(c) || in_range(c, 'a', 'f');
}

static inline bool
is_space(const char c)
{
  switch (c) {
  case ' ':
  case '\f':
  case '\n':
  case '\r':
  case '\t':
  case '\v':
    return true;
  default:
    return false;
  }
}

static inline bool
is_print(const int c)
{
  return c >= 0x20 && c <= 0x7E;
}

static inline bool
is_base64(const uint8_t c)
{
  return is_alpha(c) || is_digit(c) || c == '+' || c == '/' || c == '=';
}

static inline bool
is_windows_path(const uint8_t* path)
{
  return is_alpha(path[0]) && (path[1] == ':' || path[1] == '|') &&
         (path[2] == '/' || path[2] == '\\');
}

size_t
serd_substrlen(const uint8_t* str,
               size_t         len,
               size_t*        n_bytes,
               SerdNodeFlags* flags);

static inline char
serd_to_upper(const char c)
{
  return (char)((c >= 'a' && c <= 'z') ? c - 32 : c);
}

static inline int
serd_strncasecmp(const char* s1, const char* s2, size_t n)
{
  for (; n > 0 && *s2; s1++, s2++, --n) {
    if (serd_to_upper(*s1) != serd_to_upper(*s2)) {
      return ((*(const uint8_t*)s1 < *(const uint8_t*)s2) ? -1 : +1);
    }
  }

  return 0;
}

static inline uint32_t
utf8_num_bytes(const uint8_t c)
{
  if ((c & 0x80) == 0) { // Starts with `0'
    return 1;
  }

  if ((c & 0xE0) == 0xC0) { // Starts with `110'
    return 2;
  }

  if ((c & 0xF0) == 0xE0) { // Starts with `1110'
    return 3;
  }

  if ((c & 0xF8) == 0xF0) { // Starts with `11110'
    return 4;
  }

  return 0;
}

/// Return the code point of a UTF-8 character with known length
static inline uint32_t
parse_counted_utf8_char(const uint8_t* utf8, size_t size)
{
  uint32_t c = utf8[0] & ((1u << (8 - size)) - 1);
  for (size_t i = 1; i < size; ++i) {
    const uint8_t in = utf8[i] & 0x3F;
    c                = (c << 6) | in;
  }
  return c;
}

/// Parse a UTF-8 character, set *size to the length, and return the code point
static inline uint32_t
parse_utf8_char(const uint8_t* utf8, size_t* size)
{
  switch (*size = utf8_num_bytes(utf8[0])) {
  case 1:
  case 2:
  case 3:
  case 4:
    return parse_counted_utf8_char(utf8, *size);
  default:
    *size = 0;
    return 0;
  }
}

#endif // SERD_STRING_UTILS_H
