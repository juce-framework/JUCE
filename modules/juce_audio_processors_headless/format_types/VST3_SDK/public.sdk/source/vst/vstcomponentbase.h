//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstcomponentbase.h
// Created by  : Steinberg, 05/2005
// Description : Base class for Component and Edit Controller
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
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
