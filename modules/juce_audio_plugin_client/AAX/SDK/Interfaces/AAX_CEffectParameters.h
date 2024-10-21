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
 *	\file AAX_CEffectParameters.h
 *
 *	\brief A default implementation of the AAX_IeffectParameters interface.
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CEFFECTPARAMETERS_H
#define AAX_CEFFECTPARAMETERS_H

#include "AAX_IEffectParameters.h"
#include "AAX_IPageTable.h"
#include "AAX_CString.h"
#include "AAX_CChunkDataParser.h"
#include "AAX_CParameterManager.h"
#include "AAX_CPacketDispatcher.h"

#include <set>
#include <string>
#include <vector>

class AAX_IController;
class AAX_IAutomationDelegate;
class AAX_CParameterManager;
class AAX_CPacketDispatcher;
class AAX_ITransport;

extern "C" AAX_CParamID	cPreviewID;
extern "C" AAX_CParamID	cDefaultMasterBypassID;

/** @brief Default implementation of the \ref AAX_IEffectParameters interface.
	
	@details
	This class provides a default implementation of the AAX_IEffectParameters interface.
	In nearly all cases, your plug-in's data model should inherit from this class and 
	override only those functions that you wish to explicitly customize.
	
	\legacy The default implementations in this class are mostly derived from their
	equivalent implementations in CProcess and CEffectProcess.  For additional
	CProcess-derived implementations, see \ref AAX_CEffectGUI.
	
	\section AAX_CEffectParameters_relclass Related classes
		\dotfile aax_ieffectparams_related.dot "Classes related to AAX_IEffectParameters by inheritance or composition"
		\dotfile aax_ieffectparams_contained.dot "Classes owned as member objects of AAX_CEffectParameters"
		
	\ingroup CommonInterface_DataModel
*/
class AAX_CEffectParameters : public AAX_IEffectParameters
{
public: ///////////////////////////////////////////////////////////////////////////// constructor/destructor
	AAX_CEffectParameters (void);
	~AAX_CEffectParameters (void) AAX_OVERRIDE;
	AAX_CEffectParameters& operator= (const AAX_CEffectParameters& other);
	
public: ///////////////////////////////////////////////////////////////////////////// AAX_IEffectParameters
	/** @name Initialization and uninitialization
	 */
	//@{
	/**	\copydoc AAX_IACFEffectParameters::Initialize()
		
		This default implementation calls \ref AAX_CEffectParameters::EffectInit() "EffectInit()".
		Only override \ref Initialize() when additional initialization steps must be performed
		prior to \ref AAX_CEffectParameters::EffectInit() "EffectInit()".
	 */
	AAX_Result Initialize(IACFUnknown* iController) AAX_OVERRIDE;
	AAX_Result Uninitialize (void) AAX_OVERRIDE;
	//@}end Initialization and uninitialization
	
	/** @name %AAX host and plug-in event notification
	 */
	//@{
	AAX_Result NotificationReceived( /* AAX_ENotificationEvent */ AAX_CTypeID inNotificationType, const void * inNotificationData, uint32_t	inNotificationDataSize) AAX_OVERRIDE;
	//@}end %AAX host and plug-in event notification
	
