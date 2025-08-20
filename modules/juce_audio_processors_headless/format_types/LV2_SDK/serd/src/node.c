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

#include "node.h"

#include "base64.h"
#include "string_utils.h"

#include "serd/serd.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  ifndef isnan
#    define isnan(x) _isnan(x)
#  endif
#  ifndef isinf
#    define isinf(x) (!_finite(x))
#  endif
#endif

SerdNode
serd_node_from_string(SerdType type, const uint8_t* str)
{
  if (!str) {
    return SERD_NODE_NULL;
  }

  SerdNodeFlags flags       = 0;
  size_t        buf_n_bytes = 0;
  const size_t  buf_n_chars = serd_strlen(str, &buf_n_bytes, &flags);
  SerdNode      ret         = {str, buf_n_bytes, buf_n_chars, flags, type};
  return ret;
}

SerdNode
serd_node_from_substring(SerdType type, const uint8_t* str, const size_t len)
{
  if (!str) {
    return SERD_NODE_NULL;
  }

  SerdNodeFlags flags       = 0;
  size_t        buf_n_bytes = 0;
  const size_t  buf_n_chars = serd_substrlen(str, len, &buf_n_bytes, &flags);
  assert(buf_n_bytes <= len);
  SerdNode ret = {str, buf_n_bytes, buf_n_chars, flags, type};
  return ret;
}

SerdNode
serd_node_copy(const SerdNode* node)
{
  if (!node || !node->buf) {
    return SERD_NODE_NULL;
  }

  SerdNode copy = *node;
  uint8_t* buf  = (uint8_t*)malloc(copy.n_bytes + 1);
  memcpy(buf, node->buf, copy.n_bytes + 1);
  copy.buf = buf;
  return copy;
}

bool
serd_node_equals(const SerdNode* a, const SerdNode* b)
{
  return (a == b) ||
         (a->type == b->type && a->n_bytes == b->n_bytes &&
          a->n_chars == b->n_chars &&
          ((a->buf == b->buf) ||
           !memcmp((const char*)a->buf, (const char*)b->buf, a->n_bytes + 1)));
}

static size_t
serd_uri_string_length(const SerdURI* uri)
{
  size_t len = uri->path_base.len;

#define ADD_LEN(field, n_delims)     \
  if ((field).len) {                 \
    len += (field).len + (n_delims); \
  }

  ADD_LEN(uri->path, 1)      // + possible leading `/'
  ADD_LEN(uri->scheme, 1)    // + trailing `:'
  ADD_LEN(uri->authority, 2) // + leading `//'
  ADD_LEN(uri->query, 1)     // + leading `?'
  ADD_LEN(uri->fragment, 1)  // + leading `#'

  return len + 2; // + 2 for authority `//'
}

static size_t
string_sink(const void* buf, size_t len, void* stream)
{
  uint8_t** ptr = (uint8_t**)stream;
  memcpy(*ptr, buf, len);
  *ptr += len;
  return len;
}

SerdNode
serd_node_new_uri_from_node(const SerdNode* uri_node,
                            const SerdURI*  base,
                            SerdURI*        out)
{
  return (uri_node->type == SERD_URI && uri_node->buf)
           ? serd_node_new_uri_from_string(uri_node->buf, base, out)
           : SERD_NODE_NULL;
}

SerdNode
serd_node_new_uri_from_string(const uint8_t* str,
                              const SerdURI* base,
                              SerdURI*       out)
{
  if (!str || str[0] == '\0') {
    // Empty URI => Base URI, or nothing if no base is given
    return base ? serd_node_new_uri(base, NULL, out) : SERD_NODE_NULL;
  }

  SerdURI uri;
  serd_uri_parse(str, &uri);
  return serd_node_new_uri(&uri, base, out); // Resolve/Serialise
}

static inline bool
is_uri_path_char(const uint8_t c)
{
  if (is_alpha(c) || is_digit(c)) {
    return true;
  }

  switch (c) {
  // unreserved:
  case '-':
  case '.':
  case '_':
  case '~':
  // pchar:
  case ':':
  case '@':
  // separator:
  case '/':
  // sub-delimeters:
  case '!':
  case '$':
  case '&':
  case '\'':
  case '(':
  case ')':
  case '*':
  case '+':
  case ',':
  case ';':
  case '=':
    return true;
  default:
    return false;
  }
}

SerdNode
serd_node_new_file_uri(const uint8_t* path,
                       const uint8_t* hostname,
                       SerdURI*       out,
                       bool           escape)
{
  const size_t path_len     = strlen((const char*)path);
  const size_t hostname_len = hostname ? strlen((const char*)hostname) : 0;
  const bool   is_windows   = is_windows_path(path);
  size_t       uri_len      = 0;
  uint8_t*     uri          = NULL;

  if (path[0] == '/' || is_windows) {
    uri_len = strlen("file://") + hostname_len + is_windows;
    uri     = (uint8_t*)calloc(uri_len + 1, 1);

    memcpy(uri, "file://", 7);

    if (hostname) {
      memcpy(uri + 7, hostname, hostname_len);
    }

    if (is_windows) {
      ((char*)uri)[7 + hostname_len] = '/';
    }
  }

  SerdChunk chunk = {uri, uri_len};
  for (size_t i = 0; i < path_len; ++i) {
    if (is_windows && path[i] == '\\') {
      serd_chunk_sink("/", 1, &chunk);
    } else if (path[i] == '%') {
      serd_chunk_sink("%%", 2, &chunk);
    } else if (!escape || is_uri_path_char(path[i])) {
      serd_chunk_sink(path + i, 1, &chunk);
    } else {
      char escape_str[4] = {'%', 0, 0, 0};
      snprintf(escape_str + 1, sizeof(escape_str) - 1, "%X", (unsigned)path[i]);
      serd_chunk_sink(escape_str, 3, &chunk);
    }
  }

  serd_chunk_sink_finish(&chunk);

  if (out) {
    serd_uri_parse(chunk.buf, out);
  }

  return serd_node_from_substring(SERD_URI, chunk.buf, chunk.len);
}

