/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFController.h
 *
 *	\brief Interface for the %AAX host's view of a single instance of an 
 *	effect.  Used by both clients of the AAXHost and by effect components.
 *
 */ 
/*================================================================================================*/


#ifndef _AAX_IACFCONTROLLER_H_
#define _AAX_IACFCONTROLLER_H_

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

// Forward declarations
class AAX_IPageTable;
class AAX_IString;

/** \brief Interface for the %AAX host's view of a single instance of an 
 *	effect.  Used by both clients of the AAXHost and by effect components.
 */
class AAX_IACFController : public IACFUnknown
{
public:	
			
		// Host information getters
		/** \copydoc AAX_IController::GetEffectID() */
		virtual 
		AAX_Result
		GetEffectID (
			AAX_IString *	outEffectID) const = 0;
		
		/** \copydoc AAX_IController::GetSampleRate() */
		virtual
		AAX_Result
		GetSampleRate (
			AAX_CSampleRate *outSampleRate ) const = 0;

		/** \copydoc AAX_IController::GetInputStemFormat() */
		virtual
		AAX_Result
		GetInputStemFormat (
			AAX_EStemFormat *outStemFormat ) const = 0;

		/** \copydoc AAX_IController::GetOutputStemFormat() */
		virtual
		AAX_Result
		GetOutputStemFormat (
			AAX_EStemFormat *outStemFormat) const = 0;
	 
		/** \copydoc AAX_IController::GetSignalLatency() */
		virtual 
		AAX_Result	
		GetSignalLatency( 
			int32_t* outSamples) const = 0;
	
		/** \copydoc AAX_IController::GetCycleCount() */
		virtual
		AAX_Result
		GetCycleCount(
			AAX_EProperty inWhichCycleCount,
			AAX_CPropertyValue* outNumCycles) const = 0;

		/** \copydoc AAX_IController::GetTODLocation() */
		virtual
		AAX_Result 
		GetTODLocation ( 
			AAX_CTimeOfDay* outTODLocation ) const = 0;

		//Host Information Setters (Dynamic info)
		/** \copydoc AAX_IController::SetSignalLatency() */
		virtual 
		AAX_Result	
		SetSignalLatency(
			int32_t inNumSamples) = 0;
	
		/** \copydoc AAX_IController::SetCycleCount() */
		virtual
		AAX_Result
		SetCycleCount(
			AAX_EProperty* inWhichCycleCounts,
			AAX_CPropertyValue* iValues,
			int32_t numValues) = 0;
		
		// Posting functions
		/** \copydoc AAX_IController::PostPacket() */
		virtual 
		AAX_Result
		PostPacket (
			AAX_CFieldIndex		inFieldIndex,
			const void *	inPayloadP, 
			uint32_t inPayloadSize) = 0;

		//Metering functions
		/** \copydoc AAX_IController::GetCurrentMeterValue() */
		virtual 
		AAX_Result	
		GetCurrentMeterValue ( 
			AAX_CTypeID inMeterID,
			float * outMeterValue ) const = 0;
	
		/** \copydoc AAX_IController::GetMeterPeakValue() */
		virtual
		AAX_Result
		GetMeterPeakValue ( 
			AAX_CTypeID inMeterID,
			float * outMeterPeakValue ) const = 0;
	
		/** \copydoc AAX_IController::ClearMeterPeakValue() */
		virtual
		AAX_Result
		ClearMeterPeakValue ( 
			AAX_CTypeID inMeterID ) const = 0;

		/** \copydoc AAX_IController::GetMeterClipped() */
		virtual
		AAX_Result
		GetMeterClipped ( 
			AAX_CTypeID inMeterID,
			AAX_CBoolean * outClipped ) const = 0;

		/** \copydoc AAX_IController::ClearMeterClipped() */
		virtual
		AAX_Result
		ClearMeterClipped ( 
			AAX_CTypeID inMeterID ) const = 0;
	
		/** \copydoc AAX_IController::GetMeterCount() */
		virtual 
		AAX_Result	
		GetMeterCount ( 
			uint32_t * outMeterCount ) const = 0;

		// MIDI methods
		/** \copydoc AAX_IController::GetNextMIDIPacket() */
		virtual
		AAX_Result
		GetNextMIDIPacket (
			AAX_CFieldIndex* outPort,
			AAX_CMidiPacket* outPacket ) = 0;

	};

/** @copydoc AAX_IACFController
 */
class AAX_IACFController_V2 : public AAX_IACFController
{
public:
	// Notification method
	/** \copydoc AAX_IController::SendNotification() */
	virtual 
	AAX_Result 
	SendNotification (
		AAX_CTypeID inNotificationType,
		const void* inNotificationData,
		uint32_t inNotificationDataSize) = 0;

    /** \copydoc AAX_IController::GetHybridSignalLatency() */
	virtual
    AAX_Result
    GetHybridSignalLatency(
    	int32_t* outSamples) const = 0;
    
    /** \copydoc AAX_IController::GetCurrentAutomationTimestamp() */
	virtual
    AAX_Result
    GetCurrentAutomationTimestamp(
    	AAX_CTransportCounter* outTimestamp) const = 0;
    
    /** \copydoc AAX_IController::GetHostName() */
	virtual
	AAX_Result
	GetHostName(
		AAX_IString* outHostNameString) const = 0;
};

/** @copydoc AAX_IACFController
 */
class AAX_IACFController_V3 : public AAX_IACFController_V2
{
public:
	/** \copydoc AAX_IController::GetPlugInTargetPlatform() */
	virtual
	AAX_Result
	GetPlugInTargetPlatform(
		AAX_CTargetPlatform* outTargetPlatform) const = 0;

	/** \copydoc AAX_IController::GetIsAudioSuite() */
	virtual
	AAX_Result
	GetIsAudioSuite(AAX_CBoolean* outIsAudioSuite) const = 0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // #ifndef _AAX_IACFCONTROLLER_H_
