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
 *	\file   AAX_VSessionDocument.h
 */ 
/*================================================================================================*/

#pragma once
#ifndef AAX_VSessionDocument_H
#define AAX_VSessionDocument_H

#include "AAX_ISessionDocument.h"
#include "ACFPtr.h"

class AAX_IACFSessionDocument;
class AAX_IDataBufferWrapper;

class AAX_VSessionDocument : public AAX_ISessionDocument
{
public:
	explicit AAX_VSessionDocument(IACFUnknown * iUnknown);
	~AAX_VSessionDocument() AAX_OVERRIDE;

	class VTempoMap : public AAX_ISessionDocument::TempoMap
	{
	public:
		~VTempoMap() AAX_OVERRIDE;
		explicit VTempoMap(IACFUnknown & inDataBuffer);
		int32_t Size() const AAX_OVERRIDE;
		AAX_CTempoBreakpoint const * Data() const AAX_OVERRIDE;
	private:
		std::unique_ptr<AAX_IDataBufferWrapper const> mDataBuffer;
	};

	/** 
	 * \brief Release all interface references
	 */
	void Clear();

	bool Valid() const AAX_OVERRIDE;
	std::unique_ptr<AAX_ISessionDocument::TempoMap const> GetTempoMap() AAX_OVERRIDE;
	AAX_Result GetDocumentData(AAX_DocumentData_UID const & inDataType, IACFUnknown ** outData) AAX_OVERRIDE;

private:
	ACFPtr<AAX_IACFSessionDocument> mSessionDocumentV1;
};

#endif // AAX_VSessionDocument_H
