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
 * \file  AAX_VTask.h
 */ 
/*================================================================================================*/

#pragma once

#ifndef AAX_VTask_H
#define AAX_VTask_H

#include "AAX_ITask.h"
#include "AAX.h"

#include "ACFPtr.h"

class IACFUnknown;

/*!
 \brief Version-managed concrete \ref AAX_ITask
 */
class AAX_VTask : public AAX_ITask
{
public:
	explicit AAX_VTask( IACFUnknown* pUnknown );
	~AAX_VTask() AAX_OVERRIDE;

	AAX_Result GetType(AAX_CTypeID * oType) const AAX_OVERRIDE; ///< \copydoc AAX_ITask::GetType()
	AAX_IACFDataBuffer const * GetArgumentOfType(AAX_CTypeID iType) const AAX_OVERRIDE; ///< \copydoc AAX_ITask::GetArgumentOfType()

	AAX_Result SetProgress(float iProgress)  AAX_OVERRIDE; ///< \copydoc AAX_ITask::SetProgress()
	float GetProgress() const AAX_OVERRIDE; ///< \copydoc AAX_ITask::GetProgress()
	AAX_Result AddResult(AAX_IACFDataBuffer const * iResult) AAX_OVERRIDE; ///< \copydoc AAX_ITask::AddResult()
	AAX_ITask * SetDone(AAX_TaskCompletionStatus iStatus) AAX_OVERRIDE; ///< \copydoc AAX_ITask::SetDone()
private:
	ACFPtr<AAX_IACFTask> mTaskV1;
};

#endif
