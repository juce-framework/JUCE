/*================================================================================================*/
/*
 *
 *	Copyright 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_CTaskAgent.h
 *
 *	\brief A default implementation of the \ref AAX_ITaskAgent interface.
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CTaskAgent_H
#define AAX_CTaskAgent_H

#include "AAX_ITaskAgent.h"
#include <memory>

class AAX_IController;
class AAX_IEffectParameters;
class AAX_ITask;

/** @brief Default implementation of the \ref AAX_ITaskAgent interface.
	
	@details
	This class provides a default implementation of the \ref AAX_ITaskAgent interface.
	Your plug-in's task agent implementation should inherit from this class and 
	override the remaining interface functions.
		
	\ingroup AuxInterface_TaskAgent
*/
class AAX_CTaskAgent : public AAX_ITaskAgent
{
public: ///////////////////////////////////////////////////////////////////////////// constructor/destructor
	AAX_CTaskAgent (void) = default;
	~AAX_CTaskAgent (void) AAX_OVERRIDE;
	
public: ///////////////////////////////////////////////////////////////////////////// AAX_IACFTaskAgent

	/** @name Initialization and uninitialization
	 */
	//@{
	AAX_Result Initialize (IACFUnknown * iController ) AAX_OVERRIDE; ///< \copydoc AAX_IACFTaskAgent::Initialize()
	AAX_Result Uninitialize (void) AAX_OVERRIDE; ///< \copydoc AAX_IACFTaskAgent::Uninitialize()
	//@}end Initialization and uninitialization
	
	/** @name Task management
	 */
	//@{
	/**
	 * \brief Default implemenation of AddTask()
	 *
	 * \details
	 * Convenience implementation that converts the \ref IACFUnknown
	 * into an \ref AAX_ITask . Implementations should override the
	 * version that provides an \ref AAX_ITask object.
	 */
	AAX_Result AddTask(IACFUnknown * iTask) AAX_OVERRIDE;
	AAX_Result CancelAllTasks() AAX_OVERRIDE;
	//@} Task management
	
protected:
	
	/**
	 * \brief Convenience method for adding versioned tasks
	 * 
	 * \deprecated Use \ref ReceiveTask() instead
	 */
	virtual AAX_Result AddTask(std::unique_ptr<AAX_ITask> iTask);
	
	/**
	 * \brief Convenience method for adding versioned tasks
	 */
	virtual AAX_Result ReceiveTask(std::unique_ptr<AAX_ITask> iTask);
	
public: ///////////////////////////////////////////////////////////////////////////// AAX_CTaskAgent

	/** @name Private member accessors
	 */
	//@{
	/*!
	 *  \brief Returns a pointer to the plug-in's controller interface
	 */
	AAX_IController* GetController (void) { return mController; };
	/*!
	 *  \brief Returns a pointer to the plug-in's data model interface
	 */
	AAX_IEffectParameters* GetEffectParameters (void) { return mEffectParameters; }
	//@}end Private member accessors
	
private:
	void ReleaseObjects();

	AAX_IController* mController = nullptr;
	AAX_IEffectParameters* mEffectParameters = nullptr;
};


#endif
