//-----------------------------------------------------------------------------
// Project     : VST SDK
// Flags       : clang-format SMTGSequencer
//
// Category    : moduleinfo
// Filename    : public.sdk/source/vst/moduleinfo/moduleinfocreator.h
// Created by  : Steinberg, 12/2021
// Description : utility functions to create moduleinfo json files
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
#include "public.sdk/source/vst/hosting/module.h"
#include <iostream>
#include <optional>
#include <string_view>

//------------------------------------------------------------------------
namespace Steinberg::ModuleInfoLib {

//------------------------------------------------------------------------
/** create a ModuleInfo from a module
 *
 *	@param module module to create the module info from
 *	@param includeDiscardableClasses if true adds the current available classes to the module info
 *	@return a ModuleInfo struct with the classes and factory info of the module
 */
ModuleInfo createModuleInfo (const VST3::Hosting::Module& module, bool includeDiscardableClasses);

//------------------------------------------------------------------------
/** output the ModuleInfo as json to the stream
 *
 *	@param info module info
 *	@param output output stream
 */
void outputJson (const ModuleInfo& info, std::ostream& output);

//------------------------------------------------------------------------
} // Steinberg::ModuelInfoLib
