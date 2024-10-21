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
 *	\file  AAX_CRangeTaperDelegate.h
 *
 *	\brief A range taper delegate decorator.
 *
 */ 
/*================================================================================================*/


#ifndef	AAX_CRANGETAPERDELEGATE_H
#define AAX_CRANGETAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"
#include "AAX.h"	//for types

#include <cmath>	//for floor()
#include <vector>


/** \brief A piecewise-linear taper conforming to AAX_ITaperDelegate
	
	\details
	This taper spaces a parameter's real values between its minimum and maximum using a series
	of linear regions to create the full mapping between the parameter's real and
	normalized values.

	Here is an example of how this taper can be used:

	\code
		float rangePoints[] = { 0.0, 1.0, 100.0, 1000.0, 2000.0 };
		double rangeSteps[] = { 0.1, 1.0, 10.0, 25.0 }; // number of steps per range: 10, 99, 90, 40 
		const long cNumRanges = sizeof(rangeSteps)/sizeof(rangeSteps[0]);
	
		long numSteps = 0;
		for (int i = 0; i < cNumRanges; i++)
		{
			numSteps += (rangePoints[i+1] - rangePoints[i]) / rangeSteps[i];
		}
	
		AAX_CRangeTaperDelegate<float> nonLinearTaper(rangePoints, rangeSteps, cNumRanges);
	
		float controlValue = 1.5;
	
		double normalized = nonLinearTaper.RealToNormalized(controlValue);
		float real = nonLinearTaper.NormalizedToReal(normalized);
	\endcode

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
template <typename T, int32_t RealPrecision=1000>
class AAX_CRangeTaperDelegate : public AAX_ITaperDelegate<T>
{
public: 
	/** \brief Constructs a Range Taper with specified minimum and maximum values.  
	 *
	 *	\note The parameter's default value should lie within the min to max range.
	 *
	 *	\param[in] range
	 *		An array of range endpoints along the taper's mapping range
	 *	\param[in] rangesSteps
	 *		Step values for each region in the taper's stepwise-linear map. No values in this array may be zero.
	 *	\param[in] numRanges
	 *		The total number of linear regions in the taper's map
	 *	\param[in] useSmartRounding
	 *		\todo Document useSmartRounding parameter
	 */
	AAX_CRangeTaperDelegate(T* range, double* rangesSteps, unsigned long numRanges, bool useSmartRounding = true);
	AAX_CRangeTaperDelegate( const AAX_CRangeTaperDelegate& rhs);
	AAX_CRangeTaperDelegate& operator=( AAX_CRangeTaperDelegate& rhs );

	//Virtual Overrides
	AAX_CRangeTaperDelegate<T, RealPrecision>*	Clone() const AAX_OVERRIDE;
	T		GetMinimumValue()	const AAX_OVERRIDE			{ return mMinValue; }
	T		GetMaximumValue()	const  AAX_OVERRIDE			{ return mMaxValue; }
	T		ConstrainRealValue(T value)	const AAX_OVERRIDE;
	T		NormalizedToReal(double normalizedValue) const AAX_OVERRIDE;
	double	RealToNormalized(T realValue) const AAX_OVERRIDE;

protected:
	T	Round(double iValue) const;
	T	SmartRound(double value) const;		///< \todo Document

private:
	T		mMinValue;
	T		mMaxValue;
	unsigned long	mNumRanges;
	std::vector<T> mRanges;
	std::vector<double> mRangesSteps;		///< \todo Document
	std::vector<double> mRangesPercents;	///< \todo Document
	std::vector<double> mRangesStepsCount;	///< \todo Document
	bool	mUseSmartRounding;
};

template <typename T, int32_t RealPrecision>
AAX_CRangeTaperDelegate<T, RealPrecision>::AAX_CRangeTaperDelegate(T* ranges, double* rangesSteps, unsigned long numRanges, bool useSmartRounding) :
	AAX_ITaperDelegate<T>(),
	mMinValue(*ranges), 
	mMaxValue(*(ranges + numRanges)),
	mNumRanges(numRanges),
	mRanges( ranges, ranges + numRanges + 1),
	mRangesSteps( rangesSteps, rangesSteps + numRanges),
	mUseSmartRounding( useSmartRounding )
{
	mRangesStepsCount.reserve(mNumRanges);
	mRangesPercents.reserve(mNumRanges);
	unsigned int i = 0;
	for (; i < mNumRanges; i++)
	{
		mRangesStepsCount.push_back( (mRanges.at(i + 1) - mRanges.at(i)) / mRangesSteps.at(i));
	}
	double numSteps = 0;
	for (i = 0; i < mNumRanges; i++)
	{
		numSteps += mRangesStepsCount.at(i);
	}
	for (i = 0; i < mNumRanges; i++)
	{
		mRangesPercents.push_back( mRangesStepsCount.at(i) / numSteps );
	}
}

