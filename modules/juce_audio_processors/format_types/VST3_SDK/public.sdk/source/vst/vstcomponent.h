//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstcomponent.h
// Created by  : Steinberg, 04/2005
// Description : Basic VST Plug-in Implementation
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2021, Steinberg Media Technologies GmbH, All Rights Reserved
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

#include "public.sdk/source/vst/vstcomponentbase.h"
#include "public.sdk/source/vst/vstbus.h"
#include "pluginterfaces/vst/ivstcomponent.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
/** Default implementation for a VST 3 Component.
\ingroup vstClasses
Can be used as base class for a VST 3 component implementation.
*/
class Component : public ComponentBase, public IComponent
{
public:
//------------------------------------------------------------------------
	/** Constructor */
	Component ();

	//---Internal Methods-------
	/** Sets the controller Class ID associated to its component. */
	void setControllerClass (const FUID& cid) { controllerClass = cid; }

	/** Removes all Audio Busses. */
	tresult removeAudioBusses ();

	/** Removes all Event Busses. */
	tresult removeEventBusses ();

	/** Renames a specific bus. Do not forget to inform the host about this (see \ref
	 * IComponentHandler::restartComponent (kIoTitlesChanged)). */
	tresult renameBus (MediaType type, BusDirection dir, int32 index, const String128 newName);

	//---from IComponent--------
	tresult PLUGIN_API getControllerClassId (TUID classID) SMTG_OVERRIDE;
	tresult PLUGIN_API setIoMode (IoMode mode) SMTG_OVERRIDE;
	int32 PLUGIN_API getBusCount (MediaType type, BusDirection dir) SMTG_OVERRIDE;
	tresult PLUGIN_API getBusInfo (MediaType type, BusDirection dir, int32 index, BusInfo& info) SMTG_OVERRIDE;
	tresult PLUGIN_API getRoutingInfo (RoutingInfo& inInfo, RoutingInfo& outInfo) SMTG_OVERRIDE;
	tresult PLUGIN_API activateBus (MediaType type, BusDirection dir, int32 index, TBool state) SMTG_OVERRIDE;
	tresult PLUGIN_API setActive (TBool state) SMTG_OVERRIDE;
	tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;

	//---from ComponentBase------
	tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;
	tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	//---Interface---------
	OBJ_METHODS (Component, ComponentBase)
	DEFINE_INTERFACES
		DEF_INTERFACE (IComponent)
	END_DEFINE_INTERFACES (ComponentBase)
	REFCOUNT_METHODS (ComponentBase)

//------------------------------------------------------------------------
protected:
	FUID controllerClass;
	BusList audioInputs;
	BusList audioOutputs;
	BusList eventInputs;
	BusList eventOutputs;

	BusList* getBusList (MediaType type, BusDirection dir);
	tresult removeAllBusses ();
};

//------------------------------------------------------------------------
// some Helper functions....
//------------------------------------------------------------------------

/** Gets the channel index of a given speaker in a arrangement, returns kResultFalse if speaker not
 * part of the arrangement else returns kResultTrue. */
tresult getSpeakerChannelIndex (SpeakerArrangement arrangement, uint64 speaker, int32& channel);

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
