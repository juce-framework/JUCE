/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_SliderConversions.h
 *	
 *	\brief Legacy utilities for converting parameter values to and from the normalized full-scale
 *	32-bit fixed domain that was used for RTAS/TDM plug-ins.
 *
 *	\details
 *	\legacy These utilities may be required in order to maintain settings chunk compatibility with
 *	plug-ins that were ported from the legacy RTAS/TDM format.
 *									
 *	\note %AAX does not provide facilities for converting to and from extended80 data types.  If you
 *	use these types in your plug-in settings then you must provide your own chunk data parsing 
 *	routines.
 *
 */ 
/*================================================================================================*/


#pragma once

#ifndef AAX_SLIDERCONVERSIONS_H
#define AAX_SLIDERCONVERSIONS_H

#include "AAX.h"
#include <algorithm>
#include <stdint.h>


#define AAX_LIMIT(v1,firstVal,secondVal) ( (secondVal > firstVal) ? (std::max)((std::min)(v1,secondVal),firstVal) :  (std::min)((std::max)(v1,secondVal),firstVal) )

int32_t LongControlToNewRange (int32_t aValue, int32_t rangeMin, int32_t rangeMax);

/**	\brief Convert from int32_t control value 0x80000000...0x7FFFFFFF
 *	to a int32_t ranging from rangeMin to rangeMax (linear)
 */
int32_t LongToLongControl (int32_t aValue, int32_t rangeMin, int32_t rangeMax);

/**	\brief Convert from int32_t control value 0x80000000...0x7FFFFFFF
 *	to an double ranging from firstVal to secondVal (linear)
 */
double LongControlToDouble(int32_t aValue, double firstVal, double secondVal);

/**	\brief Convert from an double ranging from firstVal to secondVal (linear)
 *	to int32_t control value 0x80000000...0x7FFFFFFF 
 */
int32_t DoubleToLongControl (double aValue, double firstVal, double secondVal);

int32_t DoubleToLongControlNonlinear(double aValue, double* minVal, double* rangePercent, int32_t numRanges);
double LongControlToDoubleNonlinear(int32_t aValue, double* minVal, double* rangePercent, int32_t numRanges);

/**	\brief Convert from int32_t control value 0x80000000...0x7FFFFFFF
 *	to an double ranging from minVal to maxVal (logarithmic)
 *	
 *	\details
 *	\note This is LOGARITHMIC, so minVal & maxVal have to be > zero!
 */
double LongControlToLogDouble(int32_t aValue, double minVal, double maxVal);

/**	\brief Convert from an double ranging from minVal to maxVal (logarithmic)
 *	to int32_t control value 0x80000000...0x7FFFFFFF 
 *	
 *	\details
 *	\note This is LOGARITHMIC, so minVal & maxVal have to be > zero!
 */
int32_t LogDoubleToLongControl(double aValue, double minVal, double maxVal);

#endif // AAX_SLIDERCONVERSIONS_H

