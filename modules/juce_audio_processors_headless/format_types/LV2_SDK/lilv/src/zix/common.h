/*
  Copyright 2016-2020 David Robillard <d@drobilla.net>

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

#ifndef ZIX_COMMON_H
#define ZIX_COMMON_H

#include <stdbool.h>

/**
   @addtogroup zix
   @{
*/

/** @cond */
#if defined(_WIN32) && !defined(ZIX_STATIC) && defined(ZIX_INTERNAL)
#  define ZIX_API __declspec(dllexport)
#elif defined(_WIN32) && !defined(ZIX_STATIC)
#  define ZIX_API __declspec(dllimport)
#elif defined(__GNUC__)
#  define ZIX_API __attribute__((visibility("default")))
#else
#  define ZIX_API
#endif

#ifdef __GNUC__
#  define ZIX_PURE_FUNC __attribute__((pure))
#  define ZIX_CONST_FUNC __attribute__((const))
#  define ZIX_MALLOC_FUNC __attribute__((malloc))
#else
#  define ZIX_PURE_FUNC
#  define ZIX_CONST_FUNC
#  define ZIX_MALLOC_FUNC
#endif

#define ZIX_PURE_API \
  ZIX_API            \
  ZIX_PURE_FUNC

#define ZIX_CONST_API \
  ZIX_API             \
  ZIX_CONST_FUNC

#define ZIX_MALLOC_API \
  ZIX_API              \
  ZIX_MALLOC_FUNC

/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#  define ZIX_LOG_FUNC(fmt, arg1) __attribute__((format(printf, fmt, arg1)))
#else
#  define ZIX_LOG_FUNC(fmt, arg1)
#endif

// Unused parameter macro to suppresses warnings and make it impossible to use
#if defined(__cplusplus)
#  define ZIX_UNUSED(name)
#elif defined(__GNUC__)
#  define ZIX_UNUSED(name) name##_unused __attribute__((__unused__))
#else
#  define ZIX_UNUSED(name) name
#endif

typedef enum {
  ZIX_STATUS_SUCCESS,
  ZIX_STATUS_ERROR,
  ZIX_STATUS_NO_MEM,
  ZIX_STATUS_NOT_FOUND,
  ZIX_STATUS_EXISTS,
  ZIX_STATUS_BAD_ARG,
  ZIX_STATUS_BAD_PERMS
} ZixStatus;

static inline const char*
zix_strerror(const ZixStatus status)
{
  switch (status) {
  case ZIX_STATUS_SUCCESS:
    return "Success";
  case ZIX_STATUS_ERROR:
    return "Unknown error";
  case ZIX_STATUS_NO_MEM:
    return "Out of memory";
  case ZIX_STATUS_NOT_FOUND:
    return "Not found";
  case ZIX_STATUS_EXISTS:
    return "Exists";
  case ZIX_STATUS_BAD_ARG:
    return "Bad argument";
  case ZIX_STATUS_BAD_PERMS:
    return "Bad permissions";
  }
  return "Unknown error";
}

/**
   Function for comparing two elements.
*/
typedef int (*ZixComparator)(const void* a,
                             const void* b,
                             const void* user_data);

/**
   Function for testing equality of two elements.
*/
typedef bool (*ZixEqualFunc)(const void* a, const void* b);

/**
   Function to destroy an element.
*/
typedef void (*ZixDestroyFunc)(void* ptr);

/**
   @}
*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ZIX_COMMON_H */
