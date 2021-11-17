//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstinitiids.cpp
// Created by  : Steinberg, 10/2009
// Description : Interface symbols file
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

#include "pluginterfaces/base/funknown.h"

#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstautomationstate.h"
#include "pluginterfaces/vst/ivstchannelcontextinfo.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/ivstinterappaudio.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivstmidilearn.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstparameterfunctionname.h"
#include "pluginterfaces/vst/ivstphysicalui.h"
#include "pluginterfaces/vst/ivstpluginterfacesupport.h"
#include "pluginterfaces/vst/ivstplugview.h"
#include "pluginterfaces/vst/ivstprefetchablesupport.h"
#include "pluginterfaces/vst/ivstrepresentation.h"
#include "pluginterfaces/vst/ivsttestplugprovider.h"
#include "pluginterfaces/vst/ivstunits.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//----VST 3.0--------------------------------
DEF_CLASS_IID (IComponent)
DEF_CLASS_IID (IAudioProcessor)
DEF_CLASS_IID (IUnitData)
DEF_CLASS_IID (IProgramListData)

DEF_CLASS_IID (IEditController)
DEF_CLASS_IID (IUnitInfo)

DEF_CLASS_IID (IConnectionPoint)

DEF_CLASS_IID (IComponentHandler)
DEF_CLASS_IID (IUnitHandler)

DEF_CLASS_IID (IParamValueQueue)
DEF_CLASS_IID (IParameterChanges)

DEF_CLASS_IID (IEventList)
DEF_CLASS_IID (IMessage)

DEF_CLASS_IID (IHostApplication)
DEF_CLASS_IID (IAttributeList)

//----VST 3.0.1--------------------------------
DEF_CLASS_IID (IMidiMapping)

//----VST 3.0.2--------------------------------
DEF_CLASS_IID (IParameterFinder)

//----VST 3.1----------------------------------
DEF_CLASS_IID (IComponentHandler2)
DEF_CLASS_IID (IEditController2)
DEF_CLASS_IID (IAudioPresentationLatency)
DEF_CLASS_IID (IVst3ToVst2Wrapper)
DEF_CLASS_IID (IVst3ToAUWrapper)

//----VST 3.5----------------------------------
DEF_CLASS_IID (INoteExpressionController)
DEF_CLASS_IID (IKeyswitchController)
DEF_CLASS_IID (IContextMenuTarget)
DEF_CLASS_IID (IContextMenu)
DEF_CLASS_IID (IComponentHandler3)
DEF_CLASS_IID (IEditControllerHostEditing)
DEF_CLASS_IID (IXmlRepresentationController)

//----VST 3.6----------------------------------
DEF_CLASS_IID (IInterAppAudioHost)
DEF_CLASS_IID (IInterAppAudioConnectionNotification)
DEF_CLASS_IID (IInterAppAudioPresetManager)
DEF_CLASS_IID (IStreamAttributes)

//----VST 3.6.5--------------------------------
DEF_CLASS_IID (ChannelContext::IInfoListener)
DEF_CLASS_IID (IPrefetchableSupport)
DEF_CLASS_IID (IUnitHandler2)
DEF_CLASS_IID (IAutomationState)

//----VST 3.6.8--------------------------------
DEF_CLASS_IID (IComponentHandlerBusActivation)
DEF_CLASS_IID (IVst3ToAAXWrapper)

//----VST 3.6.11--------------------------------
DEF_CLASS_IID (INoteExpressionPhysicalUIMapping)

//----VST 3.6.12--------------------------------
DEF_CLASS_IID (IMidiLearn)
DEF_CLASS_IID (IPlugInterfaceSupport)
DEF_CLASS_IID (IVst3WrapperMPESupport)

//----VST 3.6.13--------------------------------
DEF_CLASS_IID (ITestPlugProvider)

//----VST 3.7-----------------------------------
DEF_CLASS_IID (IParameterFunctionName)
DEF_CLASS_IID (IProcessContextRequirements)
DEF_CLASS_IID (IProgress)
DEF_CLASS_IID (ITestPlugProvider2)

//------------------------------------------------------------------------
} // Vst
} // Steinberg
