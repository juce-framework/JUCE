/*================================================================================================*/
/*
 *	Copyright 2013-2017, 2019, 2021, 2023-2024 Avid Technology, Inc.
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
/*================================================================================================*/

#include "AAX_VController.h"
#include "AAX_VPageTable.h"
#include "AAX_IACFPageTableController.h"
#include "AAX_UIDs.h"
#include "AAX_Assert.h"

#include "acfbaseapi.h"

#include <memory>

// ******************************************************************************************
// METHOD:	AAX_VController
// ******************************************************************************************
AAX_VController::AAX_VController( IACFUnknown* pUnknown )
{
	if ( pUnknown )
	{
		pUnknown->QueryInterface(IID_IAAXControllerV1, (void **)&mIController);
		pUnknown->QueryInterface(IID_IAAXControllerV2, (void **)&mIControllerV2);
		pUnknown->QueryInterface(IID_IAAXControllerV3, (void **)&mIControllerV3);
		pUnknown->QueryInterface(IID_IAAXPageTableController, (void **)&mIPageTableController);
		pUnknown->QueryInterface(IID_IAAXPageTableControllerV2, (void **)&mIPageTableControllerV2);
		
		try
		{
			mComponentFactory = ACFPtr<IACFComponentFactory>(IID_IACFComponentFactory, pUnknown);
		}
		catch (const ACFRESULT& /*ex*/)
		{
			// swallow any failure here - just clear the smart pointer
			mComponentFactory = NULL;
		}
	}	
}

// ******************************************************************************************
// METHOD:	~AAX_VController
// ******************************************************************************************
AAX_VController::~AAX_VController()
{
}

