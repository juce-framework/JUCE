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
 *	\file  AAX_IACFEffectParameters.h
 *
 *	\brief The data model interface that is exposed to the host application
 *
 */ 
/*================================================================================================*/
 

#ifndef AAX_IACFEFFECTPARAMETERS_H
#define AAX_IACFEFFECTPARAMETERS_H

#include "AAX.h"

class AAX_IString;
class AAX_IParameter;


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/** @brief	The interface for an %AAX Plug-in's data model
	
	@details
	This is the interface for an instance of a plug-in's data model that gets exposed 
	to the host application.  The %AAX host interacts with your plug-in's data model
	via this interface, which includes methods that store and update of your plug-in's
	internal data. See \ref CommonInterface_DataModel.
	
	\note Your implementation of this interface must inherit from
	\ref AAX_IEffectParameters.

	\todo Add documentation for expected error state return values
		
	\ingroup CommonInterface_DataModel
*/
class AAX_IACFEffectParameters : public IACFUnknown
{
public:
	
	/** @name Initialization and uninitialization
	 */
	//@{
	/*!
	 *  \brief Main data model initialization. Called when plug-in instance is first instantiated.
	 *	
	 *	\note Most plug-ins should override \ref AAX_CEffectParameters::EffectInit() rather than directly overriding this method
	 *	
	 *  \param[in] iController
	 *		A versioned reference that resolves to an AAX_IController interface
	 */	
	virtual AAX_Result			Initialize(IACFUnknown * iController) = 0;
	/*!
	 *  \brief Main data model uninitialization
	 *
	 *	\todo Docs: When exactly is AAX_IACFEffectParameters::Uninitialize() called, and under what conditions?
	 *
	 */
	virtual AAX_Result			Uninitialize () = 0;
	//@}end Initialization and uninitialization
	

