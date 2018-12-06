//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/typesizecheck.h
// Created by  : Steinberg, 08/2018
// Description : Compile time type size check macro
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/fplatform.h"

#if SMTG_CPP11
/** Check the size of a structure depending on compilation platform
 *	Used to check that structure sizes don't change between SDK releases.
 */
#define SMTG_TYPE_SIZE_CHECK(Type, Platform64Size, MacOS32Size, Win32Size, Linux32Size)                         \
	namespace {                                                                                                 \
	template <typename Type, size_t w, size_t x, size_t y, size_t z>                                            \
	struct SizeCheck##Type                                                                                      \
	{                                                                                                           \
		constexpr SizeCheck##Type ()                                                                            \
		{                                                                                                       \
			static_assert (sizeof (Type) == (SMTG_PLATFORM_64 ? w : SMTG_OS_MACOS ? x : SMTG_OS_LINUX ? z : y), \
			               "Struct Size Error: " #Type);                                                        \
		}                                                                                                       \
	};                                                                                                          \
	static constexpr SizeCheck##Type<Type, Platform64Size, MacOS32Size, Win32Size, Linux32Size> instance##Type; \
	}

#else
// need static_assert
#define SMTG_TYPE_SIZE_CHECK(Type, Platform64Size, MacOS32Size, Win32Size, Linux32Size)
#endif

