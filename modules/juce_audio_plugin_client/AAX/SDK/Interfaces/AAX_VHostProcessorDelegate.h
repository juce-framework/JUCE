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
 *	\file  AAX_VHostProcessorDelegate.h
 *
 *	\brief Version-managed concrete HostProcessorDelegate class
 *
 */ 
/*================================================================================================*/

#ifndef AAX_VHOSTPROCESSORDELEGATE_H
#define AAX_VHOSTPROCESSORDELEGATE_H

#include "AAX_IHostProcessorDelegate.h"
#include "AAX_IACFHostProcessorDelegate.h"
#include "ACFPtr.h"


class IACFUnknown;
class AAX_IACFHostProcessorDelegate;

/*! \brief Version-managed concrete \ref AAX_IHostProcessorDelegate "Host Processor delegate" class
 	
 	\details
	\ingroup AuxInterface_HostProcessor
*/
class AAX_VHostProcessorDelegate : public AAX_IHostProcessorDelegate
{
public:
	AAX_VHostProcessorDelegate( IACFUnknown* pUnknown );
	
	AAX_Result	GetAudio ( const float * const inAudioIns [], int32_t inAudioInCount, int64_t inLocation, int32_t * ioNumSamples ) AAX_OVERRIDE; ///< \copydoc AAX_IHostProcessorDelegate::GetAudio()
	int32_t		GetSideChainInputNum () AAX_OVERRIDE; ///< \copydoc AAX_IHostProcessorDelegate::GetSideChainInputNum()
	AAX_Result	ForceAnalyze () AAX_OVERRIDE; ///< \copydoc AAX_IHostProcessorDelegate::ForceAnalyze()
	AAX_Result	ForceProcess () AAX_OVERRIDE; ///< \copydoc AAX_IHostProcessorDelegate::ForceProcess()

private:	
	ACFPtr<AAX_IACFHostProcessorDelegate>		mIHostProcessorDelegate;
	ACFPtr<AAX_IACFHostProcessorDelegate_V2>	mIHostProcessorDelegateV2;
	ACFPtr<AAX_IACFHostProcessorDelegate_V3>	mIHostProcessorDelegateV3;
};



#endif //AAX_IAUTOMATIONDELEGATE_H