	/** @name %AAX host and plug-in event notification
	 */
	//@{
	/*!
	 *	\brief Notification Hook 
	 *
	 *	Called from the host to deliver notifications to this object.
	 *
	 *	Look at the \ref AAX_ENotificationEvent enumeration to see a description of events you can listen for and the
	 *	data they come with.
	 *
	 *	- \note some notifications are sent only to the plug-in GUI while other notifications are sent only to the
	 *	  plug-in data model. If you are not seeing an expected notification, try checking the other plug-in objects'
	 *	  \c NotificationReceived() methods.
	 *	- \note the host may dispatch notifications synchronously or asynchronously, and calls to this method may
	 *	  occur concurrently on multiple threads.
	 *
	 *	A plug-in may also dispatch custom notifications using \ref AAX_IController::SendNotification(). Custom
	 *	notifications will be posted back to the plug-in's other objects which support a \c NotificationReceived()
	 *	method (e.g. the GUI).
	 *
	 *	\param[in] inNotificationType
	 *		Type of notification being received. Notifications form the host are one of \ref AAX_ENotificationEvent
	 *	\param[in] inNotificationData
	 *		Block of incoming notification data
	 *	\param[in] inNotificationDataSize
	 *		Size of \p inNotificationData, in bytes
	 */
	virtual	AAX_Result			NotificationReceived( /* AAX_ENotificationEvent */ AAX_CTypeID inNotificationType, const void * inNotificationData, uint32_t	inNotificationDataSize) = 0;
	//@}end %AAX host and plug-in event notification
	
	
	/** @name Parameter information
	 *
	 *	These methods are used by the %AAX host to retrieve information about the plug-in's data
	 *	model.
	 *	
	 *	\n \n
	 *	
	 *	For information about adding parameters to the plug-in and otherwise modifying
	 *	the plug-in's data model, see AAX_CParameterManager.  For information about parameters,
	 *	see AAX_IParameter.
	 */
	//@{
	/*!
	 *  \brief CALL: Retrieves the total number of plug-in parameters
	 *
	 *	\param[out] oNumControls
	 *		The number of parameters in the plug-in's Parameter Manager
	 */
	virtual AAX_Result			GetNumberOfParameters ( int32_t * oNumControls )   const = 0;
	/*!
	 *  \brief CALL: Retrieves the ID of the plug-in's Master Bypass parameter.
	 *
	 *	This is required if you want our master bypass functionality in the host to hook up to your bypass parameters.
	 *
	 *	\param[out] oIDString
	 *		The ID of the plug-in's Master Bypass control
	 */
	virtual AAX_Result			GetMasterBypassParameter ( AAX_IString * oIDString )   const = 0;
	/*!
	 *  \brief CALL: Retrieves information about a parameter's automatable status
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oAutomatable
	 *		True if the queried parameter is automatable, false if it is not
	 *
	 */
	virtual AAX_Result			GetParameterIsAutomatable ( AAX_CParamID iParameterID, AAX_CBoolean * oAutomatable )   const = 0;
	/*!
	 *  \brief CALL: Retrieves the number of discrete steps for a parameter
	 *
     *  \note The value returned for \p oNumSteps MUST be greater than zero.  All other values
     *  will be considered an error by the host.
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oNumSteps
	 *		The number of steps for this parameter
	 *
	 */
	virtual AAX_Result			GetParameterNumberOfSteps ( AAX_CParamID iParameterID, int32_t * oNumSteps )   const = 0;
	/*!
	 *  \brief CALL: Retrieves the full name for a parameter
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oName
	 *		Reference to an \ref AAX_IString owned by the host. The plug-in must set this string equal to
	 *		the parameter's full name.
	 *
	 */
	virtual AAX_Result			GetParameterName ( AAX_CParamID iParameterID, AAX_IString * oName )   const = 0;
	/*!
	 *  \brief CALL: Retrieves an abbreviated name for a parameter
	 *
	 *	In general, lengths of 3 through 8 and 31 should be specifically addressed.
	 *
     *  \compatibility In most cases, the %AAX host will call
	 *  \ref AAX_IACFEffectParameters::GetParameterName() "GetParameterName()" or
	 *  \ref AAX_IACFEffectParameters::GetParameterNameOfLength() "GetParameterNameOfLength()" to retrieve
	 *  parameter names for display.  However, when Pro Tools is retrieving a plug-in name for display on
     *  a control surface the XML data stored in the plug-in's page tables will be used in preference to
     *  values retrieved from these methods.
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oName
	 *		Reference to an \ref AAX_IString owned by the host. The plug-in must set this string equal to
	 *		an abbreviated name for the parameter, using \c iNameLength characters or fewer.
	 *	\param[in] iNameLength
	 *		The maximum number of characters in \c oName
	 *
	 */
	virtual AAX_Result			GetParameterNameOfLength ( AAX_CParamID iParameterID, AAX_IString * oName, int32_t iNameLength )   const = 0;
	/*!
	 *  \brief CALL: Retrieves default value of a parameter
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oValue
	 *		The parameter's default value
	 *
	 */
	virtual AAX_Result			GetParameterDefaultNormalizedValue ( AAX_CParamID iParameterID, double * oValue )   const = 0;
	/*!
	 *  \brief CALL: Sets the default value of a parameter
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being updated
	 *	\param[out] iValue
	 *		The parameter's new default value
	 *
	 *  \todo THIS IS NOT CALLED FROM HOST.  USEFUL FOR INTERNAL USE ONLY?
	 *
	 */
	virtual AAX_Result			SetParameterDefaultNormalizedValue ( AAX_CParamID iParameterID, double iValue ) = 0;
	/*!
	 *  \brief CALL: Retrieves the type of a parameter
	 *
	 *	\todo The concept of parameter type needs more documentation
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oParameterType
	 *		The parameter's type
	 *
	 */
	virtual AAX_Result			GetParameterType ( AAX_CParamID iParameterID, AAX_EParameterType * oParameterType )   const = 0;
	/*!
	 *  \brief CALL: Retrieves the orientation that should be applied to a parameter's controls
	 *  
	 *	\todo update this documentation
	 *
	 *	This method allows you to specify the orientation of knob controls that are managed
     *  by the host (e.g. knobs on an attached control surface.)
     *
     *  Here is an example override of this method that reverses the orientation of a
     *  control for a parameter:
	 
		\code
		 	// AAX_IParameter* myBackwardsParameter
            if (iParameterID == myBackwardsParameter->Identifier())
		 	{
				*oParameterType =
				   AAX_eParameterOrientation_BottomMinTopMax |
				   AAX_eParameterOrientation_LeftMinRightMax |
				   AAX_eParameterOrientation_RotaryWrapMode |
				   AAX_eParameterOrientation_RotaryLeftMinRightMax;
			 }
		\endcode

	 *	The orientation options are set according to \ref AAX_EParameterOrientationBits
     *
     *  \legacy \ref AAX_IEffectParameters::GetParameterOrientation() corresponds to the
     *  GetControlOrientation() method in the legacy RTAS/TDM SDK.
	 *	
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oParameterOrientation
	 *		The orientation of the parameter
	 *
	 */
	virtual AAX_Result			GetParameterOrientation ( AAX_CParamID iParameterID, AAX_EParameterOrientation * oParameterOrientation )   const = 0;
	/*!
	 *  \brief CALL: Retrieves an arbitrary setting within a parameter.
	 *
	 *	This is a convenience function for accessing the richer parameter interface
	 *	from the plug-in's other modules.
	 *
	 *	\note This function must not be called by the host; \ref AAX_IParameter is not
	 *	safe for passing across the binary boundary with the host!
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oParameter
	 *		A pointer to the returned parameter
	 *
	 */
	virtual AAX_Result			GetParameter ( AAX_CParamID iParameterID, AAX_IParameter ** oParameter )  = 0;
	/*!
	 *  \brief CALL: Retrieves the index of a parameter
	 *
	 *	Although parameters are normally referenced by their AAX_CParamID, each parameter
	 *	is also associated with a unique numeric index.
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oControlIndex
	 *		The parameter's numeric index
	 *
	 */
	virtual AAX_Result			GetParameterIndex ( AAX_CParamID iParameterID, int32_t * oControlIndex )   const = 0;
	/*!
	 *  \brief CALL: Retrieves the ID of a parameter
	 *
	 *	This method can be used to convert a parameter's unique numeric index to its AAX_CParamID
	 *
	 *	\param[in] iControlIndex
	 *		The numeric index of the parameter that is being queried
	 *	\param[out] oParameterIDString
	 *		Reference to an \ref AAX_IString owned by the host. The plug-in must set this string equal to
	 *		the parameter's ID.
	 *
	 */
	virtual AAX_Result			GetParameterIDFromIndex ( int32_t iControlIndex, AAX_IString * oParameterIDString )   const = 0;
	/*!
	 *  \brief CALL: Retrieves a property of a parameter
	 *
     *  This is a general purpose query that is specialized based on the value of \p iSelector.
     *  The currently supported selector values are described by
     *  \ref AAX_EParameterValueInfoSelector .  The meaning of \p oValue is dependent upon
     *  \p iSelector .
     *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[in] iSelector
	 *		The selector of the parameter value to retrieve. See \ref AAX_EParameterValueInfoSelector
	 *	\param[out] oValue
	 *		The value of the specified parameter
	 */
	virtual AAX_Result			GetParameterValueInfo ( AAX_CParamID iParameterID, int32_t iSelector, int32_t* oValue) const = 0;
	//@}end Parameter information
	
	
	
