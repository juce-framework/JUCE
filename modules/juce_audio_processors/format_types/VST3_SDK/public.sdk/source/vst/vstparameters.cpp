//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vstparameters.cpp
// Created by  : Steinberg, 03/2008
// Description : VST Parameter Implementation
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

#include "vstparameters.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/base/ustring.h"
#include <cstdlib>

namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
// Parameter Implementation
//------------------------------------------------------------------------
Parameter::Parameter ()
{
}

//------------------------------------------------------------------------
Parameter::Parameter (const ParameterInfo& info)
: info (info), valueNormalized (info.defaultNormalizedValue)
{
}

//------------------------------------------------------------------------
Parameter::Parameter (const TChar* title, ParamID tag, const TChar* units,
                      ParamValue defaultValueNormalized, int32 stepCount, int32 flags,
                      UnitID unitID, const TChar* shortTitle)
{
	UString (info.title, str16BufferSize (String128)).assign (title);
	if (units)
		UString (info.units, str16BufferSize (String128)).assign (units);
	if (shortTitle)
		UString (info.shortTitle, str16BufferSize (String128)).assign (shortTitle);

	info.stepCount = stepCount;
	info.defaultNormalizedValue = valueNormalized = defaultValueNormalized;
	info.flags = flags;
	info.id = tag;
	info.unitId = unitID;
}

//------------------------------------------------------------------------
Parameter::~Parameter ()
{
}

