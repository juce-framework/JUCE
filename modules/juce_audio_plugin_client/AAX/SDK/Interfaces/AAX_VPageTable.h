/*================================================================================================*/
/*
 *	Copyright 2016-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 */

#ifndef AAXLibrary_AAX_VPageTable_h
#define AAXLibrary_AAX_VPageTable_h

#include "AAX_IPageTable.h"
#include "AAX_IACFPageTable.h"
#include "ACFPtr.h"

/**
 *	\brief Version-managed concrete \ref AAX_IPageTable class
 *
 */
class AAX_VPageTable : public AAX_IPageTable
{
public:
	AAX_VPageTable( IACFUnknown* pUnknown );
	~AAX_VPageTable() AAX_OVERRIDE;
	
	// AAX_IACFPageTable
	AAX_Result Clear() AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::Clear()
	AAX_Result Empty(AAX_CBoolean& oEmpty) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::Empty()
	AAX_Result GetNumPages(int32_t& oNumPages) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNumPages()
	AAX_Result InsertPage(int32_t iPage) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::InsertPage()
	AAX_Result RemovePage(int32_t iPage) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::RemovePage()
	AAX_Result GetNumMappedParameterIDs(int32_t iPage, int32_t& oNumParameterIdentifiers) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNumMappedParameterIDs()
	AAX_Result ClearMappedParameter(int32_t iPage, int32_t iIndex) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::ClearMappedParameter()
	AAX_Result GetMappedParameterID(int32_t iPage, int32_t iIndex, AAX_IString& oParameterIdentifier) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetMappedParameterID()
	AAX_Result MapParameterID(AAX_CParamID iParameterIdentifier, int32_t iPage, int32_t iIndex) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::MapParameterID()
	
	// AAX_IACFPageTable_V2
	AAX_Result GetNumParametersWithNameVariations(int32_t& oNumParameterIdentifiers) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNumParametersWithNameVariations()
	AAX_Result GetNameVariationParameterIDAtIndex(int32_t iIndex, AAX_IString& oParameterIdentifier) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNameVariationParameterIDAtIndex()
	AAX_Result GetNumNameVariationsForParameter(AAX_CPageTableParamID iParameterIdentifier, int32_t& oNumVariations) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetNumNameVariationsForParameter()
	AAX_Result GetParameterNameVariationAtIndex(AAX_CPageTableParamID iParameterIdentifier, int32_t iIndex, AAX_IString& oNameVariation, int32_t& oLength) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetParameterNameVariationAtIndex()
	AAX_Result GetParameterNameVariationOfLength(AAX_CPageTableParamID iParameterIdentifier, int32_t iLength, AAX_IString& oNameVariation) const AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::GetParameterNameVariationOfLength()
	AAX_Result ClearParameterNameVariations() AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::ClearParameterNameVariations()
	AAX_Result ClearNameVariationsForParameter(AAX_CPageTableParamID iParameterIdentifier) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::ClearNameVariationsForParameter()
	AAX_Result SetParameterNameVariation(AAX_CPageTableParamID iParameterIdentifier, const AAX_IString& iNameVariation, int32_t iLength) AAX_OVERRIDE; ///< \copydoc AAX_IPageTable::SetParameterNameVariation()
	
	// AAX_VPageTable
	
	/** Returns the latest supported versioned ACF interface (e.g. an \ref AAX_IACFPageTable) which
	 is wrapped by this \ref AAX_IPageTable
	 */
	const IACFUnknown* AsUnknown() const
	{
		return mIPageTable.inArg();
	}
	
	/** \copydoc AAX_VPageTable::AsUnknown() const
	 */
	IACFUnknown* AsUnknown()
	{
		return mIPageTable.inArg();
	}
	
	bool IsSupported() const { return !mIPageTable.isNull(); }
	
private:
	ACFPtr<AAX_IACFPageTable>		mIPageTable;
	ACFPtr<AAX_IACFPageTable_V2>	mIPageTable2;
};

#endif
