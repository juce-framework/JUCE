//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/hosting/hostclasses.cpp
// Created by  : Steinberg, 03/05/2008.
// Description : VST 3 hostclasses, example implementations for IHostApplication, IAttributeList and IMessage
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
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

#include <algorithm>

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
HostApplication::HostApplication ()
{
	FUNKNOWN_CTOR

	mPlugInterfaceSupport = owned (NEW PlugInterfaceSupport);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostApplication::getName (String128 name)
{
	String str ("My VST3 HostApplication");
	str.copyTo16 (name, 0, 127);
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostApplication::createInstance (TUID cid, TUID _iid, void** obj)
{
	FUID classID (FUID::fromTUID (cid));
	FUID interfaceID (FUID::fromTUID (_iid));
	if (classID == IMessage::iid && interfaceID == IMessage::iid)
	{
		*obj = new HostMessage;
		return kResultTrue;
	}
	else if (classID == IAttributeList::iid && interfaceID == IAttributeList::iid)
	{
		*obj = new HostAttributeList;
		return kResultTrue;
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
HostMessage::HostMessage () : messageId (nullptr), attributeList (nullptr)
{
	FUNKNOWN_CTOR
}

//-----------------------------------------------------------------------------
HostMessage::~HostMessage ()
{
	setMessageID (nullptr);
	if (attributeList)
		attributeList->release ();
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
		attributeList = new HostAttributeList;
	return attributeList;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class HostAttribute
{
public:
	enum Type
	{
		kInteger,
		kFloat,
		kString,
		kBinary
	};

	HostAttribute (int64 value) : size (0), type (kInteger) { v.intValue = value; }
	HostAttribute (double value) : size (0), type (kFloat) { v.floatValue = value; }
	HostAttribute (const TChar* value, uint32 size) : size (size), type (kString)
	{
		v.stringValue = new TChar[size];
		memcpy (v.stringValue, value, size * sizeof (TChar));
	}
	HostAttribute (const void* value, uint32 size) : size (size), type (kBinary)
	{
		v.binaryValue = new char[size];
		memcpy (v.binaryValue, value, size);
	}
	~HostAttribute ()
	{
		if (size)
			delete[] v.binaryValue;
	}

	int64 intValue () const { return v.intValue; }
	double floatValue () const { return v.floatValue; }
	const TChar* stringValue (uint32& stringSize)
	{
		stringSize = size;
		return v.stringValue;
	}
	const void* binaryValue (uint32& binarySize)
	{
		binarySize = size;
		return v.binaryValue;
	}

	Type getType () const { return type; }

protected:
	union v
	{
		int64 intValue;
		double floatValue;
		TChar* stringValue;
		char* binaryValue;
	} v;
	uint32 size;
	Type type;
};

using mapIterator = std::map<String, HostAttribute*>::iterator;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IMPLEMENT_FUNKNOWN_METHODS (HostAttributeList, IAttributeList, IAttributeList::iid)
//-----------------------------------------------------------------------------
HostAttributeList::HostAttributeList ()
{
	FUNKNOWN_CTOR
}

//-----------------------------------------------------------------------------
HostAttributeList::~HostAttributeList ()
{
	std::map<String, HostAttribute*>::reverse_iterator it = list.rbegin ();
	while (it != list.rend ())
	{
		delete it->second;
		it++;
	}
	FUNKNOWN_DTOR
}

//-----------------------------------------------------------------------------
void HostAttributeList::removeAttrID (AttrID aid)
{
	mapIterator it = list.find (aid);
	if (it != list.end ())
	{
		delete it->second;
		list.erase (it);
	}
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setInt (AttrID aid, int64 value)
{
	removeAttrID (aid);
	list[aid] = new HostAttribute (value);
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getInt (AttrID aid, int64& value)
{
	mapIterator it = list.find (aid);
	if (it != list.end () && it->second)
	{
		value = it->second->intValue ();
		return kResultTrue;
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setFloat (AttrID aid, double value)
{
	removeAttrID (aid);
	list[aid] = new HostAttribute (value);
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getFloat (AttrID aid, double& value)
{
	mapIterator it = list.find (aid);
	if (it != list.end () && it->second)
	{
		value = it->second->floatValue ();
		return kResultTrue;
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setString (AttrID aid, const TChar* string)
{
	removeAttrID (aid);
	list[aid] = new HostAttribute (string, String (const_cast<TChar*> (string)).length ());
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getString (AttrID aid, TChar* string, uint32 size)
{
	mapIterator it = list.find (aid);
	if (it != list.end () && it->second)
	{
		uint32 stringSize = 0;
		const TChar* _string = it->second->stringValue (stringSize);
		memcpy (string, _string, std::min<uint32> (stringSize, size) * sizeof (TChar));
		return kResultTrue;
	}
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setBinary (AttrID aid, const void* data, uint32 size)
{
	removeAttrID (aid);
	list[aid] = new HostAttribute (data, size);
	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getBinary (AttrID aid, const void*& data, uint32& size)
{
	mapIterator it = list.find (aid);
	if (it != list.end () && it->second)
	{
		data = it->second->binaryValue (size);
		return kResultTrue;
	}
	size = 0;
	return kResultFalse;
}
}
} // namespace
