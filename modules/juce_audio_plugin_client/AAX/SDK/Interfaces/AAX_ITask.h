/*================================================================================================*/
/*
 *
 * Copyright 2023-2025 Avid Technology, Inc.
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
 * \file  AAX_ITask.h
 */ 
/*================================================================================================*/

#pragma once

#ifndef AAX_ITask_H
#define AAX_ITask_H

#include "AAX_IACFTask.h"
#include "AAX.h"
#include <memory>

class AAX_IACFDataBuffer;


/**
 * \brief Interface representing a request to perform a task
 * 
 * \details
 * \hostimp
 * 
 * Used by the \ref AAX_ITaskAgent "task agent".
 * 
 * This interface describes a task request and provides a way for
 * the agent to express one or more results of the task as well as
 * the progress of the task.
 * 
 * This interface is open-ended for both inputs and outputs. The
 * host and agent must use common definitions for specific task
 * types, their possible arguments, and the expected results.
 * 
 * \ingroup AuxInterface_TaskAgent
 */
class AAX_ITask
{
public:
	virtual ~AAX_ITask() = default;

	/**
	 * An identifier defining the type of the requested task
	 *
	 * \param[out] oType
	 * The type of this task request
	 */
	virtual AAX_Result GetType(AAX_CTypeID * oType) const = 0;
	
	/**
	 * Additional information defining the request, depending on
	 * the task type
	 * 
	 * \param[in] iType
	 * The type of argument requested. Possible argument types, if
	 * any, and the resulting data buffer format must be defined per
	 * task type.
	 * 
	 * \return The requested argument data, or nullptr. This data
	 * buffer's type ID is expected to match \c iType . The caller
	 * takes ownership of this object.
	 */
	virtual AAX_IACFDataBuffer const * GetArgumentOfType(AAX_CTypeID iType) const = 0;

	/**
	 * Inform the host about the current status of the task
	 * 
	 * \param[in] iProgress
	 * A value between 0 (no progress) and 1 (complete)
	 */
	virtual AAX_Result SetProgress(float iProgress) = 0;
	
	/**
	 * Returns the current progress
	 */
	virtual float GetProgress() const = 0;

	/**
	 * \brief Attach result data to this task
	 * 
	 * \details
	 * This can be called multiple times to add multiple types
	 * of results to a single task.
	 * 
	 * The host may process the result data immediately or may
	 * wait for the task to complete.
	 * 
	 * The plug-in is expected to release the data buffer upon
	 * making this call. At a minimum, the data buffer must not
	 * be changed after this call is made. See \c ACFPtr::inArg()
	 * 
	 * \param[in] iResult
	 * A buffer containing the result data. Expected result types, if
	 * any, and their data buffer format must be defined per task type.
	 */
	virtual AAX_Result AddResult(AAX_IACFDataBuffer const * iResult) = 0;

	/**
	 * \brief Inform the host that the task is completed.
	 * 
	 * \details
	 * If successful, returns a null pointer. Otherwise, returns
	 * a pointer back to the same object. See
	 * \ref SetDone(std::unique_ptr<AAX_ITask>&, AAX_TaskCompletionStatus)
	 * for an example usage pattern.
	 * 
	 * \param[in] iStatus
	 * The final status of the task. This indicates to the host
	 * whether or not the task was performed as requested.
	 */
	virtual AAX_ITask * SetDone(AAX_TaskCompletionStatus iStatus) = 0;

	/**
	 * \brief The identifier for this task
	 */
	virtual AAX_Result GetID(AAX_CTaskID * outID) const = 0;

	/**
	 * \brief Set a description of the current progress state
	 *
	 * \details
	 * This label may be shown by the host in a progress dialog or
	 * other progress reporting UI. If possible, use a localized
	 * string based on \ref AAX_eNotificationEvent_HostLocale .
	 *
	 * \param[in] iDescription
	 * A UTF-8 string describing the current progress state.
	 */
	virtual AAX_Result SetProgressLabel(const char * iLabel) = 0;

	/// Helper for \ref SetDone() when using a unique_ptr
	static void SetDone(std::unique_ptr<AAX_ITask> & iTask, AAX_TaskCompletionStatus iStatus) {
		if (iTask) {
			AAX_ITask* const pt = iTask->SetDone(iStatus);
			if (pt != iTask.get()) {
				iTask.reset(pt);
			}
		}
	}

	/// Helper for \ref SetDone() when using a shared_ptr
	static void SetDone(std::shared_ptr<AAX_ITask> & iTask, AAX_TaskCompletionStatus iStatus) {
		if (iTask) {
			AAX_ITask* const pt = iTask->SetDone(iStatus);
			if (pt != iTask.get()) {
				iTask.reset(pt);
			}
		}
	}
};

#endif
