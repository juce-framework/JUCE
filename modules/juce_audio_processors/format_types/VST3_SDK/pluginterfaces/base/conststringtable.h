//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/conststringtable.h
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

#pragma once

#include "ftypes.h"

namespace Steinberg {

//------------------------------------------------------------------------
/**	Constant unicode string table.
Used for conversion from ASCII string literals to char16.
*/
class ConstStringTable
{
public:
	static ConstStringTable* instance ();

	/** Returns a char16 string of a ASCII string literal*/
	const char16* getString (const char8* str) const;
	/** Returns a char16 character of a ASCII character */
	const char16 getString (const char8 str) const;

protected:
	ConstStringTable ();
	~ConstStringTable ();
};

//------------------------------------------------------------------------
} // namespace Steinberg

