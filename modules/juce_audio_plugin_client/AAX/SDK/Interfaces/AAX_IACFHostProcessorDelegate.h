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
 *	\file  AAX_IACFHostProcessorDelegate.h
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFHOSTPROCESSORDELEGATE_H
#define AAX_IACFHOSTPROCESSORDELEGATE_H

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"


/** @brief Versioned interface for host methods specific to offline processing
 */
class AAX_IACFHostProcessorDelegate : public IACFUnknown
{
public:
	virtual AAX_Result	GetAudio ( const float * const inAudioIns [], int32_t inAudioInCount, int64_t inLocation, int32_t * ioNumSamples ) = 0;	///< \copydoc AAX_IHostProcessorDelegate::GetAudio()
	virtual int32_t		GetSideChainInputNum () = 0;	///< \copydoc AAX_IHostProcessorDelegate::GetSideChainInputNum()
};


/** @brief Versioned interface for host methods specific to offline processing
 */
class AAX_IACFHostProcessorDelegate_V2 : public AAX_IACFHostProcessorDelegate
{
public:
	virtual AAX_Result	ForceAnalyze () = 0;	///< \copydoc AAX_IHostProcessorDelegate::ForceAnalyze()
};

/** @brief Versioned interface for host methods specific to offline processing
 */
class AAX_IACFHostProcessorDelegate_V3 : public AAX_IACFHostProcessorDelegate_V2
{
public:
	virtual AAX_Result	ForceProcess () = 0;	///< \copydoc AAX_IHostProcessorDelegate::ForceProcess()
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // #ifndef AAX_IACFHOSTPROCESSORDELEGATE_H
