/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IEffectParameters.h
 *
 *	\brief The interface for an %AAX Plug-in's data model
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IEFFECTPARAMETERS_H
#define AAX_IEFFECTPARAMETERS_H

#include "AAX_IACFEffectParameters.h"
#include "AAX.h"
#include "CACFUnknown.h"

/** @brief	The interface for an %AAX Plug-in's data model.
	
	@details
	@pluginimp
	
	The interface for an instance of a plug-in's data model. A plug-in's implementation
	of this interface is responsible for creating the plug-in's set of parameters and
	for defining how the plug-in will respond when these parameters are changed via
	control updates or preset loads.  In order for information to be routed from the
	plug-in's data model to its algorithm, the parameters that are created here must be
	registered with the host in the plug-in's
	\ref CommonInterface_Describe "Description callback".

	At \ref AAX_IACFEffectParameters::Initialize() "initialization", the host provides
	this interface with a reference to AAX_IController, which provides access from the
	data model back to the host. This reference provides a means of querying information
	from the host such as stem format or sample rate, and is also responsible for
	communication between the data model and the plug-in's (decoupled) algorithm. See
	\ref CommonInterface_Algorithm.

	You will most likely inherit your implementation of this interface from 
	\ref AAX_CEffectParameters, a default implementation that provides basic data model
	functionality such as adding custom parameters, setting control values, restoring
	state, generating coefficients, etc., which you can override and customize as
	needed.

	The following tags appear in the descriptions for methods of this class and its
	derived classes:
		\li \c CALL: Components in the plug-in should call this method to get / set
		data in the data model.

	\note
		\li This class always inherits from the latest version of the interface and thus
		requires any subclass to implement all the methods in the latest version of the
		interface. The current version of \ref AAX_CEffectParameters provides a convenient
		default implementation for all methods in the latest interface.
		\li Except where noted otherwise, the parameter values referenced by the methods in
		this interface are normalized values.  See \ref AAXLibraryFeatures_ParameterManager
		for more information.

	\legacy In the legacy plug-in SDK, these methods were found in CProcess and
	\c CEffectProcess.  For additional \c CProcess methods, see \ref AAX_IEffectGUI.

	\section AAX_IEffectParameters_relclass Related classes
	\dotfile aax_ieffectparams_related.dot "Classes related to AAX_IEffectParameters by inheritance or composition"
	\dotfile aax_ieffectparams_contained.dot "Classes owned as member objects of AAX_CEffectParameters"

	\ingroup CommonInterface_DataModel
*/
class AAX_IEffectParameters :	public AAX_IACFEffectParameters_V4
								, public CACFUnknown
{
public:
	ACF_DECLARE_STANDARD_UNKNOWN()
	
	ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppvObjOut) override;
	
	// CACFUnknown does not support operator=()
	AAX_DELETE(AAX_IEffectParameters& operator= (const AAX_IEffectParameters&));
};

#endif // AAX_IEFFECTPARAMETERS_H
