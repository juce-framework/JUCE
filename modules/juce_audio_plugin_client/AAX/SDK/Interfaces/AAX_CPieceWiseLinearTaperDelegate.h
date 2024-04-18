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
 *	\file AAX_CPieceWiseLinearTaperDelegate.h
 *
 *	\brief A piece-wise linear taper delegate.
 *
 */ 
/*================================================================================================*/


#ifndef	AAX_CPIECEWISELINEARTAPERDELEGATE_H
#define AAX_CPIECEWISELINEARTAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor()


/** \brief A piece-wise linear taper conforming to AAX_ITaperDelegate
 
 	\details
	 This taper spaces a parameter's real values in a piecewise linear fashion.
 
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
 
	 Rounding will be disabled if RealPrecision is set to a value less than 1
 
	 \ingroup TaperDelegates 
 */
template <typename T, int32_t RealPrecision=100>
class AAX_CPieceWiseLinearTaperDelegate : public AAX_ITaperDelegate<T>
{
public: 
	/** \brief Constructs a Piece-wise Linear Taper with paired normalized and real values.  
	 *
	 *	\note The parameter's default value should lie within the min to max range.
	 *
	 *	\param[in] normalizedValues is an array of the normalized values in sorted order. (make sure to include the full normalized range, 0.0-1.0 inclusive)
	 *	\param[in] realValues is an array of the corresponding real values to the normalized values passed in. 
	 *	\param[in] numValues is the number of values that have been passed in (i.e. the element length of the other input arrays)
	 */
	AAX_CPieceWiseLinearTaperDelegate(const double* normalizedValues, const T* realValues, int32_t numValues);
	
	AAX_CPieceWiseLinearTaperDelegate(const AAX_CPieceWiseLinearTaperDelegate& other);	//Explicit copy constructor because there are internal arrays.
	~AAX_CPieceWiseLinearTaperDelegate();
	
	//Virtual AAX_ITaperDelegate Overrides
	AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>*	Clone() const AAX_OVERRIDE;
	T		GetMinimumValue()	const AAX_OVERRIDE			{ return mMinValue; }
	T		GetMaximumValue()	const AAX_OVERRIDE			{ return mMaxValue; }
	T		ConstrainRealValue(T value)	const AAX_OVERRIDE;
	T		NormalizedToReal(double normalizedValue) const AAX_OVERRIDE;
	double	RealToNormalized(T realValue) const AAX_OVERRIDE;
	
protected:
	T	Round(double iValue) const;
	
private:
	double*		mNormalizedValues;
	T*			mRealValues;
	int32_t		mNumValues;
	T			mMinValue;		//Really just an optimization
	T			mMaxValue;		//Really just an optimization
};

template <typename T, int32_t RealPrecision>
T	AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>::Round(double iValue) const
{
	if (RealPrecision > 0)
		return static_cast<T>(floor(iValue * RealPrecision + 0.5) / RealPrecision);
	else
		return static_cast<T>(iValue);
}

template <typename T, int32_t RealPrecision>
AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>::AAX_CPieceWiseLinearTaperDelegate(const double* normalizedValues, const T* realValues, int32_t numValues)  :  AAX_ITaperDelegate<T>(),
	mNormalizedValues(0),
	mRealValues(0),
	mNumValues(0),
	mMinValue(0),
	mMaxValue(0)
{
	mNormalizedValues = new double[numValues];
	mRealValues = new T[numValues];
	mNumValues = numValues;

	if (numValues > 0)
	{
		mMaxValue = realValues[0];
		mMinValue = realValues[0];
	}
	for (int32_t i=0; i< numValues; i++)
	{
		mNormalizedValues[i] = normalizedValues[i];
		mRealValues[i] = realValues[i];
		if (mRealValues[i] > mMaxValue)
			mMaxValue = mRealValues[i];
		if (mRealValues[i] < mMinValue)
			mMinValue = mRealValues[i];
	}
}