// ******************************************************************************************
// METHOD:	PostPacket
// ******************************************************************************************
AAX_Result AAX_VController::PostPacket ( AAX_CFieldIndex inFieldIndex, const void * inPayloadP, uint32_t inPayloadSize )
{
	if ( mIController )
		return mIController->PostPacket ( inFieldIndex, inPayloadP, inPayloadSize );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	SendNotification
// ******************************************************************************************
AAX_Result AAX_VController::SendNotification (AAX_CTypeID inNotificationType, const void* inNotificationData, uint32_t inNotificationDataSize)
{
	if (mIControllerV2)
		return mIControllerV2->SendNotification(inNotificationType, inNotificationData, inNotificationDataSize);

	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	SendNotification
// ******************************************************************************************
AAX_Result AAX_VController::SendNotification (AAX_CTypeID inNotificationType)
{
	if (mIControllerV2)
		return mIControllerV2->SendNotification(inNotificationType, NULL, 0);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetEffectID
// ******************************************************************************************
AAX_Result	AAX_VController::GetEffectID ( AAX_IString *	outEffectID) const
{
	if (mIController)
		return mIController->GetEffectID(outEffectID);

	return AAX_ERROR_NULL_OBJECT;
}


// ******************************************************************************************
// METHOD:	GetEffectSampleRate
// ******************************************************************************************
AAX_Result AAX_VController::GetSampleRate ( AAX_CSampleRate * outSampleRate ) const
{
	if ( mIController )
		return mIController->GetSampleRate ( outSampleRate );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetEffectInputStemFormat
// ******************************************************************************************
AAX_Result AAX_VController::GetInputStemFormat ( AAX_EStemFormat * outStemFormat ) const
{
	if ( mIController )
		return mIController->GetInputStemFormat ( outStemFormat );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetEffectOutputStemFormat
// ******************************************************************************************
AAX_Result AAX_VController::GetOutputStemFormat ( AAX_EStemFormat * outStemFormat ) const
{
	if ( mIController )
		return mIController->GetOutputStemFormat ( outStemFormat );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetTODLocation
// ******************************************************************************************
AAX_Result AAX_VController::GetTODLocation ( AAX_CTimeOfDay* outTODLocation ) const
{
	if ( mIController )
		return mIController->GetTODLocation ( outTODLocation );

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetCurrentAutomationTimestamp
// ******************************************************************************************
AAX_Result  AAX_VController::GetCurrentAutomationTimestamp(AAX_CTransportCounter* outTimestamp) const
{
    if (mIControllerV2)
        return mIControllerV2->GetCurrentAutomationTimestamp(outTimestamp);
    
    *outTimestamp = 0;
    return AAX_ERROR_UNIMPLEMENTED;
}


// ******************************************************************************************
// METHOD:	GetSignalLatency
// ******************************************************************************************
AAX_Result	AAX_VController::GetSignalLatency( int32_t* outSamples) const
{
	if (mIController )
		return mIController->GetSignalLatency( outSamples );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetHybridSignalLatency
// ******************************************************************************************
AAX_Result AAX_VController::GetHybridSignalLatency(int32_t* outSamples) const
{
	if (mIControllerV2 )
		return mIControllerV2->GetHybridSignalLatency( outSamples );
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	SetSignalLatency
// ******************************************************************************************
AAX_Result	AAX_VController::SetSignalLatency(int32_t numSamples)
{
	if (mIController )
		return mIController->SetSignalLatency( numSamples );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetCycleCount
// ******************************************************************************************
AAX_Result	AAX_VController::GetCycleCount( AAX_EProperty inWhichCycleCount, AAX_CPropertyValue* outNumCycles) const
{
	if (mIController )
		return mIController->GetCycleCount( inWhichCycleCount, outNumCycles );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	SetCycleCount
// ******************************************************************************************
AAX_Result	AAX_VController::SetCycleCount( AAX_EProperty* inWhichCycleCounts, AAX_CPropertyValue* inValues, int32_t inNumValues)
{
	if (mIController )
		return mIController->SetCycleCount( inWhichCycleCounts, inValues, inNumValues );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetCurrentMeterValue
// ******************************************************************************************
AAX_Result	AAX_VController::GetCurrentMeterValue( AAX_CTypeID inMeterID, float * outMeterValue ) const
{
	if ( mIController )
		return mIController->GetCurrentMeterValue ( inMeterID, outMeterValue );

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetMeterPeakValue
// ******************************************************************************************
AAX_Result	AAX_VController::GetMeterPeakValue( AAX_CTypeID inMeterID, float * outMeterPeakValue ) const
{
	if ( mIController )
		return mIController->GetMeterPeakValue ( inMeterID, outMeterPeakValue );

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	ClearMeterPeakValue
// ******************************************************************************************
AAX_Result	AAX_VController::ClearMeterPeakValue( AAX_CTypeID inMeterID ) const
{
	if ( mIController )
		return mIController->ClearMeterPeakValue ( inMeterID );

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetMeterClipped
// ******************************************************************************************
AAX_Result	AAX_VController::GetMeterClipped( AAX_CTypeID inMeterID, AAX_CBoolean * outClipped ) const
{
	if ( mIController )
		return mIController->GetMeterClipped ( inMeterID, outClipped );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	ClearMeterClipped
// ******************************************************************************************
AAX_Result	AAX_VController::ClearMeterClipped( AAX_CTypeID inMeterID ) const
{
	if ( mIController )
		return mIController->ClearMeterClipped ( inMeterID );
	
	return AAX_ERROR_NULL_OBJECT;
}


// ******************************************************************************************
// METHOD:	GetMeterCount
// ******************************************************************************************
AAX_Result	AAX_VController::GetMeterCount( uint32_t * outMeterCount ) const
{
	if ( mIController )
		return mIController->GetMeterCount ( outMeterCount );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetNextMIDIPacket
// ******************************************************************************************
AAX_Result AAX_VController::GetNextMIDIPacket( AAX_CFieldIndex* outPort, AAX_CMidiPacket* outPacket )
{
	if ( mIController )
		return mIController->GetNextMIDIPacket( outPort, outPacket );

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetPlugInTargetPlatform
// ******************************************************************************************
AAX_Result AAX_VController::GetPlugInTargetPlatform(AAX_CTargetPlatform* outTargetPlatform) const
{
	if (mIControllerV3)
		return mIControllerV3->GetPlugInTargetPlatform(outTargetPlatform);

	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetIsAudioSuite
// ******************************************************************************************
AAX_Result AAX_VController::GetIsAudioSuite(AAX_CBoolean* outIsAudioSuite) const
{
	if (mIControllerV3)
		return mIControllerV3->GetIsAudioSuite(outIsAudioSuite);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetHostName
// ******************************************************************************************
AAX_Result AAX_VController::GetHostName(AAX_IString* outHostNameString) const
{
    if (mIControllerV2)
        return mIControllerV2->GetHostName(outHostNameString);

    return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	CreateTableCopyForEffect
// ******************************************************************************************
AAX_IPageTable* AAX_VController::CreateTableCopyForEffect(AAX_CPropertyValue inManufacturerID,
														  AAX_CPropertyValue inProductID,
														  AAX_CPropertyValue inPlugInID,
														  uint32_t inTableType,
														  int32_t inTablePageSize) const
{
	if (!mIPageTableController) { return NULL; }
	ACFPtr<AAX_IACFPageTable_V2> oPageTable(this->CreatePageTable());
	if (AAX_SUCCESS != mIPageTableController->CopyTableForEffect(inManufacturerID, inProductID, inPlugInID, inTableType, inTablePageSize, oPageTable.inArg()))
	{
		oPageTable = NULL;
	}
	return oPageTable.isNull() ? NULL : new AAX_VPageTable(oPageTable.inArg());
}

// ******************************************************************************************
// METHOD:	CreateTableCopyForLayout
// ******************************************************************************************
AAX_IPageTable* AAX_VController::CreateTableCopyForLayout(const char * inEffectID,
														  const char * inLayoutName,
														  uint32_t inTableType,
														  int32_t inTablePageSize) const
{
	if (!mIPageTableController) { return NULL; }
	ACFPtr<AAX_IACFPageTable_V2> oPageTable(this->CreatePageTable());
	if (AAX_SUCCESS != mIPageTableController->CopyTableOfLayoutForEffect(inEffectID, inLayoutName, inTableType, inTablePageSize, oPageTable.inArg()))
	{
		oPageTable = NULL;
	}
	return oPageTable.isNull() ? NULL : new AAX_VPageTable(oPageTable.inArg());
}

// ******************************************************************************************
// METHOD:	CreateTableCopyForEffectFromFile
// ******************************************************************************************
AAX_IPageTable* AAX_VController::CreateTableCopyForEffectFromFile(const char* inPageTableFilePath,
																  AAX_ETextEncoding inFilePathEncoding,
																  AAX_CPropertyValue inManufacturerID,
																  AAX_CPropertyValue inProductID,
																  AAX_CPropertyValue inPlugInID,
																  uint32_t inTableType,
																  int32_t inTablePageSize) const
{
	if (!mIPageTableControllerV2) { return NULL; }
	ACFPtr<AAX_IACFPageTable_V2> oPageTable(this->CreatePageTable());
	if (AAX_SUCCESS != mIPageTableControllerV2->CopyTableForEffectFromFile(inPageTableFilePath, inFilePathEncoding, inManufacturerID, inProductID, inPlugInID, inTableType, inTablePageSize, oPageTable.inArg()))
	{
		oPageTable = NULL;
	}
	return oPageTable.isNull() ? NULL : new AAX_VPageTable(oPageTable.inArg());
}

// ******************************************************************************************
// METHOD:	CreateTableCopyForLayoutFromFile
// ******************************************************************************************
AAX_IPageTable* AAX_VController::CreateTableCopyForLayoutFromFile(const char* inPageTableFilePath,
																AAX_ETextEncoding inFilePathEncoding,
																const char* inLayoutName,
																uint32_t inTableType,
																int32_t inTablePageSize) const
{
	if (!mIPageTableControllerV2) { return NULL; }
	ACFPtr<AAX_IACFPageTable> oPageTable(this->CreatePageTable());
	if (AAX_SUCCESS != mIPageTableControllerV2->CopyTableOfLayoutFromFile(inPageTableFilePath, inFilePathEncoding, inLayoutName, inTableType, inTablePageSize, oPageTable.inArg()))
	{
		oPageTable = NULL;
	}
	return oPageTable.isNull() ? NULL : new AAX_VPageTable(oPageTable.inArg());
}

// ******************************************************************************************
// METHOD:	CreatePageTable
// ******************************************************************************************
ACFPtr<AAX_IACFPageTable_V2> AAX_VController::CreatePageTable() const
{
	ACFPtr<AAX_IACFPageTable_V2> mIPageTable;
	
	// If this unknown does not support query of an existing page table then check to
	// see if it supports creation of a new, empty page table object
	if ( mComponentFactory )
	{
		// Create the object and get the base interface for it
		const AAX_Result result = static_cast<AAX_Result>(const_cast<IACFComponentFactory&>(*mComponentFactory).CreateComponent(AAXCompID_PageTable, 0, IID_IAAXPageTableV2, (void **)&mIPageTable));
		if (ACF_OK != result)
		{
			mIPageTable = NULL;
		}
	}
	
	return mIPageTable;
}

