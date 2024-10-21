/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_CDecibelDisplayDelegateDecorator.h
 *
 *	\brief A decibel display delegate.
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CDECIBELDISPLAYDELEGATEDECORATOR_H
#define AAX_CDECIBELDISPLAYDELEGATEDECORATOR_H


#include "AAX_IDisplayDelegateDecorator.h"
#include <cmath>



/** \brief A percent decorator conforming to AAX_IDisplayDelegateDecorator
	
	\details
	This class is an \ref AAX_IDisplayDelegateDecorator, meaning that it acts as a wrapper for
	other display delegates or concrete display types. For more information about display
	delegate decorators in AAX, see \ref displaydelegates_decorators

	The behavior of this class it to provide conversion to and from dB values.  It performs
	a decibel conversion on the square of the provided value (i.e. 20 log) before passing the
	value on to a concrete display delegate to get a value string.  This class then appends
	the "dB" suffix to signify that the value was converted.  This allows something like a
	gain value to remain internally linear at all times even though its display is converted
	to decibels.

	The inverse is also supported; this class can convert a decibel-formatted string into its
	associated real value.  The string will first be converted to a number, then that number
	will have the inverse dB calculation applied to it to retrieve the parameter's real value.

	\ingroup AAXLibraryFeatures_ParameterManager_DisplayDelegates_Decorators

*/
template <typename T>
class AAX_CDecibelDisplayDelegateDecorator : public AAX_IDisplayDelegateDecorator<T>
{
public:
	AAX_CDecibelDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate);
	
	//Virtual Overrides
	AAX_CDecibelDisplayDelegateDecorator<T>*	Clone() const AAX_OVERRIDE;
	bool		ValueToString(T value, AAX_CString* valueString) const AAX_OVERRIDE;
	bool		ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const AAX_OVERRIDE;
	bool		StringToValue(const AAX_CString& valueString, T* value) const AAX_OVERRIDE;
};






template <typename T>
AAX_CDecibelDisplayDelegateDecorator<T>::AAX_CDecibelDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate)  :
	AAX_IDisplayDelegateDecorator<T>(displayDelegate)
{

}

template <typename T>
AAX_CDecibelDisplayDelegateDecorator<T>*	AAX_CDecibelDisplayDelegateDecorator<T>::Clone() const
{
	return new AAX_CDecibelDisplayDelegateDecorator(*this);
}

template <typename T>
bool	AAX_CDecibelDisplayDelegateDecorator<T>::ValueToString(T value, AAX_CString* valueString) const 
{
	bool succeeded = false;
	if (value <= 0)
	{
		//*valueString = AAX_CString("--- dB");
		*valueString = AAX_CString("-INF ");
		succeeded = true;
	}
	else 
	{
		value = (T)(20.0*log10(value));
		if ( value > -0.01f && value < 0.0f) //To prevent minus for 0.0 value in automation turned on
			value = 0.0f;
		
		succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);		
	}
	
	*valueString += AAX_CString("dB");
	return succeeded;
}

template <typename T>
bool	AAX_CDecibelDisplayDelegateDecorator<T>::ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const
{
	if (value <= 0)
	{
		*valueString = AAX_CString("-INF");
		if (maxNumChars >= 7)
			valueString->Append(" dB");	//<DMT> Add a space for longer strings and dB
		return true;
	}

	value = (T)(20.0*log10(value));
	if ( value > -0.01f && value < 0.0f) //To prevent minus for 0.0 value in automation turned on
		value = 0.0f;
	bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars, valueString);
	
	
	//<DMT> Check current string length and see if there is room to add units.  I believe these units are usually less important than precision on control surfaces.
	uint32_t strlen = valueString->Length();
	const uint32_t maxNumCharsUnsigned = (0 <= maxNumChars) ? static_cast<uint32_t>(maxNumChars) : 0;
	if (maxNumCharsUnsigned >= (strlen + 2))	//length of string plus 2 for the "dB"
		*valueString += AAX_CString("dB");
	return succeeded;
}


template <typename T>
bool	AAX_CDecibelDisplayDelegateDecorator<T>::StringToValue(const AAX_CString& valueString, T* value) const
{		
	//Just call through if there is obviously no unit string.
	if (valueString.Length() <= 2)
	{
		bool success = AAX_IDisplayDelegateDecorator<T>::StringToValue(valueString, value);
		*value = (T)pow((T)10.0, (*value / (T)20.0));
		return success;
	}
		
	//Just call through if the end of this string does not match the unit string.	
	AAX_CString unitSubString;
	valueString.SubString(valueString.Length() - 2, 2, &unitSubString);
	if (unitSubString != AAX_CString("dB"))
	{
		bool success = AAX_IDisplayDelegateDecorator<T>::StringToValue(valueString, value);
		*value = (T)pow((T)10.0, *value / (T)20.0);
		return success;
	}
			
	//Call through with the stripped down value string.  
	AAX_CString valueSubString;
	valueString.SubString(0, valueString.Length() - 2, &valueSubString);
	bool success = AAX_IDisplayDelegateDecorator<T>::StringToValue(valueSubString, value);
	*value = (T)pow((T)10.0, *value / (T)20.0);
	return success;
}




#endif //AAX_CDECIBELDISPLAYDELEGATEDECORATOR_H
