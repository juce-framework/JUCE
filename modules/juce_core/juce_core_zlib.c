/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include <juce_core/system/juce_CompilerWarnings.h>

#if ! defined (JUCE_INCLUDE_ZLIB_CODE) || JUCE_INCLUDE_ZLIB_CODE

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-align",
                                     "-Wshorten-64-to-32",
                                     "-Wsign-conversion",
                                     "-Wredundant-decls",
                                     "-Wshadow",
                                     "-Wcomma",
                                     "-Wimplicit-fallthrough")

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6385 6326)

#include "juce_core/zip/zlib/adler32.c"
#include "juce_core/zip/zlib/compress.c"
#include "juce_core/zip/zlib/crc32.c"
#include "juce_core/zip/zlib/deflate.c"
#include "juce_core/zip/zlib/inffast.c"
#undef GZIP
#include "juce_core/zip/zlib/inflate.c"
#include "juce_core/zip/zlib/inftrees.c"
#include "juce_core/zip/zlib/trees.c"
#include "juce_core/zip/zlib/zutil.c"

#else
// To get around 'empty translation unit' errors and 'has no symbols' warnings emitted by libtool
void juce_coreZlibPlaceholder (void);
void juce_coreZlibPlaceholder (void) {}
#endif
