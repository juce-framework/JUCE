//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/funknown.cpp
// Created by  : Steinberg, 01/2004
// Description : Basic Interface
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses. 
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#include "funknown.h"

#include "fstrdefs.h"

#include <stdio.h>

#if SMTG_OS_WINDOWS
#include <objbase.h>
#endif

#if SMTG_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#if defined(__GNUC__) && (__GNUC__ >= 4) && !__LP64__
// on 32 bit Mac OS X we can safely ignore the format warnings as sizeof(int) == sizeof(long)
#pragma GCC diagnostic ignored "-Wformat"
#endif

#endif

#if SMTG_OS_LINUX
#include <ext/atomicity.h>
#endif

namespace Steinberg {

//------------------------------------------------------------------------
#if COM_COMPATIBLE
#if SMTG_OS_WINDOWS
#define GuidStruct GUID
#else
struct GuidStruct
{
	uint32 Data1;
	uint16 Data2;
	uint16 Data3;
	uint8 Data4[8];
};
#endif
#endif

static void toString8 (char8* string, const char* data, int32 i1, int32 i2);
static void fromString8 (const char8* string, char* data, int32 i1, int32 i2);
static uint32 makeLong (uint8 b1, uint8 b2, uint8 b3, uint8 b4);

//------------------------------------------------------------------------
//  FUnknownPrivate
//------------------------------------------------------------------------
namespace FUnknownPrivate {
//------------------------------------------------------------------------
int32 PLUGIN_API atomicAdd (int32& var, int32 d)
{
#if SMTG_OS_WINDOWS
	return InterlockedExchangeAdd (&var, d) + d;
#elif SMTG_OS_MACOS
	return OSAtomicAdd32Barrier (d, (int32_t*)&var);
#elif SMTG_OS_LINUX
	__gnu_cxx::__atomic_add (&var, d);
	return var;
#else
#warning implement me!
	var += d;
	return var;
#endif
}
} // FUnknownPrivate

//------------------------------------------------------------------------
//	FUID implementation
//------------------------------------------------------------------------

FUID::FUID ()
{
	memset (data, 0, sizeof (TUID));
}

//------------------------------------------------------------------------
FUID::FUID (uint32 l1, uint32 l2, uint32 l3, uint32 l4)
{
	from4Int (l1, l2, l3, l4);
}

//------------------------------------------------------------------------
FUID::FUID (const FUID& f)
{
	memcpy (data, f.data, sizeof (TUID));
}

//------------------------------------------------------------------------
#if SMTG_CPP11_STDLIBSUPPORT
FUID::FUID (FUID&& other)
{
	memcpy (data, other.data, sizeof (TUID));
}

FUID& FUID::operator= (FUID&& other)
{
	memcpy (data, other.data, sizeof (TUID));
	return *this;
}
#endif

//------------------------------------------------------------------------
bool FUID::generate ()
{
#if SMTG_OS_WINDOWS
#if defined(_M_ARM64) || defined(_M_ARM)
	//#warning implement me!
	return false;
#else
	GUID guid;
	HRESULT hr = CoCreateGuid (&guid);
	switch (hr)
	{
		case RPC_S_OK: memcpy (data, (char*)&guid, sizeof (TUID)); return true;

		case RPC_S_UUID_LOCAL_ONLY:
		default: return false;
	}
#endif

#elif SMTG_OS_MACOS
	CFUUIDRef uuid = CFUUIDCreate (kCFAllocatorDefault);
	if (uuid)
	{
		CFUUIDBytes bytes = CFUUIDGetUUIDBytes (uuid);
		memcpy (data, (char*)&bytes, sizeof (TUID));
		CFRelease (uuid);
		return true;
	}
	return false;

#else
#warning implement me!
	return false;
#endif
}

//------------------------------------------------------------------------
bool FUID::isValid () const
{
	TUID nulluid = {0};

	return memcmp (data, nulluid, sizeof (TUID)) != 0;
}

//------------------------------------------------------------------------
FUID& FUID::operator= (const FUID& f)
{
	memcpy (data, f.data, sizeof (TUID));
	return *this;
}

//------------------------------------------------------------------------
void FUID::from4Int (uint32 l1, uint32 l2, uint32 l3, uint32 l4)
{
#if COM_COMPATIBLE
	data [0]  = (char)((l1 & 0x000000FF)      );
	data [1]  = (char)((l1 & 0x0000FF00) >>  8);
	data [2]  = (char)((l1 & 0x00FF0000) >> 16);
	data [3]  = (char)((l1 & 0xFF000000) >> 24);
	data [4]  = (char)((l2 & 0x00FF0000) >> 16);
	data [5]  = (char)((l2 & 0xFF000000) >> 24);
	data [6]  = (char)((l2 & 0x000000FF)      );
	data [7]  = (char)((l2 & 0x0000FF00) >>  8);
	data [8]  = (char)((l3 & 0xFF000000) >> 24);
	data [9]  = (char)((l3 & 0x00FF0000) >> 16);
	data [10] = (char)((l3 & 0x0000FF00) >>  8);
	data [11] = (char)((l3 & 0x000000FF)      );
	data [12] = (char)((l4 & 0xFF000000) >> 24);
	data [13] = (char)((l4 & 0x00FF0000) >> 16);
	data [14] = (char)((l4 & 0x0000FF00) >>  8);
	data [15] = (char)((l4 & 0x000000FF)      );
#else
	data [0]  = (char)((l1 & 0xFF000000) >> 24);
	data [1]  = (char)((l1 & 0x00FF0000) >> 16);
	data [2]  = (char)((l1 & 0x0000FF00) >>  8);
	data [3]  = (char)((l1 & 0x000000FF)      );
	data [4]  = (char)((l2 & 0xFF000000) >> 24);
	data [5]  = (char)((l2 & 0x00FF0000) >> 16);
	data [6]  = (char)((l2 & 0x0000FF00) >>  8);
	data [7]  = (char)((l2 & 0x000000FF)      );
	data [8]  = (char)((l3 & 0xFF000000) >> 24);
	data [9]  = (char)((l3 & 0x00FF0000) >> 16);
	data [10] = (char)((l3 & 0x0000FF00) >>  8);
	data [11] = (char)((l3 & 0x000000FF)      );
	data [12] = (char)((l4 & 0xFF000000) >> 24);
	data [13] = (char)((l4 & 0x00FF0000) >> 16);
	data [14] = (char)((l4 & 0x0000FF00) >>  8);
	data [15] = (char)((l4 & 0x000000FF)      );
#endif
}

//------------------------------------------------------------------------
void FUID::to4Int (uint32& d1, uint32& d2, uint32& d3, uint32& d4) const
{
	d1 = getLong1 ();
	d2 = getLong2 ();
	d3 = getLong3 ();
	d4 = getLong4 ();
}

//------------------------------------------------------------------------
uint32 FUID::getLong1 () const
{
#if COM_COMPATIBLE
	return makeLong (data[3], data[2], data[1], data[0]);
#else
	return makeLong (data[0], data[1], data[2], data[3]);
#endif
}

//------------------------------------------------------------------------
uint32 FUID::getLong2 () const
{
#if COM_COMPATIBLE
	return makeLong (data[5], data[4], data[7], data[6]);
#else
	return makeLong (data[4], data[5], data[6], data[7]);
#endif
}

//------------------------------------------------------------------------
uint32 FUID::getLong3 () const
{
#if COM_COMPATIBLE
	return makeLong (data[8], data[9], data[10], data[11]);
#else
	return makeLong (data[8], data[9], data[10], data[11]);
#endif
}

//------------------------------------------------------------------------
uint32 FUID::getLong4 () const
{
#if COM_COMPATIBLE
	return makeLong (data[12], data[13], data[14], data[15]);
#else
	return makeLong (data[12], data[13], data[14], data[15]);
#endif
}

//------------------------------------------------------------------------
void FUID::toString (char8* string) const
{
	if (!string)
		return;

#if COM_COMPATIBLE
	GuidStruct* g = (GuidStruct*)data;

	char8 s[17];
	Steinberg::toString8 (s, data, 8, 16);

	sprintf (string, "%08X%04X%04X%s", g->Data1, g->Data2, g->Data3, s);
#else
	Steinberg::toString8 (string, data, 0, 16);
#endif
}

//------------------------------------------------------------------------
bool FUID::fromString (const char8* string)
{
	if (!string || !*string)
		return false;
	if (strlen (string) != 32)
		return false;

#if COM_COMPATIBLE
	GuidStruct g;
	char s[33];

	strcpy (s, string);
	s[8] = 0;
	sscanf (s, "%x", &g.Data1);
	strcpy (s, string + 8);
	s[4] = 0;
	sscanf (s, "%hx", &g.Data2);
	strcpy (s, string + 12);
	s[4] = 0;
	sscanf (s, "%hx", &g.Data3);

	memcpy (data, &g, 8);
	Steinberg::fromString8 (string + 16, data, 8, 16);
#else
	Steinberg::fromString8 (string, data, 0, 16);
#endif

	return true;
}

//------------------------------------------------------------------------
bool FUID::fromRegistryString (const char8* string)
{
	if (!string || !*string)
		return false;
	if (strlen (string) != 38)
		return false;

// e.g. {c200e360-38c5-11ce-ae62-08002b2b79ef}

#if COM_COMPATIBLE
	GuidStruct g;
	char8 s[10];

	strncpy (s, string + 1, 8);
	s[8] = 0;
	sscanf (s, "%x", &g.Data1);
	strncpy (s, string + 10, 4);
	s[4] = 0;
	sscanf (s, "%hx", &g.Data2);
	strncpy (s, string + 15, 4);
	s[4] = 0;
	sscanf (s, "%hx", &g.Data3);
	memcpy (data, &g, 8);

	Steinberg::fromString8 (string + 20, data, 8, 10);
	Steinberg::fromString8 (string + 25, data, 10, 16);
#else
	Steinberg::fromString8 (string + 1, data, 0, 4);
	Steinberg::fromString8 (string + 10, data, 4, 6);
	Steinberg::fromString8 (string + 15, data, 6, 8);
	Steinberg::fromString8 (string + 20, data, 8, 10);
	Steinberg::fromString8 (string + 25, data, 10, 16);
#endif

	return true;
}

//------------------------------------------------------------------------
void FUID::toRegistryString (char8* string) const
{
// e.g. {c200e360-38c5-11ce-ae62-08002b2b79ef}

#if COM_COMPATIBLE
	GuidStruct* g = (GuidStruct*)data;

	char8 s1[5];
	Steinberg::toString8 (s1, data, 8, 10);

	char8 s2[13];
	Steinberg::toString8 (s2, data, 10, 16);

	sprintf (string, "{%08X-%04X-%04X-%s-%s}", g->Data1, g->Data2, g->Data3, s1, s2);
#else
	char8 s1[9];
	Steinberg::toString8 (s1, data, 0, 4);
	char8 s2[5];
	Steinberg::toString8 (s2, data, 4, 6);
	char8 s3[5];
	Steinberg::toString8 (s3, data, 6, 8);
	char8 s4[5];
	Steinberg::toString8 (s4, data, 8, 10);
	char8 s5[13];
	Steinberg::toString8 (s5, data, 10, 16);

	sprintf (string, "{%s-%s-%s-%s-%s}", s1, s2, s3, s4, s5);
#endif
}

//------------------------------------------------------------------------
void FUID::print (char8* string, int32 style) const
{
	if (!string) // no string: debug output
	{
		char8 str[128];
		print (str, style);

#if SMTG_OS_WINDOWS
		OutputDebugStringA (str);
		OutputDebugStringA ("\n");
#else
		fprintf (stdout, "%s\n", str);
#endif
		return;
	}

	uint32 l1, l2, l3, l4;
	to4Int (l1, l2, l3, l4);

	switch (style)
	{
		case kINLINE_UID:
			sprintf (string, "INLINE_UID (0x%08X, 0x%08X, 0x%08X, 0x%08X)", l1, l2, l3, l4);
			break;

		case kDECLARE_UID:
			sprintf (string, "DECLARE_UID (0x%08X, 0x%08X, 0x%08X, 0x%08X)", l1, l2, l3, l4);
			break;

		case kFUID:
			sprintf (string, "FUID (0x%08X, 0x%08X, 0x%08X, 0x%08X)", l1, l2, l3, l4);
			break;

		case kCLASS_UID:
		default:
			sprintf (string, "DECLARE_CLASS_IID (Interface, 0x%08X, 0x%08X, 0x%08X, 0x%08X)", l1,
			         l2, l3, l4);
			break;
	}
}

//------------------------------------------------------------------------
//  helpers
//------------------------------------------------------------------------
static uint32 makeLong (uint8 b1, uint8 b2, uint8 b3, uint8 b4)
{
	return (uint32 (b1) << 24) | (uint32 (b2) << 16) | (uint32 (b3) << 8) | uint32 (b4);
}

//------------------------------------------------------------------------
static void toString8 (char8* string, const char* data, int32 i1, int32 i2)
{
	*string = 0;
	for (int32 i = i1; i < i2; i++)
	{
		char8 s[3];
		sprintf (s, "%02X", (uint8)data[i]);
		strcat (string, s);
	}
}

//------------------------------------------------------------------------
static void fromString8 (const char8* string, char* data, int32 i1, int32 i2)
{
	for (int32 i = i1; i < i2; i++)
	{
		char8 s[3];
		s[0] = *string++;
		s[1] = *string++;
		s[2] = 0;

		int32 d = 0;
		sscanf (s, "%2x", &d);
		data[i] = (char)d;
	}
}

//------------------------------------------------------------------------
} // namespace Steinberg
