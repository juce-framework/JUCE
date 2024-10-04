//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstcomponent.cpp
// Created by  : Steinberg, 04/2005
// Description : Basic VST Plug-in Implementation
//
//-----------------------------------------------------------------------------
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

#include "vstcomponent.h"

namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
// Component Implementation
//------------------------------------------------------------------------
Component::Component ()
: audioInputs (kAudio, kInput)
, audioOutputs (kAudio, kOutput)
, eventInputs (kEvent, kInput)
, eventOutputs (kEvent, kOutput)
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::initialize (FUnknown* context)
{
	return ComponentBase::initialize (context);
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::terminate ()
{
	// remove all busses
	removeAllBusses ();

	return ComponentBase::terminate ();
}

//------------------------------------------------------------------------
BusList* Component::getBusList (MediaType type, BusDirection dir)
{
	if (type == kAudio)
		return dir == kInput ? &audioInputs : &audioOutputs;
	if (type == kEvent)
		return dir == kInput ? &eventInputs : &eventOutputs;
	return nullptr;
}

//------------------------------------------------------------------------
tresult Component::removeAudioBusses ()
{
	audioInputs.clear ();
	audioOutputs.clear ();

	return kResultOk;
}

//------------------------------------------------------------------------
tresult Component::removeEventBusses ()
{
	eventInputs.clear ();
	eventOutputs.clear ();

	return kResultOk;
}

//------------------------------------------------------------------------
tresult Component::removeAllBusses ()
{
	removeAudioBusses ();
	removeEventBusses ();

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::getControllerClassId (TUID classID)
{
	if (controllerClass.isValid ())
	{
		controllerClass.toTUID (classID);
		return kResultTrue;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::setIoMode (IoMode /*mode*/)
{
	return kNotImplemented;
}

//------------------------------------------------------------------------
int32 PLUGIN_API Component::getBusCount (MediaType type, BusDirection dir)
{
	BusList* busList = getBusList (type, dir);
	return busList ? static_cast<int32> (busList->size ()) : 0;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::getBusInfo (MediaType type, BusDirection dir, int32 index,
                                          BusInfo& info)
{
	if (index < 0)
		return kInvalidArgument;
	BusList* busList = getBusList (type, dir);
	if (busList == nullptr)
		return kInvalidArgument;
	if (index >= static_cast<int32> (busList->size ()))
		return kInvalidArgument;

	Bus* bus = busList->at (index);
	info.mediaType = type;
	info.direction = dir;
	if (bus->getInfo (info))
		return kResultTrue;
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::getRoutingInfo (RoutingInfo& /*inInfo*/, RoutingInfo& /*outInfo*/)
{
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::activateBus (MediaType type, BusDirection dir, int32 index,
                                           TBool state)
{
	if (index < 0)
		return kInvalidArgument;
	BusList* busList = getBusList (type, dir);
	if (busList == nullptr)
		return kInvalidArgument;
	if (index >= static_cast<int32> (busList->size ()))
		return kInvalidArgument;

	Bus* bus = busList->at (index);
	bus->setActive (state);
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::setActive (TBool /*state*/)
{
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::setState (IBStream* /*state*/)
{
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult PLUGIN_API Component::getState (IBStream* /*state*/)
{
	return kNotImplemented;
}

//------------------------------------------------------------------------
tresult Component::renameBus (MediaType type, BusDirection dir, int32 index,
                              const String128 newName)
{
	if (index < 0)
		return kInvalidArgument;
	BusList* busList = getBusList (type, dir);
	if (busList == nullptr)
		return kInvalidArgument;
	if (index >= static_cast<int32> (busList->size ()))
		return kInvalidArgument;

	Bus* bus = busList->at (index);
	bus->setName (newName);
	return kResultTrue;
}

//------------------------------------------------------------------------
// Helpers Implementation
//------------------------------------------------------------------------
tresult getSpeakerChannelIndex (SpeakerArrangement arrangement, uint64 speaker, int32& channel)
{
	channel = SpeakerArr::getSpeakerIndex (speaker, arrangement);
	return (channel < 0) == true ? kResultFalse : kResultTrue;
}
} // namespace Vst
} // namespace Steinberg
