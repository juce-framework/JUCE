//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/conststringtable.cpp
// Created by  : Steinberg, 09/2007
// Description : constant unicode string table
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#include "conststringtable.h"
#include <cstring>
#include <map>

namespace Steinberg {

static std::map<const char8*, char16*>* stringMap;
static std::map<const char8, char16>* charMap;

static char16* generateUTF16 (const char8* str);

//----------------------------------------------------------------------------
ConstStringTable* ConstStringTable::instance ()
{
	static ConstStringTable stringTable;
	return &stringTable;
}

//----------------------------------------------------------------------------
const char16* ConstStringTable::getString (const char8* str) const
{
	std::map<const char8*, char16*>::iterator iter = stringMap->find (str);
	if (iter != stringMap->end ())
		return iter->second;
	char16* uStr = generateUTF16 (str);
	stringMap->insert (std::make_pair (str, uStr));
	return uStr;
}

//----------------------------------------------------------------------------
const char16 ConstStringTable::getString (const char8 str) const
{
	std::map<const char8, char16>::iterator iter = charMap->find (str);
	if (iter != charMap->end ())
		return iter->second;
	char16 uStr = 0;
#if BYTEORDER == kBigEndian
	char8* puStr = (char8*)&uStr;
	puStr[1] = str;
#else
	uStr = str;
#endif
	charMap->insert (std::make_pair (str, uStr));
	return uStr;
}

//----------------------------------------------------------------------------
ConstStringTable::ConstStringTable ()
{
	stringMap = new std::map<const char8*, char16*>;
	charMap = new std::map<const char8, char16>;
}

//----------------------------------------------------------------------------
ConstStringTable::~ConstStringTable ()
{
	// free out allocated strings
	{
		std::map<const char8*, char16*>::iterator iter = stringMap->begin ();
		while (iter != stringMap->end ())
		{
			delete[] iter->second;
			iter++;
		}
	} // delete iterator on map before deleting the map

	delete stringMap;
	delete charMap;
}

//----------------------------------------------------------------------------
char16* generateUTF16 (const char8* str)
{
	int32 len = (int32)strlen (str);
	char16* result = new char16[len + 1];
	for (int32 i = 0; i < len; i++)
	{
#if BYTEORDER == kBigEndian
		char8* pChr = (char8*)&result[i];
		pChr[0] = 0;
		pChr[1] = str[i];
#else
		result[i] = str[i];
#endif
	}
	result[len] = 0;
	return result;
}
//------------------------------------------------------------------------
} // namespace Steinberg
