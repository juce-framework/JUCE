//-----------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST SDK
//
// Category    : readfile
// Filename    : public.sdk/source/common/readfile.h
// Created by  : Steinberg, 3/2023
// Description : read file routine
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include <string>

namespace Steinberg {

//------------------------------------------------------------------------
/** Reads entire file content
\ingroup sdkBase

Returns entire file content at the given path
\endcode
*/
std::string readFile (const std::string& path);

//------------------------------------------------------------------------
} // namespace Steinberg
