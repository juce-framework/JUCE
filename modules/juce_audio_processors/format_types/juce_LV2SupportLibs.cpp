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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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

#include "juce_lv2_config.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wc99-extensions",
                                     "-Wcast-align",
                                     "-Wconversion",
                                     "-Wdeprecated-declarations",
                                     "-Wextra-semi",
                                     "-Wfloat-conversion",
                                     "-Wfloat-equal",
                                     "-Wformat-overflow",
                                     "-Wimplicit-float-conversion",
                                     "-Wimplicit-int-conversion",
                                     "-Wmicrosoft-include",
                                     "-Wmissing-field-initializers",
                                     "-Wnullability-extension",
                                     "-Wnullable-to-nonnull-conversion",
                                     "-Wparentheses",
                                     "-Wpedantic",
                                     "-Wredundant-decls",
                                     "-Wshorten-64-to-32",
                                     "-Wsign-conversion",
                                     "-Wswitch-enum",
                                     "-Wunused-parameter",
                                     "-Wzero-as-null-pointer-constant")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100 4200 4244 4267 4389 4702 4706 4800 4996 6308 28182 28183 6385 6386 6387 6011 6282 6323 6330 6001 6031)

extern "C"
{

#include <math.h>

#define is_windows_path serd_is_windows_path

#include "serd/src/base64.c"
#include "serd/src/byte_source.c"
#include "serd/src/env.c"
#include "serd/src/n3.c"
#undef TRY

#include "serd/src/node.c"
#include "serd/src/reader.c"
#include "serd/src/string.c"
#include "serd/src/system.c"
#include "serd/src/uri.c"
#include "serd/src/writer.c"

#undef is_windows_path

#include "sord/src/sord.c"
#include "sord/src/syntax.c"

#include "lilv/src/collections.c"
#include "lilv/src/filesystem.c"
#include "lilv/src/instance.c"
#include "lilv/src/lib.c"
#include "lilv/src/node.c"
#include "lilv/src/plugin.c"
#include "lilv/src/pluginclass.c"
#include "lilv/src/port.c"
#include "lilv/src/query.c"
#include "lilv/src/scalepoint.c"
#include "lilv/src/state.c"
#include "lilv/src/ui.c"
#include "lilv/src/util.c"
#include "lilv/src/world.c"
#include "lilv/src/zix/tree.c"

#undef NS_RDF
#undef NS_XSD
#undef USTR

#define read_object sratom_read_object
#define read_literal sratom_read_literal

#pragma push_macro ("nil")
#undef nil
#include "LV2_SDK/sratom/src/sratom.c"
#pragma pop_macro ("nil")

} // extern "C"

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
