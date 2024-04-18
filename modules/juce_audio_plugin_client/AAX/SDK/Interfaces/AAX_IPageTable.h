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
 */

#ifndef AAXLibrary_AAX_IPageTable_h
#define AAXLibrary_AAX_IPageTable_h

#include "AAX.h"
#include "AAX_IString.h"

class IACFUnknown;


/** \brief Interface to the host's representation of a plug-in instance's page table
 
 \sa \ref AAX_IEffectParameters::UpdatePageTable()
 */
class AAX_IPageTable
{
public:
	
	/** \brief Virtual destructor
	 *
	 *	\note This destructor MUST be virtual to prevent memory leaks.
	 */
	virtual	~AAX_IPageTable()	{ }
	
	//
	// AAX_IACFPageTable
	//
	
	/** \brief Clears all parameter mappings from the table
	 
	 This method does not clear any parameter name variations from the table. For that, use
	 \ref AAX_IPageTable::ClearParameterNameVariations() or
	 \ref AAX_IPageTable::ClearNameVariationsForParameter()
	 */
	virtual AAX_Result Clear() = 0;
	
	/** \brief Indicates whether the table is empty
	 
	 A table is empty if it contains no pages. A table which contains pages but no parameter
	 assignments is not empty. A table which has associated parameter name variations but no pages
	 is empty.
	 
	 \param[out] oEmpty
		\c true if this table is empty
	 */
	virtual AAX_Result Empty(AAX_CBoolean& oEmpty) const = 0;
	
	/** \brief Get the number of pages currently in this table
	 
	 \param[out] oNumPages
		The number of pages which are present in the page table. Some pages might not contain any
		parameter assignments.
	 */
	virtual AAX_Result GetNumPages(int32_t& oNumPages) const = 0;
	
	/** \brief Insert a new empty page before the page at index \c iPage
	 
	 \returns \ref AAX_ERROR_INVALID_ARGUMENT if \p iPage is greater than the total number of pages
	 
	 \param[in] iPage
		The insertion point page index
	 */
	virtual AAX_Result InsertPage(int32_t iPage) = 0;
	
	/** \brief Remove the page at index \c iPage
	 
	 \returns \ref AAX_ERROR_INVALID_ARGUMENT if \p iPage is greater than the index of the last existing page
	 
	 \param[in] iPage
		The target page index
	 */
	virtual AAX_Result RemovePage(int32_t iPage) = 0;
	
	/** \brief Returns the total number of parameter IDs which are mapped to a page
	 
	 \note The number of mapped parameter IDs does not correspond to the actual slot indices of
	 the parameter assignments. For example, a page could have three total parameter assignments
	 with parameters mapped to slots 2, 4, and 6.
	 
	 \returns \ref AAX_ERROR_INVALID_ARGUMENT if \p iPage is greater than the index of the last existing page
	 
	 \param[in] iPage
		The target page index
	 \param[out] oNumParameterIdentifiers
		The number of parameter identifiers which are mapped to the target page
	 */
	virtual AAX_Result GetNumMappedParameterIDs(int32_t iPage, int32_t& oNumParameterIdentifiers) const = 0;
	
	/** \brief Clear the parameter at a particular index in this table
	 
	 \returns \ref AAX_SUCCESS even if no parameter was mapped at the given index (the index is still clear)
	 
	 \param[in] iPage
		The target page index
	 \param[in] iIndex
		The target parameter slot index within the target page
	 */
	virtual AAX_Result ClearMappedParameter(int32_t iPage, int32_t iIndex) = 0;
	
	/** \brief Get the parameter identifier which is currently mapped to an index in this table
	 
	 \returns \ref AAX_ERROR_INVALID_ARGUMENT if no parameter is mapped at the specified page/index location
	 
	 \param[in] iPage
		The target page index
	 \param[in] iIndex
		The target parameter slot index within the target page
	 \param[out] oParameterIdentifier
		The identifier used for the mapped parameter in the page table (may be parameter name or ID)
	 */
	virtual AAX_Result GetMappedParameterID(int32_t iPage, int32_t iIndex, AAX_IString& oParameterIdentifier) const = 0;
	
