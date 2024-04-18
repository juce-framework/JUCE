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
 *	\file AAX_CHostProcessor.h
 *
 *	\brief Concrete implementation of the AAX_IHostProcessor interface for non-real-time processing
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CHOSTPROCESSOR_H
#define AAX_CHOSTPROCESSOR_H

#include "AAX_IEffectParameters.h"
#include "AAX_IHostProcessor.h"
#include "ACFPtr.h"


class AAX_IHostProcessorDelegate;
class AAX_IController;
class AAX_IEffectParameters;
class IACFUnknown;

/**
 *	\brief Concrete implementation of the AAX_IHostProcessor interface for
 *	non-real-time processing
 *  
 *	\details
 *	Host processor objects are used to process regions of audio data in a
 *	non-real-time context.
 *	- Host processors must generate output samples
 *	  linearly and incrementally, but may randomly access samples from the
 *	  processing region on the timeline for input. See
 *	  \ref AAX_IHostProcessorDelegate::GetAudio() "GetAudio()" for more
 *	  information.
 *	- Host processors may re-define the processing region using
 *	  \ref AAX_CHostProcessor::TranslateOutputBounds().
 *	
 *  \sa AAX_IHostProcessorDelegate
 *
 *	\ingroup AuxInterface_HostProcessor
 */
class AAX_CHostProcessor : public AAX_IHostProcessor
{
public:
	/* default constructor */	AAX_CHostProcessor (void);
	virtual /* destructor */	~AAX_CHostProcessor ();
	
	/** @name Initialization and uninitialization
	 */
	//@{
	/** \brief Host Processor initialization
	 *
	 *  \param[in] iController
	 *		A versioned reference that can be resolved to both an
	 *		AAX_IController interface and an AAX_IHostProcessorDelegate
	 */	
	AAX_Result		Initialize(IACFUnknown* iController) AAX_OVERRIDE;
	/** \brief Host Processor teardown
	 */	
	AAX_Result		Uninitialize() AAX_OVERRIDE;
	//@}end Initialization and uninitialization
	
	/** @name Host processor interface
	 */
	//@{
    /** \brief Sets the processing region
	 *
	 *  This method allows offline processing plug-ins to vary the length
	 *  and/or start/end points of the audio processing region.
	 *  
	 *	This method is called in a few different scenarios:
	 *  - Before an analyze, process or preview of data begins.
	 *  - At the end of every preview loop.
	 *  - After the user makes a new data selection on the timeline.
	 *  
	 *	Plug-ins that inherit from \ref AAX_CHostProcessor should not override
	 *	this method. Instead, use the following convenience functions:
	 *  - To retrieve the length or boundaries of the processing region,
	 *    use \ref AAX_CHostProcessor::GetInputRange() "GetInputRange()",
	 *    \ref AAX_CHostProcessor::GetSrcStart() "GetSrcStart()", etc.
	 *  - To change the boundaries of the processing region before processing
	 *    begins, use \ref AAX_CHostProcessor::TranslateOutputBounds()
     *	
	 *	\note Currently, a host processor may not randomly access samples
	 *	outside of the boundary defined by \c oDstStart and \c oDstEnd
	 *  
     *  \legacy DAE no longer makes use of the mStartBound and mEndBounds 
     *  member variables that existed in the legacy RTAS/TDM SDK. Use
	 *	\c oDstStart and \c oDstEnd instead (preferably by overriding
	 *	\ref AAX_CHostProcessor::TranslateOutputBounds() "TranslateOutputBounds()".)
	 *
	 *	\param[in] iSrcStart
	 *		The selection start of the user selected region. This is will always 
     *      return 0 for a given selection on the timeline.
     *	\param[in] iSrcEnd
	 *		The selection end of the user selected region. This will always
     *      return the value of the selection length on the timeline.
     *	\param[in] oDstStart
	 *		The starting sample location in the output audio region. By default,
	 *		this is the same as \c iSrcStart.
	 *	\param[in] oDstEnd
	 *		The ending sample location in the output audio region. By default,
	 *		this is the same as \c iSrcEnd.
	 */
	AAX_Result		InitOutputBounds ( int64_t iSrcStart, int64_t iSrcEnd, int64_t * oDstStart, int64_t * oDstEnd ) AAX_OVERRIDE;
	
