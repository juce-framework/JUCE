//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/hosting/hostclasses.cpp
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

#include "hostclasses.h"
#include "public.sdk/source/vst/utility/stringconvert.h"

#include <algorithm>

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
HostApplication::HostApplication ()
{
	FUNKNOWN_CTOR

	mPlugInterfaceSupport = owned (new PlugInterfaceSupport);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostApplication::getName (String128 name)
{
	return VST3::StringConvert::convert ("My VST3 HostApplication", name) ? kResultTrue :
	                                                                        kInternalError;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostApplication::createInstance (TUID cid, TUID _iid, void** obj)
{
	if (FUnknownPrivate::iidEqual (cid, IMessage::iid) &&
	    FUnknownPrivate::iidEqual (_iid, IMessage::iid))
	{
		*obj = new HostMessage;
		return kResultTrue;
	}
	if (FUnknownPrivate::iidEqual (cid, IAttributeList::iid) &&
	    FUnknownPrivate::iidEqual (_iid, IAttributeList::iid))
	{
		if (auto al = HostAttributeList::make ())
		{
			*obj = al.take ();
			return kResultTrue;
		}
		return kOutOfMemory;
	}
	*obj = nullptr;
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostApplication::queryInterface (const char* _iid, void** obj)
{
	QUERY_INTERFACE (_iid, obj, FUnknown::iid, IHostApplication)
	QUERY_INTERFACE (_iid, obj, IHostApplication::iid, IHostApplication)

	if (mPlugInterfaceSupport && mPlugInterfaceSupport->queryInterface (iid, obj) == kResultTrue)
		return kResultOk;

	*obj = nullptr;
	return kResultFalse;
}

//-----------------------------------------------------------------------------
uint32 PLUGIN_API HostApplication::addRef ()
{
	return 1;
}

//-----------------------------------------------------------------------------
uint32 PLUGIN_API HostApplication::release ()
{
	return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IMPLEMENT_FUNKNOWN_METHODS (HostMessage, IMessage, IMessage::iid)
//-----------------------------------------------------------------------------
HostMessage::HostMessage () {FUNKNOWN_CTOR}

//-----------------------------------------------------------------------------
HostMessage::~HostMessage () noexcept
{
	setMessageID (nullptr);
	FUNKNOWN_DTOR
}

//-----------------------------------------------------------------------------
const char* PLUGIN_API HostMessage::getMessageID ()
{
	return messageId;
}

//-----------------------------------------------------------------------------
void PLUGIN_API HostMessage::setMessageID (const char* mid)
{
	if (messageId)
		delete[] messageId;
	messageId = nullptr;
	if (mid)
	{
		size_t len = strlen (mid) + 1;
		messageId = new char[len];
		strcpy (messageId, mid);
	}
}

//-----------------------------------------------------------------------------
IAttributeList* PLUGIN_API HostMessage::getAttributes ()
{
	if (!attributeList)
		attributeList = HostAttributeList::make ();
	return attributeList;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct HostAttributeList::Attribute
{
	enum class Type
	{
		kUninitialized,
		kInteger,
		kFloat,
		kString,
		kBinary
	};
	Attribute () = default;

	Attribute (int64 value) : type (Type::kInteger) { v.intValue = value; }
	Attribute (double value) : type (Type::kFloat) { v.floatValue = value; }
	/* size is in code unit (count of TChar) */
	Attribute (const TChar* value, uint32 sizeInCodeUnit)
	: size (sizeInCodeUnit), type (Type::kString)
	{
		v.stringValue = new TChar[sizeInCodeUnit];
		memcpy (v.stringValue, value, sizeInCodeUnit * sizeof (TChar));
	}
	Attribute (const void* value, uint32 sizeInBytes) : size (sizeInBytes), type (Type::kBinary)
	{
		v.binaryValue = new char[sizeInBytes];
		memcpy (v.binaryValue, value, sizeInBytes);
	}
	Attribute (Attribute&& o) { *this = std::move (o); }
	Attribute& operator= (Attribute&& o)
	{
		v = o.v;
		size = o.size;
		type = o.type;
		o.size = 0;
		o.type = Type::kUninitialized;
		o.v = {};
		return *this;
	}
	~Attribute () noexcept
	{
		if (size)
			delete[] v.binaryValue;
	}

	int64 intValue () const { return v.intValue; }
	double floatValue () const { return v.floatValue; }
	/* sizeInCodeUnit is in code unit (count of TChar) */
	const TChar* stringValue (uint32& sizeInCodeUnit)
	{
		sizeInCodeUnit = size;
		return v.stringValue;
	}
	const void* binaryValue (uint32& sizeInBytes)
	{
		sizeInBytes = size;
		return v.binaryValue;
	}

	Type getType () const { return type; }

private:
	union v
	{
		int64 intValue;
		double floatValue;
		TChar* stringValue;
		char* binaryValue;
	} v;
	uint32 size {0};
	Type type {Type::kUninitialized};
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IMPLEMENT_FUNKNOWN_METHODS (HostAttributeList, IAttributeList, IAttributeList::iid)

//-----------------------------------------------------------------------------
IPtr<IAttributeList> HostAttributeList::make ()
{
	return owned (new HostAttributeList);
}

//-----------------------------------------------------------------------------
HostAttributeList::HostAttributeList () {FUNKNOWN_CTOR}

//-----------------------------------------------------------------------------
HostAttributeList::~HostAttributeList () noexcept
{
	FUNKNOWN_DTOR
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setInt (AttrID aid, int64 value)
{
	if (!aid)
		return kInvalidArgument;
	list[aid] = Attribute (value);
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getInt (AttrID aid, int64& value)
{
	if (!aid)
		return kInvalidArgument;
	auto it = list.find (aid);
	if (it != list.end () && it->second.getType () == Attribute::Type::kInteger)
	{
		value = it->second.intValue ();
		return kResultTrue;
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setFloat (AttrID aid, double value)
{
	if (!aid)
		return kInvalidArgument;
	list[aid] = Attribute (value);
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getFloat (AttrID aid, double& value)
{
	if (!aid)
		return kInvalidArgument;
	auto it = list.find (aid);
	if (it != list.end () && it->second.getType () == Attribute::Type::kFloat)
	{
		value = it->second.floatValue ();
		return kResultTrue;
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setString (AttrID aid, const TChar* string)
{
	if (!aid)
		return kInvalidArgument;
	// + 1 for the null-terminate
	auto length = tstrlen (string) + 1;
	list[aid] = Attribute (string, length);
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getString (AttrID aid, TChar* string, uint32 sizeInBytes)
{
	if (!aid)
		return kInvalidArgument;
	auto it = list.find (aid);
	if (it != list.end () && it->second.getType () == Attribute::Type::kString)
	{
		uint32 sizeInCodeUnit = 0;
		const TChar* _string = it->second.stringValue (sizeInCodeUnit);
		memcpy (string, _string, std::min<uint32> (sizeInCodeUnit * sizeof (TChar), sizeInBytes));
		return kResultTrue;
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setBinary (AttrID aid, const void* data, uint32 sizeInBytes)
{
	if (!aid)
		return kInvalidArgument;
	list[aid] = Attribute (data, sizeInBytes);
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getBinary (AttrID aid, const void*& data, uint32& sizeInBytes)
{
	if (!aid)
		return kInvalidArgument;
	auto it = list.find (aid);
	if (it != list.end () && it->second.getType () == Attribute::Type::kBinary)
	{
		data = it->second.binaryValue (sizeInBytes);
		return kResultTrue;
	}
	sizeInBytes = 0;
	return kResultFalse;
}

} // Vst
} // Steinberg
