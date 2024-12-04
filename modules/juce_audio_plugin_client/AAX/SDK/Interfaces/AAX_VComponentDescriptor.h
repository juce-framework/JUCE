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
 *	\file  AAX_VComponentDescriptor.h
 *
 *	\brief Version-managed concrete ComponentDescriptor class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VCOMPONENTDESCRIPTOR_H
#define AAX_VCOMPONENTDESCRIPTOR_H

// AAX Includes
#include "AAX_IComponentDescriptor.h"
#include "AAX_IDma.h"
#include "AAX_IACFComponentDescriptor.h"

// ACF Includes
#include "acfunknown.h"
#include "ACFPtr.h"

// Standard Includes
#include <set>


class AAX_IPropertyMap;
class AAX_IACFComponentDescriptor;
class AAX_IACFComponentDescriptorV2;
class IACFUnknown;

/**
 *	\brief Version-managed concrete \ref AAX_IComponentDescriptor class
 *
 */
class AAX_VComponentDescriptor : public AAX_IComponentDescriptor
{
public:
	AAX_VComponentDescriptor ( IACFUnknown * pUnkHost );
	~AAX_VComponentDescriptor () AAX_OVERRIDE;
	
	AAX_Result			Clear () AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::Clear()
	AAX_Result			AddReservedField ( AAX_CFieldIndex inFieldIndex, uint32_t inFieldType ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddReservedField()
	AAX_Result			AddAudioIn ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddAudioIn()
	AAX_Result			AddAudioOut ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddAudioOut()
	AAX_Result			AddAudioBufferLength ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddAudioBufferLength()
	AAX_Result			AddSampleRate ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddSampleRate()
	AAX_Result			AddClock ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddClock()
	AAX_Result			AddSideChainIn ( AAX_CFieldIndex inFieldIndex ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddSideChainIn()

	AAX_Result			AddDataInPort ( AAX_CFieldIndex inFieldIndex, uint32_t inPacketSize, AAX_EDataInPortType inPortType ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddDataInPort()
	AAX_Result			AddAuxOutputStem ( AAX_CFieldIndex inFieldIndex, int32_t inStemFormat, const char inNameUTF8[]) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddAuxOutputStem()
	AAX_Result			AddPrivateData ( AAX_CFieldIndex inFieldIndex, int32_t inDataSize, uint32_t inOptions ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddPrivateData()
    AAX_Result          AddTemporaryData( AAX_CFieldIndex inFieldIndex, uint32_t inDataElementSize) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddTemporaryData()
	AAX_Result			AddDmaInstance ( AAX_CFieldIndex inFieldIndex, AAX_IDma::EMode inDmaMode ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddDmaInstance()
	AAX_Result			AddMeters ( AAX_CFieldIndex inFieldIndex, const AAX_CTypeID* inMeterIDs, const uint32_t inMeterCount) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddMeters()
	AAX_Result			AddMIDINode ( AAX_CFieldIndex inFieldIndex, AAX_EMIDINodeType inNodeType, const char inNodeName[], uint32_t channelMask ) AAX_OVERRIDE; ///< \copydoc AAX_IComponentDescriptor::AddMIDINode()


	/** \copydoc AAX_IComponentDescriptor::NewPropertyMap()
	 *	
	 *	This implementation retains each generated \ref AAX_IPropertyMap and destroys the property map upon \ref AAX_VComponentDescriptor destruction
	 */
	AAX_IPropertyMap *	NewPropertyMap () const AAX_OVERRIDE;
	/** \copydoc AAX_IComponentDescriptor::DuplicatePropertyMap()
	 *	
	 *	This implementation retains each generated \ref AAX_IPropertyMap and destroys the property map upon \ref AAX_VComponentDescriptor destruction
	 */
	AAX_IPropertyMap *	DuplicatePropertyMap (AAX_IPropertyMap* inPropertyMap) const AAX_OVERRIDE;
	/** \copydoc AAX_IComponentDescriptor::AddProcessProc_Native()
	 */
	virtual AAX_Result			AddProcessProc_Native (
		AAX_CProcessProc inProcessProc, 
		AAX_IPropertyMap * inProperties = NULL, 
		AAX_CInstanceInitProc inInstanceInitProc = NULL, 
		AAX_CBackgroundProc inBackgroundProc = NULL, 
		AAX_CSelector * outProcID = NULL ) AAX_OVERRIDE;
	/** \copydoc AAX_IComponentDescriptor::AddProcessProc_TI()
	 */
	virtual AAX_Result			AddProcessProc_TI ( 
		const char inDLLFileNameUTF8[], 
		const char inProcessProcSymbol[], 
		AAX_IPropertyMap * inProperties = NULL,  
		const char	inInstanceInitProcSymbol [] = NULL, 
		const char	inBackgroundProcSymbol [] = NULL, 
		AAX_CSelector * outProcID = NULL ) AAX_OVERRIDE;
	/** \copydoc AAX_IComponentDescriptor::AddProcessProc()
	 */
	virtual AAX_Result AddProcessProc (
		AAX_IPropertyMap* inProperties,
		AAX_CSelector* outProcIDs = NULL,
		int32_t inProcIDsSize = 0) AAX_OVERRIDE;


	IACFUnknown*				GetIUnknown(void) const;

private:
	// Used for backwards compatibility with clients which do not support AddProcessProc
	friend class AAX_VPropertyMap;
	static const std::set<AAX_EProperty>& PointerPropertiesUsedByAddProcessProc();
	
private:
	ACFPtr<IACFUnknown>					mUnkHost;
	ACFPtr<AAX_IACFComponentDescriptor>	mIACFComponentDescriptor;
    ACFPtr<AAX_IACFComponentDescriptor_V2> mIACFComponentDescriptorV2;
    ACFPtr<AAX_IACFComponentDescriptor_V3> mIACFComponentDescriptorV3;
	std::set<AAX_IPropertyMap *>		mPropertyMaps;	
};


#endif // #ifndef _AAX_ICOMPONENTDESCRIPTOR_H_