	/** @name Parameter setters and getters
	 *
	 *	These methods are used by the %AAX host and by the plug-in's UI to retrieve and modify
	 *	the values of the plug-in's parameters.
	 *
	 *	\note The parameter setters in this section may generate asynchronous requests.
	 */
	//@{
	/*!
	 *  \brief CALL: Converts a value string to a value
	 *
	 *	This method uses the queried parameter's display delegate and taper to convert a \c char*
	 *  string into its corresponding value.  The formatting of valueString must be
	 *	supported by the parameter's display delegate in order for this call to succeed.
	 *  
	 *	\legacy This method corresponds to CProcess::MapControlStringToVal() in the RTAS/TDM SDK
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oValue
	 *		The value associated with valueString
	 *	\param[in] iValueString
	 *		The formatted value string that will be converted into a value
	 *
	 */
	virtual AAX_Result			GetParameterValueFromString ( AAX_CParamID iParameterID, double * oValue, const AAX_IString & iValueString )   const = 0;
	/*!
	 *  \brief CALL: Converts a normalized parameter value into a string representing its
	 *	corresponding real value
	 *
	 *	This method uses the queried parameter's display delegate and taper to convert a
	 *	normalized value into the corresponding \c char* value string for its real value.
	 *  
	 *	\legacy This method corresponds to CProcess::MapControlValToString() in the RTAS/TDM SDK
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[in] iValue
	 *		The normalized value that will be converted to a formatted valueString
	 *	\param[out] oValueString
	 *		The formatted value string associated with value
	 *	\param[in] iMaxLength
	 *		The maximum length of valueString
	 *
	 */
	virtual	AAX_Result			GetParameterStringFromValue ( AAX_CParamID iParameterID, double iValue, AAX_IString * oValueString, int32_t iMaxLength )   const = 0;
	/*!
	 *  \brief CALL: Retrieves the value string associated with a parameter's current value
	 *
	 *	This method uses the queried parameter's display delegate and taper to convert
	 *	its current value into a corresponding \c char* value string.
	 *  
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oValueString
	 *		The formatted value string associated with the parameter's current value
	 *	\param[in] iMaxLength
	 *		The maximum length of valueString
	 *
	 */
	virtual AAX_Result			GetParameterValueString ( AAX_CParamID iParameterID, AAX_IString* oValueString, int32_t iMaxLength  )   const = 0;
	/*!
	 *  \brief CALL: Retrieves a parameter's current value
	 *  
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[out] oValuePtr
	 *		The parameter's current value
	 *
	 */
	virtual AAX_Result			GetParameterNormalizedValue ( AAX_CParamID iParameterID, double * oValuePtr )   const = 0;
	/*!
	 *  \brief CALL: Sets the specified parameter to a new value
	 *
	 *	SetParameterNormalizedValue() is responsible for initiating any process that is required in order to update
	 *	all of the parameter's controls (e.g. in the plug-in's GUI, on control surfaces, in
	 *	automation lanes, etc.)  In most cases, the parameter manager will handle this initiation
	 *	step.
	 *	
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being set
	 *	\param[in] iValue
	 *		The value to which the parameter should be set
	 *
	 */
	virtual AAX_Result			SetParameterNormalizedValue ( AAX_CParamID iParameterID, double iValue ) = 0;	
	/*!
	 *  \brief CALL: Sets the specified parameter to a new value relative to its current value
	 *
	 *	This method is used in cases when a relative control value is more convenient, for example
	 *	when updating a GUI control using a mouse wheel or the arrow keys.  Note that the host may
	 *	apply the parameter's step size prior to calling SetParameterNormalizedRelative() in order to
	 *	determine the correct value for aValue.
	 *
	 *	SetParameterNormalizedRelative() can be used to incorporate "wrapping" behavior in a parameter's controls, if
	 *	desired.  If this behavior is not desired, then this method must properly account for
	 *	overflow of the parameter's normalized value.
	 *
	 *	SetParameterNormalizedRelative() is responsible for initiating any process that is required in order to update
	 *	all of the parameter's controls (e.g. in the plug-in's GUI, on control surfaces, in
	 *	automation lanes, etc.)  In most cases, the parameter manager will handle this initiation
	 *	step.
	 *
	 *	See also UpdateParameterNormalizedRelative().
	 *
	 *	\todo REMOVE THIS METHOD (?)
	 *	
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being queried
	 *	\param[in] iValue
	 *		The change in value that should be applied to the parameter
	 *
	 *  \todo NOT CURRENTLY CALLED FROM THE HOST. USEFUL FOR INTERNAL USE ONLY?
	 *
	 */
	virtual AAX_Result			SetParameterNormalizedRelative ( AAX_CParamID iParameterID, double iValue ) = 0;
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
	/*!
	 *  \brief "Touches" (locks) a parameter in the automation system to a particular
	 *	control in preparation for updates
	 *
	 *	This method is called by the Parameter Manager to prime a parameter for receiving
	 *	new automation data.  When an automatable parameter is touched by a control,
	 *	it will reject input from other controls until it is released.
	 *
	 *	\note You should never need to override this method when using
	 *	AAX_CEffectParameters.
	 *	
	 *	\param[in] iParameterID
	 *		The parameter that is being touched
	 *
	 */
	virtual AAX_Result			TouchParameter ( AAX_CParamID iParameterID ) = 0;
	/*!
	 *  \brief Releases a parameter from a "touched" state
	 *
	 *	This method is called by the Parameter Manager to release a parameter so that
	 *	any control may send updates to the parameter.
	 *
	 *	\note You should never need to override this method when using
	 *	\ref AAX_CEffectParameters.
	 *	
	 *	\param[in] iParameterID
	 *		The parameter that is being released
	 *
	 */
	virtual AAX_Result			ReleaseParameter ( AAX_CParamID iParameterID ) = 0;
	/*!
	 *  \brief Sets a "touched" state on a parameter
	 *
	 *	\note This method should be overriden when dealing with linked parameters.  Do NOT use this
     *  method to keep track of touch states.  Use the 
	 *  \ref AAX_IACFAutomationDelegate "automation delegate" for that.
	 *	
	 *	\param[in] iParameterID
	 *		The parameter that is changing touch states.
	 *	\param[in] iTouchState
	 *		The touch state of the parameter.
	 *
	 */
	virtual AAX_Result			UpdateParameterTouch ( AAX_CParamID iParameterID, AAX_CBoolean iTouchState ) = 0;
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
	/*!
	 *  \brief Updates a single parameter's state to its current value
	 *
	 *	\note Do \em not call this method from the plug-in.  This method should be called by the
	 *	host only.  To set parameter values from within the plug-in, use the \ref AAX_IParameter interface.
	 *
	 *	\todo FLAGGED FOR CONSIDERATION OF REVISION
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being updated
	 *	\param[in] iValue
	 *		The parameter's current value, to which its internal state must be updated
	 *	\param[in] iSource
	 *		The source of the update
	 *
	 */	
	virtual AAX_Result			UpdateParameterNormalizedValue ( AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource ) = 0; 
	/*!
	 *  \brief Updates a single parameter's state to its current value, as a difference
	 *	with the parameter's previous value
	 *
	 *	\deprecated This is not called from the host. It <EM>may</EM> still be useful for internal
	 *	calls within the plug-in, though it should only ever be used to update non-automatable parameters.
	 *	Automatable parameters should always be updated through the \ref AAX_IParameter interface, which
	 *	will ensure proper coordination with other automation clients.
	 *
	 *	UpdateParameterNormalizedRelative() can be used to incorporate "wraparound" behavior in a parameter's
	 *	controls, if desired.  If this behavior is not desired, then this method must properly
	 *	account for overflow of the parameter's normalized value.
	 *
	 *	\sa \ref SetParameterNormalizedRelative()
	 *
	 *	\param[in] iParameterID
	 *		The ID of the parameter that is being updated
	 *	\param[in] iValue
	 *		The difference between the parameter's current value and its previous
	 *		value (normalized).  The parameter's state must be updated to reflect this
	 *		difference.
	 */	
	virtual AAX_Result			UpdateParameterNormalizedRelative ( AAX_CParamID iParameterID, double iValue ) = 0;
	/*!
	 *  \brief Generates and dispatches new coefficient packets.
	 *
	 *	This method is responsible for updating the coefficient packets associated with all
	 *	parameters whose states have changed since the last call to
	 *	\ref AAX_IACFEffectParameters::GenerateCoefficients() "GenerateCoefficients()".  The
	 *	host may call this method once for every parameter update, or it may "batch" parameter
	 *	updates such that changes for several parameters are all handled by a single call to
	 *	\ref AAX_IACFEffectParameters::GenerateCoefficients() "GenerateCoefficients()".
	 *	
	 *	For more information on tracking parameters' statuses using the \ref AAX_CPacketDispatcher,
	 *	helper class, see \ref AAX_CPacketDispatcher::SetDirty().
	 *
	 *	\note Do \em not call this method from the plug-in.  This method should be called by the
	 *	host only.  To set parameter values from within the plug-in, use the \ref AAX_IParameter interface.
	 *
	 */
	virtual AAX_Result			GenerateCoefficients() = 0; 
	//@}end Asynchronous parameter update methods


