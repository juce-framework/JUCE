/*================================================================================================*/
/*
 *	Copyright 2016-2017, 2023-2024 Avid Technology, Inc.
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
/*================================================================================================*/

#include "AAX_VPageTable.h"
#include "AAX_UIDs.h"


// ******************************************************************************************
// METHOD:	AAX_VPageTable
// ******************************************************************************************
AAX_VPageTable::AAX_VPageTable( IACFUnknown* pUnknown )
: mIPageTable()
{
    if ( pUnknown )
    {
		// If this unknown supports query of an existing page table then the AAX_VPageTable
		// object will use the existing page table
		pUnknown->QueryInterface(IID_IAAXPageTableV1, (void **)&mIPageTable);
		pUnknown->QueryInterface(IID_IAAXPageTableV2, (void **)&mIPageTable2);
    }
}

// ******************************************************************************************
// METHOD:	~AAX_VPageTable
// ******************************************************************************************
AAX_VPageTable::~AAX_VPageTable()
{
}

// ******************************************************************************************
// METHOD:	Clear
// ******************************************************************************************
AAX_Result AAX_VPageTable::Clear()
{
	if (mIPageTable)
		return mIPageTable->Clear();
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	Empty
// ******************************************************************************************
AAX_Result AAX_VPageTable::Empty(AAX_CBoolean& oEmpty) const
{
	if (mIPageTable)
		return mIPageTable->Empty(oEmpty);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetNumPages
// ******************************************************************************************
AAX_Result AAX_VPageTable::GetNumPages(int32_t& oNumPages) const
{
	if (mIPageTable)
		return mIPageTable->GetNumPages(oNumPages);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	InsertPage
// ******************************************************************************************
AAX_Result AAX_VPageTable::InsertPage(int32_t iPage)
{
	if (mIPageTable)
		return mIPageTable->InsertPage(iPage);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	RemovePage
// ******************************************************************************************
AAX_Result AAX_VPageTable::RemovePage(int32_t iPage)
{
	if (mIPageTable)
		return mIPageTable->RemovePage(iPage);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetNumMappedParameterIDs
// ******************************************************************************************
AAX_Result AAX_VPageTable::GetNumMappedParameterIDs(int32_t iPage, int32_t& oNumParameterIdentifiers) const
{
	if (mIPageTable)
		return mIPageTable->GetNumMappedParameterIDs(iPage, oNumParameterIdentifiers);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	ClearMappedParameter
// ******************************************************************************************
AAX_Result AAX_VPageTable::ClearMappedParameter(int32_t iPage, int32_t iIndex)
{
	if (mIPageTable)
		return mIPageTable->ClearMappedParameter(iPage, iIndex);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetMappedParameterID
// ******************************************************************************************
AAX_Result AAX_VPageTable::GetMappedParameterID(int32_t iPage, int32_t iIndex, AAX_IString& oParameterIdentifier) const
{
	if (mIPageTable)
		return mIPageTable->GetMappedParameterID(iPage, iIndex, oParameterIdentifier);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	MapParameterID
// ******************************************************************************************
AAX_Result AAX_VPageTable::MapParameterID(AAX_CParamID iParameterIdentifier, int32_t iPage, int32_t iIndex)
{
	if (mIPageTable)
		return mIPageTable->MapParameterID(iParameterIdentifier, iPage, iIndex);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetNumParametersWithNameVariations
// ******************************************************************************************
AAX_Result AAX_VPageTable::GetNumParametersWithNameVariations(int32_t& oNumParameterIdentifiers) const
{
	if (mIPageTable2)
		return mIPageTable2->GetNumParametersWithNameVariations(oNumParameterIdentifiers);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetNameVariationParameterIDAtIndex
// ******************************************************************************************
AAX_Result AAX_VPageTable::GetNameVariationParameterIDAtIndex(int32_t iIndex, AAX_IString& oParameterIdentifier) const
{
	if (mIPageTable2)
		return mIPageTable2->GetNameVariationParameterIDAtIndex(iIndex, oParameterIdentifier);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetNumNameVariationsForParameter
// ******************************************************************************************
AAX_Result AAX_VPageTable::GetNumNameVariationsForParameter(AAX_CPageTableParamID iParameterIdentifier, int32_t& oNumVariations) const
{
	if (mIPageTable2)
		return mIPageTable2->GetNumNameVariationsForParameter(iParameterIdentifier, oNumVariations);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetParameterNameVariationAtIndex
// ******************************************************************************************
AAX_Result AAX_VPageTable::GetParameterNameVariationAtIndex(AAX_CPageTableParamID iParameterIdentifier, int32_t iIndex, AAX_IString& oNameVariation, int32_t& oLength) const
{
	if (mIPageTable2)
		return mIPageTable2->GetParameterNameVariationAtIndex(iParameterIdentifier, iIndex, oNameVariation, oLength);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	GetParameterNameVariationOfLength
// ******************************************************************************************
AAX_Result AAX_VPageTable::GetParameterNameVariationOfLength(AAX_CPageTableParamID iParameterIdentifier, int32_t iLength, AAX_IString& oNameVariation) const
{
	if (mIPageTable2)
		return mIPageTable2->GetParameterNameVariationOfLength(iParameterIdentifier, iLength, oNameVariation);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	ClearParameterNameVariations
// ******************************************************************************************
AAX_Result AAX_VPageTable::ClearParameterNameVariations()
{
	if (mIPageTable2)
		return mIPageTable2->ClearParameterNameVariations();

	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	ClearNameVariationsForParameter
// ******************************************************************************************
AAX_Result AAX_VPageTable::ClearNameVariationsForParameter(AAX_CPageTableParamID iParameterIdentifier)
{
	if (mIPageTable2)
		return mIPageTable2->ClearNameVariationsForParameter(iParameterIdentifier);
	
	return AAX_ERROR_UNIMPLEMENTED;
}

// ******************************************************************************************
// METHOD:	SetParameterNameVariation
// ******************************************************************************************
AAX_Result AAX_VPageTable::SetParameterNameVariation(AAX_CPageTableParamID iParameterIdentifier, const AAX_IString& iNameVariation, int32_t iLength)
{
	if (mIPageTable2)
		return mIPageTable2->SetParameterNameVariation(iParameterIdentifier, iNameVariation, iLength);
	
	return AAX_ERROR_UNIMPLEMENTED;
}
