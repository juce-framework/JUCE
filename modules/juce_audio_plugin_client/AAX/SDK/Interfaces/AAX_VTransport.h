/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_VTransport.h
 *
 *	\brief Version-managed concrete Transport class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VTRANSPORT_H
#define AAX_VTRANSPORT_H

#pragma once

#include "AAX_ITransport.h"
#include "AAX_IACFTransport.h"
#include "AAX_IACFTransportControl.h"
#include "ACFPtr.h"

/**
 *	\brief Version-managed concrete \ref AAX_ITransport class
 *
 */
class AAX_VTransport : public AAX_ITransport
{
public:
	AAX_VTransport( IACFUnknown* pUnknown );
	~AAX_VTransport() AAX_OVERRIDE;
	
	// Transport Information Getters
	// AAX_IACFTransport
	AAX_Result	GetCurrentTempo ( double* TempoBPM ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentTempo()
	AAX_Result	GetCurrentMeter ( int32_t* MeterNumerator, int32_t* MeterDenominator ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentMeter()
	AAX_Result	IsTransportPlaying ( bool* isPlaying ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::IsTransportPlaying()
	AAX_Result	GetCurrentTickPosition ( int64_t* TickPosition ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentTickPosition()
	AAX_Result	GetCurrentLoopPosition ( bool* bLooping, int64_t* LoopStartTick, int64_t* LoopEndTick ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentLoopPosition()
	AAX_Result	GetCurrentNativeSampleLocation ( int64_t* SampleLocation ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentNativeSampleLocation()
	AAX_Result	GetCustomTickPosition( int64_t* oTickPosition, int64_t iSampleLocation) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCustomTickPosition()
	AAX_Result	GetBarBeatPosition(int32_t* Bars, int32_t* Beats, int64_t* DisplayTicks, int64_t SampleLocation) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetBarBeatPosition()
	AAX_Result	GetTicksPerQuarter ( uint32_t* ticks ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetTicksPerQuarter()
	AAX_Result	GetCurrentTicksPerBeat ( uint32_t* ticks ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetCurrentTicksPerBeat()

	// AAX_IACFTransport_V2
	AAX_Result	GetTimelineSelectionStartPosition ( int64_t* oSampleLocation ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetTimelineSelectionStartPosition()
	AAX_Result	GetTimeCodeInfo( AAX_EFrameRate* oFrameRate, int32_t* oOffset ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetTimeCodeInfo()
	AAX_Result	GetFeetFramesInfo( AAX_EFeetFramesRate* oFeetFramesRate, int64_t* oOffset ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetFeetFramesInfo()
	AAX_Result	IsMetronomeEnabled ( int32_t* isEnabled ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::IsMetronomeEnabled()
	
	// AAX_IACFTransport_V3
	AAX_Result	GetHDTimeCodeInfo( AAX_EFrameRate* oHDFrameRate, int64_t* oHDOffset ) const AAX_OVERRIDE; ///< \copydoc AAX_ITransport::GetHDTimeCodeInfo()

	// AAX_IACFTransport_V4
	AAX_Result	GetTimelineSelectionEndPosition( int64_t* oSampleLocation ) const AAX_OVERRIDE;	///< \copydoc AAX_ITransport::GetTimelineSelectionEndPosition()

	// AAX_IACFTransport_V5
	AAX_Result	GetKeySignature( int64_t iSampleLocation, uint32_t* oKeySignature ) const AAX_OVERRIDE;	///< \copydoc AAX_ITransport::GetKeySignature()

	// AAX_IACFTransportControl
	AAX_Result RequestTransportStart() AAX_OVERRIDE; ///< \copydoc AAX_ITransport::RequestTransportStart()
	AAX_Result RequestTransportStop() AAX_OVERRIDE; ///< \copydoc AAX_ITransport::RequestTransportStop()
	
private:
	ACFPtr<AAX_IACFTransport>		mITransport;
	ACFPtr<AAX_IACFTransport_V2>	mITransportV2;
	ACFPtr<AAX_IACFTransport_V3>	mITransportV3;
	ACFPtr<AAX_IACFTransport_V4>	mITransportV4;
	ACFPtr<AAX_IACFTransport_V5>	mITransportV5;
	ACFPtr<AAX_IACFTransportControl>	mITransportControl;
};

#endif // AAX_VTRANSPORT_H

