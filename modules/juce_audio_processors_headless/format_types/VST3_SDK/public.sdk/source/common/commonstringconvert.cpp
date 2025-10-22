//-----------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/common/commonstringconvert.cpp
// Created by  : Steinberg, 07/2024
// Description : c++11 unicode string convert functions
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#include "commonstringconvert.h"

#include <codecvt>
#include <istream>
#include <locale>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

//------------------------------------------------------------------------
namespace Steinberg {
namespace StringConvert {

//------------------------------------------------------------------------
namespace {

#if defined(_MSC_VER) && _MSC_VER >= 1900
#define USE_WCHAR_AS_UTF16TYPE
using UTF16Type = wchar_t;
#else
using UTF16Type = char16_t;
#endif

using Converter = std::wstring_convert<std::codecvt_utf8_utf16<UTF16Type>, UTF16Type>;

//------------------------------------------------------------------------
Converter& converter ()
{
	static Converter conv;
	return conv;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
std::u16string convert (const std::string& utf8Str)
{
#if defined(USE_WCHAR_AS_UTF16TYPE)
	auto wstr = converter ().from_bytes (utf8Str);
	return {wstr.data (), wstr.data () + wstr.size ()};
#else
	return converter ().from_bytes (utf8Str);
#endif
}

//------------------------------------------------------------------------
std::string convert (const std::u16string& str)
{
	return converter ().to_bytes (reinterpret_cast<const UTF16Type*> (str.data ()),
	                              reinterpret_cast<const UTF16Type*> (str.data () + str.size ()));
}

//------------------------------------------------------------------------
std::string convert (const char* str, uint32_t max)
{
	std::string result;
	if (str)
	{
		result.reserve (max);
		for (uint32_t i = 0; i < max; ++i, ++str)
		{
			if (*str == 0)
				break;
			result += *str;
		}
	}
	return result;
}

//------------------------------------------------------------------------
} // StringConvert
} // Steinberg

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
