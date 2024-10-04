/*================================================================================================*/
/*
 *
 *	Copyright 2014-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_CommonConversions.h
 *
 */ 
/*================================================================================================*/


#ifndef AAX_COMMONCONVERIONS_H
#define AAX_COMMONCONVERIONS_H

#include <math.h>
#include "AAX.h"


const int32_t k32BitPosMax			= 0x7FFFFFFF;
const int32_t k32BitAbsMax			= 0x80000000;
const int32_t k32BitNegMax			= 0x80000000;

const int32_t k56kFracPosMax			= 0x007FFFFF;	// Positive Max Value 
const int32_t k56kFracAbsMax			= 0x00800000;	// Absolute Max Value. Essentially negative one without the sign extension.
const int32_t k56kFracHalf			= 0x00400000;
const int32_t k56kFracNegOne			= 0xFF800000;	//Note sign extension!!!
const int32_t k56kFracNegMax			= k56kFracNegOne;	//Note sign extension!!!
const int32_t k56kFracZero			= 0x00000000;

const double kOneOver56kFracAbsMax	= 1.0/double(k56kFracAbsMax);
const double k56kFloatPosMax		= double(k56kFracPosMax)/double(k56kFracAbsMax);		//56k Max value represented in floating point format.
const double k56kFloatNegMax		= -1.0;		//56k Min value represented in floating point format.
const double kNeg144DB				= -144.0;
const double kNeg144Gain			= 6.3095734448019324943436013662234e-8; //pow(10.0, kNeg144DB / 20.0);

/**	\brief Convert Gain to dB
 *	
 *	\details
 *	\todo This should be incorporated into parameters' tapers and not called separately
 */
inline double	GainToDB(double aGain)
{
	if (aGain == 0.0)
		return kNeg144DB;
	else
	{
		double	dB;
		
		dB = log10(aGain) * 20.0;
		
		if (dB < kNeg144DB)
			dB = kNeg144DB;
		return (dB);		// convert factor to dB
	}
}

/**	\brief Convert dB to Gain
 *
 *	\details
 *	\todo This should be incorporated into parameters' tapers and not called separately
 */
inline double DBToGain(double dB)
{
	return pow(10.0, dB / 20.0);
}

/**	\brief Convert Long to Double
 *
 *	\details
 *	LongToDouble:	convert 24 bit fixed point in a int32_t to floating point equivalent
 */
inline double LongToDouble (int32_t aLong)
{
	if (aLong > k56kFracPosMax)
		aLong = k56kFracPosMax;
	else if (aLong < k56kFracNegMax)
		aLong = k56kFracNegMax;
	return (double(aLong) * kOneOver56kFracAbsMax);
}

/**	\brief convert floating point equivalent back to int32_t
 */
int32_t DoubleToLong (double aDouble);
	
/**	\brief Convert Double to DSPCoef
 */
inline int32_t DoubleToDSPCoef(double d, double max = k56kFloatPosMax, double min = k56kFloatNegMax)
{
	if(d >= max) // k56kFloatPosMax unless specified by the caller
	{
		return k56kFracPosMax;	
	};
	if(d < min) // k56kFloatNegMax unless specified by the caller
	{
		return k56kFracNegMax; 	
	}
	return static_cast<int32_t>(d*k56kFracAbsMax);
}

/**	\brief Convert DSPCoef to Double
 */
inline double DSPCoefToDouble(int32_t c, int32_t max = k56kFracPosMax, int32_t min = k56kFracNegMax)
{
    if (c > max) // k56kFracPosMax unless specified by the caller
		c = k56kFracPosMax;
	else if (c < min) // k56kFracNegMax unless specified by the caller
		c = k56kFracNegMax;
	return (double(c) * kOneOver56kFracAbsMax);
}

/**	\brief ThirtyTwoBitDSPCoefToDouble
 */
inline double ThirtyTwoBitDSPCoefToDouble(int32_t c)
{
    return DSPCoefToDouble(c, k32BitPosMax, k32BitNegMax);
}

/**	\brief DoubleTo32BitDSPCoefRnd
 */
inline int32_t DoubleTo32BitDSPCoefRnd(double d)
{
    return DoubleToDSPCoef(d, k32BitPosMax, k32BitNegMax);
}

int32_t DoubleTo32BitDSPCoef(double d);
int32_t DoubleToDSPCoefRnd(double d, double max, double min);

#endif // AAX_COMMONCONVERIONS_H