template <typename T, int32_t RealPrecision>
AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>::AAX_CPieceWiseLinearTaperDelegate(const AAX_CPieceWiseLinearTaperDelegate& other)  :  AAX_ITaperDelegate<T>(),
	mNormalizedValues(0),
	mRealValues(0),
	mNumValues(0),
	mMinValue(0),
	mMaxValue(0)
{
	mNormalizedValues = new double[other.mNumValues];
	mRealValues = new T[other.mNumValues];
	mNumValues = other.mNumValues;
	mMaxValue = other.mMaxValue;
	mMinValue = other.mMinValue;
	for (int32_t i=0; i< mNumValues; i++)
	{
		mNormalizedValues[i] = other.mNormalizedValues[i];
		mRealValues[i] = other.mRealValues[i];
	}	
}

template <typename T, int32_t RealPrecision>
AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>::~AAX_CPieceWiseLinearTaperDelegate() 
{
	mNumValues = 0;
	delete [] mNormalizedValues; 
	delete [] mRealValues;
}


template <typename T, int32_t RealPrecision>
AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>*		AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>::Clone() const
{
	return new AAX_CPieceWiseLinearTaperDelegate(*this);
}

template <typename T, int32_t RealPrecision>
T		AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
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
T		AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>::NormalizedToReal(double normalizedValue) const
{

	// Clip to normalized range.
	if (normalizedValue > 1.0)
		normalizedValue = 1.0;
	if (normalizedValue < 0.0)
		normalizedValue = 0.0;
	
	// This is basically linear interpolation so let's first find the bounding normalized points from our specified array.
	int32_t mLowerIndex = 0;
	int32_t mUpperIndex = 0;
	for (int32_t i=1;i<mNumValues;i++)
	{
		mUpperIndex++;
		if (mNormalizedValues[i] >= normalizedValue)
			break;
		mLowerIndex++;
	}
	
	// Do the interpolation.
	double delta = normalizedValue - mNormalizedValues[mLowerIndex];
	double slope = double(mRealValues[mUpperIndex] - mRealValues[mLowerIndex]) / (mNormalizedValues[mUpperIndex] - mNormalizedValues[mLowerIndex]);
	double interpolatedValue = mRealValues[mLowerIndex] + (delta * slope);																															
	return ConstrainRealValue(static_cast<T>(interpolatedValue));
}

template <typename T, int32_t RealPrecision>
double	AAX_CPieceWiseLinearTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue) const
{
	realValue = ConstrainRealValue(realValue);
	
	// This is basically linear interpolation so let's first find the bounding normalized points from our specified array.
	int32_t mLowerIndex = 0;
	int32_t mUpperIndex = 0;
	if (mRealValues[0] < mRealValues[mNumValues-1])
	{
		//Increasing real values (positive slope)
		for (int32_t i=1;i<mNumValues;i++)
		{
			mUpperIndex++;
			if (mRealValues[i] >= realValue)
				break;
			mLowerIndex++;
		}
	}
	else 
	{
		//Decreasing real values (negative slope)
		for (int32_t i=1;i<mNumValues;i++)
		{
			mUpperIndex++;
			if (mRealValues[i] <= realValue)
				break;
			mLowerIndex++;
		}		
	}
	
	// Do the interpolation.
	double delta = realValue - mRealValues[mLowerIndex];
	double slope = (mRealValues[mUpperIndex] == mRealValues[mLowerIndex]) ? 0.5 : double(mNormalizedValues[mUpperIndex] - mNormalizedValues[mLowerIndex]) / (mRealValues[mUpperIndex] - mRealValues[mLowerIndex]);
	double interpolatedValue = mNormalizedValues[mLowerIndex] + (delta * slope);																															
	return static_cast<T>(interpolatedValue);
}




#endif //AAX_CPIECEWISELINEARTAPERDELEGATE_H
