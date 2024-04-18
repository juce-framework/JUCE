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
 *	\file AAX_CLinearTaperDelegate.h
 *
 *	\brief A linear taper delegate.
 *
 */ 
/*================================================================================================*/


#ifndef	AAX_CLINEARTAPERDELEGATE_H
#define AAX_CLINEARTAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor()


/** \brief A linear taper conforming to AAX_ITaperDelegate
	
	\details
	This taper spaces a parameter's real values evenly between its minimum and maximum, with a
	linear mapping between the parameter's real and normalized values.

	\par RealPrecision
	In addition to its type templatization, this taper includes a precision template parameter.
	RealPrecision is a multiplier that works in conjunction with the round()
	function to limit the precision of the real values provided by this taper.  For example, if
	RealPrecision is 1000, it will round to the closest 0.001 when doing any
	sort of value conversion.  If RealPrecision is 1, it will round to the nearest integer.
	If RealPrecision is 1000000, it will round to the nearest 0.000001.  This
	is particularly useful for preventing things like 1.9999999 truncating down to 1 instead of
	rounding up to 2.
	
	To accomplish this behavior, the taper multiplies its unrounded parameter values by
	RealPrecision, rounds the result to the nearest valid value, then divides RealPrecision
	back out.
	
	Rounding will be disabled if RealPrecision is set to a value less than 1.  This is the default.

	\ingroup TaperDelegates

 */
template <typename T, int32_t RealPrecision=0>
class AAX_CLinearTaperDelegate : public AAX_ITaperDelegate<T>
{
public: 
	/** \brief Constructs a Linear Taper with specified minimum and maximum values.  
	 *
	 *	\note The parameter's default value should lie within the min to max range.
	 *
	 *	\param[in] minValue
	 *	\param[in] maxValue
	 */
	AAX_CLinearTaperDelegate(T minValue=0, T maxValue=1);
	
	//Virtual AAX_ITaperDelegate Overrides
	AAX_CLinearTaperDelegate<T, RealPrecision>*	Clone() const AAX_OVERRIDE;
	T		GetMinimumValue()	const AAX_OVERRIDE			{ return mMinValue; }
	T		GetMaximumValue()	const AAX_OVERRIDE			{ return mMaxValue; }
	T		ConstrainRealValue(T value)	const AAX_OVERRIDE;
	T		NormalizedToReal(double normalizedValue) const AAX_OVERRIDE;
	double	RealToNormalized(T realValue) const AAX_OVERRIDE;
	
protected:
	T	Round(double iValue) const;

private:
	T	mMinValue;
	T	mMaxValue;
};

template <typename T, int32_t RealPrecision>
T	AAX_CLinearTaperDelegate<T, RealPrecision>::Round(double iValue) const
{
	double precision = RealPrecision;
	if (precision > 0)
		return static_cast<T>(floor(iValue * precision + 0.5) / precision);
    return static_cast<T>(iValue);
}

template <typename T, int32_t RealPrecision>
AAX_CLinearTaperDelegate<T, RealPrecision>::AAX_CLinearTaperDelegate(T minValue, T maxValue)  :  AAX_ITaperDelegate<T>(),
	mMinValue(minValue),
	mMaxValue(maxValue)
{

}

template <typename T, int32_t RealPrecision>
AAX_CLinearTaperDelegate<T, RealPrecision>*		AAX_CLinearTaperDelegate<T, RealPrecision>::Clone() const
{
	return new AAX_CLinearTaperDelegate(*this);
}

template <typename T, int32_t RealPrecision>
T		AAX_CLinearTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
{
	if (mMinValue == mMaxValue)
		return mMinValue;
	
	if (RealPrecision)
		value = Round(value);		//reduce the precision to get proper rounding behavior with integers.
	
	const T& highValue = mMaxValue > mMinValue ? mMaxValue : mMinValue;
	const T& lowValue = mMaxValue > mMinValue ? mMinValue : mMaxValue;
	
	if (value > highValue)
		return highValue;
	if (value < lowValue)
		return lowValue;
	
	return value;		
}

template <typename T, int32_t RealPrecision>
T		AAX_CLinearTaperDelegate<T, RealPrecision>::NormalizedToReal(double normalizedValue) const
{
	double doubleRealValue = normalizedValue * (double(mMaxValue) - double(mMinValue)) + double(mMinValue);
	
	// If RealPrecision is set, reduce the precision to get proper rounding behavior with integers.
	T realValue = (0 != RealPrecision) ? Round(doubleRealValue) : static_cast<T>(doubleRealValue);
	
	return ConstrainRealValue(realValue);
}

template <typename T, int32_t RealPrecision>
double	AAX_CLinearTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue) const
{
	realValue = ConstrainRealValue(realValue);
	double normalizedValue = (mMaxValue == mMinValue) ? 0.5 : (double(realValue) - double(mMinValue)) / (double(mMaxValue) - double(mMinValue));
	return normalizedValue;
}
	



#endif //AAX_CLINEARTAPERDELEGATE_H