	/** \brief Map a parameter to this table
	 
	 If \p iParameterIdentifier is an empty string then the parameter assignment will be cleared
	 
	 \returns \ref AAX_ERROR_NULL_ARGUMENT if \p iParameterIdentifier is null
	 
	 \returns \ref AAX_ERROR_INVALID_ARGUMENT if \p iPage is greater than the index of the last existing page
	 
	 \returns \ref AAX_ERROR_INVALID_ARGUMENT if \p iIndex is negative
	 
	 \param[in] iParameterIdentifier
		The identifier for the parameter which will be mapped
	 \param[in] iPage
		The target page index
	 \param[in] iIndex
		The target parameter slot index within the target page
	 */
	virtual AAX_Result MapParameterID(AAX_CPageTableParamID iParameterIdentifier, int32_t iPage, int32_t iIndex) = 0;
	
	/** Get the number of parameters with name variations defined for the current table type
	 
	 Provides the number of parameters with <TT>%lt;ControlNameVariations%lt;</TT> which are explicitly
	 defined for the current page table type.
	 
	 \note Normally parameter name variations are only used with the <TT>'PgTL'</TT> table type
	 
	 - \sa \ref AAX_IPageTable::GetNameVariationParameterIDAtIndex()
	 
	 \param[out] oNumParameterIdentifiers
		The number of parameters with name variations explicitly associated with the current table type.
	 
	 */
	virtual AAX_Result GetNumParametersWithNameVariations(int32_t& oNumParameterIdentifiers) const = 0;
	
	/** Get the identifier for a parameter with name variations defined for the current table type
	 
	 \note Normally parameter name variations are only used with the <TT>'PgTL'</TT> table type
	 
	 - \sa \ref AAX_IPageTable::GetNumParametersWithNameVariations()
	 
	 \param[in] iIndex
		The target parameter index within the list of parameters with explicit name variations defined
		for this table type.
	 \param[out] oParameterIdentifier
		The identifier used for the parameter in the page table name variations list (may be parameter
		name or ID)
	 
	 */
	virtual AAX_Result GetNameVariationParameterIDAtIndex(int32_t iIndex, AAX_IString& oParameterIdentifier) const = 0;
	
	/** Get the number of name variations defined for a parameter
	 
	 Provides the number of <TT>%lt;ControlNameVariations%lt;</TT> which are explicitly defined for
	 \p iParameterIdentifier for the current page table type. No fallback logic is used to resolve this to
	 the list of variations which would actually be used for an attached control surface if no explicit
	 variations are defined for the current table type.
	 
	 \note Normally parameter name variations are only used with the <TT>'PgTL'</TT> table type
	 
	 - \sa \ref AAX_IPageTable::GetParameterNameVariationAtIndex()
	 
	 \returns \ref AAX_SUCCESS and provides zero to \p oNumVariations if \p iParameterIdentifier is not
	 found
	 
	 \param[in] iParameterIdentifier
		The identifier for the parameter
	 \param[out] oNumVariations
		The number of name variations which are defined for this parameter and explicitly associated with
		the current table type.
	 
	 */
	virtual AAX_Result GetNumNameVariationsForParameter(AAX_CPageTableParamID iParameterIdentifier, int32_t& oNumVariations) const = 0;
	
	/** Get a parameter name variation from the page table
	 
	 Only returns <TT>%lt;ControlNameVariations%lt;</TT> which are explicitly defined for the
	 current page table type. No fallback logic is used to resolve this to the abbreviation which would
	 actually be shown on an attached control surface if no explicit variation is defined for the current
	 table type.
	 
	 \note Normally parameter name variations are only used with the <TT>'PgTL'</TT> table type
	 
	 - \sa \ref AAX_IPageTable::GetNumNameVariationsForParameter()
	 - \sa \ref AAX_IPageTable::GetParameterNameVariationOfLength()
	 
	 \returns \ref AAX_ERROR_NO_ABBREVIATED_PARAMETER_NAME if no suitable variation is defined for this table
	 
	 \returns \ref AAX_ERROR_ARGUMENT_OUT_OF_RANGE if \p iIndex is out of range
	 
	 \param[in] iParameterIdentifier
		The identifier for the parameter
	 \param[in] iIndex
		Index of the name variation
	 \param[out] oNameVariation
		The name variation, if one is explicitly defined for this table type
	 \param[out] oLength
		The length value for this name variation. This corresponds to the variation's <TT>sz</TT> attribute
		in the page table XML and may be different from the string length of \p iNameVariation.
	 */
	virtual AAX_Result GetParameterNameVariationAtIndex(AAX_CPageTableParamID iParameterIdentifier, int32_t iIndex, AAX_IString& oNameVariation, int32_t& oLength) const = 0;
	