	/** @name State reset handlers
	 */
	//@{
	/*!
	 *	\brief Called by the host to reset a private data field in the plug-in's algorithm
	 *
	 *	This method is called sequentially for all private data fields on Effect initialization
	 *	and during any "reset" event, such as priming for a non-real-time render. This method is
	 *	called before the algorithm's optional initialization callback, and the initialized
	 *	private data will be available within that callback via its context block.
	 *	
	 *	\sa \ref alg_initialization.
	 *	
	 *	\warning Any data structures that will be passed between platforms (for example, sent to
	 *	a TI DSP in an %AAX DSP plug-in) must be properly data-aligned for compatibility across
	 *	both platforms. See \ref AAX_ALIGN_FILE_ALG for more information about guaranteeing
	 *	cross-platform compatibility of data structures used for algorithm processing.
	 *
	 *	\param[in] inFieldIndex
	 *		The index of the field that is being initialized
	 *	\param[out] oData
	 *		The pre-allocated block of data that should be initialized
	 *	\param[in] inDataSize
	 *		The size of the data block, in bytes
	 *
	 */
	virtual AAX_Result			ResetFieldData (AAX_CFieldIndex inFieldIndex, void * oData, uint32_t inDataSize) const = 0;
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
	 *	example if your plug-in contains any persistent state that is not encapsulated by its set
	 *	of registered parameters.
	 *
	 *	\warning Remember that plug-in chunk data may be loaded on a different platform from the
	 *	one where it is saved. All data structures in the chunk must be properly data-aligned for
	 *	compatibility across all platforms that the plug-in supports. See \ref AAX_ALIGN_FILE_ALG
	 *	for notes about common cross-platform pitfalls for data structure alignment.
	 *
	 *	For reference, see also:
	 *		\li AAX_CChunkDataParser
	 *		\li AAX_SPlugInChunk
	 *
	 */
	//@{
	/*!
	 *  \brief Retrieves the number of chunks used by this plug-in
	 *
	 *	\param[out] oNumChunks
	 *		The number of distinct chunks used by this plug-in
	 *
	 */	
	virtual AAX_Result			GetNumberOfChunks ( int32_t * oNumChunks )   const = 0;
	/*!
	 *  \brief Retrieves the ID associated with a chunk index.
	 *
	 *	\param[in] iIndex
	 *		Index of the queried chunk
	 *	\param[out] oChunkID
	 *		ID of the queried chunk
	 *
	 */
	virtual AAX_Result			GetChunkIDFromIndex ( int32_t iIndex, AAX_CTypeID * oChunkID )   const = 0;
	/*!
	 *  \brief Get the size of the data structure that can hold all of a chunk's information.
	 *  
	 *	If \a chunkID is one of the plug-in's custom chunks, initialize \a *size to the size
	 *	of the chunk's data in bytes.
	 *	
	 *	This method is invoked every time a chunk is saved, therefore it is possible to have 
	 *	dynamically sized chunks.  However, note that each call to GetChunkSize() will correspond
	 *	to a following call to GetChunk().  The chunk provided in GetChunk() \e must have the same
	 *	size as the \a size provided by GetChunkSize().
	 *
	 *	\legacy In AAX, the value provided by GetChunkSize() should \e NOT include the size of the
	 *	chunk header.  The value should \e ONLY reflect the size of the chunk's data.
	 *
	 *	\param[in] iChunkID
	 *		ID of the queried chunk
	 *	\param[out] oSize
	 *		The chunk's size in bytes
	 *
	 */
	virtual AAX_Result			GetChunkSize ( AAX_CTypeID iChunkID, uint32_t * oSize )   const = 0;
	/*!
	 *  \brief Fills a block of data with chunk information representing the plug-in's current state
	 *	
	 *	By calling this method, the host is requesting information about the current state of
	 *	the plug-in.  The following chunk fields should be explicitly populated in this method.
	 *	Other fields will be populated by the host.
	 *
	 *	\li AAX_SPlugInChunk::fData
	 *	\li AAX_SPlugInChunk::fVersion
	 *	\li AAX_SPlugInChunk::fName (Optional)
	 *	\li AAX_SPlugInChunk::fSize (Data size only)
	 *
	 *	\warning Remember that this chunk data may be loaded on a different platform from the one
	 *	where it is saved. All data structures in the chunk must be properly data-aligned for
	 *	compatibility across all platforms that the plug-in supports. See \ref AAX_ALIGN_FILE_ALG
	 *	for notes about common cross-platform pitfalls for data structure alignment.
	 *
	 *	\param[in] iChunkID
	 *		ID of the chunk that should be provided
	 *	\param[out] oChunk
	 *		A preallocated block of memory that should be populated with the chunk's data.
	 *
	 */
	virtual AAX_Result			GetChunk ( AAX_CTypeID iChunkID, AAX_SPlugInChunk * oChunk )   const = 0;
	/*!
	 *  \brief Restores a set of plug-in parameters based on chunk information.
	 *  
	 *  By calling this method, the host is attempting to update the plug-in's current state to
	 *	match the data stored in a chunk.  The plug-in should initialize itself to this new
	 *	state by calling \ref SetParameterNormalizedValue() for each of the relevant parameters.
	 *
	 *	\param[in] iChunkID
	 *		ID of the chunk that is being set
	 *	\param[in] iChunk
	 *		The chunk
	 *
	 */
	virtual AAX_Result			SetChunk ( AAX_CTypeID iChunkID, const AAX_SPlugInChunk * iChunk ) = 0;
	/*!
	 *  \brief Determine if a chunk represents settings that are equivalent to the plug-in's
	 *	current state
	 *
	 *	\compatibility In Pro Tools, this method will only be called if a prior call to
	 *	\ref AAX_IACFEffectParameters::GetNumberOfChanges() "GetNumberOfChanges()" has
	 *	indicated that the plug-in's state has changed.  If the plug-in's current settings
	 *	are different from the settings in \c aChunkP then the plug-in's Compare Light will
	 *	be illuminated in the plug-in header, allowing users to toggle between the
	 *	plug-in's custom state and its saved state.
	 *
	 *	\param[in] iChunkP
	 *		The chunk that is to be tested
	 *	\param[out] oIsEqual
	 *		True if the chunk represents equivalent settings when compared with the plug-in's
	 *		current state.  False if the chunk represents non-equivalent settings
	 *
	 */
	virtual AAX_Result			CompareActiveChunk ( const AAX_SPlugInChunk * iChunkP, AAX_CBoolean * oIsEqual )   const = 0;
	/*!
	 *  \brief Retrieves the number of parameter changes made since the plug-in's creation
	 *
	 *	This method is polled regularly by the host, and can additionally be triggered by some
	 *	events such as mouse clicks.  When the number provided by this method changes, the host
	 *	subsequently calls \ref CompareActiveChunk() to determine if the plug-in's Compare light
	 *	should be activated.
	 *
	 *	The value provided by this method should increment with each call to
	 *	\ref AAX_IACFEffectParameters::UpdateParameterNormalizedValue() "UpdateParameterNormalizedValue()"
	 *
	 *	\ingroup CommonInterface_DataModel_Overrides
	 *
	 *	\param[out] oNumChanges
	 *		Must be set to indicate the number of parameter changes that have occurred since plug-in
	 *		initialization.
	 */
	virtual AAX_Result			GetNumberOfChanges ( int32_t * oNumChanges )   const = 0;
	//@}end Chunk methods
	
