/*================================================================================================*/
/*
 *	Copyright 2010-2017, 2023-2024 Avid Technology, Inc.
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

#include "AAX_VHostProcessorDelegate.h"

#include "AAX_UIDs.h"

// ******************************************************************************************
// METHOD:	AAX_VHostProcessorDelegate
// ******************************************************************************************
AAX_VHostProcessorDelegate::AAX_VHostProcessorDelegate( IACFUnknown* pUnknown )
	: mIHostProcessorDelegate(NULL)
	, mIHostProcessorDelegateV2(NULL)
	, mIHostProcessorDelegateV3(NULL)
{
	if ( pUnknown )
	{
		pUnknown->QueryInterface(IID_IAAXHostProcessorDelegateV3, (void **)&mIHostProcessorDelegateV3);
		if (mIHostProcessorDelegateV3)
		{
			mIHostProcessorDelegate = mIHostProcessorDelegateV2 = mIHostProcessorDelegateV3;
		}
		else {
			pUnknown->QueryInterface(IID_IAAXHostProcessorDelegateV2, (void **)&mIHostProcessorDelegateV2);
			if (mIHostProcessorDelegateV2)
			{
				mIHostProcessorDelegate = mIHostProcessorDelegateV2;
			}
			else {
				pUnknown->QueryInterface(IID_IAAXHostProcessorDelegateV1, (void **)&mIHostProcessorDelegate);
			}
		}
	}
}

// ******************************************************************************************
// METHOD:	GetAudio
// ******************************************************************************************
AAX_Result AAX_VHostProcessorDelegate::GetAudio ( const float * const inAudioIns [], int32_t inAudioInCount, int64_t inLocation, int32_t * ioNumSamples )
{
	if ( mIHostProcessorDelegate )
		return mIHostProcessorDelegate->GetAudio( inAudioIns, inAudioInCount, inLocation, ioNumSamples );
	
	return AAX_ERROR_NULL_OBJECT;
}

// ******************************************************************************************
// METHOD:	GetSideChainInputNum
// ******************************************************************************************
int32_t AAX_VHostProcessorDelegate::GetSideChainInputNum ()
{
	int32_t	result = 0;
	if ( mIHostProcessorDelegate )
		return mIHostProcessorDelegate->GetSideChainInputNum();
	
	return result;
}

// ******************************************************************************************
// METHOD:	ForceAnalyze
// ******************************************************************************************
AAX_Result AAX_VHostProcessorDelegate::ForceAnalyze ()
{
	if ( mIHostProcessorDelegateV2 )
		return mIHostProcessorDelegateV2->ForceAnalyze();

	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	ForceProcess
// ******************************************************************************************
AAX_Result AAX_VHostProcessorDelegate::ForceProcess ()
{
	if ( mIHostProcessorDelegateV3 )
		return mIHostProcessorDelegateV3->ForceProcess();

	return AAX_ERROR_UNIMPLEMENTED;
}
