//-----------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST SDK
//
// Category    : stringconvert
// Filename    : public.sdk/source/common/commonstringconvert.h
// Created by  : Steinberg, 07/2024
// Description : read file routine
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

namespace Steinberg {
namespace StringConvert {

//------------------------------------------------------------------------
/**
 *  convert an UTF-8 string to an UTF-16 string
 *
 *  @param utf8Str UTF-8 string
 *
 *  @return UTF-16 string
 */
std::u16string convert (const std::string& utf8Str);

//------------------------------------------------------------------------
/**
 *  convert an UTF-16 string to an UTF-8 string
 *
 *  @param str UTF-16 string
 *
 *  @return UTF-8 string
 */
std::string convert (const std::u16string& str);

//------------------------------------------------------------------------
/**
 *  convert a ASCII string buffer to an UTF-8 string
 *
 *  @param str ASCII string buffer
 *	@param max maximum characters in str
 *
 *  @return UTF-8 string
 */
std::string convert (const char* str, uint32_t max);

//------------------------------------------------------------------------
/**
 *	convert a number to an UTF-16 string
 *
 *	@param value number
 *
 *	@return UTF-16 string
 */
template <typename NumberT>
std::u16string toString (NumberT value)
{
	auto u8str = std::to_string (value);
	return StringConvert::convert (u8str);
}

//------------------------------------------------------------------------
} // namespace StringConvert
} // namespace Steinberg
