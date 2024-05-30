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
// LICENSE
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
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
