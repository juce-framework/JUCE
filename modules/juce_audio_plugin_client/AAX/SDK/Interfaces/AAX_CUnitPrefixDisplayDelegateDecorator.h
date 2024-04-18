/*================================================================================================*/
/*
 *
 *	Copyright 2014-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_CUnitPrefixDisplayDelegateDecorator.h
 *
 *	\brief A unit prefix display delegate decorator.
 *
 */ 
/*================================================================================================*/


#ifndef	AAX_CUNITPREFIXDISPLAYDELEGATEDECORATOR_H
#define AAX_CUNITPREFIXDISPLAYDELEGATEDECORATOR_H

#include "AAX_IDisplayDelegateDecorator.h"


/** \brief A unit prefix decorator conforming to AAX_IDisplayDelegateDecorator
	
	\details
	This class is an \ref AAX_IDisplayDelegateDecorator, meaning that it acts as a wrapper for
	other display delegates or concrete display types. For more information about display
	delegate decorators in AAX, see \ref displaydelegates_decorators
	
	The behavior of this class it to provide unit prefixes such as the k in kHz or the m in
	mm.  It takes the value passed in and determines if the value is large or small enough to
	benefit from a unit modifier.  If so, it adds that unit prefix character to the display
	string after scaling the number and calling deeper into the decorator pattern to get the
	concrete ValueToString() result.
	
	The inverse is also supported, so if you type 1.5k in a text box and this decorator is in
	place, it should find the k and multiply the value by 1000 before converting it to a real
	value.
	
	This decorator supports the following unit prefixes:
		\li M (mega-)
		\li k (kilo-)
		\li m (milli-)
		\li u (micro-)
	
	\note This class is not implemented for integer values as the conversions result in
	fractional numbers.  Those would get truncated through the system and be pretty much
	useless.

	\ingroup AAXLibraryFeatures_ParameterManager_DisplayDelegates_Decorators
	
*/
template <typename T>
class AAX_CUnitPrefixDisplayDelegateDecorator : public AAX_IDisplayDelegateDecorator<T>
{
public:
	AAX_CUnitPrefixDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate);
	
	//Virtual overrides
	AAX_CUnitPrefixDisplayDelegateDecorator<T>*	Clone() const AAX_OVERRIDE;
	bool		ValueToString(T value, AAX_CString* valueString) const AAX_OVERRIDE;
	bool		ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const AAX_OVERRIDE;
	bool		StringToValue(const AAX_CString& valueString, T* value) const AAX_OVERRIDE;
};



template <typename T>
AAX_CUnitPrefixDisplayDelegateDecorator<T>::AAX_CUnitPrefixDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate)  :
	AAX_IDisplayDelegateDecorator<T>(displayDelegate)
{

}


template <typename T>
AAX_CUnitPrefixDisplayDelegateDecorator<T>*		AAX_CUnitPrefixDisplayDelegateDecorator<T>::Clone() const
{
	return new AAX_CUnitPrefixDisplayDelegateDecorator(*this);
}

template <typename T>
bool		AAX_CUnitPrefixDisplayDelegateDecorator<T>::ValueToString(T value, AAX_CString* valueString) const 
{
	//Find the proper unit prefix.
	T absValue = fabsf(float(value));	//If you fail to compile on this line, you're trying to use this class with an integer type, which is not supported.
	if (absValue >= 1000000.0)
	{
		value = value / ((T) 1000000.0);
		bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);
		*valueString += AAX_CString("M");
		return succeeded;
	}
	if (absValue >= ((T) 1000.0))
	{
		value = value / ((T) 1000.0);
		bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);
		*valueString += AAX_CString("k");
		return succeeded;
	}
	if (absValue >= ((T) 1.0))
	{
		return AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);
	}
	if (absValue >= ((T) 0.001))
	{
		value = value / ((T) 0.001);
		bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);
		*valueString += AAX_CString("m");
		return succeeded;
	}
	if (absValue >= ((T) 0.000001))
	{
		value = value / ((T) 0.000001);
		bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);
		*valueString += AAX_CString("u");
		return succeeded;
	}
	return AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);
}

template <typename T>
bool		AAX_CUnitPrefixDisplayDelegateDecorator<T>::ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const
{
	//Find the proper unit prefix.
	//<DMT> The maxNumChars is decremented by 1 in case of the unit modifier being required as this is more important than precision.
	
	T absValue = fabsf(float(value));	//If you fail to compile on this line, you're trying to use this class with an integer type, which is not supported.
	if (absValue >= 1000000.0)
	{
		value = value / ((T) 1000000.0);
		bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars-1, valueString);
		*valueString += AAX_CString("M");
		return succeeded;
	}
	if (absValue >= ((T) 1000.0))
	{
		value = value / ((T) 1000.0);
		bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars-1, valueString);
		*valueString += AAX_CString("k");
		return succeeded;
	}
	if (absValue >= ((T) 1.0))
	{
		return AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars, valueString);
	}
	if (absValue >= ((T) 0.001))
	{
		value = value / ((T) 0.001);
		bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars-1, valueString);
		*valueString += AAX_CString("m");
		return succeeded;
	}
	if (absValue >= ((T) 0.000001))
	{
		value = value / ((T) 0.000001);
		bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars-1, valueString);
		*valueString += AAX_CString("u");
		return succeeded;
	}
	return AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars, valueString);
}


template <typename T>
bool		AAX_CUnitPrefixDisplayDelegateDecorator<T>::StringToValue(const AAX_CString& valueString, T* value) const
{
	//Just call through if there is obviously no unit string.
	if (valueString.Length() <= 1)
		return AAX_IDisplayDelegateDecorator<T>::StringToValue(valueString, value);
		
	//Just call through if the end of this string does not match the unit string.	
	AAX_CString valueStringCopy(valueString);
	T valueScalar = 1;
	T valueDivScalar = 1;
	switch(valueString[valueString.Length()-1])
	{
		case 'M':
			valueScalar = ((T) 1000000.0);
			valueStringCopy.Erase(valueString.Length()-1, 1);
			break;
		case 'k':
			valueScalar = ((T) 1000.0);
			valueStringCopy.Erase(valueString.Length()-1, 1);
			break;
		case 'm':
			valueScalar = ((T) 0.001);
			valueStringCopy.Erase(valueString.Length()-1, 1);
			break;
		case 'u':
			// Rounding errors occur when trying to use 0.000001 so went to a div scalar instead.
			// See bug https://audio-jira.avid.com/browse/PTSW-149426.
			valueDivScalar = ((T) 1000000.0);
			valueStringCopy.Erase(valueString.Length()-1, 1);
			break;
	}
	 
	bool success = AAX_IDisplayDelegateDecorator<T>::StringToValue(valueStringCopy, value);
	*value = valueScalar * (*value);
	*value = (*value) / valueDivScalar;
	return success;
}



#endif //AAX_CUNITPREFIXDISPLAYDELEGATEDECORATOR
