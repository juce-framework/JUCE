/*================================================================================================*/
/*
 *	Copyright 2015, 2023-2024 Avid Technology, Inc.
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
 *  \file AAX_CAtomicQueue.h
 *
 *	\brief	Atomic, non-blocking queue
 *
 */
/*================================================================================================*/
/// @cond ignore
#ifndef AAX_CATOMICQUEUE_H
#define AAX_CATOMICQUEUE_H
/// @endcond


// AAX Includes
#include "AAX_IPointerQueue.h"
#include "AAX_Atomic.h"
#include "AAX_CMutex.h"

// Standard Includes
#include <cstring>


/** Multi-writer, single-reader implementation of \ref AAX_IPointerQueue
 
 @details
 Template parameters:
 - \c T: Type of the objects pointed to by this queue
 - \c S: Size of the queue's ring buffer. Should be a power of two less than \c UINT_32_MAX
 
 Properties:
 - Read operations are non-blocking
 - Write operations are synchronized, but very fast
 - Supports only one read thread - do not call \ref Pop() or \ref Peek() concurrently
 - Supports any number of write threads
 - Does not support placing \c NULL values onto the queue. \ref AAX_CAtomicQueue<>::Push() "Push"
   will return \ref eStatus_Unsupported if a \c NULL value is pushed onto the queue, and
   the value will be ignored.
 
 */
template <typename T, size_t S>
class AAX_CAtomicQueue : public AAX_IPointerQueue<T>
{
public:
	virtual ~AAX_CAtomicQueue() {}
	AAX_CAtomicQueue();
	
public:
	static const size_t template_size = S;                               ///< The size used for this template instance
	
	typedef typename AAX_IPointerQueue<T>::template_type template_type;  ///< @copydoc AAX_IPointerQueue::template_type
	typedef typename AAX_IPointerQueue<T>::value_type value_type;        ///< @copydoc AAX_IPointerQueue::value_type
	
public: // AAX_IContainer
	virtual void Clear();                                                ///< @copydoc AAX_IPointerQueue::Clear()
	
public: // AAX_IPointerQueue
	virtual AAX_IContainer::EStatus Push(value_type inElem);             ///< @copydoc AAX_IPointerQueue::Push()
	virtual value_type Pop();                                            ///< @copydoc AAX_IPointerQueue::Pop()
	virtual value_type Peek() const;                                     ///< @copydoc AAX_IPointerQueue::Peek()
	
private:
	AAX_CMutex mMutex;
	uint32_t mReadIdx;
	uint32_t mWriteIdx;
	value_type mRingBuffer[S];
};


/// @cond ignore

template <typename T, size_t S>
inline AAX_CAtomicQueue<T, S>::AAX_CAtomicQueue()
: AAX_IPointerQueue<T>()
, mMutex()
, mReadIdx(0)
, mWriteIdx(0)
{
	Clear();
}

template <typename T, size_t S>
inline void AAX_CAtomicQueue<T, S>::Clear()
{
	std::memset((void*)mRingBuffer, 0x0, sizeof(mRingBuffer));
}

