//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstcomponentbase.cpp
// Created by  : Steinberg, 05/2005
// Description : Base class for VST Component and Edit Controller
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#include "vstcomponentbase.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/funknownimpl.h"

namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
// ComponentBase Implementation
//------------------------------------------------------------------------
ComponentBase::ComponentBase ()
{
}

//------------------------------------------------------------------------
ComponentBase::~ComponentBase ()
{
}

//------------------------------------------------------------------------
tresult PLUGIN_API ComponentBase::initialize (FUnknown* context)
{
	// check if already initialized
	if (hostContext)
		return kResultFalse;

	hostContext = context;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ComponentBase::terminate ()
{
	// release host interfaces
	hostContext = nullptr;

	// in case host did not disconnect us,
	// release peer now
	if (peerConnection)
	{
		peerConnection->disconnect (this);
		peerConnection = nullptr;
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ComponentBase::connect (IConnectionPoint* other)
{
	if (!other)
		return kInvalidArgument;

	// check if already connected
	if (peerConnection)
		return kResultFalse;

	peerConnection = other;
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ComponentBase::disconnect (IConnectionPoint* other)
{
	if (peerConnection && other == peerConnection)
	{
		peerConnection = nullptr;
		return kResultOk;
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ComponentBase::notify (IMessage* message)
{
	if (!message)
		return kInvalidArgument;

	if (FIDStringsEqual (message->getMessageID (), "TextMessage"))
	{
		TChar string[256] = {0};
		if (message->getAttributes ()->getString ("Text", string, sizeof (string)) == kResultOk)
		{
			String tmp (string);
			tmp.toMultiByte (kCP_Utf8);
			return receiveText (tmp.text8 ());
		}
	}

	return kResultFalse;
}

//------------------------------------------------------------------------
IMessage* ComponentBase::allocateMessage () const
{
	if (auto hostApp = U::cast<IHostApplication> (hostContext))
		return Vst::allocateMessage (hostApp);
	return nullptr;
}

//------------------------------------------------------------------------
tresult ComponentBase::sendMessage (IMessage* message) const
{
	if (message != nullptr && getPeer () != nullptr)
		return getPeer ()->notify (message);
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult ComponentBase::sendTextMessage (const char8* text) const
{
	if (auto msg = owned (allocateMessage ()))
	{
		msg->setMessageID ("TextMessage");
		String tmp (text, kCP_Utf8);
		if (tmp.length () >= 256)
			tmp.remove (255);
		msg->getAttributes ()->setString ("Text", tmp.text16 ());
		return sendMessage (msg);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult ComponentBase::sendMessageID (const char8* messageID) const
{
	if (auto msg = owned (allocateMessage ()))
	{
		msg->setMessageID (messageID);
		return sendMessage (msg);
	}
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult ComponentBase::receiveText (const char8* /*text*/)
{
	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
