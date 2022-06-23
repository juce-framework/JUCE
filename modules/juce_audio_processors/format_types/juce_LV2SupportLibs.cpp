/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "juce_lv2_config.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wc99-extensions",
                                     "-Wcast-align",
                                     "-Wconversion",
                                     "-Wdeprecated-declarations",
                                     "-Wextra-semi",
                                     "-Wfloat-conversion",
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
