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
 *	\file AAX_CUnitDisplayDelegateDecorator.h
 *
 *	\brief A unit display delgate decorator.
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CUNITDISPLAYDELEGATEDECORATOR_H
#define AAX_CUNITDISPLAYDELEGATEDECORATOR_H

#include "AAX_IDisplayDelegateDecorator.h"


/** \brief A unit type decorator conforming to AAX_IDisplayDelegateDecorator
	
	\details
	This class is an \ref AAX_IDisplayDelegateDecorator, meaning that it acts as a wrapper for
	other display delegates or concrete display types. For more information about display
	delegate decorators in AAX, see \ref displaydelegates_decorators

	The behavior of this class it to decorate parameter value strings with arbitrary units,
	such as "Hz" or "V".  The inverse is also supported, so the unit string is pulled off of
	value strings when they are converted to real parameter values.

	\ingroup AAXLibraryFeatures_ParameterManager_DisplayDelegates_Decorators
	
*/
template <typename T>
class AAX_CUnitDisplayDelegateDecorator : public AAX_IDisplayDelegateDecorator<T>
{
public:
	/** \brief Constructor
	 *
	 *	Along with the standard decorator pattern argument, this class also takes a unit string.
	 *	This is the string that will be added to the end of valueString.
	 *
	 *	\param[in] displayDelegate
	 *	\param[in] unitString
	 */
	AAX_CUnitDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate, const AAX_CString& unitString);
	
	//Virtual Overrides
	AAX_CUnitDisplayDelegateDecorator<T>*	Clone() const AAX_OVERRIDE;
	bool		ValueToString(T value, AAX_CString* valueString) const AAX_OVERRIDE;
	bool		ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const AAX_OVERRIDE;
	bool		StringToValue(const AAX_CString& valueString, T* value) const AAX_OVERRIDE;

protected:
	const AAX_CString	mUnitString;
};




template <typename T>
AAX_CUnitDisplayDelegateDecorator<T>::AAX_CUnitDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate, const AAX_CString& unitString)  :
	AAX_IDisplayDelegateDecorator<T>(displayDelegate),
	mUnitString(unitString)
{

}

template <typename T>
AAX_CUnitDisplayDelegateDecorator<T>*	AAX_CUnitDisplayDelegateDecorator<T>::Clone() const
{
	return new AAX_CUnitDisplayDelegateDecorator(*this);
}

template <typename T>
bool		AAX_CUnitDisplayDelegateDecorator<T>::ValueToString(T value, AAX_CString* valueString) const 
{
	bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, valueString);
	*valueString += mUnitString;
	return succeeded;
}

template <typename T>
bool		AAX_CUnitDisplayDelegateDecorator<T>::ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const
{
	bool succeeded = AAX_IDisplayDelegateDecorator<T>::ValueToString(value, maxNumChars, valueString);
	uint32_t strlen = valueString->Length();
	const uint32_t maxNumCharsUnsigned = (0 <= maxNumChars) ? static_cast<uint32_t>(maxNumChars) : 0;
	if (maxNumCharsUnsigned > strlen && (maxNumCharsUnsigned-strlen >= mUnitString.Length()))
		*valueString += mUnitString;
	return succeeded;
}


template <typename T>
bool		AAX_CUnitDisplayDelegateDecorator<T>::StringToValue(const AAX_CString& valueString, T* value) const
{
	//Just call through if there is obviously no unit string.
	if (valueString.Length() <= mUnitString.Length())
		return AAX_IDisplayDelegateDecorator<T>::StringToValue(valueString, value);
		
	//Just call through if the end of this string does not match the unit string.	
	AAX_CString unitSubString;
	valueString.SubString(valueString.Length() - mUnitString.Length(), mUnitString.Length(), &unitSubString);
	if (unitSubString != mUnitString)
		return AAX_IDisplayDelegateDecorator<T>::StringToValue(valueString, value);
	
	//Call through with the stripped down value string.  
	AAX_CString valueSubString;
	valueString.SubString(0, valueString.Length() - mUnitString.Length(), &valueSubString);
	return AAX_IDisplayDelegateDecorator<T>::StringToValue(valueSubString, value);
}





#endif //AAX_CUNITDISPLAYDELEGATEDECORATOR_H
