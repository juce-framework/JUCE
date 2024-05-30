//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/hosting/hostclasses.h
// Created by  : Steinberg, 03/05/2008.
// Description : VST 3 hostclasses, example impl. for IHostApplication, IAttributeList and IMessage
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

#include "public.sdk/source/vst/hosting/pluginterfacesupport.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include <map>
#include <memory>
#include <string>

namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
/** Implementation's example of IHostApplication.
\ingroup hostingBase
*/
class HostApplication : public IHostApplication
{
public:
	HostApplication ();
	virtual ~HostApplication () noexcept {FUNKNOWN_DTOR}

	//--- IHostApplication ---------------
	tresult PLUGIN_API getName (String128 name) override;
	tresult PLUGIN_API createInstance (TUID cid, TUID _iid, void** obj) override;

	DECLARE_FUNKNOWN_METHODS

	PlugInterfaceSupport* getPlugInterfaceSupport () const { return mPlugInterfaceSupport; }

private:
	IPtr<PlugInterfaceSupport> mPlugInterfaceSupport;
};

//------------------------------------------------------------------------
/** Example, ready to use implementation of IAttributeList.
\ingroup hostingBase
*/
class HostAttributeList final : public IAttributeList
{
public:
	/** make a new attribute list instance */
	static IPtr<IAttributeList> make ();

	tresult PLUGIN_API setInt (AttrID aid, int64 value) override;
	tresult PLUGIN_API getInt (AttrID aid, int64& value) override;
	tresult PLUGIN_API setFloat (AttrID aid, double value) override;
	tresult PLUGIN_API getFloat (AttrID aid, double& value) override;
	tresult PLUGIN_API setString (AttrID aid, const TChar* string) override;
	tresult PLUGIN_API getString (AttrID aid, TChar* string, uint32 sizeInBytes) override;
	tresult PLUGIN_API setBinary (AttrID aid, const void* data, uint32 sizeInBytes) override;
	tresult PLUGIN_API getBinary (AttrID aid, const void*& data, uint32& sizeInBytes) override;

	virtual ~HostAttributeList () noexcept;
	DECLARE_FUNKNOWN_METHODS
private:
	HostAttributeList ();

	struct Attribute;
	std::map<std::string, Attribute> list;
};

//------------------------------------------------------------------------
/** Example implementation of IMessage.
\ingroup hostingBase
*/
class HostMessage final : public IMessage
{
public:
	HostMessage ();
	virtual ~HostMessage () noexcept;

	const char* PLUGIN_API getMessageID () override;
	void PLUGIN_API setMessageID (const char* messageID) override;
	IAttributeList* PLUGIN_API getAttributes () override;

	DECLARE_FUNKNOWN_METHODS
private:
	char* messageId {nullptr};
	IPtr<IAttributeList> attributeList;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