	/** @name Parameter information
	 *
	 *	These methods are used by the %AAX host to retrieve information about the plug-in's data
	 *	model.  For information about adding parameters to the plug-in and otherwise modifying
	 *	the plug-in's data model, see AAX_CParameterManager.  For information about parameters,
	 *	see AAX_IParameter.
	 */
	//@{
	AAX_Result GetNumberOfParameters (int32_t * oNumControls)  const AAX_OVERRIDE;
	AAX_Result GetMasterBypassParameter (AAX_IString * oIDString)  const AAX_OVERRIDE;
	AAX_Result GetParameterIsAutomatable (AAX_CParamID iParameterID, AAX_CBoolean * oAutomatable)  const AAX_OVERRIDE;
	AAX_Result GetParameterNumberOfSteps (AAX_CParamID iParameterID, int32_t * oNumSteps )  const AAX_OVERRIDE;
	AAX_Result GetParameterName (AAX_CParamID iParameterID, AAX_IString * oName )  const AAX_OVERRIDE;
	AAX_Result GetParameterNameOfLength (AAX_CParamID iParameterID, AAX_IString * oName, int32_t iNameLength )  const AAX_OVERRIDE;
	AAX_Result GetParameterDefaultNormalizedValue (AAX_CParamID iParameterID, double * oValue )  const AAX_OVERRIDE;
	AAX_Result SetParameterDefaultNormalizedValue (AAX_CParamID iParameterID, double iValue ) AAX_OVERRIDE;
	AAX_Result GetParameterType (AAX_CParamID iParameterID, AAX_EParameterType * oParameterType )  const AAX_OVERRIDE;
	AAX_Result GetParameterOrientation (AAX_CParamID iParameterID, AAX_EParameterOrientation * oParameterOrientation )  const AAX_OVERRIDE;
	AAX_Result GetParameter (AAX_CParamID iParameterID, AAX_IParameter ** oParameter ) AAX_OVERRIDE;
	AAX_Result GetParameterIndex (AAX_CParamID iParameterID, int32_t * oControlIndex )  const AAX_OVERRIDE;
	AAX_Result GetParameterIDFromIndex (int32_t iControlIndex, AAX_IString * oParameterIDString )  const AAX_OVERRIDE;
	AAX_Result GetParameterValueInfo ( AAX_CParamID iParameterID, int32_t iSelector, int32_t* oValue) const AAX_OVERRIDE;
	//@}end Parameter information
	
	/** @name Parameter setters and getters
	 *
	 *	These methods are used by the %AAX host and by the plug-in's UI to retrieve and modify
	 *	the values of the plug-in's parameters.
	 *
	 *	\note The parameter setters in this section may generate asynchronous requests.
	 */
	//@{
	AAX_Result GetParameterValueFromString (AAX_CParamID iParameterID, double * oValue, const AAX_IString & iValueString )  const AAX_OVERRIDE;
	AAX_Result GetParameterStringFromValue (AAX_CParamID iParameterID, double iValue, AAX_IString * oValueString, int32_t iMaxLength )  const AAX_OVERRIDE;
	AAX_Result GetParameterValueString (AAX_CParamID iParameterID, AAX_IString * oValueString, int32_t iMaxLength)  const AAX_OVERRIDE;
	AAX_Result GetParameterNormalizedValue (AAX_CParamID iParameterID, double * oValuePtr )  const AAX_OVERRIDE;
	AAX_Result SetParameterNormalizedValue (AAX_CParamID iParameterID, double iValue ) AAX_OVERRIDE;
	AAX_Result SetParameterNormalizedRelative (AAX_CParamID iParameterID, double iValue ) AAX_OVERRIDE;
	//@}end Parameter setters and getters
	
	/** @name Automated parameter helpers
	 *
	 *	These methods are used to lock and unlock automation system 'resources' when
	 *	updating automatable parameters.
	 *
	 *	\note You should never need to override these methods to extend their
	 *	behavior beyond what is provided in AAX_CEffectParameters and AAX_IParameter
	 *
	 */
	//@{
	AAX_Result TouchParameter ( AAX_CParamID iParameterID ) AAX_OVERRIDE;
	AAX_Result ReleaseParameter ( AAX_CParamID iParameterID ) AAX_OVERRIDE;
	AAX_Result UpdateParameterTouch ( AAX_CParamID iParameterID, AAX_CBoolean iTouchState ) AAX_OVERRIDE;
	//@}end Automated parameter helpers
		
