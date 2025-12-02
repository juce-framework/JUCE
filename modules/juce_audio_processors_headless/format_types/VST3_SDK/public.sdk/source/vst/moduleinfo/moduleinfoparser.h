//-----------------------------------------------------------------------------
// Project     : VST SDK
// Flags       : clang-format SMTGSequencer
//
// Category    : moduleinfo
// Filename    : public.sdk/source/vst/moduleinfo/moduleinfoparser.h
// Created by  : Steinberg, 01/2022
// Description : utility functions to parse moduleinfo json files
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "moduleinfo.h"
#include <iostream>
#include <optional>
#include <string_view>

//------------------------------------------------------------------------
namespace Steinberg::ModuleInfoLib {

//------------------------------------------------------------------------
/** parse a json formatted string to a ModuleInfo struct
 *
 *	@param jsonData a string view to a json formatted string
 *	@param optErrorOutput optional error output stream where to print parse error
 *	@return ModuleInfo if parsing succeeded
 */
std::optional<ModuleInfo> parseJson (std::string_view jsonData, std::ostream* optErrorOutput);

//------------------------------------------------------------------------
/** parse a json formatted string to a ModuleInfo::CompatibilityList
 *
 *	@param jsonData a string view to a json formatted string
 *	@param optErrorOutput optional error output stream where to print parse error
 *	@return ModuleInfo::CompatibilityList if parsing succeeded
 */
std::optional<ModuleInfo::CompatibilityList> parseCompatibilityJson (std::string_view jsonData,
                                                                     std::ostream* optErrorOutput);

//------------------------------------------------------------------------
} // Steinberg::ModuelInfoLib
