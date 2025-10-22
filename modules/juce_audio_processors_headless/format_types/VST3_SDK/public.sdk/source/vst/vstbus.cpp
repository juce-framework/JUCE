//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstbus.cpp
// Created by  : Steinberg, 03/2008
// Description : VST Bus Implementation
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
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
	memset (info.name, 0, sizeof (String128));
	name.copy (info.name, 128);
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
