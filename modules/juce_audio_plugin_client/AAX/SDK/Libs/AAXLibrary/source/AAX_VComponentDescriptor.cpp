/*================================================================================================*/
/*
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
/*================================================================================================*/

// Self Include
#include "AAX_VComponentDescriptor.h"

// AAX Includes
#include "AAX_VPropertyMap.h"
#include "AAX_UIDs.h"
#include "AAX_Assert.h"

// ACF Includes
#include "acfbaseapi.h"
#include "ACFPtr.h"

// Standard Includes
#include <vector>


// Disable warnings about unused variables to fix issues with result variables
// only being used in AAX_ASSERT checks (AAX_ASSERT is a no-op in release builds)
#ifdef __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wunused-variable"
#elif defined _MSC_VER
#	pragma warning(push)
#	pragma warning(disable : 4189)
#endif

// Helper methods
namespace
{
	// Add ProcessProcs manually based on a property map - for backwards compatibility with
	// hosts that do not support the generic AddProcessProc() method
	//
	// IMPORTANT: If any new pointer properties are queried by this method then they must be
	// added to AAX_VComponentDescriptor::PointerPropertiesUsedByAddProcessProc()
	AAX_Result ManuallyAddProcessProcs(AAX_VComponentDescriptor& inComponentDescriptor,
									   AAX_IPropertyMap* inProperties,
									   AAX_CSelector* outProcIDs,
									   int32_t inProcIDsSize)
	{
		AAX_CPropertyValue nativeID = 0, asID = 0, tiID = 0;
		const AAX_CBoolean hasNative = inProperties->GetProperty(AAX_eProperty_PlugInID_Native, &nativeID);
		const AAX_CBoolean hasAS = inProperties->GetProperty(AAX_eProperty_PlugInID_AudioSuite, &asID);
		const AAX_CBoolean hasTI = inProperties->GetProperty(AAX_eProperty_PlugInID_TI, &tiID);
		
		std::vector<AAX_CSelector> procIDs;
		AAX_Result result = AAX_SUCCESS;
		
		if ((AAX_SUCCESS == result) && (hasNative || hasAS))
		{
			const void* processProc = 0;
			if (false == inProperties->GetPointerProperty(AAX_eProperty_NativeProcessProc, &processProc))
			{
				AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAX_VComponentDescriptor.cpp: ManuallyAddProcessProcs() - no value found for AAX_eProperty_NativeProcessProc");
			}
			
			const void* initProc = 0;
			inProperties->GetPointerProperty(AAX_eProperty_NativeInstanceInitProc, &initProc);
			
			const void* backgroundProc = 0;
			inProperties->GetPointerProperty(AAX_eProperty_NativeBackgroundProc, &backgroundProc);
			
			// Strip out AAX_eProperty_AudioBufferLength, which should only be used for DSP types
			AAX_IPropertyMap* nativeProps = inProperties;
			AAX_CPropertyValue bufferLengthProperty = 0;
			if (0 != inProperties->GetProperty(AAX_eProperty_AudioBufferLength, &bufferLengthProperty))
			{
				// this object's lifetime is managed by the component descriptor
				AAX_IPropertyMap* const tempProps = inComponentDescriptor.DuplicatePropertyMap(inProperties);
				
				// Verify that the duplicate map object is valid
				if (NULL != tempProps && NULL != tempProps->GetIUnknown())
				{
					const AAX_CBoolean gotPropertySuccess = tempProps->GetProperty(AAX_eProperty_AudioBufferLength, &bufferLengthProperty);
					AAX_ASSERT(0 != gotPropertySuccess);
					if (gotPropertySuccess)
					{
						const AAX_Result removeNativeAudioBufferLengthResult = tempProps->RemoveProperty(AAX_eProperty_AudioBufferLength);
						AAX_ASSERT(AAX_SUCCESS == removeNativeAudioBufferLengthResult);
						nativeProps = tempProps;
					}
				}
			}
			
			// Do the Native ProcessProc registration call (includes both Native and AudioSuite)
			AAX_CSelector nativeProcID = 0;
			result = inComponentDescriptor.AddProcessProc_Native(
				reinterpret_cast<AAX_CProcessProc>(const_cast<void*>(processProc)),
				nativeProps,
				reinterpret_cast<AAX_CInstanceInitProc>(const_cast<void*>(initProc)),
				reinterpret_cast<AAX_CBackgroundProc>(const_cast<void*>(backgroundProc)),
				&nativeProcID);
			
			if (AAX_SUCCESS == result)
			{
				procIDs.push_back(nativeProcID);
			}
		}
		
		if ((AAX_SUCCESS == result) && hasTI)
		{
			const char* fileName = NULL;
			if (false == inProperties->GetPointerProperty(AAX_eProperty_TIDLLFileName, reinterpret_cast<const void**>(&fileName)))
			{
				AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAX_VComponentDescriptor.cpp: ManuallyAddProcessProcs() - no value found for AAX_eProperty_TIDLLFileName");
			}
			
			const char* processProcSymbol = NULL;
			if (false == inProperties->GetPointerProperty(AAX_eProperty_TIProcessProc, reinterpret_cast<const void**>(&processProcSymbol)))
			{
				AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAX_VComponentDescriptor.cpp: ManuallyAddProcessProcs() - no value found for AAX_eProperty_TIProcessProc");
			}
			
			const char* initProcSymbol = NULL;
			inProperties->GetPointerProperty(AAX_eProperty_TIInstanceInitProc, reinterpret_cast<const void**>(&initProcSymbol));
			
			const char* backgroundProcSymbol = NULL;
			inProperties->GetPointerProperty(AAX_eProperty_TIBackgroundProc, reinterpret_cast<const void**>(&backgroundProcSymbol));
			
			AAX_CSelector tiProcID = 0;
			result = inComponentDescriptor.AddProcessProc_TI(
				reinterpret_cast<const char*>(fileName),
				reinterpret_cast<const char*>(processProcSymbol),
				inProperties,
				reinterpret_cast<const char*>(initProcSymbol),
				reinterpret_cast<const char*>(backgroundProcSymbol),
				&tiProcID);
			
			if (AAX_SUCCESS == result)
			{
				procIDs.push_back(tiProcID);
			}
		}
		
		if (AAX_SUCCESS == result && NULL != outProcIDs)
		{
			const size_t numProcIDs = procIDs.size();
			if (numProcIDs < INT32_MAX && inProcIDsSize > (int32_t)numProcIDs)
			{
				for (size_t i = 0; i < numProcIDs; ++i)
				{
					outProcIDs[i] = procIDs.at(i);
				}
				outProcIDs[numProcIDs] = 0;
			}
			else
			{
				result = AAX_ERROR_ARGUMENT_BUFFER_OVERFLOW;
			}
		}
		
		return result;
	}
}