	/** @name Asynchronous parameter update methods
	 *
	 *	These methods are called by the %AAX host when parameter values have been updated.  They are
	 *	called by the host and can be triggered by other plug-in modules via calls to
	 *	\ref AAX_IParameter's \c SetValue methods, e.g.
	 *	\ref AAX_IParameter::SetValueWithFloat() "SetValueWithFloat()"
	 *
	 *	These methods are responsible for updating parameter values.
	 *
	 *	Do not call these methods directly! To ensure proper
	 *	synchronization and to avoid problematic dependency chains, other methods (e.g.
	 *	\ref SetParameterNormalizedValue()) and components (e.g. \ref AAX_IEffectGUI) should always
	 *	call a \c SetValue method on \ref AAX_IParameter to update parameter values. The \c SetValue
	 *	method will properly manage automation locks and other system resources.
	 *	
	 */
	//@{
	AAX_Result UpdateParameterNormalizedValue (AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource ) AAX_OVERRIDE;
	AAX_Result UpdateParameterNormalizedRelative (AAX_CParamID iParameterID, double iValue ) AAX_OVERRIDE;
	AAX_Result GenerateCoefficients(void) AAX_OVERRIDE;
	//@}end Asynchronous parameter update methods

	/** @name State reset handlers
	 */
	//@{
	AAX_Result ResetFieldData (AAX_CFieldIndex inFieldIndex, void * oData, uint32_t inDataSize) const AAX_OVERRIDE;
	//@}end State reset handlers

	/** @name Chunk methods
	 *	
	 *	These methods are used to save and restore collections of plug-in state information, known
	 *	as chunks.  Chunks are used by the host when saving or restoring presets and session
	 *	settings and when providing "compare" functionality for plug-ins.
	 *
	 *	The default implementation of these methods in \ref AAX_CEffectParameters supports a single
	 *	chunk that includes state information for all of the plug-in's registered parameters.
	 *	Override all of these methods to add support for additional chunks in your plug-in, for
	 *	example if your plug-ni contains any persistent state that is not encapsulated by its set
	 *	of registered parameters.
	 *
	 *	For reference, see also:
	 *		\li AAX_CChunkDataParser
	 *		\li AAX_SPlugInChunk
	 *
	 */
	//@{
	AAX_Result GetNumberOfChunks (int32_t * oNumChunks )  const AAX_OVERRIDE;
	AAX_Result GetChunkIDFromIndex (int32_t iIndex, AAX_CTypeID * oChunkID )  const AAX_OVERRIDE;
	AAX_Result GetChunkSize (AAX_CTypeID iChunkID, uint32_t * oSize )  const AAX_OVERRIDE;
	AAX_Result GetChunk (AAX_CTypeID iChunkID, AAX_SPlugInChunk * oChunk ) const AAX_OVERRIDE;
	AAX_Result SetChunk (AAX_CTypeID iChunkID, const AAX_SPlugInChunk * iChunk ) AAX_OVERRIDE;
	AAX_Result CompareActiveChunk (const AAX_SPlugInChunk * iChunkP, AAX_CBoolean * oIsEqual )  const AAX_OVERRIDE;
	AAX_Result GetNumberOfChanges (int32_t * oNumChanges )  const AAX_OVERRIDE;
	//@}end Chunk methods
	
	/** @name Threads
	 *	Threading functions
	 */
	//@{
	AAX_Result TimerWakeup() AAX_OVERRIDE;
	//@}end Threads
	
	/** @name Auxiliary UI methods
	 *	Methods defining the presentation of the plug-in on auxiliary UIs such as control surfaces
	 */
	//@{	
	AAX_Result GetCurveData( /* AAX_ECurveType */ AAX_CTypeID iCurveType, const float * iValues, uint32_t iNumValues, float * oValues ) const AAX_OVERRIDE;
	AAX_Result GetCurveDataMeterIds( /* AAX_ECurveType */ AAX_CTypeID iCurveType, uint32_t *oXMeterId, uint32_t *oYMeterId)  const AAX_OVERRIDE;
	AAX_Result GetCurveDataDisplayRange( /* AAX_ECurveType */ AAX_CTypeID iCurveType, float *oXMin, float *oXMax, float *oYMin, float *oYMax ) const AAX_OVERRIDE;
	
