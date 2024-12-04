/*================================================================================================*/
/*
 *	Copyright 2007-2015, 2023-2024 Avid Technology, Inc.
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
/*================================================================================================*/
#include "AAX_SliderConversions.h"

#include "AAX_UtilsNative.h"
#include "AAX.h"

// Standard headers
#include <climits>
#include <cmath>


using namespace std;


static const double kControlMin = -2147483648.0;
static const double kControlMax = 2147483647.0;
/*===================================================================================================*/
int32_t LongControlToNewRange (int32_t aValue, int32_t rangeMin, int32_t rangeMax)
{
	double controlPartial = ((double)aValue - kControlMin) / (kControlMax - kControlMin);
	return int32_t(floor(rangeMin +  controlPartial*((double)(rangeMax) - (double)(rangeMin)) + 0.5));
}

/*===================================================================================================*/
int32_t LongToLongControl (int32_t aValue, int32_t rangeMin, int32_t rangeMax)
{
	double controlMin = -2147483648.0;
	double controlMax = 2147483647.0;

	if (aValue > rangeMax)
		aValue = rangeMax;
	else if (aValue < rangeMin)
		aValue = rangeMin;
		
	double controlFraction = ((double)aValue - (double)rangeMin) / ((double)rangeMax - (double)rangeMin);
	double control = controlFraction * (controlMax - controlMin) + controlMin;
	return (int32_t)control;
}


/*===================================================================================================
	- 8/15/07 - Changed optimization level to -O0 (no optimization) in XCode project file to fix bug 
	96679 in LongControlToDouble. - MJH
===================================================================================================*/
double LongControlToDouble(int32_t aValue, double firstVal, double secondVal)
{
	// convert from int32_t control value 0x80000000...0x7FFFFFFF
	// to an double ranging from firstVal to secondVal (linear)
	
	double controlPartial = ((double)aValue - kControlMin) / (kControlMax - kControlMin);
	return firstVal + controlPartial*(secondVal - firstVal);
}


/*===================================================================================================*/
int32_t DoubleToLongControl(double aValue, double firstVal, double secondVal)
{
	// convert from an double ranging from firstVal to secondVal (linear)
	// to int32_t control value 0x80000000...0x7FFFFFFF 

	aValue = AAX_LIMIT(aValue,firstVal,secondVal);
	
	double controlPartial = (aValue - firstVal) / (secondVal - firstVal);
	return int32_t(floor(kControlMin + controlPartial*(kControlMax - kControlMin)+0.5));
}


// The 2 following routines map between piecewise linear ranges of floating point values
// and a 32-bit control value.  You must pass in a pointer to an array of range endpoints that
// define the linear ranges and a pointer to an array of 'percents' that indicate the percentage 
// used by each range relative to the entire range taken by all the linear pieces.  Here is example code:
/*
	// This example shows a control that ranges from .10 to 20.0 with three ranges.
	
	const int32_t cNumControlRanges = 3;

	double mControlRangePoints[cNumControlRanges + 1] = {.10, 1.0, 10.0, 20.0};
	double mControlRangePercents[cNumControlRanges];
	
	const double cNumStepsControlRange1	90.0
	const double cNumStepsControlRange2	90.0
	const double cNumStepsControlRange3	10.0
	
	const double cNumStepsControl = cNumStepsControlRange1 + cNumStepsControlRange2 + cNumStepsControlRange3;

	mControlRangePercents[0] = cNumStepsControlRange1/cNumStepsControl;
	mControlRangePercents[1] = cNumStepsControlRange2/cNumStepsControl;
	mControlRangePercents[2] = cNumStepsControlRange3/cNumStepsControl;
	
	double controlValue = 1.5;
	
	int32_t longValue = ExtToLongControlNonlinear(controlValue, mControlRangePoints, mControlRangePercents, kNumControlRanges);
	
	controlValue = LongControlToExtNonlinear(longValue, mControlRangePoints, mControlRangePercents, kNumControlRanges);
*/

/*===================================================================================================*/
int32_t DoubleToLongControlNonlinear(double aValue, double* range, double* rangePercent, int32_t numRanges)
{
	int32_t	extSt;
	int32_t	i = 0;
	double percentTotal = 0.0;
	
	aValue = AAX_LIMIT(aValue,range[0],range[numRanges]);	//limit input to lowest range and highest range

	while (i < numRanges)
	{
		if ((aValue >= range[i])  && (aValue < range[i+1]))
			break;
		percentTotal += rangePercent[i];
		i++;
	}

	if (i == numRanges)		// if aValue == range[numRanges] = maximum possible value
		percentTotal = 1.0;	// our control is 100% of maximum
	else
		percentTotal += (aValue - range[i])/(range[i+1] - range[i]) * rangePercent[i];
	
	double val = (double)AAX_INT32_MIN + ((double)AAX_INT32_MAX - (double)AAX_INT32_MIN) * percentTotal;
	extSt = (int32_t)val;
	return(extSt);
}


/*===================================================================================================*/
double LongControlToDoubleNonlinear(int32_t aValue, double* range, double* rangePercent, int32_t numRanges)
{
	int32_t	i = 0;
	double percentTotal = ((double)AAX_INT32_MIN - (double)aValue)/((double)AAX_INT32_MIN)/2.0;
	double percent = 0.0;
	double extValue;
	
	while (i < numRanges)
	{
		if ((percentTotal >= percent)  && (percentTotal < (percent+rangePercent[i])))
			break;
		percent += rangePercent[i];
		i++;
	}

	// percentTotal will always be slightly < 1.0, even when aValue == LONG_MAX // YS: use INT32_MAX; sizeof(long) == 64 on x64 mac!
	// Therefore this check is not strictly necessary, but is provided for consistency

	if (i == numRanges)			// if percentTotal == 1.0 = maximum possible value
		extValue = AAX_INT32_MAX;	// our control is 100% of maximum
	else
		extValue = range[i] + (range[i+1] - range[i]) * (percentTotal - percent)/(rangePercent[i]);
	
	return(extValue);
}

/*===================================================================================================*/
double LongControlToLogDouble(int32_t aValue, double minVal, double maxVal)
{
	// convert from int32_t control value 0x80000000...0x7FFFFFFF
	// to an double ranging from minVal to maxVal (logarithmic)
	// NOTE!!!!  This is LOGARITHMIC, so minVal & maxVal have to be > zero!

	double	extSt;
	extSt = exp(LongControlToDouble(aValue, AAX::SafeLog(minVal), AAX::SafeLog(maxVal)));
	// Guard against numerical inaccuracies
	if(extSt < minVal) extSt = minVal;
	if(extSt > maxVal) extSt = maxVal;
	return(extSt);
}


/*===================================================================================================*/
int32_t LogDoubleToLongControl(double aValue, double minVal, double maxVal)
{
	// convert from an double ranging from minVal to maxVal (logarithmic)
	// to int32_t control value 0x80000000...0x7FFFFFFF 
	// NOTE!!!!  This is LOGARITHMIC, so minVal & maxVal have to be > zero!

	int32_t	extSt;

	aValue = AAX_LIMIT(aValue,minVal,maxVal);
	extSt = DoubleToLongControl(AAX::SafeLog(aValue),AAX::SafeLog(minVal),AAX::SafeLog(maxVal));
	return(extSt);
}
