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

#ifndef SERD_INTERNAL_H
#define SERD_INTERNAL_H

#include "serd/serd.h"

#include <stdio.h>

#define NS_XSD "http://www.w3.org/2001/XMLSchema#"
#define NS_RDF "http://www.w3.org/1999/02/22-rdf-syntax-ns#"

#define SERD_PAGE_SIZE 4096

#ifndef MIN
#  define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* Error reporting */

static inline void
serd_error(SerdErrorSink error_sink, void* handle, const SerdError* e)
{
  if (error_sink) {
    error_sink(handle, e);
  } else {
    fprintf(stderr, "error: %s:%u:%u: ", e->filename, e->line, e->col);
    vfprintf(stderr, e->fmt, *e->args);
  }
}

#endif // SERD_INTERNAL_H
