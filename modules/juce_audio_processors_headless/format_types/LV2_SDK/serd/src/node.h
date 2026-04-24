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

#ifndef SERD_NODE_H
#define SERD_NODE_H

#include "serd/serd.h"

#include <stddef.h>

struct SerdNodeImpl {
  size_t        n_bytes; /**< Size in bytes (not including null) */
  SerdNodeFlags flags;   /**< Node flags (e.g. string properties) */
  SerdType      type;    /**< Node type */
};

static inline char*
serd_node_buffer(SerdNode* node)
{
  return (char*)(node + 1);
}

static inline const char*
serd_node_buffer_c(const SerdNode* node)
{
  return (const char*)(node + 1);
}

SerdNode*
serd_node_malloc(size_t n_bytes, SerdNodeFlags flags, SerdType type);

void
serd_node_set(SerdNode** dst, const SerdNode* src);

#endif // SERD_NODE_H
