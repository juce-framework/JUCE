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
 *	\file  AAX_CPercentDisplayDelegateDecorator.h
 *
 *	\brief A percent display delegate decorator.
 *
 */ 
/*================================================================================================*/


#pragma once

#ifndef AAX_CPERCENTDISPLAYDELEGATEDECORATOR_H
#define AAX_CPERCENTDISPLAYDELEGATEDECORATOR_H

#include "AAX_IDisplayDelegateDecorator.h"

#include <cmath>


/** \brief A percent decorator conforming to AAX_IDisplayDelegateDecorator
	
	\details
	This class is an \ref AAX_IDisplayDelegateDecorator, meaning that it acts as a wrapper for
	other display delegates or concrete display types. For more information about display
	delegate decorators in AAX, see \ref displaydelegates_decorators

	The behavior of this class it to provide string conversion to and from percentage (%)
	values.  When converting a parameter value to a string, it takes the real value and 
	performs a % conversion before passing the value on to a concrete implementation to get
	a value string.  It then adds on the "%" string at the end to signify that the value was
	converted.  This allows something like a gain value to remain internally linear at all
	times even though its display is converted to a percentage.

	The inverse operation is also supported; this class can convert a percentage-formatted
	string into its associated real value.  The string will first be converted to a number,
	then that number will have the inverse % calculation applied to it to retrieve the
	parameter's actual value.

	\ingroup AAXLibraryFeatures_ParameterManager_DisplayDelegates_Decorators
	
*/
template <typename T>
class AAX_CPercentDisplayDelegateDecorator : public AAX_IDisplayDelegateDecorator<T>
{
public:
	AAX_CPercentDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate);
	
	//Virtual Overrides
	AAX_CPercentDisplayDelegateDecorator<T>*	Clone() const AAX_OVERRIDE;
	bool		ValueToString(T value, AAX_CString* valueString) const AAX_OVERRIDE;
	bool		ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const AAX_OVERRIDE;
	bool		StringToValue(const AAX_CString& valueString, T* value) const AAX_OVERRIDE;
};

template <typename T>
AAX_CPercentDisplayDelegateDecorator<T>::AAX_CPercentDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate)  :
	AAX_IDisplayDelegateDecorator<T>(displayDelegate)
{
}

template <typename T>
AAX_CPercentDisplayDelegateDecorator<T>*	AAX_CPercentDisplayDelegateDecorator<T>::Clone() const
{
	return new AAX_CPercentDisplayDelegateDecorator(*this);
}

template <typename T>
bool AAX_CPercentDisplayDelegateDecorator<T>::ValueToString(T value, AAX_CString* valueString) const 
{
	value *= 100;
	bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);
	*valueString += AAX_CString("%");
	return succeeded;
}

template <typename T>
bool AAX_CPercentDisplayDelegateDecorator<T>::ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const 
{
	value *= 100;
	bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars-1, valueString);		//<DMT>  Make room for percentage symbol.
	*valueString += AAX_CString("%");
	return succeeded;
}


template <typename T>
bool AAX_CPercentDisplayDelegateDecorator<T>::StringToValue(const AAX_CString& valueString, T* value) const
{
	//Just call through if there is obviously no unit string.
	if (valueString.Length() <= 2)
	{
		bool success = AAX_IDisplayDelegateDecorator<T>::StringToValue(valueString, value);
		*value /= 100.0f;
		return success;
	}

	//Just call through if the end of this string does not match the unit string.	
	AAX_CString unitSubString;
	valueString.SubString(valueString.Length() - 1, 1, &unitSubString);
	if (unitSubString != AAX_CString("%"))
	{
		bool success = AAX_IDisplayDelegateDecorator<T>::StringToValue(valueString, value);
		*value /= 100.0f;
		return success;
	}

	//Call through with the stripped down value string.  
	AAX_CString valueSubString;
	valueString.SubString(0, valueString.Length() - 1, &valueSubString);
	bool success = AAX_IDisplayDelegateDecorator<T>::StringToValue(valueSubString, value);
	*value /= 100.0f;
	return success;
}


#endif

