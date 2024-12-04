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
 *	\file AAX_EndianSwap.h
 *
 *	\brief Utility functions for byte-swapping.  Used by AAX_CChunkDataParser.
 *
 */ 
/*================================================================================================*/


#pragma once

#ifndef	ENDIANSWAP_H
#define	ENDIANSWAP_H

// Standard headers
#include <algorithm>

//! \brief Byte swap data in-place.
template<class T>	void AAX_EndianSwapInPlace(T* theDataP);

//! \brief Make a byte-swapped copy of data.
template<class T>	T AAX_EndianSwap(T theData);


//! \brief Convert data in-place between Big Endian and native byte ordering.
template<class T>	void AAX_BigEndianNativeSwapInPlace(T* theDataP);

//! \brief Copy and convert data between Big Endian and native byte ordering.
template<class T>	T AAX_BigEndianNativeSwap(T theData);

//! \brief Convert data in-place from the native byte ordering to Little Endian byte ordering.
template<class T>	void AAX_LittleEndianNativeSwapInPlace(T* theDataP);

//! \brief Copy and convert data from the native byte ordering to Little Endian byte ordering.
template<class T>	T AAX_LittleEndianNativeSwap(T theData);



//! \brief Byte swap a sequence of data in-place.
template<class Iter>	void AAX_EndianSwapSequenceInPlace(Iter beginI, Iter endI);


//! \brief Convert an sequence of data in-place between Big Endian and native byte ordering.
template<class Iter>	void AAX_BigEndianNativeSwapSequenceInPlace(Iter beginI, Iter endI);

//! \brief Convert an sequence of data in-place from the native byte ordering to Little Endian byte ordering.
template<class Iter>	void AAX_LittleEndianNativeSwapSequenceInPlace(Iter beginI, Iter endI);


//
// Implementations
//

template<class T>
inline 
void
AAX_EndianSwapInPlace(T* theDataP)
{
	char *begin, *end;

	begin = reinterpret_cast<char*>(theDataP);
	end = begin + sizeof( T );
	std::reverse( begin, end );
}


template<class T>
inline 
T AAX_EndianSwap(T theData)
{
	AAX_EndianSwapInPlace(&theData);
	return theData;
}


template<class T>
inline 
void AAX_BigEndianNativeSwapInPlace(T* theDataP)
{
#if (!defined __BIG_ENDIAN__) || (0 == __BIG_ENDIAN__)
	AAX_EndianSwapInPlace(theDataP);
#endif
}


template<class T>	
inline 
T AAX_BigEndianNativeSwap(T theData)
{
	AAX_BigEndianNativeSwapInPlace(&theData);
	return theData;
}


template<class T>	
inline 
void AAX_LittleEndianNativeSwapInPlace(T* theDataP)
{
#if (defined __BIG_ENDIAN__) && (0 != __BIG_ENDIAN__)
	AAX_EndianSwapInPlace(theDataP);
#endif
}


template<class T>	
inline 
T AAX_LittleEndianNativeSwap(T theData)
{
	AAX_LittleEndianNativeSwapInPlace(&theData);
	return theData;
}


template<class Iter>	
inline
void AAX_EndianSwapSequenceInPlace(Iter beginI, Iter endI)
{
	for(Iter i = beginI; i != endI; ++i)
	{
		// WARNING : Will this give a compile error if a use mistakenly uses it on a const sequence?
		AAX_EndianSwapInPlace(&(*i));
	};
}


template<class Iter>	
inline
void AAX_BigEndianNativeSwapSequenceInPlace(Iter beginI, Iter endI)
{
#if (!defined __BIG_ENDIAN__) || (0 == __BIG_ENDIAN__)
	AAX_EndianSwapSequenceInPlace(beginI, endI);
#endif
}


template<class Iter>	
inline
void AAX_LittleEndianNativeSwapSequenceInPlace(Iter beginI, Iter endI)
{
#if (defined __BIG_ENDIAN__) && (0 != __BIG_ENDIAN__)
	AAX_EndianSwapSequenceInPlace(beginI, endI);
#endif
}

#endif