	/** \brief Updates the relative sample location of the current processing frame
	 *  
	 *  This method is called by the host to update the relative sample location of
	 *  the current processing frame.
	 *  
	 *  \note Plug-ins should not override this method; instead, use
	 *  \ref AAX_CHostProcessor::GetLocation() to retrieve the current relative
	 *  sample location.
	 *  
	 *  \param[in] iSample
	 *		The sample location of the first sample in the current processing frame
	 *		relative to the beginning of the full processing buffer
	 */
	AAX_Result		SetLocation ( int64_t iSample ) AAX_OVERRIDE;

    /** \brief Perform the signal processing
	 *
	 *	This method is called by the host to invoke the plug-in's signal
	 *  processing.
     *
     *  \legacy This method is a replacement for the AudioSuite
	 *  \c ProcessAudio method
     *
	 *	\param[in] inAudioIns
	 *		Input audio buffer
     *	\param[in] inAudioInCount
	 *		The number if input channels
     *	\param[in] iAudioOuts
	 *		The number of output channels
     *	\param[in] iAudioOutCount
	 *		A user defined destination end of the ingested audio
     *	\param[in] ioWindowSize
	 *		Window buffer length of the received audio
	 */
	AAX_Result		RenderAudio ( const float * const inAudioIns [], int32_t inAudioInCount, float * const iAudioOuts [], int32_t iAudioOutCount, int32_t * ioWindowSize ) AAX_OVERRIDE;
    
    /** \brief Invoked right before the start of a Preview or Render pass
	 *
	 *	This method is called by the host to allow a plug-in to make any initializations before 
     *  processing actually begins. Upon a Preview pass, PreRender will also be called at the 
     *  beginning of every "loop".
     *
	 *  \see AAX_eProcessingState_StartPass, \ref AAX_eProcessingState_BeginPassGroup
     *
	 *	\param[in] inAudioInCount
	 *		The number if input channels
     *	\param[in] iAudioOutCount
	 *		The number of output channels
     *	\param[in] iWindowSize
     *      Window buffer length of the ingested audio
	 */
	AAX_Result		PreRender ( int32_t inAudioInCount, int32_t iAudioOutCount, int32_t iWindowSize ) AAX_OVERRIDE;
    
    /** \brief Invoked at the end of a Render pass
	 *
	 *	\note Upon a Preview pass, PostRender will not be called until Preview has stopped.
     *
	 *  \see AAX_eProcessingState_StopPass, \ref AAX_eProcessingState_EndPassGroup
     *
	 */
	AAX_Result		PostRender () AAX_OVERRIDE;

    /** \brief Override this method if the plug-in needs to analyze the audio prior to a Render pass
	 *
	 *	Use this after declaring the appropriate properties in Describe. See
	 *  \ref AAX_eProperty_RequiresAnalysis and \ref AAX_eProperty_OptionalAnalysis
     *
	 *  To request an analysis pass from within a plug-in, use
	 *  \ref AAX_IHostProcessorDelegate::ForceAnalyze()
	 *
     *  \legacy Ported from AudioSuite's \c AnalyzeAudio(bool \c isMasterBypassed) method
     *
	 *	\param[in] inAudioIns
	 *		Input audio buffer
     *	\param[in] inAudioInCount
	 *		The number of input channels
     *	\param[in] ioWindowSize
     *      Window buffer length of the ingested audio
	 */
	AAX_Result		AnalyzeAudio ( const float * const inAudioIns [], int32_t inAudioInCount, int32_t * ioWindowSize ) AAX_OVERRIDE;
	
