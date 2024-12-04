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
 *	\file  AAX_IAutomationDelegate.h
 *
 *	\brief Interface allowing an %AAX plug-in to interact with the host's automation system
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IAUTOMATIONDELEGATE_H
#define AAX_IAUTOMATIONDELEGATE_H

#include "AAX.h"

/**	\brief Interface allowing an %AAX plug-in to interact with the host's event system
 *	
 *	\details
 *	\hostimp
 * 
 *	This delegate provides a means of interacting with the host's event system in order to ensure
 *	that events such as parameter updates are properly arbitrated and broadcast to all listeners.  The automation
 *	delegate is used regardless of whether or not an individual parameter is "automatable" or "automation-enabled".
 *
 *	A parameter must be registered with the automation delegate in order for updates to the parameter's control in the 
 *	plug-in's GUI or other controller (control surface, etc.) to be successfully processed by the host and sent to the
 *	\ref AAX_IEffectParameters object.
 *
 *	The parameter identifiers used by this interface correspond to the control IDs used to identify parameters in the
 *	\ref AAX_CParameterManager "Parameter Mananger".
 */
class AAX_IAutomationDelegate
{
public:

	virtual ~AAX_IAutomationDelegate() {}
	
	/**
	 *	Register a control with the automation system using a char* based control identifier
	 *
	 *	The automation delegate owns a list of the IDs of all of the parameters that have been registered with it. This
	 *	list is used to set up listeners for all of the registered parameters such that the automation delegate may
	 *	update the plug-in when the state of any of the registered parameters have been modified.
	 *
	 *	\sa AAX_IAutomationDelegate::UnregisterParameter()
	 *
	 *	\param[in] iParameterID
	 *		Parameter ID that is being registered
	 */
	virtual AAX_Result	RegisterParameter ( AAX_CParamID iParameterID ) = 0;

	/**
	 *	Unregister a control with the automation system using a char* based control identifier
	 *
	 *	\note All registered controls should be unregistered or the system might leak.
	 *
	 *	\sa AAX_IAutomationDelegate::RegisterParameter()
	 *
	 *	\param[in] iParameterID
	 *		Parameter ID that is being registered
	 */
	virtual AAX_Result	UnregisterParameter ( AAX_CParamID iParameterID ) = 0;

	/**
	 *	Submits a request for the given parameter's value to be changed
	 *
	 *	\param[in] iParameterID
	 *		ID of the parameter for which a change is requested
	 *	\param[in] normalizedValue
	 *		The requested new parameter value, formatted as a double and normalized to [0 1]
	 */
	virtual AAX_Result	PostSetValueRequest ( AAX_CParamID iParameterID, double normalizedValue ) const = 0;

	/**
	 *	Notifies listeners that a parameter's value has changed
	 *
	 *	\param[in] iParameterID
	 *		ID of the parameter that has been updated
	 *	\param[in] normalizedValue
	 *		The current parameter value, formatted as a double and normalized to [0 1]
	 */
	virtual AAX_Result	PostCurrentValue( AAX_CParamID iParameterID, double normalizedValue ) const = 0;
	
	/**
	 *	Requests that the given parameter be "touched", i.e. locked for updates by the current client
	 *
	 *	\param[in] iParameterID
	 *		ID of the parameter that will be touched
	 */
	virtual AAX_Result	PostTouchRequest( AAX_CParamID iParameterID ) = 0;
	
	/**
	 *	Requests that the given parameter be "released", i.e. available for updates from any client
	 *
	 *	\param[in] iParameterID
	 *		ID of the parameter that will be released
	 */
	virtual AAX_Result	PostReleaseRequest( AAX_CParamID iParameterID ) = 0;
	
	/**
	 *	Gets the current touched state of a parameter
	 *
	 *	\param[in] iParameterID
	 *		ID of the parameter that is being queried
	 *	\param[out] oTouched
	 *		The current touch state of the parameter
	 */
	virtual AAX_Result GetTouchState ( AAX_CParamID iParameterID, AAX_CBoolean * oTouched ) = 0;
	
	/**
	 *	Notify listeners that the parameter's display name has changed
	 *
	 *	Note that this is not part of the underlying automation delegate interface with
	 *	the host; it is converted on the %AAX side to a notification posted to the host
	 *	via the \ref AAX_IController .
	 *
	 *	\param[in] iParameterID
	 *		ID of the parameter that has been updated
	 */
	virtual AAX_Result ParameterNameChanged ( AAX_CParamID iParameterID ) = 0;
};


#endif ////AAX_IAUTOMATIONDELEGATE_H
