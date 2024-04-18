/*================================================================================================*/
/*
 *
 * Copyright 2023-2024 Avid Technology, Inc.
 * All rights reserved.
 * 
 * This file is part of the Avid AAX SDK.
 * 
 * The AAX SDK is subject to commercial or open-source licensing.
 * 
 * By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
 * Agreement and Avid Privacy Policy.
 * 
 * AAX SDK License: https://developer.avid.com/aax
 * Privacy Policy: https://www.avid.com/legal/privacy-policy-statement
 * 
 * Or: You may also use this code under the terms of the GPL v3 (see
 * www.gnu.org/licenses).
 * 
 * THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 * EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 * DISCLAIMED.
 *
 */

/**  
 * \file  AAX_IACFTaskAgent.h
 */ 
/*================================================================================================*/

#pragma once

#ifndef AAX_IACFTaskAgent_H
#define AAX_IACFTaskAgent_H

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

class IACFUnknown;


/** 
 * \brief Versioned interface for a component that accepts task requests
 *
 * \details
 * \pluginimp
 * 
 * The task agent is expected to complete the requested tasks asynchronously
 * and to provide progress and completion details via calls on the
 * \ref AAX_IACFTask interface as the tasks proceed.
 * 
 * \sa AAX_ITask
 * 
 * \ingroup AuxInterface_TaskAgent
 */
class AAX_IACFTaskAgent : public IACFUnknown
{
public:
	/** @name Initialization and uninitialization
	 */
	//@{
	/**
	 * Initialize the object
	 * 
	 * \param[in] iController
	 * Interface allowing access to other objects in the object graph
	 * such as the plug-in's data model.
	 */
	virtual AAX_Result Initialize(IACFUnknown* iController) = 0;
	/**
	 * Uninitialize the object
	 * 
	 * This method should release references to any shared objects
	 */
	virtual AAX_Result Uninitialize() = 0;
	//@} Initialization and uninitialization
	
	/** @name Task management
	 */
	//@{
	/**
	 * Request that the agent perform a task
	 * 
	 * \param[in] iTask
	 * The task to perform. The agent must retain a reference to
	 * this task if it will be used beyond the scope of this method.
	 * This object should support at least \ref AAX_IACFTask .
	 */
	virtual AAX_Result AddTask(IACFUnknown * iTask) = 0;
	/**
	 * Request that the agent cancel all outstanding tasks
	 */
	virtual AAX_Result CancelAllTasks() = 0;
	//@} Task management
};


#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif
