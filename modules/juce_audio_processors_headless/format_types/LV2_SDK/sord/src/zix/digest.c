/*
  Copyright 2012-2020 David Robillard <d@drobilla.net>

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

#include "zix/digest.h"

#ifdef __SSE4_2__
#  include <smmintrin.h>
#endif

#include <assert.h>
#include <stdint.h>

#ifdef __SSE4_2__

// SSE 4.2 CRC32

uint32_t
zix_digest_start(void)
{
  return 1;
}

uint32_t
zix_digest_add(uint32_t hash, const void* const buf, const size_t len)
{
  const uint8_t* str = (const uint8_t*)buf;

#  ifdef __x86_64__
  for (size_t i = 0; i < (len / sizeof(uint64_t)); ++i) {
    hash = (uint32_t)_mm_crc32_u64(hash, *(const uint64_t*)str);
    str += sizeof(uint64_t);
  }
  if (len & sizeof(uint32_t)) {
    hash = _mm_crc32_u32(hash, *(const uint32_t*)str);
    str += sizeof(uint32_t);
  }
#  else
  for (size_t i = 0; i < (len / sizeof(uint32_t)); ++i) {
    hash = _mm_crc32_u32(hash, *(const uint32_t*)str);
    str += sizeof(uint32_t);
  }
#  endif
  if (len & sizeof(uint16_t)) {
    hash = _mm_crc32_u16(hash, *(const uint16_t*)str);
    str += sizeof(uint16_t);
  }
  if (len & sizeof(uint8_t)) {
    hash = _mm_crc32_u8(hash, *(const uint8_t*)str);
  }

  return hash;
}

uint32_t
zix_digest_add_64(uint32_t hash, const void* const buf, const size_t len)
{
  assert((uintptr_t)buf % sizeof(uint64_t) == 0);
  assert(len % sizeof(uint64_t) == 0);

#  ifdef __x86_64__
  const uint64_t* ptr = (const uint64_t*)buf;

  for (size_t i = 0; i < (len / sizeof(uint64_t)); ++i) {
    hash = (uint32_t)_mm_crc32_u64(hash, *ptr);
    ++ptr;
  }

  return hash;
#  else
  const uint32_t* ptr = (const uint32_t*)buf;

  for (size_t i = 0; i < (len / sizeof(uint32_t)); ++i) {
    hash = _mm_crc32_u32(hash, *ptr);
    ++ptr;
  }

  return hash;
#  endif
}

uint32_t
zix_digest_add_ptr(const uint32_t hash, const void* const ptr)
{
#  ifdef __x86_64__
  return (uint32_t)_mm_crc32_u64(hash, (uintptr_t)ptr);
#  else
  return _mm_crc32_u32(hash, (uintptr_t)ptr);
#  endif
}

#else

// Classic DJB hash

uint32_t
zix_digest_start(void)
{
  return 5381;
}

uint32_t
zix_digest_add(uint32_t hash, const void* const buf, const size_t len)
{
  const uint8_t* str = (const uint8_t*)buf;

  for (size_t i = 0; i < len; ++i) {
    hash = (hash << 5u) + hash + str[i];
  }

  return hash;
}

uint32_t
zix_digest_add_64(uint32_t hash, const void* const buf, const size_t len)
{
  assert((uintptr_t)buf % sizeof(uint64_t) == 0);
  assert(len % sizeof(uint64_t) == 0);

  return zix_digest_add(hash, buf, len);
}

uint32_t
zix_digest_add_ptr(const uint32_t hash, const void* const ptr)
{
  return zix_digest_add(hash, &ptr, sizeof(ptr));
}

#endif