    /** \brief Invoked right before the start of an Analysis pass
	 *
     *	This method is called by the host to allow a plug-in to make any initializations before 
     *  an Analysis pass actually begins.
     *
	 *  \see AAX_eProcessingState_StartPass, \ref AAX_eProcessingState_BeginPassGroup
	 *
	 *	\param[in] inAudioInCount
	 *		The number if input channels
      *	\param[in] iWindowSize
     *      Window buffer length of the ingested audio
	 */
    AAX_Result		PreAnalyze ( int32_t inAudioInCount, int32_t iWindowSize ) AAX_OVERRIDE;
    
    /** \brief Invoked at the end of an Analysis pass
	 *
	 *	\note In Pro Tools, a long execution time for this method will hold off the main
	 *	application thread and cause a visible hang. If the plug-in must perform any long
	 *	running tasks before initiating processing then it is best to perform these tasks
	 *	in \ref AAX_IHostProcessor::PreRender()
     *
	 *  \see \ref AAX_eProcessingState_StopPass, \ref AAX_eProcessingState_EndPassGroup
	 */
	AAX_Result		PostAnalyze () AAX_OVERRIDE;
	/*!
	 *	\brief Called by host application to retrieve a custom string to be appended to the clip name.
	 *
	 *	If no string is provided then the host's default will be used.
	 *
	 *	\param[in] inMaxLength
	 *		The maximum allowed string length, not including the NULL terminating char
	 *
	 *	\param[out] outString
	 *		Add a value to this string to provide a custom clip suffix
     *
	 */
    AAX_Result		GetClipNameSuffix ( int32_t inMaxLength, AAX_IString* outString ) const AAX_OVERRIDE;
	//@}end Host processor interface
	
	
	/** @name Convenience methods
	 */
	//@{
	AAX_IEffectParameters *				GetEffectParameters () { return mEffectParameters; }
	const AAX_IEffectParameters *		GetEffectParameters () const { return mEffectParameters; }
	AAX_IHostProcessorDelegate* 		GetHostProcessorDelegate () { return mHostProcessingDelegate; }
	const AAX_IHostProcessorDelegate* 	GetHostProcessorDelegate () const { return mHostProcessingDelegate; }
    
    /** \brief The relative sample location of the current processing frame
	 *
     *	This method returns the relative sample location for the current \ref RenderAudio()
     *  processing frame. For example, if a value of 10 is provided for the
	 *  \ref RenderAudio() \c ioWindow parameter, then calls to this method from within
     *  each execution of \ref RenderAudio() will return 0, 10, 20,... 
     *
	 */
	int64_t							GetLocation() const { return mLocation; }
    
