//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstbus.h
// Created by  : Steinberg, 03/2008
// Description : VST Bus Implementation
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

#pragma once

#include "base/source/fobject.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

#include <string>
#if SMTG_CPP_17
#include <string_view>
#endif
#include <vector>

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
/** Basic Bus object.
\ingroup vstClasses 
*/
class Bus: public FObject
{
public:
//------------------------------------------------------------------------
	/** Constructor. */
	Bus (const TChar* name, BusType busType, int32 flags);

	/** Returns true if the bus is active. */
	TBool isActive () const { return active; }

	/** Activates the bus. */
	void setActive (TBool state) { active = state; }

#if SMTG_CPP_17
	/** Sets a new name for this bus. */
	void setName (std::u16string_view newName) { name = newName; }
#else
	/** Sets a new name for this bus. */
	void setName (const std::u16string& newName) { name = newName; }
#endif

	/** Sets a new busType for this bus. */
	void setBusType (BusType newBusType) { busType = newBusType; }

	/** Sets a new flags for this bus. */
	void setFlags (uint32 newFlags) { flags = newFlags; }

	/** Gets the BusInfo of this bus. */
	virtual bool getInfo (BusInfo&);

	OBJ_METHODS (Vst::Bus, FObject)
//------------------------------------------------------------------------
protected:
	std::u16string name;		///< name
	BusType busType;			///< kMain or kAux, see \ref BusTypes
	int32 flags;				///< flags, see \ref BusInfo::BusFlags
	TBool active;				///< activation state
};

//------------------------------------------------------------------------
/** Description of an Event Bus.
\ingroup vstClasses
*/
class EventBus: public Bus
{
public:
//------------------------------------------------------------------------
	/** Constructor of an EventBus. */
	EventBus (const TChar* name, BusType busType, int32 flags, int32 channelCount);

	//---from Bus-------
	/** Gets the BusInfo associated to this Event bus. */
	bool getInfo (BusInfo& info) SMTG_OVERRIDE;

	OBJ_METHODS (Vst::EventBus, Vst::Bus);

//------------------------------------------------------------------------
protected:
	int32 channelCount;
};

//------------------------------------------------------------------------
/** Description of an Audio Bus.
\ingroup vstClasses 
*/
class AudioBus: public Bus
{
public:
//------------------------------------------------------------------------
	AudioBus (const TChar* name, BusType busType, int32 flags, SpeakerArrangement arr);

	/** Gets the speaker arrangement defining this Audio bus. */
	SpeakerArrangement getArrangement () const { return speakerArr; }

	/** Sets the speaker arrangement defining this Audio bus. */
	void setArrangement (const SpeakerArrangement& arr) { speakerArr = arr; }

	//---from Bus---------------------
	/** Gets the BusInfo associated to this Audio bus. */
	bool getInfo (BusInfo& info) SMTG_OVERRIDE;

	OBJ_METHODS (Vst::AudioBus, Vst::Bus)

//------------------------------------------------------------------------
protected:
	SpeakerArrangement speakerArr;
};

//------------------------------------------------------------------------
/** List of Busses.
\ingroup vstClasses 
*/
class BusList: public FObject, public std::vector<IPtr<Vst::Bus> >
{
public:
//------------------------------------------------------------------------
	/** Constructor. */
	BusList (MediaType type, BusDirection dir);

	/** Returns the BusList Type. See \ref MediaType */
	MediaType getType () const { return type; }

	/** Returns the BusList direction. See \ref BusDirection */
	BusDirection getDirection () const { return direction; }

	OBJ_METHODS (Vst::BusList, FObject);

//------------------------------------------------------------------------
protected:
	MediaType type;
	BusDirection direction;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
