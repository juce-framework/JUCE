//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Helpers
// Filename    : pluginterfaces/base/ustring.cpp
// Created by  : Steinberg, 12/2005
// Description : UTF-16 String class
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#include "ustring.h"

#if SMTG_OS_WINDOWS
#include <stdio.h>
#pragma warning (disable : 4996)

#elif SMTG_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>

#elif SMTG_OS_LINUX
#include <cstring>
#include <string>
#include <codecvt>
#include <sstream>
#include <locale>

#include <wctype.h>
#include <wchar.h>

#endif

//------------------------------------------------------------------------
namespace Steinberg {

//------------------------------------------------------------------------
#if SMTG_OS_LINUX

//------------------------------------------------------------------------
namespace {

using Converter = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>;

//------------------------------------------------------------------------
Converter& converter ()
{
	static Converter instance;
	return instance;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
#endif // SMTG_OS_LINUX

//------------------------------------------------------------------------
/** Copy strings of different character width. */
//------------------------------------------------------------------------
template <class TDstChar, class TSrcChar>
void StringCopy (TDstChar* dst, int32 dstSize, const TSrcChar* src, int32 srcSize = -1)
{
	int32 count = dstSize;
	if (srcSize >= 0 && srcSize < dstSize)
		count = srcSize;
	for (int32 i = 0; i < count; i++)
	{
		dst[i] = (TDstChar)src[i];
		if (src[i] == 0)
			break;
	}
	dst[dstSize - 1] = 0;
}

//------------------------------------------------------------------------
/** Find length of null-terminated string. */
//------------------------------------------------------------------------
template <class TSrcChar>
int32 StringLength (const TSrcChar* src, int32 srcSize = -1)
{
	if (srcSize == 0)
		return 0;
	int32 length = 0;
	while (src[length])
	{
		length++;
		if (srcSize > 0 && length >= srcSize)
			break;
	}
	return length;
}

//------------------------------------------------------------------------
// UString
//------------------------------------------------------------------------
int32 UString::getLength () const
{
	return StringLength<char16> (thisBuffer, thisSize);
}

//------------------------------------------------------------------------
UString& UString::assign (const char16* src, int32 srcSize)
{
	StringCopy<char16, char16> (thisBuffer, thisSize, src, srcSize);
	return *this;
}

//------------------------------------------------------------------------
UString& UString::append (const char16* src, int32 srcSize)
{
	int32 length = getLength ();
	StringCopy<char16, char16> (thisBuffer + length, thisSize - length, src, srcSize);
	return *this;
}

//------------------------------------------------------------------------
const UString& UString::copyTo (char16* dst, int32 dstSize) const
{
	StringCopy<char16, char16> (dst, dstSize, thisBuffer, thisSize);
	return *this;
}

//------------------------------------------------------------------------
UString& UString::fromAscii (const char* src, int32 srcSize)
{
	StringCopy<char16, char> (thisBuffer, thisSize, src, srcSize);
	return *this;
}

//------------------------------------------------------------------------
const UString& UString::toAscii (char* dst, int32 dstSize) const
{
	StringCopy<char, char16> (dst, dstSize, thisBuffer, thisSize);
	return *this;
}

//------------------------------------------------------------------------
bool UString::scanFloat (double& value) const
{
#if SMTG_OS_WINDOWS
	return swscanf ((const wchar_t*)thisBuffer, L"%lf", &value) != -1;

#elif TARGET_API_MAC_CARBON
	CFStringRef cfStr = CFStringCreateWithBytes (0, (const UInt8 *)thisBuffer, getLength () * 2, kCFStringEncodingUTF16, false);
	if (cfStr)
	{
		value = CFStringGetDoubleValue (cfStr);
		CFRelease (cfStr);
		return true;
	}
	return false;

#elif SMTG_OS_LINUX
	auto str = converter ().to_bytes (thisBuffer);
	return sscanf (str.data (), "%lf", &value) == 1;

#else
#warning Implement me
	// implement me!
	return false;
#endif
}

//------------------------------------------------------------------------
bool UString::printFloat (double value, int32 precision)
{
#if SMTG_OS_WINDOWS
	return swprintf ((wchar_t*)thisBuffer, L"%.*lf", precision, value) != -1;
#elif SMTG_OS_MACOS
	bool result = false;
	CFStringRef cfStr = CFStringCreateWithFormat (0, 0, CFSTR("%.*lf"), precision, value);
	if (cfStr)
	{
		memset (thisBuffer, 0, thisSize);
		CFRange range = {0, CFStringGetLength (cfStr)};
		CFStringGetBytes (cfStr, range, kCFStringEncodingUTF16, 0, false, (UInt8*)thisBuffer, thisSize, 0);
		CFRelease (cfStr);
		return true;
	}
	return result;
#elif SMTG_OS_LINUX
	auto utf8Buffer = reinterpret_cast<char*> (thisBuffer);
	auto len = snprintf (utf8Buffer, thisSize, "%.*lf", precision, value);
	if (len > 0)
	{
		auto utf16Buffer = reinterpret_cast<char16*> (thisBuffer);
		utf16Buffer[len] = 0;
		while (--len >= 0)
		{
			utf16Buffer[len] = utf8Buffer[len];
		}
		return true;
	}
	return false;
#else
#warning Implement me
	// implement me!
	return false;
#endif
}

//------------------------------------------------------------------------
bool UString::scanInt (int64& value) const
{
#if SMTG_OS_WINDOWS
	return swscanf ((const wchar_t*)thisBuffer, L"%I64d", &value) != -1;

#elif SMTG_OS_MACOS
	CFStringRef cfStr = CFStringCreateWithBytes (0, (const UInt8 *)thisBuffer, getLength () * 2, kCFStringEncodingUTF16, false);
	if (cfStr)
	{
		value = CFStringGetIntValue (cfStr);
		CFRelease (cfStr);
		return true;
	}
	return false;

#elif SMTG_OS_LINUX
	auto str = converter ().to_bytes (thisBuffer);
	return sscanf (str.data (), "%lld", &value) == 1;

#else
#warning Implement me
	// implement me!
	return false;
#endif
}

//------------------------------------------------------------------------
bool UString::printInt (int64 value)
{
#if SMTG_OS_WINDOWS
	return swprintf ((wchar_t*)thisBuffer, L"%I64d", value) != -1;

#elif SMTG_OS_MACOS
	CFStringRef cfStr = CFStringCreateWithFormat (0, 0, CFSTR("%lld"), value);
	if (cfStr)
	{
		memset (thisBuffer, 0, thisSize);
		CFRange range = {0, CFStringGetLength (cfStr)};
		CFStringGetBytes (cfStr, range, kCFStringEncodingUTF16, 0, false, (UInt8*)thisBuffer, thisSize, 0);
		CFRelease (cfStr);
		return true;
	}
	return false;
#elif SMTG_OS_LINUX
	auto utf8Buffer = reinterpret_cast<char*> (thisBuffer);
	auto len = snprintf (utf8Buffer, thisSize, "%lld", value);
	if (len > 0)
	{
		auto utf16Buffer = reinterpret_cast<char16*> (thisBuffer);
		utf16Buffer[len] = 0;
		while (--len >= 0)
		{
			utf16Buffer[len] = utf8Buffer[len];
		}
		return true;
	}
	return false;

#else
#warning Implement me
	// implement me!
	return false;
#endif
}
} // namespace Steinberg