	/** Get a parameter name variation of a particular length from the page table
	 
	 Only returns <TT>%lt;ControlNameVariations%lt;</TT> which are explicitly defined of \p iLength for the
	 current page table type. No fallback logic is used to resolve this to the abbreviation which would
	 actually be shown on an attached control surface if no explicit variation is defined for the specified
	 length or current table type.
	 
	 \note Normally parameter name variations are only used with the <TT>'PgTL'</TT> table type
	 
	 - \sa \ref AAX_IPageTable::GetParameterNameVariationAtIndex()
	 
	 \returns \ref AAX_ERROR_NO_ABBREVIATED_PARAMETER_NAME if no suitable variation is defined for this table
	 
	 \param[in] iParameterIdentifier
		The identifier for the parameter
	 \param[in] iLength
		The variation length to check, i.e. the \c sz attribute for the name variation in the page table XML
	 \param[out] oNameVariation
		The name variation, if one is explicitly defined for this table type and \p iLength
	 */
	virtual AAX_Result GetParameterNameVariationOfLength(AAX_CPageTableParamID iParameterIdentifier, int32_t iLength, AAX_IString& oNameVariation) const = 0;
	
	/** Clears all name variations for the current page table type
	 
	 \note Normally parameter name variations are only used with the <TT>'PgTL'</TT> table type
	 
	 \sa \ref AAX_IPageTable::Clear()
	 \sa \ref AAX_IPageTable::ClearNameVariationsForParameter()
	 */
	virtual AAX_Result ClearParameterNameVariations() = 0;
	
	/** Clears all name variations for a single parameter for the current page table type
	 
	 \warning This will invalidate the list of parameter name variations indices, i.e. the parameter
	 identifier associated with each index by \ref AAX_IPageTable::GetNameVariationParameterIDAtIndex()
	 
	 \note Normally parameter name variations are only used with the <TT>'PgTL'</TT> table type
	 
	 \sa \ref AAX_IPageTable::Clear()
	 \sa \ref AAX_IPageTable::ClearParameterNameVariations()
	 
	 \returns \ref AAX_SUCCESS and provides zero to \p oNumVariations if \p iParameterIdentifier is not
	 found
	 
	 \param[in] iParameterIdentifier
		The identifier for the parameter
	 */
	virtual AAX_Result ClearNameVariationsForParameter(AAX_CPageTableParamID iParameterIdentifier) = 0;
	
	/** Sets a name variation explicitly for the current page table type
	 
	 This will add a new name variation or overwrite the existing name variation with the same length which
	 is defined for the current table type.
	 
	 \warning If no name variation previously existed for this parameter then this will invalidate the list
	 of parameter name variations indices, i.e. the parameter identifier associated with each index by
	 \ref AAX_IPageTable::GetNameVariationParameterIDAtIndex()
	 
	 \note Normally parameter name variations are only used with the <TT>'PgTL'</TT> table type
	 
	 \returns AAX_ERROR_INVALID_ARGUMENT if \p iNameVariation is empty or if \p iLength is less than zero
	 
	 \param[in] iParameterIdentifier
		The identifier for the parameter
	 \param[in] iNameVariation
		The new parameter name variation
	 \param[in] iLength
		The length value for this name variation. This corresponds to the variation's <TT>sz</TT> attribute
		in the page table XML and is not required to match the length of \p iNameVariation.
	 */
	virtual AAX_Result SetParameterNameVariation(AAX_CPageTableParamID iParameterIdentifier, const AAX_IString& iNameVariation, int32_t iLength) = 0;
};

#endif