	/** @name Thread methods
	 *
	 */
	//@{
	/*!
	 *	\brief Periodic wakeup callback for idle-time operations
	 *
	 *	This method is called from the host using a non-main thread.  In general, it should
	 *	be driven at approximately one call per 30 ms.  However, the wakeup is not guaranteed to
	 *	be called at any regular interval - for example, it could be held off by a high real-time
	 *	processing load - and there is no host contract regarding maximum latency between wakeup
	 *	calls.
	 *
	 *	This wakeup thread runs continuously and cannot be armed/disarmed or by the plug-in.
	 *
	 */
	virtual AAX_Result			TimerWakeup( ) = 0;
	//@}end Thread methods
	
	/** @name Auxiliary UI methods
	 *
	 */
	//@{
	/*! \brief Generate a set of output values based on a set of given input values.
	 *
	 *	This method is used by the host to generate graphical curves.  Given a set of input
	 *	values, e.g. frequencies in Hz, this method should generate a corresponding set of output
	 *	values, e.g. dB gain at each frequency.  The semantics of these input and output values are
	 *	dictated by \p iCurveType. See \ref AAX_ECurveType.
	 *
	 *	Plug-ins may also define custom curve type IDs to use this method internally.  For example,
	 *	the plug-in's GUI could use this method to request curve data in an arbitrary format.
	 *
	 *	- \note This method may be called by the host simultaneously from multiple threads with differents \p iValues.
	 *	- \note \p oValues must be allocated by caller with the same size as \p iValues (\p iNumValues).
	 *
	 *	\compatibility Versions of S6 software which support the
	 *	\ref AAX_IACFEffectParameters_V3::GetCurveDataDisplayRange() "GetCurveDataDisplayRange()"
	 *	method will not display a plug-in's curve data unless both
	 *	\ref AAX_IACFEffectParameters::GetCurveData() "GetCurveData()" and
	 *	\ref AAX_IACFEffectParameters_V3::GetCurveDataDisplayRange() "GetCurveDataDisplayRange()" are
	 *	supported by the plug-in.
	 *
	 *	\warning S6 currently polls this method to update a plug-in's EQ or dynamics curves
	 *	based on changes to the parameters mapped to the plug-in's EQ or dynamics center section page
	 *	tables. Parameters that are not included in these page tables will not trigger updates to the
	 *	curves displayed on S6. (GWSW-7314, \ref PTSW-195316)
	 *
	 *	\param[in] iCurveType
	 *		One of \ref AAX_ECurveType
	 *	\param[in] iValues
	 *		An array of input values
	 *	\param[in] iNumValues
	 *		The size of \p iValues
	 *	\param[out] oValues
	 *		An array of ouptut values
	 *
	 *	\return This method must return \ref AAX_ERROR_UNIMPLEMENTED if the plug-in does not support
	 *	curve data for the requested \c iCurveType
	 *
	 *	@ingroup AdditionalFeatures_CurveDisplays
	 *
	 */
	virtual AAX_Result			GetCurveData( /* AAX_ECurveType */ AAX_CTypeID iCurveType, const float * iValues, uint32_t iNumValues, float * oValues ) const = 0;
	//@}end Auxiliary UI methods
	
