/*
  Copyright 2021 David Robillard <d@drobilla.net>

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

/*
  Configuration header that defines reasonable defaults at compile time.

  This allows compile-time configuration from the command line (typically via
  the build system) while still allowing the source to be built without any
  configuration.  The build system can define SERD_NO_DEFAULT_CONFIG to disable
  defaults, in which case it must define things like HAVE_FEATURE to enable
  features.  The design here ensures that compiler warnings or
  include-what-you-use will catch any mistakes.
*/

#ifndef SERD_CONFIG_H
#define SERD_CONFIG_H

// Define version unconditionally so a warning will catch a mismatch
#define SERD_VERSION "0.30.10"

#if !defined(SERD_NO_DEFAULT_CONFIG)

// We need unistd.h to check _POSIX_VERSION
#  ifndef SERD_NO_POSIX
#    ifdef __has_include
#      if __has_include(<unistd.h>)
#        include <unistd.h>
#      endif
#    elif defined(__unix__)
#      include <unistd.h>
#    endif
#  endif

// POSIX.1-2001: fileno()
#  ifndef HAVE_FILENO
#    if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L
#      define HAVE_FILENO
#    endif
#  endif

// POSIX.1-2001: posix_fadvise()
#  ifndef HAVE_POSIX_FADVISE
#    ifndef __APPLE__
#      if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L
#        define HAVE_POSIX_FADVISE
#      endif
#    endif
#  endif

// POSIX.1-2001: posix_memalign()
#  ifndef HAVE_POSIX_MEMALIGN
#    if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L
#      define HAVE_POSIX_MEMALIGN
#    endif
#  endif

#endif // !defined(SERD_NO_DEFAULT_CONFIG)

/*
  Make corresponding USE_FEATURE defines based on the HAVE_FEATURE defines from
  above or the command line.  The code checks for these using #if (not #ifdef),
  so there will be an undefined warning if it checks for an unknown feature,
  and this header is always required by any code that checks for features, even
  if the build system defines them all.
*/

#ifdef HAVE_FILENO
#  define USE_FILENO 1
#else
#  define USE_FILENO 0
#endif

#ifdef HAVE_POSIX_FADVISE
#  define USE_POSIX_FADVISE 1
#else
#  define USE_POSIX_FADVISE 0
#endif

#ifdef HAVE_POSIX_MEMALIGN
#  define USE_POSIX_MEMALIGN 1
#else
#  define USE_POSIX_MEMALIGN 0
#endif

#endif // SERD_CONFIG_H