// Re-enable unused variable warnings
#ifdef __clang__
#	pragma clang diagnostic pop
#elif defined _MSC_VER
#	pragma warning(pop)
#endif


// ******************************************************************************************
// METHOD:	AAX_VComponentDescriptor
// ******************************************************************************************
AAX_VComponentDescriptor::AAX_VComponentDescriptor( IACFUnknown * pUnkHost ) :
	mUnkHost( pUnkHost ),
	mIACFComponentDescriptor( NULL ),
	mIACFComponentDescriptorV2( NULL ),
	mIACFComponentDescriptorV3( NULL )
{
	if ( mUnkHost )
	{
		// Get the component factory service from the host so we can create the
		// built-in plug-in definition.
		ACFPtr<IACFComponentFactory> pFactory;
		if ( pUnkHost->QueryInterface(IID_IACFComponentFactory, (void **)&pFactory) == ACF_OK )
		{
            // Create the object and get the base interface for it.
			pFactory->CreateComponent(AAXCompID_AAXComponentDescriptor, 0, IID_IAAXComponentDescriptorV1, (void **)&mIACFComponentDescriptor);
            
			// Get the V2 interface
			if (mIACFComponentDescriptor)
                mIACFComponentDescriptor->QueryInterface(IID_IAAXComponentDescriptorV2, (void**)&mIACFComponentDescriptorV2);
            
			// Get the V3 interface
			if (mIACFComponentDescriptor)
                mIACFComponentDescriptor->QueryInterface(IID_IAAXComponentDescriptorV3, (void**)&mIACFComponentDescriptorV3);
		}
	}
}