template <typename T, size_t S>
inline AAX_IContainer::EStatus AAX_CAtomicQueue<T, S>::Push(typename AAX_CAtomicQueue<T, S>::value_type inElem)
{
	if (NULL == inElem)
	{
		return AAX_IContainer::eStatus_Unsupported;
	}
	
	AAX_IContainer::EStatus result = AAX_IContainer::eStatus_Unavailable;
	
	AAX_StLock_Guard guard(mMutex);
    //
    // Possible failure case without mutex is because of several write threads try to modify
    // mWriteIdx concurrently
    //
    // Example:
    //
    // -
    // Notation:
    // First number  - write thread number
    // Second number - value number
    // 1/15 - 1st thread that write number 15
    //
    // -
    // Queue may look like this:
    //              mReadIdx
    //                 |
    // |..... | 4/3 | 4/4 | 1/4 | 1/5 | 2/7 | 2/8 | 2/9 | .....|
    //                 |
    //              mWriteIdx
    // place# |  0  |  1  |  2  |  3  |  4  |  5  |  6  | .....|
    //
    // -
    // Possible operation order (w stands for mWriteIdx, r - for mReadIdx):
    //-------------------------------------------------------
    // thread#| action | write index value | mWriteIdx       |
    //        |        | internal variable |                 |
    //-------------------------------------------------------
    //    5   |   w++  |        2          |        2        |
    //-------------------------------------------------------
    //    6   |   w++  |        3          |        3        |
    //-------------------------------------------------------
    //    5   |  false |        -          | 2not=3 => 2--=1 |
    //-------------------------------------------------------
    //  read  |   r++  |        -          |        -        |
    //-------------------------------------------------------
    //  read  |   r++  |        -          |        -        |
    //-------------------------------------------------------
    // -
    // Queue state:
    //                         mReadIdx
    //                             |
    // |..... | 4/3 |  0  |  0  | 1/5 | 2/7 | 2/8 | 2/9 | .....|
    //                 |
    //              mWriteIdx
    // place# |  0  |  1  |  2  |  3  |  4  |  5  |  6  | .....|
    //
    // -
    //-------------------------------------------------------
    //    6   | false  |        -          | 3not=1 => 3--=2 | // place 3 is still not empty to write
    //-------------------------------------------------------
    //
    // -
    // Now, some other thread (5, for example) can successfully write
    // it's value to queue and move mWriteIdx forward:
    //
    // -
    // Queue state:
    //                         mReadIdx
    //                             |
    // |..... | 4/3 |  0  | 5/1 | 1/5 | 2/7 | 2/8 | 2/9 | .....|
    //                             |
    //                         mWriteIdx
    // place# |  0  |  1  |  2  |  3  |  4  |  5  |  6  | .....|
    //
    // -
    // Thus, we have one place with NULL value left. In the next round mReadIdx will
    // stuck on place #1 (queue thinks that it's empty) until one of the write threads
    // will write the value into place #1. It could be thread #5, so we have:
    //
    // -
    // Queue state:
    //              mReadIdx
    //                 |
    // |..... | 9/1 | 5/9 | 5/1 | 1/5 | 2/7 | 2/8 | 2/9 | .....|
    //                       |
    //                   mWriteIdx
    // place# |  0  |  1  |  2  |  3  |  4  |  5  |  6  | .....|
    //
    // -
    // And we will read 5/9 before 5/1
    //
    //
    // Note that read/write both begin at index 1
    const uint32_t idx = AAX_Atomic_IncThenGet_32(mWriteIdx);
    const uint32_t widx = idx % S;
	
	// Do the push. If the value at the current write index is non-NULL then we have filled the buffer.
	const bool cxResult = AAX_Atomic_CompareAndExchange_Pointer(mRingBuffer[widx], (value_type)0x0, inElem);
	
	if (false == cxResult)
	{
		result = AAX_IContainer::eStatus_Overflow;
				
		const uint32_t ridx = (0 == idx) ? S : idx-1;
		
		// Note the write index has already been incremented, so in the event of an overflow we must
		// return the write index to its previous location.
		//
		// Note: if multiple write threads encounter concurrent push overflows then the write pointer
		// will not be fully decremented back to the overflow location, and the read index will need
		// to increment multiple positions to clear the overflow state.
//		const bool resetResult = AAX_Atomic_CompareAndExchange_32(mWriteIdx, idx, ridx);
		AAX_Atomic_CompareAndExchange_32(mWriteIdx, idx, ridx);
		
//		printf("AAX_CAtomicQueue: overflow - reset: %s, idx: %lu, widx: %lu, inElem: %p\n",
//			   resetResult ? "yes" : " no",
//			   (unsigned long)idx,
//			   (unsigned long)widx,
//			   inElem);
	}
	else
	{
		result = AAX_IContainer::eStatus_Success;
		
		// Handle wraparound
		//
		// There may be multiple write threads pushing elements at the same time, so we use
		// (wrapped index < raw index) instead of (raw index == boundary)
		//
		// This assumes overhead between S and UINT_32_MAX of at least as many elements as
		// there are write threads.
		
//		bool exchResult = false;
		if (widx < idx)
		{
//			exchResult =
			AAX_Atomic_CompareAndExchange_32(mWriteIdx, idx, widx);
		}
		
//		printf("AAX_CAtomicQueue: pushed    - reset: %s, idx: %lu, widx: %lu, inElem: %p\n",
//			   (widx < idx) ? exchResult ? "yes" : " no" : "n/a",
//			   (unsigned long)idx,
//			   (unsigned long)widx,
//			   inElem);
	}
	
	return result;
}