SerdNode
serd_node_new_uri(const SerdURI* uri, const SerdURI* base, SerdURI* out)
{
  SerdURI abs_uri = *uri;
  if (base) {
    serd_uri_resolve(uri, base, &abs_uri);
  }

  const size_t len        = serd_uri_string_length(&abs_uri);
  uint8_t*     buf        = (uint8_t*)malloc(len + 1);
  SerdNode     node       = {buf, 0, 0, 0, SERD_URI};
  uint8_t*     ptr        = buf;
  const size_t actual_len = serd_uri_serialise(&abs_uri, string_sink, &ptr);

  buf[actual_len] = '\0';
  node.n_bytes    = actual_len;
  node.n_chars    = serd_strlen(buf, NULL, NULL);

  if (out) {
    serd_uri_parse(buf, out); // TODO: cleverly avoid double parse
  }

  return node;
}

SerdNode
serd_node_new_relative_uri(const SerdURI* uri,
                           const SerdURI* base,
                           const SerdURI* root,
                           SerdURI*       out)
{
  const size_t uri_len  = serd_uri_string_length(uri);
  const size_t base_len = serd_uri_string_length(base);
  uint8_t*     buf      = (uint8_t*)malloc(uri_len + base_len + 1);
  SerdNode     node     = {buf, 0, 0, 0, SERD_URI};
  uint8_t*     ptr      = buf;
  const size_t actual_len =
    serd_uri_serialise_relative(uri, base, root, string_sink, &ptr);

  buf[actual_len] = '\0';
  node.n_bytes    = actual_len;
  node.n_chars    = serd_strlen(buf, NULL, NULL);

  if (out) {
    serd_uri_parse(buf, out); // TODO: cleverly avoid double parse
  }

  return node;
}

static inline unsigned
serd_digits(double abs)
{
  const double lg = ceil(log10(floor(abs) + 1.0));
  return lg < 1.0 ? 1U : (unsigned)lg;
}

SerdNode
serd_node_new_decimal(double d, unsigned frac_digits)
{
  if (isnan(d) || isinf(d)) {
    return SERD_NODE_NULL;
  }

  const double   abs_d      = fabs(d);
  const unsigned int_digits = serd_digits(abs_d);
  char*          buf        = (char*)calloc(int_digits + frac_digits + 3, 1);
  SerdNode       node       = {(const uint8_t*)buf, 0, 0, 0, SERD_LITERAL};
  const double   int_part   = floor(abs_d);

  // Point s to decimal point location
  char* s = buf + int_digits;
  if (d < 0.0) {
    *buf = '-';
    ++s;
  }

  // Write integer part (right to left)
  char*    t   = s - 1;
  uint64_t dec = (uint64_t)int_part;
  do {
    *t-- = (char)('0' + dec % 10);
  } while ((dec /= 10) > 0);

  *s++ = '.';

  // Write fractional part (right to left)
  double frac_part = fabs(d - int_part);
  if (frac_part < DBL_EPSILON) {
    *s++         = '0';
    node.n_bytes = node.n_chars = (size_t)(s - buf);
  } else {
    uint64_t frac = (uint64_t)llround(frac_part * pow(10.0, (int)frac_digits));
    s += frac_digits - 1;
    unsigned i = 0;

    // Skip trailing zeros
    for (; i < frac_digits - 1 && !(frac % 10); ++i, --s, frac /= 10) {
    }

    node.n_bytes = node.n_chars = (size_t)(s - buf) + 1u;

    // Write digits from last trailing zero to decimal point
    for (; i < frac_digits; ++i) {
      *s-- = (char)('0' + (frac % 10));
      frac /= 10;
    }
  }

  return node;
}

SerdNode
serd_node_new_integer(int64_t i)
{
  uint64_t       abs_i  = (i < 0) ? -i : i;
  const unsigned digits = serd_digits((double)abs_i);
  char*          buf    = (char*)calloc(digits + 2, 1);
  SerdNode       node   = {(const uint8_t*)buf, 0, 0, 0, SERD_LITERAL};

  // Point s to the end
  char* s = buf + digits - 1;
  if (i < 0) {
    *buf = '-';
    ++s;
  }

  node.n_bytes = node.n_chars = (size_t)(s - buf) + 1u;

  // Write integer part (right to left)
  do {
    *s-- = (char)('0' + (abs_i % 10));
  } while ((abs_i /= 10) > 0);

  return node;
}

SerdNode
serd_node_new_blob(const void* buf, size_t size, bool wrap_lines)
{
  const size_t len  = serd_base64_get_length(size, wrap_lines);
  uint8_t*     str  = (uint8_t*)calloc(len + 2, 1);
  SerdNode     node = {str, len, len, 0, SERD_LITERAL};

  if (serd_base64_encode(str, buf, size, wrap_lines)) {
    node.flags |= SERD_HAS_NEWLINE;
  }

  return node;
}

void
serd_node_free(SerdNode* node)
{
  if (node && node->buf) {
    free((uint8_t*)node->buf);
    node->buf = NULL;
  }
}
