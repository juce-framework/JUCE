//
//    ██████ ██   ██  ██████   ██████
//   ██      ██   ██ ██    ██ ██            ** Classy Header-Only Classes **
//   ██      ███████ ██    ██ ██
//   ██      ██   ██ ██    ██ ██           https://github.com/Tracktion/choc
//    ██████ ██   ██  ██████   ██████
//
//   CHOC is (C)2022 Tracktion Corporation, and is offered under the terms of the ISC license:
//
//   Permission to use, copy, modify, and/or distribute this software for any purpose with or
//   without fee is hereby granted, provided that the above copyright notice and this permission
//   notice appear in all copies. THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
//   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
//   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
//   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
//   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
//   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


/*
    Sometimes you inevitably have to include some 3rd-party headers in your code, and it's
    depressing how often even the most widely-used libraries are full of poorly-written
    code that triggers all kinds of compiler warnings.

    Obviously you're a proper programmer who always has warnings turned up to full, and "-Werror"
    enabled, so rather than having to compromise your own standards to work around other people's
    laziness, this header provides an easy way to locally disable warnings by sandwiching your
    3rd-party headers between includes of these two files, e.g:

        #include "choc_DisableAllWarnings.h"
        #include "SloppilyWrittenButEssentialLibraryCode.h"
        #include "choc_ReenableAllWarnings.h"
*/

#if __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Weverything"
#elif __GNUC__
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wall"
 #pragma GCC diagnostic ignored "-Wpragmas"
 #pragma GCC diagnostic ignored "-Wextra"
 #pragma GCC diagnostic ignored "-Wshadow"
 #pragma GCC diagnostic ignored "-Wunused-parameter"
 #pragma GCC diagnostic ignored "-Wconversion"
 #pragma GCC diagnostic ignored "-Wsign-conversion"
 #pragma GCC diagnostic ignored "-Wsign-compare"
 #pragma GCC diagnostic ignored "-Wfloat-conversion"
 #pragma GCC diagnostic ignored "-Wswitch-enum"
 #pragma GCC diagnostic ignored "-Wswitch"
 #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
 #pragma GCC diagnostic ignored "-Wunused-variable"
 #pragma GCC diagnostic ignored "-Wredundant-decls"
 #pragma GCC diagnostic ignored "-Wsubobject-linkage"
 #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
 #pragma GCC diagnostic ignored "-Wredundant-move"
 #pragma GCC diagnostic ignored "-Wstrict-aliasing"
 #pragma GCC diagnostic ignored "-Woverloaded-virtual"
 #pragma GCC diagnostic ignored "-Wc99-extensions"
 #pragma GCC diagnostic ignored "-Wmisleading-indentation"
 #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
 #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
 #pragma GCC diagnostic ignored "-Wcast-function-type"
 #pragma GCC diagnostic ignored "-Wunused-label"
 #pragma GCC diagnostic ignored "-Wnarrowing"
 #pragma GCC diagnostic ignored "-Wparentheses"
 #pragma GCC diagnostic ignored "-Wwrite-strings"
 #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
 #pragma GCC diagnostic ignored "-Wfloat-equal"

 #ifndef __MINGW32__
  #pragma GCC diagnostic ignored "-Wredundant-move"
 #endif
#else
 #pragma warning (push, 0)
 #pragma warning (disable: 4702)
 #pragma warning (disable: 4706)
#endif
