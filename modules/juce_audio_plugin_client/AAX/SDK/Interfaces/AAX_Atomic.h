/*================================================================================================*/
/*
 *	Copyright 2013-2015, 2018, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_Atomic.h
 *	
 *	\brief Atomic operation utilities
 */ 
/*================================================================================================*/


#pragma once

#ifndef AAX_ATOMIC_H_
#define AAX_ATOMIC_H_

#include "AAX.h"
#include <stdint.h>

#if (!defined AAX_PointerSize)
#error Undefined pointer size
#endif


//! Increments a 32-bit value and returns the result
uint32_t AAX_CALLBACK AAX_Atomic_IncThenGet_32(uint32_t & ioData);

//! Decrements a 32-bit value and returns the result
uint32_t AAX_CALLBACK AAX_Atomic_DecThenGet_32(uint32_t & ioData);

//! Return the original value of ioValue and then set it to inExchangeValue
uint32_t AAX_CALLBACK AAX_Atomic_Exchange_32(
	volatile uint32_t& ioValue,
	uint32_t           inExchangeValue);

//! Return the original value of ioValue and then set it to inExchangeValue
uint64_t AAX_CALLBACK AAX_Atomic_Exchange_64(
	volatile uint64_t& ioValue,
	uint64_t           inExchangeValue);

//! Perform an exchange operation on a pointer value
template<typename TPointer> TPointer* AAX_CALLBACK AAX_Atomic_Exchange_Pointer(TPointer*& ioValue, TPointer* inExchangeValue)
{
#if (AAX_PointerSize == AAXPointer_64bit)
	return (TPointer*)AAX_Atomic_Exchange_64(*(uint64_t*)(void*)&ioValue, (uint64_t)inExchangeValue);
#elif (AAX_PointerSize == AAXPointer_32bit)
	return (TPointer*)AAX_Atomic_Exchange_32(*(uint32_t*)(void*)&ioValue, (uint32_t)inExchangeValue);
#else
#error Unsupported pointer size
#endif
}

//! Perform a compare and exchange operation on a 32-bit value
bool AAX_CALLBACK AAX_Atomic_CompareAndExchange_32(
	volatile uint32_t &	ioValue,
	uint32_t            inCompareValue,
	uint32_t            inExchangeValue);

//! Perform a compare and exchange operation on a 64-bit value
bool AAX_CALLBACK AAX_Atomic_CompareAndExchange_64(
	volatile uint64_t&  ioValue,
	uint64_t            inCompareValue,
	uint64_t            inExchangeValue);

//! Perform a compare and exchange operation on a pointer value
template<typename TPointer> bool AAX_CALLBACK AAX_Atomic_CompareAndExchange_Pointer(TPointer*& ioValue, TPointer* inCompareValue, TPointer* inExchangeValue)
{
#if (AAX_PointerSize == AAXPointer_64bit)
	return AAX_Atomic_CompareAndExchange_64(*(uint64_t*)(void*)&ioValue, (uint64_t)inCompareValue, (uint64_t)inExchangeValue);
#elif (AAX_PointerSize == AAXPointer_32bit)
	return AAX_Atomic_CompareAndExchange_32(*(uint32_t*)(void*)&ioValue, (uint32_t)inCompareValue, (uint32_t)inExchangeValue);
#else
#error Unsupported pointer size
#endif
}

//! Atomically loads a pointer value
template<typename TPointer> TPointer* AAX_CALLBACK AAX_Atomic_Load_Pointer(TPointer const * const volatile * inValue);



//TODO: Update all atomic function implementatons with proper acquire/release semantics


// GCC/LLVM
#if defined(__GNUC__)

// These intrinsics require GCC 4.2 or later
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))

inline uint32_t AAX_CALLBACK
AAX_Atomic_IncThenGet_32(uint32_t& ioData)
{
	return __sync_add_and_fetch(&ioData, 1);
}

inline uint32_t AAX_CALLBACK
AAX_Atomic_DecThenGet_32(uint32_t& ioData)
{
	return __sync_sub_and_fetch(&ioData, 1);
}

inline uint32_t
AAX_Atomic_Exchange_32(
	volatile uint32_t& ioValue,
	uint32_t           inExchangeValue)
{
	return __sync_lock_test_and_set(&ioValue, inExchangeValue);
}

inline uint64_t
AAX_Atomic_Exchange_64(
	volatile uint64_t& ioValue,
	uint64_t           inExchangeValue)
{
	return __sync_lock_test_and_set(&ioValue, inExchangeValue);
}

inline bool
AAX_Atomic_CompareAndExchange_32(
	volatile uint32_t &	ioValue,
	uint32_t            inCompareValue,
	uint32_t            inExchangeValue)
{
	return __sync_bool_compare_and_swap(&ioValue, inCompareValue, inExchangeValue);
}