	/** @name Custom data methods
	 *
	 *  These functions exist as a proxiable way to move data between different modules (e.g. AAX_IEffectParameters and AAX_IEffectGUI.)  
	 *  Using these, the GUI can query any data through GetCustomData() with a plug-in defined \c typeID, \c void* and size.  This has an advantage
	 *  over just sharing memory in that this function can work as a remote proxy as we enable those sorts of features later in the platform.
	 *  Likewise, the GUI can also set arbitrary data on the data model by using the SetCustomData() function with the same idea.
	 *
	 *  \note These are plug-in internal only.  They are not called from the host right now, or likely ever.
	 */
	//@{
	/*!	\brief An optional interface hook for getting custom data from another module
	 *
	 *	\param[in] iDataBlockID
	 *		Identifier for the requested block of custom data
	 *	\param[in] inDataSize
	 *		Size of provided buffer, in bytes
	 *	\param[out] oData
	 *		Pointer to an allocated buffer.  Data will be written here.
	 *	\param[out] oDataWritten
	 *		The number of bytes actually written
	 */
	virtual AAX_Result			GetCustomData( AAX_CTypeID iDataBlockID, uint32_t inDataSize, void* oData, uint32_t* oDataWritten) const = 0;
	
	/*!	\brief An optional interface hook for setting custom data for use by another module
	 *
	 *	\param[in] iDataBlockID
	 *		Identifier for the provided block of custom data
	 *	\param[in] inDataSize
	 *		Size of provided buffer, in bytes
	 *	\param[in] iData
	 *		Pointer to the data buffer
	 */
	virtual AAX_Result			SetCustomData( AAX_CTypeID iDataBlockID, uint32_t inDataSize, const void* iData ) = 0;
	//@}end Custom Data methods

