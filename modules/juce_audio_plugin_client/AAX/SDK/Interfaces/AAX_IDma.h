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
 *	\file  AAX_IDma.h
 *
 *	\brief Cross-platform interface for access to the host's direct memory access (DMA) facilities
 *
 */ 
/*================================================================================================*/


#pragma once

#ifndef AAX_IDMA_H
#define AAX_IDMA_H

#include "AAX.h"


#ifndef AAX_DMA_API
#	ifdef _MSC_VER
#		define AAX_DMA_API		__cdecl
#	else
#		define AAX_DMA_API
#	endif
#endif // AAX_DMA_API

/**
 *	\brief Cross-platform interface for access to the host's direct memory access (DMA) facilities
 *
 *	\details
 *	\hostimp
 *
 *	This interface is provided via a DMA port in the plug-in's algorithm context.
 *
 *	\sa \ref AAX_IComponentDescriptor::AddDmaInstance()
 *	\sa \ref additionalFeatures_DMA
 */
class AAX_IDma 
{
public:
	enum EState
	{
		eState_Error = -1,
		eState_Init = 0,
		eState_Running = 1,
		eState_Complete = 2,
		eState_Pending = 3
	};

	// WARNING! These need to be kept in sync with the TI dMAX microcode EventType IDs!
	/** @brief DMA mode IDs
	 
		These IDs are used to bind DMA context fields to a particular DMA mode when
		describing the fields with \ref AAX_IComponentDescriptor::AddDmaInstance()
	 */
	enum EMode
	{
		eMode_Error = -1,
		
		eMode_Burst = 6,		///< Burst mode (uncommon)
		eMode_Gather = 10,		///< Gather mode
		eMode_Scatter = 11,		///< Scatter mode
		
	}; 
	

public:
	virtual	~AAX_IDma() {}

	////////////////////////////////////////////////////////////////////////////////////////////
	/** @name Basic DMA operation
	 *
	 *	
	 *
	 */
	//@{
	/**	@brief Posts the transfer request to the DMA server
	 *
	 *	@note Whichever mode this method is called on first will be the first mode to start
	 *	transferring.  Most plug-ins should therefore call this method for their Scatter
	 *	DMA fields before their Gather DMA fields so that the scattered data is available
	 *	as quickly as possible for future gathers.
	 *
 	 *	@returns \c AAX_SUCCESS on success
 	 */
	virtual AAX_Result	AAX_DMA_API		PostRequest() = 0;
	/**	@brief Query whether a transfer has completed
	 *	
	 *	A return value of false indicates an error, and that the DMA missed its cycle
	 *	count deadline
	 *	
	 *	@note This function should not be used for polling within a Process loop!
	 *	Instead, it can be used as a test for DMA failure.  This test is usually
	 *	performed via a Debug-only assert.
	 *
	 *	@todo Clarify return value meaning -- ambiguity in documentation
	 *
	 * 	@returns \b true if all pending transfers are complete
	 *	@returns \b false if pending transfers are not complete
 	 */
	virtual int32_t		AAX_DMA_API		IsTransferComplete() = 0;
	/**	@brief Sets the DMA State
	 *
	 *	@note This method is part of the host interface and should not be used by plug-ins
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result	AAX_DMA_API		SetDmaState( EState iState ) = 0;
	/**	@brief Inquire to find the state of the DMA instance.
	 */
	virtual EState		AAX_DMA_API		GetDmaState() const = 0;
	/**	@brief Inquire to find the mode of the DMA instance.
	 *
	 *	This value does not change, so there is no setter.
	 */
	virtual EMode		AAX_DMA_API		GetDmaMode() const = 0;
	//@} end Basic DMA operation
			
	
	////////////////////////////////////////////////////////////////////////////////////////////
	/** @name Methods for Burst operation
	 *
	 *	Use these methods in conjunction with \ref AAX_IDma::eMode_Burst
	 */
	//@{
	/**	@brief Sets the address of the source buffer
	 *
	 *	@param[in] iSrc
	 *		Address of the location in the source buffer where
	 *		the read transfer should begin
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetSrc( int8_t * iSrc ) = 0;
	/**	@brief Gets the address of the source buffer
	 */
	virtual int8_t *		AAX_DMA_API		GetSrc() = 0;
	/**	@brief Sets the address of the destination buffer
	 *
	 *	@param[in] iDst
	 *		Address of the location in the destination buffer where
	 *		the write transfer should begin
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetDst( int8_t * iDst ) = 0;
	/**	@brief Gets the address of the destination buffer
	 */
	virtual int8_t *		AAX_DMA_API		GetDst() = 0;
	
