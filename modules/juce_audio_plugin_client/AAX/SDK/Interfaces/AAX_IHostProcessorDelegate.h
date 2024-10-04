/*================================================================================================*/
/*
 *
 *	Copyright 2014-2017, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IHostProcessorDelegate.h
 *
 *  \brief Interface allowing plug-in's HostProcessor to interact with the host's side
 */ 
/*================================================================================================*/


#ifndef AAX_IHOSTPROCESSORDELEGATE_H
#define AAX_IHOSTPROCESSORDELEGATE_H

#include "AAX.h"

/*! @brief Versioned interface for host methods specific to offline processing
	
	\details
	\hostimp
	
	The host provides a host processor delegate to a plug-in's \ref AAX_IHostProcessor "host processor" object at initialization.
	The host processor object may make calls to this object to get information about the current render pass or to affect the
	plug-in's offline processing behavior.

	\ingroup AuxInterface_HostProcessor
*/
class AAX_IHostProcessorDelegate
{
public:
	
	virtual ~AAX_IHostProcessorDelegate() {}
	
	/*!
	 *	\brief CALL: Randomly access audio from the timeline
	 *
	 *	Called from within \ref AAX_IHostProcessor::RenderAudio(), this method fills a buffer of samples with randomly-accessed
	 *	data from the current input processing region on the timeline, including any extra samples such as processing "handles".
	 *	
	 *	\note Plug-ins that use this feature must set \ref AAX_eProperty_UsesRandomAccess to \c true
	 *	\note It is not possible to retrieve samples from outside of the current input processing region
	 *	\note Always check the return value of this method before using the randomly-accessed samples
	 *	
	 *	\param[in] inAudioIns
	 *		Timeline audio buffer(s). This must be set to \c inAudioIns from \ref AAX_IHostProcessor::RenderAudio()
	 *		\internal See PTSW-183848: \ref AAX_IHostProcessorDelegate::GetAudio() ignores input audio buffer parameter \endinternal
	 *	\param[in] inAudioInCount
	 *		Number of buffers in \c inAudioIns. This must be set to \c inAudioInCount from \ref AAX_IHostProcessor::RenderAudio()
	 *		\internal See PTSW-183848: \ref AAX_IHostProcessorDelegate::GetAudio() ignores input audio buffer parameter \endinternal
	 *	\param[in] inLocation
	 *		A sample location relative to the beginning of the currently processed region, e.g. a value of 0 corresponds to the
	 *		timeline location returned by \ref AAX_CHostProcessor::GetSrcStart()
	 *	\param[in,out] ioNumSamples
	 *		\li Input: The maximum number of samples to read.
	 *		\li Output: The actual number of samples that were read from the timeline
	 */
	virtual AAX_Result	GetAudio ( const float * const inAudioIns [], int32_t inAudioInCount, int64_t inLocation, int32_t * ioNumSamples ) = 0;
	/*! \brief CALL: Returns the index of the side chain input buffer
	 *
	 *	Called from within \ref AAX_IHostProcessor::RenderAudio(), this method returns the index of the side chain input sample
	 *	buffer within \c inAudioIns.
	 *
	 */
	virtual int32_t		GetSideChainInputNum () = 0;
	/*!
	 *	\brief CALL: Request an analysis pass
	 *
	 *	Call this method to request an analysis pass from within the plug-in. Most plug-ins should rely on the host to trigger
	 *	analysis passes when appropriate. However, plug-ins that require an analysis pass a) outside of the context of
	 *	host-driven render or analysis, or b) when internal plug-in data changes may need to call \c ForceAnalyze().
	 */
	virtual AAX_Result	ForceAnalyze () = 0;
	/*!
	 *	\brief CALL: Request a process pass
	 *
	 *	Call this method to request a process pass from within the plug-in. If \ref AAX_eProperty_RequiresAnalysis is defined,
	 *	the resulting process pass will be preceded by an analysis pass. This method should only be used in rare circumstances
	 *	by plug-ins that must launch processing outside of the normal host AudioSuite workflow.
	 */
	virtual AAX_Result	ForceProcess () = 0;
};

#endif // #ifndef _AAX_IPLUGIN_H_