//------------------------------------------------------------------------
bool Parameter::setNormalized (ParamValue normValue)
{
	if (normValue > 1.0)
	{
		normValue = 1.0;
	}
	else if (normValue < 0.)
	{
		normValue = 0.;
	}

	if (normValue != valueNormalized)
	{
		valueNormalized = normValue;
		changed ();
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
void Parameter::toString (ParamValue normValue, String128 string) const
{
	UString wrapper (string, str16BufferSize (String128));
	if (info.stepCount == 1)
	{
		if (normValue > 0.5)
		{
			wrapper.assign (STR16 ("On"));
		}
		else
		{
			wrapper.assign (STR16 ("Off"));
		}
	}
	else
	{
		if (!wrapper.printFloat (normValue, precision))
			string[0] = 0;
	}
}

//------------------------------------------------------------------------
bool Parameter::fromString (const TChar* string, ParamValue& normValue) const
{
	UString wrapper (const_cast<TChar*> (string), tstrlen (string));
	return wrapper.scanFloat (normValue);
}

//------------------------------------------------------------------------
ParamValue Parameter::toPlain (ParamValue normValue) const
{
	return normValue;
}

//------------------------------------------------------------------------
ParamValue Parameter::toNormalized (ParamValue plainValue) const
{
	return plainValue;
}

//------------------------------------------------------------------------
// RangeParameter Implementation
//------------------------------------------------------------------------
RangeParameter::RangeParameter () : minPlain (0), maxPlain (1)
{
}

//------------------------------------------------------------------------
RangeParameter::RangeParameter (const ParameterInfo& paramInfo, ParamValue min, ParamValue max)
: Parameter (paramInfo), minPlain (min), maxPlain (max)
{
}

//------------------------------------------------------------------------
RangeParameter::RangeParameter (const TChar* title, ParamID tag, const TChar* units,
                                ParamValue minPlain, ParamValue maxPlain,
                                ParamValue defaultValuePlain, int32 stepCount, int32 flags,
                                UnitID unitID, const TChar* shortTitle)
: minPlain (minPlain), maxPlain (maxPlain)
{
	UString (info.title, str16BufferSize (String128)).assign (title);
	if (units)
		UString (info.units, str16BufferSize (String128)).assign (units);
	if (shortTitle)
		UString (info.shortTitle, str16BufferSize (String128)).assign (shortTitle);

	info.stepCount = stepCount;
	info.defaultNormalizedValue = valueNormalized = toNormalized (defaultValuePlain);
	info.flags = flags;
	info.id = tag;
	info.unitId = unitID;
}

//------------------------------------------------------------------------
void RangeParameter::toString (ParamValue _valueNormalized, String128 string) const
{
	if (info.stepCount > 1)
	{
		UString wrapper (string, str16BufferSize (String128));
		int64 plain = static_cast<int64> (toPlain (_valueNormalized));
		if (!wrapper.printInt (plain))
			string[0] = 0;
	}
	else
	{
		Parameter::toString (toPlain (_valueNormalized), string);
	}
}

//------------------------------------------------------------------------
bool RangeParameter::fromString (const TChar* string, ParamValue& _valueNormalized) const
{
	UString wrapper (const_cast<TChar*> (string), tstrlen (string));
	if (info.stepCount > 1)
	{
		int64 plainValue;
		if (wrapper.scanInt (plainValue))
		{
			_valueNormalized = toNormalized ((ParamValue)plainValue);
			return true;
		}
		return false;
	}
	if (wrapper.scanFloat (_valueNormalized))
	{
		if (_valueNormalized < getMin ())
			_valueNormalized = getMin ();
		else if (_valueNormalized > getMax ())
			_valueNormalized = getMax ();
		_valueNormalized = toNormalized (_valueNormalized);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
ParamValue RangeParameter::toPlain (ParamValue _valueNormalized) const
{
	if (info.stepCount > 1)
		return FromNormalized<ParamValue> (_valueNormalized, info.stepCount) + getMin ();
	return _valueNormalized * (getMax () - getMin ()) + getMin ();
}

//------------------------------------------------------------------------
ParamValue RangeParameter::toNormalized (ParamValue plainValue) const
{
	if (info.stepCount > 1)
		return ToNormalized <ParamValue>(plainValue - getMin (), info.stepCount);
	return (plainValue - getMin ()) / (getMax () - getMin ());
}

//------------------------------------------------------------------------
// StringListParameter Implementation
//------------------------------------------------------------------------
StringListParameter::StringListParameter (const ParameterInfo& paramInfo) : Parameter (paramInfo)
{
}

//------------------------------------------------------------------------
StringListParameter::StringListParameter (const TChar* title, ParamID tag, const TChar* units,
                                          int32 flags, UnitID unitID, const TChar* shortTitle)
{
	UString (info.title, str16BufferSize (String128)).assign (title);
	if (units)
		UString (info.units, str16BufferSize (String128)).assign (units);
	if (shortTitle)
		UString (info.shortTitle, str16BufferSize (String128)).assign (shortTitle);

	info.stepCount = -1;
	info.defaultNormalizedValue = 0;
	info.flags = flags;
	info.id = tag;
	info.unitId = unitID;
}

//------------------------------------------------------------------------
StringListParameter::~StringListParameter ()
{
	for (auto& string : strings)
		std::free (string);
}

//------------------------------------------------------------------------
void StringListParameter::appendString (const String128 string)
{
	int32 length = strlen16 (string);
	TChar* buffer = (TChar*)std::malloc ((length + 1) * sizeof (TChar));
	if (!buffer)
		return;

	memcpy (buffer, string, length * sizeof (TChar));
	buffer[length] = 0;
	strings.push_back (buffer);
	info.stepCount++;
}

//------------------------------------------------------------------------
bool StringListParameter::replaceString (int32 index, const String128 string)
{
	TChar* str = strings.at (index);
	if (!str)
		return false;

	int32 length = strlen16 (string);
	TChar* buffer = (TChar*)malloc ((length + 1) * sizeof (TChar));
	if (!buffer)
		return false;

	memcpy (buffer, string, length * sizeof (TChar));
	buffer[length] = 0;
	strings[index] = buffer;
	std::free (str);
	return true;
}

//------------------------------------------------------------------------
void StringListParameter::toString (ParamValue _valueNormalized, String128 string) const
{
	int32 index = static_cast<int32> (toPlain (_valueNormalized));
	if (const TChar* valueString = strings.at (index))
	{
		UString (string, str16BufferSize (String128)).assign (valueString);
	}
	else
		string[0] = 0;
}

//------------------------------------------------------------------------
bool StringListParameter::fromString (const TChar* string, ParamValue& _valueNormalized) const
{
	int32 index = 0;
	for (auto it = strings.begin (), end = strings.end (); it != end; ++it, ++index)
	{
		if (strcmp16 (*it, string) == 0)
		{
			_valueNormalized = toNormalized ((ParamValue)index);
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------
ParamValue StringListParameter::toPlain (ParamValue _valueNormalized) const
{
	if (info.stepCount <= 0)
		return 0;
	return FromNormalized<ParamValue> (_valueNormalized, info.stepCount);
}

//------------------------------------------------------------------------
ParamValue StringListParameter::toNormalized (ParamValue plainValue) const
{
	if (info.stepCount <= 0)
		return 0;
	return ToNormalized<ParamValue> (plainValue, info.stepCount);
}

//------------------------------------------------------------------------
// ParameterContainer Implementation
//------------------------------------------------------------------------
ParameterContainer::ParameterContainer ()
{
}

//------------------------------------------------------------------------
ParameterContainer::~ParameterContainer ()
{
	if (params)
		delete params;
}

//------------------------------------------------------------------------
void ParameterContainer::init (int32 initialSize, int32 /*resizeDelta*/)
{
	if (!params)
	{
		params = new ParameterPtrVector;
		if (initialSize > 0)
			params->reserve (initialSize);
	}
}

//------------------------------------------------------------------------
Parameter* ParameterContainer::addParameter (Parameter* p)
{
	if (!params)
		init ();
	id2index[p->getInfo ().id] = params->size ();
	params->push_back (IPtr<Parameter> (p, false));
	return p;
}

//------------------------------------------------------------------------
Parameter* ParameterContainer::addParameter (const ParameterInfo& info)
{
	if (!params)
		init ();
	auto* p = new Parameter (info);
	if (addParameter (p))
		return p;
	p->release ();
	return nullptr;
}

//------------------------------------------------------------------------
Parameter* ParameterContainer::getParameterByIndex (int32 index) const
{
	if (!params || index < 0 || index >= static_cast<int32> (params->size ()))
		return nullptr;
	return params->at (index);
}

//------------------------------------------------------------------------
Parameter* ParameterContainer::getParameter (ParamID tag) const
{
	if (params)
	{
		auto it = id2index.find (tag);
		if (it != id2index.end ())
			return params->at (it->second);
	}
	return nullptr;
}

//------------------------------------------------------------------------
bool ParameterContainer::removeParameter (ParamID tag)
{
	if (!params)
		return false;
	
	IndexMap::const_iterator it = id2index.find (tag);
	if (it != id2index.end ())
	{
		params->erase (params->begin () + it->second);
		id2index.erase (it);
	}
	return false;
}

//------------------------------------------------------------------------
Parameter* ParameterContainer::addParameter (const TChar* title, const TChar* units,
                                             int32 stepCount, ParamValue defaultNormalizedValue,
                                             int32 flags, int32 tag, UnitID unitID, const TChar* shortTitle)
{
	if (!title)
	{
		return nullptr;
	}

	ParameterInfo info = {};

	UString (info.title, str16BufferSize (String128)).assign (title);
	if (units)
		UString (info.units, str16BufferSize (String128)).assign (units);
	if (shortTitle)
		UString (info.shortTitle, str16BufferSize (String128)).assign (shortTitle);

	info.stepCount = stepCount;
	info.defaultNormalizedValue = defaultNormalizedValue;
	info.flags = flags;
	info.id = (tag >= 0) ? tag : getParameterCount ();
	info.unitId = unitID;

	return addParameter (info);
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