	/**	@brief Sets the length of each burst
	 *
	 *	@note Burst length must be between 1 and 64 Bytes, inclusive
	 *	@note 64-Byte transfers are recommended for the fastest overall transfer speed
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetBurstLength( int32_t iBurstLengthBytes ) = 0;
	/**	@brief Gets the length of each burst
	 */
	virtual int32_t			AAX_DMA_API		GetBurstLength() = 0;
	/**	@brief Sets the number of bursts to perform before giving up priority to other DMA transfers.
	 *
	 *	Valid values are 1, 2, 4, or 16.
	 *
	 *	The full transmission may be broken up into several series of bursts, and thus the total size
	 *	of the data being transferred is not bounded by the number of bursts times the burst length.
	 *
	 *	@param[in] iNumBursts
	 *		The number of bursts
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetNumBursts( int32_t iNumBursts ) = 0;
	/**	@brief Gets the number of bursts to perform before giving up priority to other DMA transfers
	 */
	virtual int32_t			AAX_DMA_API		GetNumBursts() = 0;
	/**	@brief Sets the size of the whole transfer
	 *
	 *	@param[in] iTransferSizeBytes
	 *		The transfer size, in Bytes
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetTransferSize( int32_t iTransferSizeBytes ) = 0;
	/**	@brief Gets the size of the whole transfer, in Bytes
	 */
	virtual int32_t			AAX_DMA_API		GetTransferSize() = 0;
	//@} end Methods for Burst operation


	////////////////////////////////////////////////////////////////////////////////////////////
	/** @name Methods for Scatter and Gather operation
	 *
	 *	Use these methods in conjunction with \ref AAX_IDma::eMode_Scatter and
	 *	\ref AAX_IDma::eMode_Gather
	 */
	//@{
	/**	@brief Sets the address of the FIFO buffer for the DMA transfer (usually the external memory block)
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetFifoBuffer( int8_t * iFifoBase ) = 0;
	/**	@brief Gets the address of the FIFO buffer for the DMA transfer
	 */
	virtual int8_t *		AAX_DMA_API		GetFifoBuffer() = 0;
	/**	@brief Sets the address of the linear buffer for the DMA transfer (usually the internal memory block)
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetLinearBuffer( int8_t * iLinearBase ) = 0;
	/**	@brief Gets the address of the linear buffer for the DMA transfer
	 */
	virtual int8_t *		AAX_DMA_API		GetLinearBuffer() = 0;
	/**	@brief Sets the offset table for the DMA transfer
	 *
	 *	The offset table provides a list of Byte-aligned memory offsets into the FIFO
	 *	buffer.  The transfer will be broken into a series of individual bursts, each
	 *	beginning at the specified offset locations within the FIFO buffer.  The size
	 *	of each burst is set by \ref AAX_IDma::SetBurstLength "SetBurstLength()".
	 *
	 *	@sa AAX_IDma::SetNumOffsets()
	 *	@sa AAX_IDma::SetBaseOffset()
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetOffsetTable( const int32_t * iOffsetTable ) = 0;
	/**	@brief Gets the offset table for the DMA transfer
	 */
	virtual const int32_t *	AAX_DMA_API		GetOffsetTable() = 0;
	/**	@brief Sets the number of offets in the offset table
	 *
	 *	@sa AAX_IDma::SetOffsetTable()
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetNumOffsets( int32_t iNumOffsets ) = 0;
	/**	@brief Gets the number of offets in the offset table
	 */
	virtual int32_t			AAX_DMA_API		GetNumOffsets() = 0;
	/**	@brief Sets the relative base offset into the FIFO where transfers will begin
	 *
	 *	The base offset will be added to each value in the offset table in order to determine
	 *	the starting offset within the FIFO buffer for each burst.
	 *	
	 *	@sa AAX_IDma::SetOffsetTable()
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetBaseOffset( int32_t iBaseOffsetBytes ) = 0;
	/**	@brief Gets the relative base offset into the FIFO where transfers will begin
	 */
	virtual int32_t			AAX_DMA_API		GetBaseOffset() = 0;
	/**	@brief Sets the size of the FIFO buffer, in bytes
	 *
	 *	@note The FIFO buffer must be padded with at least enough memory to accommodate
	 *	one burst, as defined by \ref AAX_IDma::SetBurstLength "SetBurstLength()".
	 *
 	 *	@returns \c AAX_SUCCESS on success
	 */
	virtual AAX_Result		AAX_DMA_API		SetFifoSize( int32_t iSizeBytes ) = 0;
	/**	@brief Gets the size of the FIFO buffer, in bytes
	 */
	virtual int32_t			AAX_DMA_API		GetFifoSize() = 0;
	//@} end Methods for Scatter and Gather operation
};



#endif // AAX_IDMA_H
