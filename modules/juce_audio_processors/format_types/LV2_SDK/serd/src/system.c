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

#define _POSIX_C_SOURCE 200809L /* for posix_memalign and posix_fadvise */

#include "system.h"

#include "serd_config.h"
#include "serd_internal.h"

#if USE_POSIX_FADVISE && USE_FILENO
#  include <fcntl.h>
#endif

#ifdef _WIN32
#  include <malloc.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE*
serd_fopen(const char* path, const char* mode)
{
  FILE* fd = fopen(path, mode);
  if (!fd) {
    fprintf(
      stderr, "error: failed to open file %s (%s)\n", path, strerror(errno));
    return NULL;
  }

#if USE_POSIX_FADVISE && USE_FILENO
  posix_fadvise(fileno(fd), 0, 0, POSIX_FADV_SEQUENTIAL);
#endif
  return fd;
}

void*
serd_malloc_aligned(const size_t alignment, const size_t size)
{
#if defined(_WIN32)
  return _aligned_malloc(size, alignment);
#elif USE_POSIX_MEMALIGN
  void*     ptr = NULL;
  const int ret = posix_memalign(&ptr, alignment, size);
  return ret ? NULL : ptr;
#else
  (void)alignment;
  return malloc(size);
#endif
}

void*
serd_allocate_buffer(const size_t size)
{
  return serd_malloc_aligned(SERD_PAGE_SIZE, size);
}

void
serd_free_aligned(void* const ptr)
{
#ifdef _WIN32
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}
