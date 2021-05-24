//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstcomponentbase.h
// Created by  : Steinberg, 05/2005
// Description : Base class for Component and Edit Controller
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

#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "base/source/fobject.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
/** Base class for VST 3 Component and Edit Controller.
\ingroup vstClasses 
*/
class ComponentBase: public FObject,
					 public IPluginBase,
	                 public IConnectionPoint
{
public:
//------------------------------------------------------------------------
	ComponentBase ();
	~ComponentBase () override;

	//--- Internal Methods------
	/** Returns the hostContext (set by the host during initialize call). */
	FUnknown* getHostContext () const { return hostContext; }

	/** Returns the peer for the messaging communication (you can only use IConnectionPoint::notify
	 * for communicate between peers, do not try to cast peerConnection. */
	IConnectionPoint* getPeer () const { return peerConnection; }

	/** Allocates a message instance (do not forget to release it). */
	IMessage* allocateMessage () const;

	/** Sends the given message to the peer. */
	tresult sendMessage (IMessage* message) const;

	/** Sends a simple text message to the peer (max 255 characters).
	Text is interpreted as UTF-8.	 */
	tresult sendTextMessage (const char8* text) const;

	/** Sends a message with a given ID without any other payload. */
	tresult sendMessageID (const char8* messageID) const;

	/** Receives a simple text message from the peer (max 255 characters). Text is UTF-8 encoded. */
	virtual tresult receiveText (const char8* text);

	//---from IPluginBase------
	tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;
	tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	//---from IConnectionPoint-----------
	tresult PLUGIN_API connect (IConnectionPoint* other) SMTG_OVERRIDE;
	tresult PLUGIN_API disconnect (IConnectionPoint* other) SMTG_OVERRIDE;
	tresult PLUGIN_API notify (IMessage* message) SMTG_OVERRIDE;

	//---Interface------
	OBJ_METHODS (ComponentBase, FObject)
	DEFINE_INTERFACES
		DEF_INTERFACE (IPluginBase)
		DEF_INTERFACE (IConnectionPoint)
	END_DEFINE_INTERFACES (FObject)
	REFCOUNT_METHODS (FObject)

//------------------------------------------------------------------------
protected:
	IPtr<FUnknown> hostContext;
	IPtr<IConnectionPoint> peerConnection;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
