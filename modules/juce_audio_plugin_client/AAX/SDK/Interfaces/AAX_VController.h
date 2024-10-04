/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_VController.h
 *
 *	\brief Version-managed concrete Controller class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VCONTROLLER_H
#define AAX_VCONTROLLER_H

#include "AAX_IController.h"
#include "AAX_IACFController.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "ACFPtr.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

class IACFUnknown;
class IACFComponentFactory;
class AAX_IACFPageTableController;
class AAX_IACFPageTableController_V2;
class AAX_IACFPageTable_V2;

/*!
 \brief Version-managed concrete \ref AAX_IController "Controller" class
 
 \details
 For usage information, see \ref using_acf_host_provided_interfaces
 
 */
class AAX_VController : public AAX_IController
{
public:
	AAX_VController( IACFUnknown* pUnknown );
	~AAX_VController() override;
	
	//Host Information Getters
	AAX_Result	GetEffectID ( AAX_IString *	outEffectID) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetEffectID()
	AAX_Result	GetSampleRate ( AAX_CSampleRate * outSampleRate ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetSampleRate()
	AAX_Result	GetInputStemFormat ( AAX_EStemFormat * outStemFormat ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetInputStemFormat()
	AAX_Result	GetOutputStemFormat ( AAX_EStemFormat * outStemFormat ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetOutputStemFormat()
	AAX_Result	GetSignalLatency( int32_t* outSamples) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetSignalLatency()
	AAX_Result	GetHybridSignalLatency(int32_t* outSamples) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetHybridSignalLatency()
	AAX_Result	GetPlugInTargetPlatform(AAX_CTargetPlatform* outTargetPlatform) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetPlugInTargetPlatform()
	AAX_Result	GetIsAudioSuite(AAX_CBoolean* outIsAudioSuite) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetIsAudioSuite()
	AAX_Result	GetCycleCount( AAX_EProperty inWhichCycleCount, AAX_CPropertyValue* outNumCycles) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetCycleCount()
	AAX_Result	GetTODLocation ( AAX_CTimeOfDay* outTODLocation ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetTODLocation()
	AAX_Result	GetCurrentAutomationTimestamp(AAX_CTransportCounter* outTimestamp) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetCurrentAutomationTimestamp()
	AAX_Result	GetHostName(AAX_IString* outHostNameString) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetHostName()

	//Host Information Setters (Dynamic info)
	AAX_Result	SetSignalLatency(int32_t inNumSamples) AAX_OVERRIDE; ///< \copydoc AAX_IController::SetSignalLatency()
	AAX_Result	SetCycleCount( AAX_EProperty* inWhichCycleCounts, AAX_CPropertyValue* iValues, int32_t numValues) AAX_OVERRIDE; ///< \copydoc AAX_IController::SetCycleCount()
	
	//Posting functions.
	AAX_Result	PostPacket ( AAX_CFieldIndex inFieldIndex, const void * inPayloadP, uint32_t inPayloadSize ) AAX_OVERRIDE; ///< \copydoc AAX_IController::PostPacket()
	
	// Notification functions
	AAX_Result	SendNotification ( AAX_CTypeID inNotificationType, const void* inNotificationData, uint32_t inNotificationDataSize ) AAX_OVERRIDE; ///< \copydoc AAX_IController::SendNotification(AAX_CTypeID, const void*, uint32_t)
	AAX_Result	SendNotification ( AAX_CTypeID inNotificationType) AAX_OVERRIDE; ///< \copydoc AAX_IController::SendNotification(AAX_CTypeID) \note Not an AAX interface method
	
	//Metering functions
	AAX_Result	GetCurrentMeterValue ( AAX_CTypeID inMeterID, float * outMeterValue ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetCurrentMeterValue()
	AAX_Result	GetMeterPeakValue( AAX_CTypeID inMeterID, float * outMeterPeakValue ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetMeterPeakValue()
	AAX_Result	ClearMeterPeakValue ( AAX_CTypeID inMeterID ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::ClearMeterPeakValue()
	AAX_Result	GetMeterClipped ( AAX_CTypeID inMeterID, AAX_CBoolean * outClipped ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetMeterClipped()
	AAX_Result	ClearMeterClipped ( AAX_CTypeID inMeterID ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::ClearMeterClipped()
	AAX_Result	GetMeterCount ( uint32_t * outMeterCount ) const AAX_OVERRIDE; ///< \copydoc AAX_IController::GetMeterCount()

	//MIDI functions
	AAX_Result	GetNextMIDIPacket( AAX_CFieldIndex* outPort, AAX_CMidiPacket* outPacket ) AAX_OVERRIDE; ///< \copydoc AAX_IController::GetNextMIDIPacket()

	// PageTables functions
	/** \copydoc AAX_IController::CreateTableCopyForEffect()
	 */
	AAX_IPageTable*
	CreateTableCopyForEffect(AAX_CPropertyValue inManufacturerID,
							 AAX_CPropertyValue inProductID,
							 AAX_CPropertyValue inPlugInID,
							 uint32_t inTableType,
							 int32_t inTablePageSize) const AAX_OVERRIDE;
	/** \copydoc AAX_IController::CreateTableCopyForLayout()
	 */
	AAX_IPageTable*
	CreateTableCopyForLayout(const char * inEffectID,
							 const char * inLayoutName,
							 uint32_t inTableType,
							 int32_t inTablePageSize) const AAX_OVERRIDE;
	/** \copydoc AAX_IController::CreateTableCopyForEffectFromFile()
	 */
	AAX_IPageTable*
	CreateTableCopyForEffectFromFile(const char* inPageTableFilePath,
									 AAX_ETextEncoding inFilePathEncoding,
									 AAX_CPropertyValue inManufacturerID,
									 AAX_CPropertyValue inProductID,
									 AAX_CPropertyValue inPlugInID,
									 uint32_t inTableType,
									 int32_t inTablePageSize) const AAX_OVERRIDE;
	/** \copydoc AAX_IController::CreateTableCopyForLayoutFromFile()
	 */
	AAX_IPageTable*
	CreateTableCopyForLayoutFromFile(const char* inPageTableFilePath,
									AAX_ETextEncoding inFilePathEncoding,
									const char* inLayoutName,
									uint32_t inTableType,
									int32_t inTablePageSize) const AAX_OVERRIDE;

private:
	/** @name Component factory methods
	 *
	 */
	//@{
	/** Creates a new, empty \ref AAX_IPageTable object
	 
	 The returned pointer may be NULL if the host does not support the page table interface.
	 */
	ACFPtr<AAX_IACFPageTable_V2> CreatePageTable() const;
	//@}end Component factory methods
	
private:
	ACFPtr<AAX_IACFController>		mIController;
	ACFPtr<AAX_IACFController_V2>	mIControllerV2;
	ACFPtr<AAX_IACFController_V3>	mIControllerV3;
	
	// AAX_IACFPageTableController interface methods are aggregated into AAX_IController
	ACFPtr<AAX_IACFPageTableController>		mIPageTableController;
	ACFPtr<AAX_IACFPageTableController_V2>	mIPageTableControllerV2;
	
	ACFPtr<IACFComponentFactory>	mComponentFactory;
};


#endif // AAX_VCONTROLLER_H