	/** @copydoc AAX_IEffectParameters::UpdatePageTable()
	 
	 \note For convenience, do not override this method. Instead, override the
	 \ref AAX_CEffectParameters::UpdatePageTable(uint32_t, int32_t, AAX_IPageTable&) const "protected overload"
	 which provides a prepared copy of the relevant \ref AAX_IPageTable host interface.
	 */
	AAX_Result UpdatePageTable(uint32_t inTableType, int32_t inTablePageSize, IACFUnknown* iHostUnknown, IACFUnknown* ioPageTableUnknown) const AAX_OVERRIDE AAX_FINAL;
	//@}end Auxiliary UI methods
	
	/** @name Custom Data Methods
	 *
	 *  These functions exist as a proxiable way to move data between different modules (e.g. AAX_IEffectParameters and AAX_IEffectGUI.)  
	 *  Using these, the GUI can query any data through GetCustomData() with a plug-in defined \c typeID, \c void* and size.  This has an advantage
	 *  over just sharing memory in that this function can work as a remote proxy as we enable those sorts of features later in the platform.
	 *  Likewise, the GUI can also set arbitrary data on the data model by using the SetCustomData() function with the same idea.
	 *
	 *  \note These are plug-in internal only.  They are not called from the host right now, or likely ever.
	 */
	//@{
	AAX_Result			GetCustomData( AAX_CTypeID iDataBlockID, uint32_t inDataSize, void* oData, uint32_t* oDataWritten) const AAX_OVERRIDE;
	AAX_Result			SetCustomData( AAX_CTypeID iDataBlockID, uint32_t inDataSize, const void* iData ) AAX_OVERRIDE;
	//@}end Custom Data Methods
	
	/** @name MIDI methods
	 *
	 */
	//@{
	AAX_Result			DoMIDITransfers() AAX_OVERRIDE { return AAX_SUCCESS; }
	AAX_Result			UpdateMIDINodes ( AAX_CFieldIndex inFieldIndex, AAX_CMidiPacket& iPacket ) AAX_OVERRIDE;
	AAX_Result			UpdateControlMIDINodes ( AAX_CTypeID nodeID, AAX_CMidiPacket& iPacket ) AAX_OVERRIDE;
	//@}end MIDI methods

    /** @name Hybrid audio methods
	 *
	 */
	//@{
    AAX_Result          RenderAudio_Hybrid(AAX_SHybridRenderInfo* ioRenderInfo) AAX_OVERRIDE;
	//@}end Hybrid audio methods



public: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectParameters	
	/**	@name Private data accessors
	 *
	 */
	//@{
	AAX_IController*				Controller(); //!< Access to the Effect controller
	const AAX_IController*			Controller() const; //!< \c const access to the Effect controller
	AAX_ITransport*					Transport(); //!< Access to the Transport object
	const AAX_ITransport*			Transport() const; //!< \c const access to the Transport object
    AAX_IAutomationDelegate*        AutomationDelegate(); //!< Access to the Effect's automation delegate
    const AAX_IAutomationDelegate*  AutomationDelegate() const; //!< \c const access to the Effect's automation delegate
	//@}end Private data accessors
	
protected:
	/** @name Parameter management methods
	 *
	 */
	//@{
	AAX_Result			SetTaperDelegate ( AAX_CParamID iParameterID, AAX_ITaperDelegateBase & iTaperDelegate, bool iPreserveValue );
	AAX_Result			SetDisplayDelegate ( AAX_CParamID iParameterID, AAX_IDisplayDelegateBase & iDisplayDelegate );
	bool				IsParameterTouched ( AAX_CParamID iParameterID ) const;
	bool				IsParameterLinkReady ( AAX_CParamID inParameterID, AAX_EUpdateSource inSource ) const;
	//@}end Parameter management methods
	
