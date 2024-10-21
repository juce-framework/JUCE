/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFTransport.h
 *
 *	\brief Interface for accessing the host's transport state
 */ 
/*================================================================================================*/

#ifndef AAX_IACFTRANSPORT_H
#define AAX_IACFTRANSPORT_H

#pragma once

#include "AAX.h"
#include "AAX_Enums.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport : public IACFUnknown
{
public:

	virtual AAX_Result	GetCurrentTempo ( double* TempoBPM ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentTempo()
	virtual AAX_Result	GetCurrentMeter ( int32_t* MeterNumerator, int32_t* MeterDenominator ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentMeter()
	virtual AAX_Result	IsTransportPlaying ( bool* isPlaying ) const = 0;	///< \copydoc AAX_ITransport::IsTransportPlaying()
	virtual AAX_Result	GetCurrentTickPosition ( int64_t* TickPosition ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentTickPosition()
	virtual AAX_Result	GetCurrentLoopPosition ( bool* bLooping, int64_t* LoopStartTick, int64_t* LoopEndTick ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentLoopPosition()
	virtual AAX_Result	GetCurrentNativeSampleLocation ( int64_t* SampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentNativeSampleLocation()
	virtual AAX_Result	GetCustomTickPosition ( int64_t* oTickPosition, int64_t iSampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetCustomTickPosition()
	virtual AAX_Result	GetBarBeatPosition ( int32_t* Bars, int32_t* Beats, int64_t* DisplayTicks, int64_t SampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetBarBeatPosition()
	virtual AAX_Result	GetTicksPerQuarter ( uint32_t* ticks ) const = 0;	///< \copydoc AAX_ITransport::GetTicksPerQuarter()
	virtual AAX_Result	GetCurrentTicksPerBeat ( uint32_t* ticks ) const = 0;	///< \copydoc AAX_ITransport::GetCurrentTicksPerBeat()

};

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport_V2 : public AAX_IACFTransport
{
public:

	virtual AAX_Result	GetTimelineSelectionStartPosition( int64_t* oSampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetTimelineSelectionStartPosition()
	virtual AAX_Result	GetTimeCodeInfo( AAX_EFrameRate* oFrameRate, int32_t* oOffset ) const = 0;	///< \copydoc AAX_ITransport::GetTimeCodeInfo() 
	virtual AAX_Result	GetFeetFramesInfo( AAX_EFeetFramesRate* oFeetFramesRate, int64_t* oOffset ) const =  0;	///< \copydoc AAX_ITransport::GetFeetFramesInfo()
	virtual AAX_Result	IsMetronomeEnabled ( int32_t* isEnabled ) const = 0;	///< \copydoc AAX_ITransport::IsMetronomeEnabled()
};

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport_V3 : public AAX_IACFTransport_V2
{
public:
	
	virtual AAX_Result	GetHDTimeCodeInfo( AAX_EFrameRate* oHDFrameRate, int64_t* oHDOffset ) const = 0;	///< \copydoc AAX_ITransport::GetHDTimeCodeInfo()
};

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport_V4 : public AAX_IACFTransport_V3
{
public:
	
	virtual AAX_Result	GetTimelineSelectionEndPosition( int64_t* oSampleLocation ) const = 0;	///< \copydoc AAX_ITransport::GetTimelineSelectionEndPosition()
};

/**	\brief Versioned interface to get information about the host's transport state
 */
class AAX_IACFTransport_V5 : public AAX_IACFTransport_V4
{
public:
	
	virtual AAX_Result	GetKeySignature( int64_t iSampleLocation, uint32_t* oKeySignature ) const = 0;	///< \copydoc AAX_ITransport::GetKeySignature()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFTRANSPORT_H