template <typename T, size_t S>
inline typename AAX_CAtomicQueue<T, S>::value_type AAX_CAtomicQueue<T, S>::Pop()
{
	// Note that read/write both begin at index 1
	mReadIdx = (mReadIdx+1) % template_size;
	value_type const val = AAX_Atomic_Exchange_Pointer(mRingBuffer[mReadIdx], (value_type)0x0);
	
//	printf("AAX_CAtomicQueue: popped    - reset: %s, idx: %lu,            val:    %p\n",
//		   (0x0 == val) ? "yes" : " no",
//		   (unsigned long)mReadIdx,
//		   val);
	
	if (0x0 == val)
	{
		// If the value is NULL then no value has yet been written to this location. Decrement the read index
		--mReadIdx; // No need to handle wraparound since the read index will be incremented before the next read
	}
	
	return val;
}

template <typename T, size_t S>
inline typename AAX_CAtomicQueue<T, S>::value_type AAX_CAtomicQueue<T, S>::Peek() const
{
	// I don't think that we need a memory barrier here because:
	// a) mReadIdx will only be modified from the read thread, and therefore presumably
	//    using the same CPU (or at least I can't see any way for mReadIndex modification
	//    ordering to be a problem between Peek() and Pop() on a single thread.)
	// b) We don't care if mRingBuffer modifications are run out of order between the read
	//    and write threads, as long as they are "close".
	const uint32_t testIdx = (mReadIdx+1) % template_size;
	return AAX_Atomic_Load_Pointer(&mRingBuffer[testIdx]);
}

// Attempt to support multiple read threads
//
// This approach is broken in the following scenario:
//
// Thread | Operation
//      A   Pop v enter
//      A   Pop - increment/get read index (get 1)
//      A   Pop - exchange pointer (get 0x0)
//  other   Push ptr1
//  other   Push ptr2
//      B   Pop v enter
//      B   Pop - increment/get read index (get 2)
//      B   Pop - exchange pointer (get ptr2)
//            ERROR: popped ptr2 before ptr1
//      B   Pop ^ exit
//      A   Pop - decrement read index (set 1)
//      A   Pop ^ exit
//    any   Pop v enter
//    any   Pop - increment/get read index (get 2)
//    any   Pop - exchange pointer (get 0x0)
//            ERROR: should be ptr2
//                   This NULL state continues for further Pop calls until either Push wraps around
//                   or another pair of concurrent calls to Pop just happens to re-aligign the read
//                   index by incrementing twice before any reads occur
//    any   Pop - decrement read index (set 1)
//    any   Pop ^ exit
//
// This could be fixed by incrementing the read index until either a non-NULL value is found or
// the initial position is reached, but that would have terrible performance.
//
// In any case, assuming a single read thread is optimal when we want maximum performance for read
// operations, since this requires the fewest number of atomic operations in the read methods
/*
template <typename T, size_t S>
inline typename AAX_CAtomicQueue<T, S>::value_type AAX_CAtomicQueue<T, S>::Pop()
{
	const uint32_t idx = AAX_Atomic_IncThenGet_32(mReadIdx);
	const uint32_t widx = idx % S;
	
	value_type const val = AAX_Atomic_Exchange_Pointer(mRingBuffer[widx], (value_type)0x0);
	
	if (0x0 == val)
	{
		// If the value is NULL then no value has yet been written to this location. Decrement the read index
		AAX_Atomic_DecThenGet_32(mReadIdx);
	}
	else
	{
		// Handle wraparound (assumes some overhead between S and UINT_32_MAX)
		if (widx < idx)
		{
			AAX_Atomic_CompareAndExchange_32(mReadIdx, idx, widx);
		}
	}
	
	return val;
}
 */

/// @endcond

/// @cond ignore
#endif /* defined(AAX_CATOMICQUEUE_H) */
/// @endcond
