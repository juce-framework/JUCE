//-----------------------------------------------------------------------------
// Project     : VST SDK
// Flags       : clang-format SMTGSequencer
//
// Category    : readfile
// Filename    : public.sdk/source/common/readfile.cpp
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

#include "readfile.h"

#include "pluginterfaces/base/fplatform.h"

#if SMTG_OS_WINDOWS
#include "commonstringconvert.h"
#endif

#include <fstream>

#if !SMTG_CPP17
#include <sstream>
#endif

namespace Steinberg {

//------------------------------------------------------------------------
std::string readFile (const std::string& path)
{
#if SMTG_OS_WINDOWS
	auto u16Path = StringConvert::convert (path);
	std::ifstream file (reinterpret_cast<const wchar_t*> (u16Path.data ()),
	                    std::ios_base::in | std::ios_base::binary);
#else
	std::ifstream file (path, std::ios_base::in | std::ios_base::binary);
#endif
	if (!file.is_open ())
		return {};

#if SMTG_CPP17
	 auto size = file.seekg (0, std::ios_base::end).tellg ();
	 file.seekg (0, std::ios_base::beg);
	 std::string data;
	 data.resize (size);
	 file.read (data.data (), data.size ());
	 if (file.bad ())
		return {};
	 return data;
#else
	std::stringstream buffer;
	buffer << file.rdbuf ();
	return buffer.str ();
#endif // SMTG_CPP17
}

//------------------------------------------------------------------------
} // namespace Steinberg