	/** @name MIDI methods
	 *
	 */
	//@{
	/** \brief MIDI update callback
	 *
	 *	Call \ref AAX_IController::GetNextMIDIPacket() from within this method to retrieve and
	 *	process MIDI packets directly within the Effect's data model.  MIDI data will also be
	 *	delivered to the Effect algorithm.
	 *
	 *	This method is called regularly by the host, similarly to
	 *	\ref AAX_IEffectParameters::TimerWakeup()
	 */
	virtual AAX_Result			DoMIDITransfers() = 0;
	//@}end MIDI methods
};

/**	@brief	Hybrid render processing context
 
	\sa AAX_IACFEffectParameters_V2::RenderAudio_Hybrid()
	
	\ingroup additionalFeatures_Hybrid
 */
struct AAX_SHybridRenderInfo
{
    float**                     mAudioInputs;
    int32_t*                    mNumAudioInputs;
    float**                     mAudioOutputs;
    int32_t*                    mNumAudioOutputs;
    int32_t*                    mNumSamples;
    AAX_CTimestamp*             mClock;                 
};

/** @brief	Supplemental interface for an %AAX Plug-in's data model
	
	@details
	This is a supplemental interface for an instance of a plug-in's data model. This
	interface gets exposed to the host application. Host applications that support
	%AAX versioned features may call into these methods. See \ref CommonInterface_DataModel.
	
	\note Your implementation of this interface must inherit from
	\ref AAX_IEffectParameters.

	\todo Add documentation for expected error state return values
		
	\ingroup CommonInterface_DataModel
*/
class AAX_IACFEffectParameters_V2 : public AAX_IACFEffectParameters
{
public:
	
	/** @name Hybrid audio methods
	 *
	 */
	//@{
	/**	\brief Hybrid audio render function
	 *
	 *	This method is called from the host to render audio for the hybrid piece of the algorithm.
	 *
	 *  \note To use this method plug-in should register some hybrid inputs and ouputs in "Describe"
	 *
	 *  \ingroup additionalFeatures_Hybrid
	 */
    virtual AAX_Result          RenderAudio_Hybrid(AAX_SHybridRenderInfo* ioRenderInfo) = 0;
	//@}end Hybrid audio methods
	
	/** @name MIDI methods
	 *
	 */
	//@{
	/**	\brief MIDI update callback
	 *
	 *	This method is called by the host for each pending MIDI packet for MIDI nodes in algorithm
	 *	context structure. Overwrite this method in Plug-In's EffectParameter class if you want to receive MIDI data packets
	 *  directly in the data model. MIDI data will also be delivered to the Effect algorithm.
	 *
	 *  The host calls this method in Effects that register one or more MIDI nodes using
	 *	\ref AAX_IComponentDescriptor::AddMIDINode(). Effects that do not require MIDI data to be sent to the
	 *	plug-in algorithm should override
	 *	\ref AAX_IACFEffectParameters_V2::UpdateControlMIDINodes() "UpdateControlMIDINodes()".
	 *
	 *	\param[in] inFieldIndex
	 *		MIDI node field index in algorithm context structure
	 *	\param[in] iPacket
	 *		The incoming MIDI packet for the node
	 */
	virtual	AAX_Result			UpdateMIDINodes ( AAX_CFieldIndex inFieldIndex, AAX_CMidiPacket& iPacket ) = 0;
	//@{
	/** \brief MIDI update callback for control MIDI nodes
	 *
	 *	This method is called by the host for each pending MIDI packet for Control MIDI nodes.
	 *	Overwrite this method in Plug-In's EffectParameter class if you want to receive MIDI data packets directly
	 *  in the data model.
	 *	
	 *	The host calls this method in Effects that register one or more Control MIDI nodes using
	 *	\ref AAX_IEffectDescriptor::AddControlMIDINode(). Effects with algorithms that use MIDI data nodes should
	 *	override \ref AAX_IACFEffectParameters_V2::UpdateMIDINodes() "UpdateMIDINodes()".
	 *
	 *	\note This method will not be called if an Effect includes any MIDI nodes in its algorithm context structure.
	 *
	 *	\param[in] nodeID
	 *		Identifier for the MIDI node
	 *	\param[in] iPacket
	 *		The incoming MIDI packet for the node
	 */
	virtual AAX_Result			UpdateControlMIDINodes ( AAX_CTypeID nodeID, AAX_CMidiPacket& iPacket ) = 0;
	//@}end MIDI methods
};