inline bool
AAX_Atomic_CompareAndExchange_64(
	volatile uint64_t&  ioValue,
	uint64_t            inCompareValue,
	uint64_t            inExchangeValue)
{
	return __sync_bool_compare_and_swap(&ioValue, inCompareValue, inExchangeValue);
}

//TODO: Add GCC version check and alternative implementations for GCC versions that do not support __atomic_load
template<typename TPointer>
inline TPointer*
AAX_Atomic_Load_Pointer(TPointer const * const volatile * inValue)
{
	TPointer* value;
	__atomic_load(const_cast<TPointer * volatile *>(inValue), &(value), __ATOMIC_ACQUIRE);
	return value;
}

#else // (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#error This file requires GCC 4.2 or later
#endif // (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
// End GCC/LLVM





// Visual C
#elif defined(_MSC_VER)

#ifndef __INTRIN_H_
#include <intrin.h>
#endif

#pragma intrinsic(	_InterlockedIncrement, \
					_InterlockedDecrement, \
					_InterlockedExchange, \
					_InterlockedCompareExchange, \
					_InterlockedCompareExchange64)

inline uint32_t AAX_CALLBACK
AAX_Atomic_IncThenGet_32(register uint32_t& ioData)
{
	return static_cast<uint32_t>(_InterlockedIncrement((volatile long*)&ioData));
}

inline uint32_t AAX_CALLBACK
AAX_Atomic_DecThenGet_32(register uint32_t& ioData)
{
	return static_cast<uint32_t>(_InterlockedDecrement((volatile long*)&ioData));
}

inline uint32_t
AAX_Atomic_Exchange_32(
	volatile uint32_t& ioDestination,
	uint32_t           inExchangeValue)
{
	return static_cast<uint32_t>(_InterlockedExchange((volatile long*)&ioDestination, (long)inExchangeValue));
}

#if (AAX_PointerSize == AAXPointer_64bit)

#pragma intrinsic(	_InterlockedExchange64, \
					_InterlockedOr64)

inline uint64_t
AAX_Atomic_Exchange_64(
	volatile uint64_t& ioValue,
	uint64_t           inExchangeValue)
{
	return static_cast<uint64_t>(_InterlockedExchange64((volatile __int64*)&ioValue, (__int64)inExchangeValue));
}

template<typename TPointer>
inline TPointer*
AAX_Atomic_Load_Pointer(TPointer const * const volatile * inValue)
{
// Itanium supports acquire semantics
#if (_M_IA64)
	return reinterpret_cast<TPointer*>(_InterlockedOr64_acq(const_cast<__int64 volatile *>(reinterpret_cast<const __int64 volatile *>(inValue)), 0x0000000000000000));
#else
	return reinterpret_cast<TPointer*>(_InterlockedOr64(const_cast<__int64 volatile *>(reinterpret_cast<const __int64 volatile *>(inValue)), 0x0000000000000000));
#endif
}

#elif (AAX_PointerSize == AAXPointer_32bit)

#pragma intrinsic(	_InterlockedOr )

// _InterlockedExchange64 is not available on 32-bit Pentium in Visual C
inline uint64_t
AAX_Atomic_Exchange_64(
	volatile uint64_t& ioValue,
	uint64_t           inExchangeValue)
{
	for(;;)
	{
		uint64_t	result = ioValue;
		if(AAX_Atomic_CompareAndExchange_64(ioValue, result, inExchangeValue))
		{
			return result;
		}
	}
	
	return 0;	// will never get here
}

template<typename TPointer>
inline TPointer*
AAX_Atomic_Load_Pointer(TPointer const * const volatile * inValue)
{
	return reinterpret_cast<TPointer*>(_InterlockedOr(const_cast<long volatile *>(reinterpret_cast<const long volatile *>(inValue)), 0x00000000));
}

#endif

inline bool
AAX_Atomic_CompareAndExchange_32(
	uint32_t volatile &	ioValue,
	uint32_t            inCompareValue,
	uint32_t            inExchangeValue)
{
	return static_cast<uint32_t>(_InterlockedCompareExchange((volatile long*)&ioValue, (long)inExchangeValue, (long)inCompareValue)) == inCompareValue;
}

inline bool
AAX_Atomic_CompareAndExchange_64(
	volatile uint64_t&  ioValue,
	uint64_t            inCompareValue,
	uint64_t            inExchangeValue)
{
	return static_cast<uint64_t>(_InterlockedCompareExchange64((volatile __int64*)&ioValue, (__int64)inExchangeValue, (__int64)inCompareValue)) == inCompareValue;
}

// End Visual C





#else // Not Visual C or GCC/LLVM
#error Provide an atomic operation implementation for this compiler
#endif // Compiler version check

#endif // #ifndef AAX_ATOMIC_H_