    /** \brief The length (in samples) of the current timeline selection
	 */
	int64_t							GetInputRange() const { return (mSrcEnd - mSrcStart); }
    /** \brief The length (in samples) of the clip that will be rendered to the timeline
	 */
	int64_t							GetOutputRange() const { return (mDstEnd - mDstStart); }
    /** \brief The sample position of the beginning of the current timeline selection relative
	 *	to the beginning of the current input selection, i.e. 0
	 */
	int64_t							GetSrcStart() const { return mSrcStart; }
    /** \brief The sample position of the end of the current timeline selection relative to the
	 *	beginning of the current input selection
	 */
	int64_t							GetSrcEnd() const { return mSrcEnd; }
    /** \brief The sample position of the beginning of the of the clip that will be rendered to
	 *	the timeline relative to the beginning of the current input selection
	 *
	 *	This value will be equal to the value returned by \ref GetSrcStart() unless the selection
	 *	boundaries have been modified by overriding \ref TranslateOutputBounds()
	 */
	int64_t							GetDstStart() const { return mDstStart; }
    /** \brief The sample position of the end of the of the clip that will be rendered to the
	 *	timeline relative to the beginning of the current input selection
	 *
	 *	This value will be equal to the value returned by \ref GetSrcStart() unless the selection
	 *	boundaries have been modified by overriding \ref TranslateOutputBounds()
	 */
	int64_t							GetDstEnd() const { return mDstEnd; }
	//@}end Convenience methods
	
protected:
    /** @name Convenience methods
	 */
	//@{
	/** \brief Define the boundaries of the clip that will be rendered to the timeline
	 *
	 *	This method is called from \ref AAX_CHostProcessor::InitOutputBounds(), providing
	 *	a convenient hook for re-defining the processing region boundaries. See
	 *	\ref AAX_CHostProcessor::InitOutputBounds() "InitOutputBounds()" for more information.
	 *
	 *	\param[in] iSrcStart
	 *		The selection start of the user selected region. This is will always 
	 *      return 0 for a given selection on the timeline.
	 *	\param[in] iSrcEnd
	 *		The selection end of the user selected region. This will always
	 *      return the value of the selection length on the timeline.
	 *	\param[in] oDstStart
	 *		The starting sample location in the output audio region. By default,
	 *		this is the same as \c iSrcStart.
	 *	\param[in] oDstEnd
	 *		The ending sample location in the output audio region. By default,
	 *		this is the same as \c iSrcEnd.
	 */
	virtual AAX_Result		TranslateOutputBounds ( int64_t iSrcStart, int64_t iSrcEnd, int64_t& oDstStart, int64_t& oDstEnd );

	/** \brief Randomly access audio from the timeline
	 *
	 *	This is a convenience wrapper around \ref AAX_IHostProcessorDelegate::GetAudio().
	 *
	 *	\param[in] inAudioIns
	 *		Timeline audio buffer(s). This must be set to \c inAudioIns from \ref AAX_IHostProcessor::RenderAudio()
	 *		\internal See PTSW-183848: AAX_IHostProcessor::GetAudio() ignores input audio buffer parameter \endinternal
	 *	\param[in] inAudioInCount
	 *		Number of buffers in \c inAudioIns. This must be set to \c inAudioInCount from \ref AAX_IHostProcessor::RenderAudio()
	 *		\internal See PTSW-183848: AAX_IHostProcessor::GetAudio() ignores input audio buffer parameter \endinternal
	 *	\param[in] inLocation
	 *		A sample location relative to the beginning of the currently processed region, e.g. a value of 0 corresponds to the
	 *		timeline location returned by \ref AAX_CHostProcessor::GetSrcStart()
	 *	\param[in,out] ioNumSamples
	 *		\li Input: The maximum number of samples to read.
	 *		\li Output: The actual number of samples that were read from the timeline
	 */
	virtual AAX_Result		GetAudio ( const float * const inAudioIns [], int32_t inAudioInCount, int64_t inLocation, int32_t * ioNumSamples );

	/*! \brief CALL: Returns the index of the side chain input buffer
	 *
	 *	This is a convenience wrapper around \ref AAX_IHostProcessorDelegate::GetSideChainInputNum()
	 */
	virtual int32_t			GetSideChainInputNum ();
	
	// Exterior Object Access
	AAX_IController*					Controller()					{ return mController; }
	const AAX_IController*				Controller() const				{ return mController; }
	AAX_IHostProcessorDelegate*			HostProcessorDelegate()			{ return mHostProcessingDelegate; }
	const AAX_IHostProcessorDelegate*	HostProcessorDelegate() const	{ return mHostProcessingDelegate; }
	AAX_IEffectParameters*				EffectParameters()				{ return mEffectParameters; }
	const AAX_IEffectParameters*		EffectParameters() const		{ return mEffectParameters; }
	//@}end Convenience methods

private:
	AAX_IController*				mController;
	AAX_IHostProcessorDelegate*		mHostProcessingDelegate;
	AAX_IEffectParameters*			mEffectParameters;
	int64_t							mSrcStart;
	int64_t							mSrcEnd;
	int64_t							mDstStart;
	int64_t							mDstEnd;	
	int64_t							mLocation; 
	
};


#endif