/** @brief	Supplemental interface for an %AAX Plug-in's data model
	
	@details
	This is a supplemental interface for an instance of a plug-in's data model. This
	interface gets exposed to the host application. Host applications that support
	%AAX versioned features may call into these methods. See \ref CommonInterface_DataModel.
	
	\note Your implementation of this interface must inherit from
	\ref AAX_IEffectParameters.

	\todo Add documentation for expected error state return values
		
	\ingroup CommonInterface_DataModel
*/
class AAX_IACFEffectParameters_V3 : public AAX_IACFEffectParameters_V2
{
public:
	
	/** @name Auxiliary UI methods
	 *
	 */
	//@{
	/*! \brief Indicates which meters correspond to the X and Y axes of the EQ or Dynamics graph.
	 *
	 *	These meters can be used by attached control surfaces to present an indicator in the same
	 *	X/Y coordinate plane as the plug-in's curve data.
	 *
	 *	\param[in] iCurveType
	 *		One of \ref AAX_ECurveType
	 *	\param[out] oXMeterId
	 *		Id of the X-axis meter
	 *	\param[out] oYMeterId
	 *		Id of the Y-axis meter
	 *
	 *	\return This method should return \ref AAX_ERROR_UNIMPLEMENTED if the plug-in does not implement it.
	 *
	 *	@ingroup AdditionalFeatures_CurveDisplays
	 *
	 */
	virtual AAX_Result			GetCurveDataMeterIds( /* AAX_ECurveType */ AAX_CTypeID iCurveType, uint32_t *oXMeterId, uint32_t *oYMeterId)  const = 0;

	/*! \brief Determines the range of the graph shown by the plug-in
	 *
	 *	Min/max arguments define the range of the axes of the graph.
	 *
	 *	\param[in] iCurveType
	 *		One of \ref AAX_ECurveType
	 *	\param[out] oXMin
	 *		Min value of X-axis range
	 *	\param[out] oXMax
	 *		Max value of X-axis range
	 *	\param[out] oYMin
	 *		Min value of Y-axis range
	 *	\param[out] oYMax
	 *		Max value of Y-axis range 
	 *
	 *	\return This method should return \ref AAX_ERROR_UNIMPLEMENTED if the plug-in does not implement it.
	 *
	 *	@ingroup AdditionalFeatures_CurveDisplays
	 *
	 */
	virtual AAX_Result          GetCurveDataDisplayRange( /* AAX_ECurveType */ AAX_CTypeID iCurveType, float *oXMin, float *oXMax, float *oYMin, float *oYMax ) const = 0;
	//@}end Auxiliary UI methods
};

/** @brief	Supplemental interface for an %AAX Plug-in's data model
 
 @details
 This is a supplemental interface for an instance of a plug-in's data model. This
 interface gets exposed to the host application. Host applications that support
 %AAX versioned features may call into these methods. See \ref CommonInterface_DataModel.
 
 \note Your implementation of this interface must inherit from
 \ref AAX_IEffectParameters.
 
 \todo Add documentation for expected error state return values
 
 \ingroup CommonInterface_DataModel
 */
class AAX_IACFEffectParameters_V4 : public AAX_IACFEffectParameters_V3
{
public:
	
	/** @name Auxiliary UI methods
	 *
	 */
	//@{
	/*! \brief Allow the plug-in to update its page tables
	 *
	 *	Called by the plug-in host, usually in response to a
	 *	\ref AAX_eNotificationEvent_ParameterMappingChanged notification sent from the
	 *	plug-in.
	 *
	 *	Use this method to change the page table mapping for the plug-in instance
	 *	or to apply other changes to auxiliary UIs which use the plug-in page tables,
	 *	such as setting focus to a new page.
	 *
	 *	See \ref AAX_Page_Table_Guide for more information about page tables.
	 *
	 *	\param[in] inTableType
	 *		Four-char type identifier for the table type (e.g. \c 'PgTL', \c 'Av81', etc.)
	 *	\param[in] inTablePageSize
	 *		Page size for the table
	 *	\param[in] iHostUnknown
	 *		\parblock
	 *		Unknown interface from the host which may support interfaces providing additional
	 *		features or information.
	 *
	 *		All interfaces queried from this unknown will be valid only within the scope of
	 *		this \ref AAX_IEffectParameters::UpdatePageTable() "UpdatePageTable()"
	 *		execution and will be relevant for only the current plug-in instance.
	 *		\endparblock
	 *	\param[in,out] ioPageTableUnknown
	 *		Unknown interface which supports \ref AAX_IPageTable. This object represents
	 *		the page table data which is currently stored by the host for this plug-in instance
	 *		for the given table type and page size. This data and may be edited within
	 *		the scope of \ref AAX_IEffectParameters::UpdatePageTable() "UpdatePageTable()"
	 *		to change the page table mapping for this plug-in instance.
	 *
	 *	\return This method should return \ref AAX_ERROR_UNIMPLEMENTED if the plug-in does
	 *	not implement it or when no change is requested by the plug-in. This allows
	 *	optimizations to be used in the host when no UI update is required following this
	 *	call.
	 *
	 *	\sa \ref AAX_eNotificationEvent_ParameterMappingChanged
	 *
	 */
	virtual AAX_Result UpdatePageTable(uint32_t inTableType, int32_t inTablePageSize, IACFUnknown* iHostUnknown, IACFUnknown* ioPageTableUnknown) const = 0;
	//@}end Auxiliary UI methods
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFEFFECTPARAMETERS_H
