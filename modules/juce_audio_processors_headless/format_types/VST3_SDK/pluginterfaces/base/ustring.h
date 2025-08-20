//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Helpers
// Filename    : pluginterfaces/base/ustring.h
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

#pragma once

#include "ftypes.h"

//------------------------------------------------------------------------
namespace Steinberg {

//------------------------------------------------------------------------
/** UTF-16 string class without buffer management.
 Note: that some characters are encoded in 2 UTF16 code units (surrogate pair),
 this means that getLength returns the number of code unit, not the count of character! */
class UString
{
public:
//------------------------------------------------------------------------
	/** Construct from UTF-16 string, size is in code unit (count of char16) */
	UString (char16* buffer, int32 size) : thisBuffer (buffer), thisSize (size) {}

	/** returns buffer size */
	int32 getSize () const { return thisSize; }
	
	/** cast to char16* */
	operator const char16* () const { return thisBuffer; }

	/** Returns length of string (in code unit). Note this is not the count of character! */
	int32 getLength () const;

	/** Copy from UTF-16 buffer (srcSize is in code unit (count of char16)). */
	UString& assign (const char16* src, int32 srcSize = -1);

	/** Append UTF-16 buffer (srcSize is in code unit (count of char16)). */
	UString& append (const char16* src, int32 srcSize = -1);

	/** Copy to UTF-16 buffer (dstSize is in code unit (count of char16)). */
	const UString& copyTo (char16* dst, int32 dstSize) const;

	/** Copy from ASCII string (srcSize is in code unit (count of char16)). */
	UString& fromAscii (const char* src, int32 srcSize = -1);
	UString& assign (const char* src, int32 srcSize = -1) { return fromAscii (src, srcSize); }

	/** Copy to ASCII string. */
	const UString& toAscii (char* dst, int32 dstSize) const;

	/** Scan integer from string. */
	bool scanInt (int64& value) const;

	/** Print integer to string. */
	bool printInt (int64 value);

	/** Scan float from string. */
	bool scanFloat (double& value) const;

	/** Print float to string. */
	bool printFloat (double value, int32 precision = 4);
//------------------------------------------------------------------------
protected:
	char16* thisBuffer;
	int32 thisSize; ///< size in code unit (not in byte!)
};

//------------------------------------------------------------------------
/** UTF-16 string with fixed buffer size.
 */
template <int32 maxSize>
class UStringBuffer : public UString
{
public:
//------------------------------------------------------------------------
	UStringBuffer () : UString (data, maxSize) { data[0] = 0; }

	/** Construct from UTF-16 string. */
	UStringBuffer (const char16* src, int32 srcSize = -1) : UString (data, maxSize)
	{
		data[0] = 0;
		if (src)
			assign (src, srcSize);
	}

	/** Construct from ASCII string. */
	UStringBuffer (const char* src, int32 srcSize = -1) : UString (data, maxSize)
	{
		data[0] = 0;
		if (src)
			fromAscii (src, srcSize);
	}
//------------------------------------------------------------------------
protected:
	char16 data[maxSize];
};

//------------------------------------------------------------------------
typedef UStringBuffer<128> UString128; ///< 128 character UTF-16 string
typedef UStringBuffer<256> UString256; ///< 256 character UTF-16 string
} // namespace Steinberg

//------------------------------------------------------------------------
#define USTRING(asciiString) Steinberg::UString256 (asciiString)
#define USTRINGSIZE(var) (sizeof (var) / sizeof (Steinberg::char16))

//------------------------------------------------------------------------
