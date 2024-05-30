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
// LICENSE
// (c) 2023, Steinberg Media Technologies GmbH, All Rights Reserved
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

#include "readfile.h"

#include "pluginterfaces/base/fplatform.h"

#if SMTG_OS_WINDOWS
#include "public.sdk/source/vst/utility/stringconvert.h"
#endif

#include <fstream>
#include <sstream>

namespace Steinberg {

//------------------------------------------------------------------------
std::string readFile (const std::string& path)
{
#if SMTG_OS_WINDOWS
	auto u16Path = VST3::StringConvert::convert (path);
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
#endif
}

//------------------------------------------------------------------------
} // namespace Steinberg