template <typename T, int32_t RealPrecision>
AAX_CRangeTaperDelegate<T, RealPrecision>::AAX_CRangeTaperDelegate( const AAX_CRangeTaperDelegate<T, RealPrecision>& rhs) :
	mMinValue(rhs.mMinValue),
	mMaxValue(rhs.mMaxValue),
	mNumRanges(rhs.mNumRanges),
	mRanges( rhs.mRanges.begin(), rhs.mRanges.end()),
	mRangesSteps( rhs.mRangesSteps.begin(), rhs.mRangesSteps.end()),
	mRangesPercents( rhs.mRangesPercents.begin(), rhs.mRangesPercents.end()),
	mRangesStepsCount( rhs.mRangesStepsCount.begin(), rhs.mRangesStepsCount.end()),
	mUseSmartRounding( rhs.mUseSmartRounding )
{
}

template <typename T, int32_t RealPrecision>
AAX_CRangeTaperDelegate<T, RealPrecision>& AAX_CRangeTaperDelegate<T, RealPrecision>::operator=( AAX_CRangeTaperDelegate<T, RealPrecision>& rhs)
{
	if (this == &rhs)
		return *this;

	this->mMinValue = rhs.mMinValue;
	this->mMaxValue = rhs.mMaxValue;
	this->mNumRanges = rhs.mNumRanges;
	this->mRanges.assign( rhs.mRanges.begin(), rhs.mRanges.end());
	this->mRangesSteps.assign( rhs.mRangesSteps.begin(), rhs.mRangesSteps.end());
	this->mRangesPercents.assign( rhs.mRangesPercents.begin(), rhs.mRangesPercents.end());
	this->mRangesStepsCount.assign( rhs.mRangesStepsCount.begin(), rhs.mRangesStepsCount.end());

	return *this;
}

template <typename T, int32_t RealPrecision>
T	AAX_CRangeTaperDelegate<T, RealPrecision>::Round(double iValue) const
{
	return ((0 >= RealPrecision) ?	static_cast<T>(iValue) :
			(0 <= iValue) ?			static_cast<T>(floor( iValue*RealPrecision + 0.5f ) / RealPrecision) :
									static_cast<T>(ceil( iValue*RealPrecision - 0.5f ) / RealPrecision)
			);
}

template <typename T, int32_t RealPrecision>
AAX_CRangeTaperDelegate<T, RealPrecision>*		AAX_CRangeTaperDelegate<T, RealPrecision>::Clone() const
{
	return new AAX_CRangeTaperDelegate<T, RealPrecision>(*this);
}

template <typename T, int32_t RealPrecision>
T		AAX_CRangeTaperDelegate<T, RealPrecision>::ConstrainRealValue(T value)	const
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
T		AAX_CRangeTaperDelegate<T, RealPrecision>::NormalizedToReal(double normalizedValue)		const
{
	double percentTotal = normalizedValue;

	double percent = 0.0;
	unsigned long i = 0;
	for (; i < mNumRanges; i++)
	{
		if ((percentTotal >= percent) && (percentTotal < (percent + mRangesPercents.at( i ) )))
			break;
		percent += mRangesPercents.at( i );
	}

	double extValue;
	if (i == mNumRanges)
		extValue = mMaxValue;	// our control is 100% of maximum
	else
		extValue = mRanges.at(i) + ((mRanges.at(i+1) - mRanges.at(i))*(percentTotal - percent)) / (mRangesPercents.at(i));

	T realValue = T(extValue);
	if ( mUseSmartRounding )
		realValue = SmartRound(extValue);		//reduce the precision to get proper rounding behavior with integers.

	return ConstrainRealValue(realValue);
}

template <typename T, int32_t RealPrecision>
double	AAX_CRangeTaperDelegate<T, RealPrecision>::RealToNormalized(T realValue)	const
{
	realValue = ConstrainRealValue(realValue);

	double percentTotal = 0.0;
	unsigned long i = 0;
	for (; i < mNumRanges; i++)
	{
		if ((realValue >= mRanges[i]) && (realValue < mRanges[i+1]))
			break;
		percentTotal += mRangesPercents[i];
	}

	if (i == mNumRanges)
		percentTotal = 1.0;		// our control is 100% of maximum
	else if (mRanges.at(i + 1) == mRanges.at(i))
		; // no action; total percent does not change
	else
		percentTotal += (realValue - mRanges.at(i))/static_cast<double>(mRanges.at(i + 1) - mRanges.at(i)) * mRangesPercents.at(i);

	double normalizedValue = percentTotal;
	return normalizedValue;
}

template <typename T, int32_t RealPrecision>
T	AAX_CRangeTaperDelegate<T, RealPrecision>::SmartRound(double value) const
{
	unsigned long	i = 0;
	for (; i < mNumRanges; i++)
	{
		if ((value >= mRanges.at(i)) && (value < mRanges.at(i + 1) ))
			break;
		if ( i == mNumRanges - 1 )
			break;
	}

	int32_t longVal = 0;
	if (value >= 0)
		longVal = int32_t(floor(value / mRangesSteps.at(i) + 0.5));
	else
		longVal = int32_t(ceil(value / mRangesSteps.at(i) - 0.5));

	return static_cast<T>(static_cast<double>(longVal) * mRangesSteps.at(i));
}


#endif
