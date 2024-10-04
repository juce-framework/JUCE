/*================================================================================================*/
/*
 *
 *	Copyright 2014-2017, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_Callbacks.h
 *
 *	\brief %AAX callback prototypes and ProcPtr definitions
 *
 */ 
/*================================================================================================*/


/// @cond ignore
#ifndef AAX_CALLBACKS_H_
#define AAX_CALLBACKS_H_
/// @endcond

#include "AAX.h"

// Callback IDs
enum AAX_CProcPtrID
{
	kAAX_ProcPtrID_Create_EffectParameters = 0,		///< \ref AAX_IEffectParameters creation procedure
	kAAX_ProcPtrID_Create_EffectGUI = 1,			///< \ref AAX_IEffectGUI creation procedure
	kAAX_ProcPtrID_Create_HostProcessor = 3,		///< \ref AAX_IHostProcessor creation procedure
	kAAX_ProcPtrID_Create_EffectDirectData = 5,		///< \ref AAX_IEffectDirectData creation procedure, used by plug-ins that want direct access to their alg memory
	kAAX_ProcPtrID_Create_TaskAgent = 6,			///< \ref AAX_ITaskAgent creation procedure, used by plug-ins that want to process task requests made by the host.
	kAAX_ProcPtrID_Create_SessionDocumentClient = 7	///< \ref AAX_ISessionDocumentClient creation procedure
};

class IACFUnknown;

typedef IACFUnknown* (AAX_CALLBACK *AAXCreateObjectProc)(void);


/**	\brief Empty class containing type declarations for the %AAX algorithm and
	associated callbacks
 */
template <typename	aContextType>
class AAX_Component
{
	public:
		
		typedef void
		(AAX_CALLBACK *CProcessProc) (
			 aContextType * const	inContextPtrsBegin [],
			 const void *			inContextPtrsEnd);
		
		typedef void *
		(AAX_CALLBACK *CPacketAllocator) (
			const aContextType *	inContextPtr,
			AAX_CFieldIndex				inOutputPort,
			AAX_CTimestamp			inTimestamp);

		typedef int32_t
		(AAX_CALLBACK *CInstanceInitProc) (
			 const aContextType *	inInstanceContextPtr,
			 AAX_EComponentInstanceInitAction iAction );

		typedef int32_t
		(AAX_CALLBACK *CBackgroundProc) ( void );
		
		typedef void
		(AAX_CALLBACK *CInitPrivateDataProc) (
			AAX_CFieldIndex		inFieldIndex,
			void *			inNewBlock,
			int32_t			inSize,
			IACFUnknown * const	inController);

};

/** @brief	A user-defined callback that %AAX calls to process data packets and/or
 audio.
 
 @details
 \par iContextPtrsBegin
 A vector of context pointers.  Each element points to the
 context for one instance of this component.  @p iContextPtrsEnd
 gives the upper bound of the vector and <tt>(inContextPtrsEnd -
 inContextPtrsBegin)</tt> gives the count.
 
 \par iContextPtrsEnd
 The upper bound of the vector at @p iContextPtrsBegin.
 <tt>(inContextPtrsEnd - iContextPtrsBegin)</tt> gives the count
 of this vector.
 
 The instance vector was originally NULL-terminated in
 earlier versions of this API.  However, the STL-style begin/end
 pattern was suggested as a more general representation that could,
 for instance, allow a vector to be split for parallel processing.
 */
typedef AAX_Component <void>::CProcessProc			AAX_CProcessProc;

/** @brief	Used by AAX_SchedulePacket()
 
 @details
 \deprecated
 
 A AAX_CProcessProc that calls AAX_SchedulePacket() must include a
 AAX_CPacketAllocator field in its context and register that field
 with AAX.  %AAX will then populate that field with a
 AAX_CPacketAllocator to pass to AAX_SchedulePacket().
 
 @see	AAX_SchedulePacket()
 */
typedef AAX_Component <void>::CPacketAllocator		AAX_CPacketAllocator;

/** @brief	A user-defined callback that %AAX calls to notify the component that
 an instance is being added or removed.  
 
 @details
 This optional callback allows the component to keep per-instance 
 data.   It's called before the instance appears in the list 
 supplied to CProcessProc, and then after the instance is removed 
 from the list. 
 
 \par iInstanceContextPtr
 A pointer to the context data structure of the instance being
 added or removed from the processing list.
 
 \par iAction
 Indicates the action that triggered the init callback, e.g.
 whether the instance is being added or removed.
 
 @retval		Should return 0 on success, anything else on failure.  Failure will
 prevent the instance from being created.
 */
typedef AAX_Component <void>::CInstanceInitProc		AAX_CInstanceInitProc;

/** @brief	A user-defined callback that %AAX calls in the %AAX Idle time
 
 @details
 This optional callback allows the component to do background processing
 in whatever manner the plug-in developer desires
 
 @retval		Should return 0 on success, anything else on failure.  Failure will
 cause the %AAX host to signal an error up the callchain.
 */
typedef AAX_Component <void>::CBackgroundProc		AAX_CBackgroundProc;

/** @brief	A user-defined callback to initialize a private data block
 
 @details
 \deprecated
 
 A component that requires private data supplies \ref AAX_CInitPrivateDataProc
 callbacks to set its private data to the state it should be in at the start
 of audio.  The component first declares one or more pointers to private data
 in its context.  It then registers each such field with %AAX along with its
 data size, various other attributes, and a \ref AAX_CInitPrivateDataProc.  The
 AAX_CInitPrivateDataProc always runs on the host system, not the DSP.
 %AAX allocates storage for each private data block and calls its associated
 \ref AAX_CInitPrivateDataProc to initialize it.  If the component's
 \ref AAX_CProcessProc runs on external hardware, %AAX initializes private data
 blocks on the host system and copies them to the remote system.
 
 \sa alg_pd_init
 
 \par	inFieldIndex
 The port ID of the block to be initialized, as generated by
 AAX_FIELD_INDEX().  A component can register a separate
 AAX_CInitPrivateDataProc for each of its private data blocks, or it
 can use fewer functions that switch on @p inFieldIndex.
 
 \par	inNewBlock
 A pointer to the block to be initialized.  If the component runs
 externally, %AAX will copy this block to the remote system after it is
 initialized.
 
 \par	inSize
 The size of the block to be initialized.  If a component has multiple
 private blocks that only need to be zeroed out, say, it can use a single
 AAX_CInitPrivateDataProc for all of these blocks that zeroes them out
 according to @p inSize.
 
 \par	inController
 A pointer to the current Effect instance's \ref AAX_IController.
 \note Do not directly reference data from this interface when populating
 @p iNewBlock.  The data in this block must be fully self-contained to
 ensure portability to a new device or memory space.
 */
typedef AAX_Component <void>::CInitPrivateDataProc	AAX_CInitPrivateDataProc;	///< \deprecated

/// @cond ignore
#endif // AAX_CALLBACKS_H_
/// @endcond