// ******************************************************************************************
// METHOD:	~AAX_VComponentDescriptor
// ******************************************************************************************
AAX_VComponentDescriptor::~AAX_VComponentDescriptor ()
{
	std::set<AAX_IPropertyMap *>::iterator iter = mPropertyMaps.begin ();
	for ( ; iter != mPropertyMaps.end (); ++iter )
		delete *iter;
}


// ******************************************************************************************
// METHOD:	GetIUnknown
// ******************************************************************************************
IACFUnknown*				
AAX_VComponentDescriptor::GetIUnknown(void) const
{
	return mIACFComponentDescriptor;
}

// ******************************************************************************************
// METHOD:	Clear
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::Clear ()
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->Clear();
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddReservedField
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddReservedField ( AAX_CFieldIndex inPortID, uint32_t inFieldType )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddReservedField ( inPortID, inFieldType );

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddAudioIn
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddAudioIn ( AAX_CFieldIndex inPortID )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddAudioIn ( inPortID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddAudioOut
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddAudioOut ( AAX_CFieldIndex inPortID )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddAudioOut ( inPortID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddAudioBufferLength
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddAudioBufferLength ( AAX_CFieldIndex inPortID )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddAudioBufferLength ( inPortID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddSampleRate
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddSampleRate ( AAX_CFieldIndex inPortID )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddSampleRate ( inPortID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddClock
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddClock ( AAX_CFieldIndex inPortID )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddClock ( inPortID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddSideChainIn
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddSideChainIn ( AAX_CFieldIndex inPortID )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddSideChainIn ( inPortID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddDataInPort
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddDataInPort ( AAX_CFieldIndex inPortID, uint32_t inPacketSize, AAX_EDataInPortType inPortType )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddDataInPort ( inPortID, inPacketSize, inPortType);
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddAuxOutputStem
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddAuxOutputStem ( AAX_CFieldIndex inPortID, int32_t inStemFormat, const char inNameUTF8[] )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddAuxOutputStem ( inPortID, inStemFormat, inNameUTF8 );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddPrivateData
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddPrivateData ( AAX_CFieldIndex inPortID, int32_t inDataSize, uint32_t /* AAX_EPrivateDataOptions */ inOptions /* = AAX_ePrivateDataOptions_DefaultOptions */ )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddPrivateData ( inPortID, inDataSize, inOptions );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddDmaInstance
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddDmaInstance ( AAX_CFieldIndex inPortID, AAX_IDma::EMode inDmaMode )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddDmaInstance ( inPortID, inDmaMode );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// ******************************************************************************************
// METHOD:	AddMeter
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddMeters (
												AAX_CFieldIndex					inPortID,
												const AAX_CTypeID*			inMeterIDs,
												const uint32_t				inMeterCount)
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddMeters ( inPortID, inMeterIDs, inMeterCount );

	return AAX_ERROR_NULL_OBJECT;
}

// METHOD:	NewPropertyMap
// ******************************************************************************************
AAX_IPropertyMap * AAX_VComponentDescriptor::NewPropertyMap () const
{
	AAX_VPropertyMap * propertyMap = AAX_VPropertyMap::Create( mUnkHost );
	const_cast<AAX_VComponentDescriptor*>(this)->mPropertyMaps.insert( propertyMap );
	return propertyMap;
}

// METHOD:	ClonePropertyMap
// ******************************************************************************************
AAX_IPropertyMap * AAX_VComponentDescriptor::DuplicatePropertyMap (AAX_IPropertyMap* inPropertyMap) const
{
	AAX_IPropertyMap * newPropertyMap = this->NewPropertyMap();
	for (AAX_EProperty curPropertyID = AAX_eProperty_MinProp;
		 curPropertyID < AAX_eProperty_MaxProp;
		 curPropertyID = (AAX_EProperty((int32_t)curPropertyID + 1)))
	{
		AAX_CPropertyValue curPropertyValue = 0;
		const void* curPointerPropertyValue = 0;
		if (inPropertyMap->GetProperty(curPropertyID, &curPropertyValue))
		{
			newPropertyMap->AddProperty(curPropertyID, curPropertyValue);
		}
		else if (inPropertyMap->GetPointerProperty(curPropertyID, &curPointerPropertyValue))
		{
			newPropertyMap->AddPointerProperty(curPropertyID, curPointerPropertyValue);
		}
	}
	
	return newPropertyMap;
}

// ******************************************************************************************
// METHOD:	AddProcessProc_Native
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddProcessProc_Native( 
	AAX_CProcessProc inProcessProc, 
	AAX_IPropertyMap * inProperties /*= NULL*/, 
	AAX_CInstanceInitProc inInstanceInitProc /*= NULL*/, 
	AAX_CBackgroundProc inBackgroundProc /*= NULL*/, 
	AAX_CSelector * outProcID /*= NULL */ )
{
	if ( mIACFComponentDescriptor )
		return mIACFComponentDescriptor->AddProcessProc_Native( 
			inProcessProc, 
			inProperties ? inProperties->GetIUnknown() : NULL,
			inInstanceInitProc, 
			inBackgroundProc, 
			outProcID );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddProcessProc_TI
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddProcessProc_TI( 
	const char inDLLFileNameUTF8[], 
	const char inProcessProcSymbol[],
	AAX_IPropertyMap * inProperties /*= NULL*/, 
	const char inInstanceInitProcSymbol [] /*= NULL*/, 
	const char inBackgroundProcSymbol [] /*= NULL*/, 
	AAX_CSelector * outProcID /*= NULL */ )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddProcessProc_TI( 
			inDLLFileNameUTF8, 
			inProcessProcSymbol, 
			inProperties ? inProperties->GetIUnknown() : NULL, 
			inInstanceInitProcSymbol, 
			inBackgroundProcSymbol, 
			outProcID );
		
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddProcessProc
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddProcessProc(
	AAX_IPropertyMap* inProperties,
	AAX_CSelector* outProcIDs /*= NULL*/,
	int32_t inProcIDsSize /*= 0*/)
{
	if ( mIACFComponentDescriptorV3 )
	{
		return mIACFComponentDescriptorV3->AddProcessProc(
			inProperties ? inProperties->GetIUnknown() : NULL,
			outProcIDs,
			inProcIDsSize);
	}
	else if ( mIACFComponentDescriptor && inProperties )
	{
		// If the full AddProcessProc routine is not supported by the host then
		// attempt to register each ProcessProc separately using the available
		// registration methods in the V1 interface.
		return ManuallyAddProcessProcs(*this, inProperties, outProcIDs, inProcIDsSize);
	}
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddMIDINode
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddMIDINode ( AAX_CFieldIndex inPortID, AAX_EMIDINodeType inNodeType, const char inNodeName[], uint32_t channelMask )
{
	if ( mIACFComponentDescriptor ) 
		return mIACFComponentDescriptor->AddMIDINode ( inPortID, inNodeType, inNodeName, channelMask );

	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	AddTemporaryData
// ******************************************************************************************
AAX_Result AAX_VComponentDescriptor::AddTemporaryData( AAX_CFieldIndex inFieldIndex, uint32_t inDataElementSize)
{
    if (mIACFComponentDescriptorV2)
        return mIACFComponentDescriptorV2->AddTemporaryData(inFieldIndex, inDataElementSize);
    return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	PointerPropertiesUsedByAddProcessProc
// ******************************************************************************************

/* static */
const std::set<AAX_EProperty>& AAX_VComponentDescriptor::PointerPropertiesUsedByAddProcessProc()
{
// we don't care that the destructor for this set runs at exit
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4640) // disable warning C4640: construction of local static object is not thread-safe
#endif
	static std::set<AAX_EProperty> props;
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
	
	if (props.empty()) // assume single-threaded
	{
		props.insert(AAX_eProperty_NativeProcessProc);
		props.insert(AAX_eProperty_NativeInstanceInitProc);
		props.insert(AAX_eProperty_NativeBackgroundProc);
		props.insert(AAX_eProperty_TIDLLFileName);
		props.insert(AAX_eProperty_TIProcessProc);
		props.insert(AAX_eProperty_TIInstanceInitProc);
		props.insert(AAX_eProperty_TIBackgroundProc);
	}
	
	return props;
}
