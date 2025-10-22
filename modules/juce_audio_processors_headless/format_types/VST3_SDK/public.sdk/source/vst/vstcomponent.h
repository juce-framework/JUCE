//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstcomponent.h
// Created by  : Steinberg, 04/2005
// Description : Basic VST Plug-in Implementation
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
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
	void setControllerClass (const TUID& cid) { controllerClass = FUID::fromTUID (cid); }

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
