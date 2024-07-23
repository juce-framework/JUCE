//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstcomponentbase.cpp
// Created by  : Steinberg, 05/2005
// Description : Base class for VST Component and Edit Controller
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

#include "vstcomponentbase.h"
#include "base/source/fstring.h"

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
	FUnknownPtr<IHostApplication> hostApp (hostContext);
	if (hostApp)
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
