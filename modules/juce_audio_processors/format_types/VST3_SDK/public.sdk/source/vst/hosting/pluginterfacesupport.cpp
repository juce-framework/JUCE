//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/hosting/pluginterfacesupport.cpp
// Created by  : Steinberg, 11/2018.
// Description : VST 3 hostclasses, example implementations for IPlugInterfaceSupport
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2023, Steinberg Media Technologies GmbH, All Rights Reserved
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

#include "pluginterfacesupport.h"

#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstunits.h"
#include "pluginterfaces/vst/ivstmessage.h"

#include <algorithm>

//-----------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
PlugInterfaceSupport::PlugInterfaceSupport ()
{
	FUNKNOWN_CTOR
	// add minimum set

	//---VST 3.0.0--------------------------------
	addPlugInterfaceSupported (IComponent::iid);
	addPlugInterfaceSupported (IAudioProcessor::iid);
	addPlugInterfaceSupported (IEditController::iid);
	addPlugInterfaceSupported (IConnectionPoint::iid);

	addPlugInterfaceSupported (IUnitInfo::iid);
	addPlugInterfaceSupported (IUnitData::iid);
	addPlugInterfaceSupported (IProgramListData::iid);

	//---VST 3.0.1--------------------------------
	addPlugInterfaceSupported (IMidiMapping::iid);

	//---VST 3.1----------------------------------
	addPlugInterfaceSupported (IEditController2::iid);

	/*
	//---VST 3.0.2--------------------------------
	addPlugInterfaceSupported (IParameterFinder::iid);

	//---VST 3.1----------------------------------
	addPlugInterfaceSupported (IAudioPresentationLatency::iid);

	//---VST 3.5----------------------------------
	addPlugInterfaceSupported (IKeyswitchController::iid);
	addPlugInterfaceSupported (IContextMenuTarget::iid);
	addPlugInterfaceSupported (IEditControllerHostEditing::iid);
	addPlugInterfaceSupported (IXmlRepresentationController::iid);
	addPlugInterfaceSupported (INoteExpressionController::iid);

	//---VST 3.6.5--------------------------------
	addPlugInterfaceSupported (ChannelContext::IInfoListener::iid);
	addPlugInterfaceSupported (IPrefetchableSupport::iid);
	addPlugInterfaceSupported (IAutomationState::iid);

	//---VST 3.6.11--------------------------------
	addPlugInterfaceSupported (INoteExpressionPhysicalUIMapping::iid);

	//---VST 3.6.12--------------------------------
	addPlugInterfaceSupported (IMidiLearn::iid);

	//---VST 3.7-----------------------------------
	addPlugInterfaceSupported (IProcessContextRequirements::iid);
	addPlugInterfaceSupported (IParameterFunctionName::iid);
	addPlugInterfaceSupported (IProgress::iid);
	*/
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugInterfaceSupport::isPlugInterfaceSupported (const TUID _iid)
{
	auto uid = FUID::fromTUID (_iid);
	if (std::find (mFUIDArray.begin (), mFUIDArray.end (), uid) != mFUIDArray.end ())
		return kResultTrue;
	return kResultFalse;
}

//-----------------------------------------------------------------------------
void PlugInterfaceSupport::addPlugInterfaceSupported (const TUID _iid)
{
	mFUIDArray.push_back (FUID::fromTUID (_iid));
}

//-----------------------------------------------------------------------------
bool PlugInterfaceSupport::removePlugInterfaceSupported (const TUID _iid)
{
	auto uid = FUID::fromTUID (_iid);
	auto it = std::find (mFUIDArray.begin (), mFUIDArray.end (), uid);
	if (it  == mFUIDArray.end ())
		return false;
	mFUIDArray.erase (it);
	return true;
}

IMPLEMENT_FUNKNOWN_METHODS (PlugInterfaceSupport, IPlugInterfaceSupport, IPlugInterfaceSupport::iid)

//-----------------------------------------------------------------------------
} // Vst
} // Steinberg
