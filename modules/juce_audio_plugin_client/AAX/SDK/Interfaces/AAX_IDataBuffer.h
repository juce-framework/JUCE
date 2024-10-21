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
 * \file  AAX_IDataBuffer.h
 */ 
/*================================================================================================*/

#pragma once

#ifndef AAX_IDataBuffer_H
#define AAX_IDataBuffer_H

#include "AAX_IACFDataBuffer.h"
#include "AAX.h"
#include "CACFUnknown.h"
#include "AAX_UIDs.h"
#include "acfextras.h"


/** 
 * \brief Interface for reference counted data buffers
 *
 * \copydetails AAX_IACFDataBuffer
 */ 
class AAX_IDataBuffer : public AAX_IACFDataBuffer
					  , public CACFUnknown
{
public:
	ACF_DECLARE_STANDARD_UNKNOWN()
	
	ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppvObjOut) AAX_OVERRIDE
	{
		if (riid == IID_IAAXDataBufferV1)
		{
			*ppvObjOut = static_cast<IACFUnknown *>(this);
			( static_cast<IACFUnknown *>(*ppvObjOut))->AddRef();
			return ACF_OK;
		}
		
		return this->CACFUnknown::InternalQueryInterface(riid, ppvObjOut);
	}
	
	// CACFUnknown does not support operator=()
	AAX_DELETE(AAX_IDataBuffer& operator= (const AAX_IDataBuffer&));
};

#endif
