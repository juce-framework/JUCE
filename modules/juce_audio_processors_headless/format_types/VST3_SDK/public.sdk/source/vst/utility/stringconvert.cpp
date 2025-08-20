//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/utility/stringconvert.cpp
// Created by  : Steinberg, 11/2014
// Description : c++11 unicode string convert functions
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
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "pluginterfaces/base/fplatform.h"

#if SMTG_OS_WINDOWS
#ifndef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif
#endif // SMTG_OS_WINDOWS

#include "public.sdk/source/vst/utility/stringconvert.h"
#include "public.sdk/source/common/commonstringconvert.h"

#include <codecvt>
#include <istream>
#include <locale>

namespace Steinberg {
namespace Vst {
namespace StringConvert {

//------------------------------------------------------------------------
namespace {

#if defined(_MSC_VER) && _MSC_VER >= 1900
#define USE_WCHAR_AS_UTF16TYPE
using UTF16Type = wchar_t;
#else
using UTF16Type = char16_t;
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

using Converter = std::wstring_convert<std::codecvt_utf8_utf16<UTF16Type>, UTF16Type>;

//------------------------------------------------------------------------
Converter& converter ()
{
	static Converter conv;
	return conv;
}

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
std::u16string convert (const std::string& utf8Str)
{
	return Steinberg::StringConvert::convert (utf8Str);
}

//------------------------------------------------------------------------
bool convert (const std::string& utf8Str, Steinberg::Vst::String128 str)
{
	return convert (utf8Str, str, 128);
}

//------------------------------------------------------------------------
bool convert (const std::string& utf8Str, Steinberg::Vst::TChar* str, uint32_t maxCharacters)
{
	auto ucs2 = convert (utf8Str);
	if (ucs2.length () < maxCharacters)
	{
		ucs2.copy (reinterpret_cast<char16_t*> (str), ucs2.length ());
		str[ucs2.length ()] = 0;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
std::string convert (const Steinberg::Vst::TChar* str)
{
	return converter ().to_bytes (reinterpret_cast<const UTF16Type*> (str));
}

//------------------------------------------------------------------------
std::string convert (const Steinberg::Vst::TChar* str, uint32_t max)
{
	std::string result;
	if (str)
	{
		Steinberg::Vst::TChar tmp[2] {};
		for (uint32_t i = 0; i < max; ++i, ++str)
		{
			tmp[0] = *str;
			if (tmp[0] == 0)
				break;
			result += convert (tmp);
		}
	}
	return result;
}

//------------------------------------------------------------------------
std::string convert (const std::u16string& str)
{
	return Steinberg::StringConvert::convert (str);
}

//------------------------------------------------------------------------
std::string convert (const char* str, uint32_t max)
{
	return Steinberg::StringConvert::convert (str, max);
}

//------------------------------------------------------------------------
} // StringConvert
} // Vst
} // Steinberg
