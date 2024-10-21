/*================================================================================================*/
/*
 *	Copyright 2010-2015, 2018, 2023-2024 Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	This file is part of the Avid AAX SDK.
 *	
 *	The AAX SDK is subject to commercial or open-source licensing.
 *	
 *	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
 *	Agreement and Avid Privacy Policy.
 *	
 *	AAX SDK License: https://developer.avid.com/aax
 *	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement
 *	
 *	Or: You may also use this code under the terms of the GPL v3 (see
 *	www.gnu.org/licenses).
 *	
 *	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 *	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 *	DISCLAIMED.
 *
 */

/** 
 *  \author David Tremblay
 *
 */
/*================================================================================================*/
#include "AAX_CParameter.h"


//----------------------------------------------------------------
// AAX_CParameterValue


template <>
bool		AAX_CParameterValue<bool>::GetValueAsBool(bool* value) const
{
	*value = mValue;
	return true;
}

template<>
bool		AAX_CParameterValue<int32_t>::GetValueAsInt32(int32_t* value) const
{
	*value = mValue;
	return true;
}

template<>
bool		AAX_CParameterValue<float>::GetValueAsFloat(float* value) const
{
	*value = mValue;
	return true;
}

template<>
bool		AAX_CParameterValue<double>::GetValueAsDouble(double* value) const
{
	*value = mValue;
	return true;
}

template<>
bool		AAX_CParameterValue<AAX_CString>::GetValueAsString(AAX_IString* value) const
{
	*value = mValue;
	return true;
}



//----------------------------------------------------------------
// AAX_CParameter

template <>
bool	AAX_CParameter<bool>::GetNormalizedValueFromBool(bool value, double *normalizedValue) const
{
	*normalizedValue = mTaperDelegate->RealToNormalized(value);
	return true;
}

template <>
bool	AAX_CParameter<int32_t>::GetNormalizedValueFromInt32(int32_t value, double *normalizedValue) const
{
	*normalizedValue = mTaperDelegate->RealToNormalized(value);
	return true;
}

template <>
bool	AAX_CParameter<float>::GetNormalizedValueFromFloat(float value, double *normalizedValue) const
{
	*normalizedValue = mTaperDelegate->RealToNormalized(value);
	return true;
}

template <>
bool	AAX_CParameter<double>::GetNormalizedValueFromDouble(double value, double *normalizedValue) const
{
	*normalizedValue = mTaperDelegate->RealToNormalized(value);
	return true;
}

template <>
bool		AAX_CParameter<bool>::GetBoolFromNormalizedValue(double inNormalizedValue, bool* value) const
{
	*value = mTaperDelegate->NormalizedToReal(inNormalizedValue);
	return true;
}

template<>
bool		AAX_CParameter<int32_t>::GetInt32FromNormalizedValue(double inNormalizedValue, int32_t* value) const
{
	*value = mTaperDelegate->NormalizedToReal(inNormalizedValue);
	return true;
}

template<>
bool		AAX_CParameter<float>::GetFloatFromNormalizedValue(double inNormalizedValue, float* value) const
{
	*value = mTaperDelegate->NormalizedToReal(inNormalizedValue);
	return true;
}

template<>
bool		AAX_CParameter<double>::GetDoubleFromNormalizedValue(double inNormalizedValue, double* value) const
{
	*value = mTaperDelegate->NormalizedToReal(inNormalizedValue);
	return true;
}

template<>
bool		AAX_CParameter<AAX_CString>::GetValueAsString(AAX_IString *value) const
{
	return mValue.GetValueAsString(value);
}

template<>
bool		AAX_CParameter<bool>::SetValueWithBool(bool value)
{
	SetValue(value);
	return true;
}

template<>
bool		AAX_CParameter<int32_t>::SetValueWithInt32(int32_t value)
{
	SetValue(value);
	return true;
}

template<>
bool		AAX_CParameter<float>::SetValueWithFloat(float value)
{
	SetValue(value);
	return true;
}

template<>
bool		AAX_CParameter<double>::SetValueWithDouble(double value)
{
	SetValue(value);
	return true;
}

template<>
bool		AAX_CParameter<AAX_CString>::SetValueWithString(const AAX_IString& value)
{
	SetValue(value);
	return true;
}
