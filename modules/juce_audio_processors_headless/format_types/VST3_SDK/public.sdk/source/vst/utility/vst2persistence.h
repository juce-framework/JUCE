//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST3 SDK
// Filename    : public.sdk/source/vst/utility/vst2persistence.h
// Created by  : Steinberg, 12/2019
// Description : vst2 persistence helper
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/utility/optional.h"
#include "pluginterfaces/base/ibstream.h"
#include <string>
#include <vector>

//------------------------------------------------------------------------
namespace VST3 {

//------------------------------------------------------------------------
using Vst2xChunk = std::vector<int8_t>;

//------------------------------------------------------------------------
/** structure holding the content of a vst2 fxp format stream
 *
 *	either the values member is valid or the chunk member but not both
 */
struct Vst2xProgram
{
	using ProgramValues = std::vector<float>;
	ProgramValues values;
	Vst2xChunk chunk;
	int32_t fxUniqueID {0};
	int32_t fxVersion {0};
	std::string name;
};

//------------------------------------------------------------------------
/** structure holding the content of a vst2 fxb format stream
 *
 *	either the programs member is valid or the chunk member but not both
 */
struct Vst2xState
{
	using Programs = std::vector<Vst2xProgram>;
	Programs programs;
	Vst2xChunk chunk;

	int32_t fxUniqueID {0};
	int32_t fxVersion {0};
	int32_t currentProgram {0};
	bool isBypassed {false};
};

//------------------------------------------------------------------------
/** Try loading the state from an old vst2 fxb format stream
 *
 *	If successfully loaded, the state has either a chunk or programs but not both
 *	The Vst2xState::isBypassed boolean will be set if a Steinberg host has written the state into a
 *	project and the plug-in was bypassed.
 *
 *	@param stream the input stream
 *	@param vst2xUniqueID vst2 unique id expected to be stored in the stream [optional]. If present
 *						the fxb unique id header entry must be the same as this otherwise the
 *						return value is empty.
 *	@return on success the optional has a Vst2xState object with the data
 */
Optional<Vst2xState> tryVst2StateLoad (Steinberg::IBStream& stream,
                                       Optional<int32_t> vst2xUniqueID = {}) noexcept;

//------------------------------------------------------------------------
/** Write a vst2 fxb stream
 *
 *	Writes the state into stream as a vst2 fxb format
 *
 *	@param state the state which should be written
 *	@param stream the stream where the state should be written into
 *	@param writeBypassState write extra chunk with bypass state
 *	@return true on success
 */
bool writeVst2State (const Vst2xState& state, Steinberg::IBStream& stream,
                     bool writeBypassState = true) noexcept;

//------------------------------------------------------------------------
/** Try loading the state from on old vst2 fxp format stream
 *
 *	If successfully loaded, the program has either a chunk or plain values but not both
 *
 *	@param stream the input stream
 *	@param vst2xUniqueID vst2 unique id expected to be stored in the stream
 *	@return on success the optional has a Vst2xProgram object with the data
 */
Optional<Vst2xProgram> tryVst2ProgramLoad (Steinberg::IBStream& stream,
                                           Optional<int32_t> vst2xUniqueID) noexcept;

//------------------------------------------------------------------------
} // VST3
