//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstbus.cpp
// Created by  : Steinberg, 03/2008
// Description : VST Bus Implementation
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
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

#include "vstbus.h"

namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
// Bus Implementation
//------------------------------------------------------------------------
Bus::Bus (const TChar* _name, BusType _busType, int32 _flags)
: name (_name), busType (_busType), flags (_flags), active (false)
{
}

//------------------------------------------------------------------------
bool Bus::getInfo (BusInfo& info)
{
	name.copyTo16 (info.name, 0, str16BufferSize (info.name) - 1);
	info.busType = busType;
	info.flags = flags;
	return true;
}

//------------------------------------------------------------------------
// EventBus Implementation
//------------------------------------------------------------------------
EventBus::EventBus (const TChar* name, BusType busType, int32 flags, int32 channelCount)
: Bus (name, busType, flags), channelCount (channelCount)
{
}

//------------------------------------------------------------------------
bool EventBus::getInfo (BusInfo& info)
{
	info.channelCount = channelCount;
	return Bus::getInfo (info);
}

//------------------------------------------------------------------------
// AudioBus Implementation
//------------------------------------------------------------------------
AudioBus::AudioBus (const TChar* name, BusType busType, int32 flags, SpeakerArrangement arr)
: Bus (name, busType, flags), speakerArr (arr)
{
}

//------------------------------------------------------------------------
bool AudioBus::getInfo (BusInfo& info)
{
	info.channelCount = SpeakerArr::getChannelCount (speakerArr);
	return Bus::getInfo (info);
}

//------------------------------------------------------------------------
// BusList Implementation
//------------------------------------------------------------------------
BusList::BusList (MediaType type, BusDirection dir) : type (type), direction (dir)
{
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