	/** @name Convenience functions
	 *
	 *  These convenience functions provide quick access to various aspects of the
	 *	default AAX_CEffectParameters implementation.
	 */
	//@{
	/*!	\brief Initialization helper routine.  Called from AAX_CEffectParameters::Initialize.
	 *   
	 *	Override to add parameters, packets, meters, and to do any other custom initialization.
	 *
	 *	Add custom parameters:
	 *	\li Create an AAX_CParameter for each parameter in the plug-in
	 *	\li Call AAX_CParameterManager::AddParameter() using mParameterManager to add parameters
	 *		to the Parameter Manager
	 *
	 *	\note See bug \ref AAXSDK-897
	 *
	 *	Register packets:
	 *	\li Call AAX_CPacketDispatcher::RegisterPacket() using mPacketDispatcher to register a
	 *		packet and handling callback.
	 */
	virtual AAX_Result	EffectInit(void) { return AAX_SUCCESS; };
	
	/*! Protected overload of \ref AAX_CEffectParameters::UpdatePageTable(uint32_t, int32_t, IACFUnknown*, IACFUnknown*) const "UpdatePageTable()"
	 
	 Override this version of the method for convenience. This allows the default
	 \ref AAX_CEffectParameters::UpdatePageTable(uint32_t, int32_t, IACFUnknown*, IACFUnknown*) const "UpdatePageTable()"
	 implementation to handle the interface conversion from \ref IACFUnknown to \ref AAX_IPageTable.
	 
	 \return This method should return \ref AAX_ERROR_UNIMPLEMENTED if the plug-in does
	 not implement it or when no change is made by the plug-in. This allows
	 optimizations to be used in the host when no UI update is required following this
	 call.
	 
	 */
	virtual AAX_Result UpdatePageTable(uint32_t /*inTableType*/, int32_t /*inTablePageSize*/, AAX_IPageTable& /*ioPageTable*/) const { return AAX_ERROR_UNIMPLEMENTED; }

	/*!	\brief CALL: Indicates the indices of parameters that should not be saved in the default
	 *	AAX_CEffectParameters chunk
	 *  
	 *	Allows specific parameters to filtered out of the default AAX_CEffectParameters "Save
	 *	Settings" functionality. This call is automatically invoked on the Master Bypass
	 *	control when specified by the DefineMasterBypassControlIndex() call.
	 *
	 *	\param[in] controlID
	 *		The ID of the parameter that should be removed from the default chunk
	 */
	void FilterParameterIDOnSave(AAX_CParamID controlID);
	//@} Convenience functions

    /*! \brief Clears out the current chunk in Chunk Parser and adds all of the new values.  Used by default implementations of GetChunk() and GetChunkSize().
     *
     */
	void BuildChunkData (void) const;
	
protected:
	int32_t							mNumPlugInChanges;	
    mutable int32_t					mChunkSize;				//this old behavior isn't const friendly yet.  Consider this a temp variable.			
	mutable AAX_CChunkDataParser	mChunkParser;           //this old behavior isn't const friendly yet.  Consider this a temp variable.
    int32_t							mNumChunkedParameters;
	AAX_CPacketDispatcher			mPacketDispatcher;	
	AAX_CParameterManager			mParameterManager;	
	std::set<std::string>			mFilteredParameters;	

private:
	// interfaces provided by the host via the IACFUnknown passed to Initialize()
	AAX_IController*				mController; ///< \sa AAX_CEffectParameters::Controller()
	AAX_ITransport*					mTransport; ///< \sa AAX_CEffectParameters::Transport()
	AAX_IAutomationDelegate*		mAutomationDelegate; ///< \sa AAX_CEffectParameters::AutomationDelegate()

};

// Convenience functions since many legacy plug-ins had internal int32 value representations.
extern int32_t NormalizedToInt32 (double normalizedValue );
extern double Int32ToNormalized (int32_t value );
extern double BoolToNormalized (bool value );

#endif
