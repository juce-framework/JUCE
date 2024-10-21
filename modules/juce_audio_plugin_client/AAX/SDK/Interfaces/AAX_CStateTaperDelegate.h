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
 *	\file  AAX_CStateTaperDelegate.h
 *
 *	\brief A state taper delegate (similar to a linear taper delegate.)
 *
 */ 
/*================================================================================================*/


#ifndef	AAX_CSTATETAPERDELEGATE_H
#define AAX_CSTATETAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor()


/** \brief A linear taper conforming to AAX_ITaperDelegate
	
	\details
	This taper spaces a parameter's real values evenly between its minimum and maximum, with a
	linear mapping between the parameter's real and normalized values.  It is essentially a
	version of AAX_CLinearTaperDelegate without that class' additional RealPrecision
	templatization.

	\ingroup TaperDelegates

 */
template <typename T>
class AAX_CStateTaperDelegate : public AAX_ITaperDelegate<T>
{
public: 
	/** \brief Constructs a State Taper with specified minimum and maximum values.  
	 *
	 *	\note The parameter's default value should lie within the min to max range.
	 *
	 *	\param[in] minValue
	 *	\param[in] maxValue
	 */
	AAX_CStateTaperDelegate(T minValue=0, T maxValue=1);
	
	//Virtual Overrides
	AAX_CStateTaperDelegate<T>*	Clone() const AAX_OVERRIDE;
	T		GetMinimumValue()	const AAX_OVERRIDE			{ return mMinValue; }
	T		GetMaximumValue()	const AAX_OVERRIDE			{ return mMaxValue; }
	T		ConstrainRealValue(T value)	const AAX_OVERRIDE;
	T		NormalizedToReal(double normalizedValue) const AAX_OVERRIDE;
	double	RealToNormalized(T realValue) const AAX_OVERRIDE;
	
private:
	T	mMinValue;
	T	mMaxValue;
};

template <typename T>
AAX_CStateTaperDelegate<T>::AAX_CStateTaperDelegate(T minValue, T maxValue)  :  AAX_ITaperDelegate<T>(),
	mMinValue(minValue),
	mMaxValue(maxValue)
{

}

template <typename T>
AAX_CStateTaperDelegate<T>*		AAX_CStateTaperDelegate<T>::Clone() const
{
	return new AAX_CStateTaperDelegate(*this);
}

template <typename T>
T		AAX_CStateTaperDelegate<T>::ConstrainRealValue(T value)	const
{
	if (mMinValue == mMaxValue)
		return mMinValue;
		
	const T& highValue = mMaxValue > mMinValue ? mMaxValue : mMinValue;
	const T& lowValue = mMaxValue > mMinValue ? mMinValue : mMaxValue;
	
	if (value > highValue)
		return highValue;
	if (value < lowValue)
		return lowValue;
	
	return value;
}

template <typename T>
T		AAX_CStateTaperDelegate<T>::NormalizedToReal(double normalizedValue) const
{
	double doubleRealValue = normalizedValue * (double(mMaxValue) - double(mMinValue)) + double(mMinValue);
	if ( doubleRealValue >= 0 )
		doubleRealValue += 0.5;
	else doubleRealValue -= 0.5;
	return ConstrainRealValue(static_cast<T>(doubleRealValue));
}

template <typename T>
double	AAX_CStateTaperDelegate<T>::RealToNormalized(T realValue) const
{
	realValue = ConstrainRealValue(realValue);
	double normalizedValue = (mMaxValue == mMinValue) ? 0.5 : (double(realValue) - double(mMinValue)) / (double(mMaxValue) - double(mMinValue));
	return normalizedValue;
}
	



#endif //AAX_CSTATETAPERDELEGATE_H
