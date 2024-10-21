//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST3 SDK
// Filename    : public.sdk/source/vst/utility/vst2persistence.h
// Created by  : Steinberg, 12/2019
// Description : vst2 persistence helper
//
//------------------------------------------------------------------------
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
