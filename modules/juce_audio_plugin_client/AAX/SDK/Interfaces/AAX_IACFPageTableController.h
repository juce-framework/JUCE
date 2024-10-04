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

#ifndef AAXLibrary_AAX_IACFPageTableController_h
#define AAXLibrary_AAX_IACFPageTableController_h

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"


/** \brief Interface for host operations related to the page tables for this plug-in
 
 \note In the %AAX Library, access to this interface is provided through \ref AAX_IController
 */
class AAX_IACFPageTableController : public IACFUnknown
{
public:
	// NOTE: Documentation is not copied directly from AAX_IController due to an adaptation of parameter types (IACFUnknown to AAX_IPageTable)
	/**
	 *	\copybrief AAX_IController::CreateTableCopyForEffect()
	 *
	 *	The host will reject the copy and return an error if the requested plug-in type is unkown, if
	 *	\p inTableType is unknown or if \p inTablePageSize is not a supported size for the given table type.
	 *
	 *	The host may also restrict plug-ins to only copying page table data from certain plug-in types,
	 *	such as plug-ins from the same manufacturer or plug-in types within the same effect.
	 *
	 *	See \ref AAX_Page_Table_Guide for more information about page tables.
	 *
	 *	\returns \ref AAX_ERROR_NULL_ARGUMENT if \p oPageTable is null
	 *
	 *	\returns \ref AAX_ERROR_INVALID_ARGUMENT if no valid page table mapping can be created due to the
	 *	specified arguments
	 *
	 *	\param[in] inManufacturerID
	 *		\ref AAX_eProperty_ManufacturerID "Manufacturer ID" of the desired plug-in type
	 *	\param[in] inProductID
	 *		\ref AAX_eProperty_ProductID "Product ID" of the desired plug-in type
	 *	\param[in] inPlugInID
	 *		Type ID of the desired plug-in type (\ref AAX_eProperty_PlugInID_Native, \ref AAX_eProperty_PlugInID_TI)
	 *	\param[in] inTableType
	 *		Four-char type identifier for the requested table type (e.g. \c 'PgTL', \c 'Av81', etc.)
	 *	\param[in] inTablePageSize
	 *		Page size for the requested table. Some tables support multiple page sizes.
	 *	\param[out] oPageTable
	 *		The page table object to which the page table data should be copied. \p oPageTable must support \ref AAX_IACFPageTable
	 *
	 *	\sa AAX_IController::CreateTableCopyForEffect()
	 *
	 */
	virtual
	AAX_Result
	CopyTableForEffect(
		AAX_CPropertyValue inManufacturerID,
		AAX_CPropertyValue inProductID,
		AAX_CPropertyValue inPlugInID,
		uint32_t inTableType,
		int32_t inTablePageSize,
		IACFUnknown* oPageTable) const = 0;
	
	// NOTE: Documentation is not copied directly from AAX_IController due to an adaptation of parameter types (IACFUnknown to AAX_IPageTable)
	/**
	 *	\copybrief AAX_IController::CreateTableCopyForLayout()
	 *
	 *	The host will reject the copy and return an error if the requested effect ID is unkown or if
	 *	\p inLayoutName is not a valid layout name for the page tables registered for the effect.
	 *
	 *	The host may also restrict plug-ins to only copying page table data from certain effects,
	 *	such as effects registered within the current AAX plug-in bundle.
	 *
	 *	See \ref AAX_Page_Table_Guide for more information about page tables.
	 *
	 *	\returns \ref AAX_ERROR_NULL_ARGUMENT if \p inEffectID, \p inLayoutName, or \p oPageTable is null
	 *
	 *	\returns \ref AAX_ERROR_INVALID_ARGUMENT if no valid page table mapping can be created due to the
	 *	specified arguments
	 *
	 *	\param[in] inEffectID
	 *		Effect ID for the desired effect. See \ref AAX_ICollection::AddEffect()
	 *	\param[in] inLayoutName
	 *		Page table layout name ("name" attribute of the \c PTLayout XML tag)
	 *	\param[in] inTableType
	 *		Four-char type identifier for the requested table type (e.g. \c 'PgTL', \c 'Av81', etc.)
	 *	\param[in] inTablePageSize
	 *		Page size for the requested table. Some tables support multiple page sizes.
	 *	\param[out] oPageTable
	 *		The page table object to which the page table data should be copied. \p oPageTable must support \ref AAX_IACFPageTable
	 *
	 *	\sa AAX_IController::CreateTableCopyForLayout()
	 */
	virtual
	AAX_Result
	CopyTableOfLayoutForEffect(
		const char * inEffectID,
		const char * inLayoutName,
		uint32_t inTableType,
		int32_t inTablePageSize,
		IACFUnknown* oPageTable) const = 0;
};

/** @copydoc AAX_IACFPageTableController
*/
class AAX_IACFPageTableController_V2 : public AAX_IACFPageTableController
{
public:
	/** \copybrief AAX_IACFPageTableController::CopyTableForEffect()
	 *
	 *	\returns \ref AAX_ERROR_NULL_ARGUMENT if \p inPageTableFilePath or \p oPageTable is null
	 *
	 *	\returns \ref AAX_ERROR_UNSUPPORTED_ENCODING if \p inFilePathEncoding has unsupported encoding value
	 *
	 *	\returns \ref AAX_ERROR_INVALID_ARGUMENT if no valid page table mapping can be created due to the
	 *	specified arguments
	 *
	 *	\param[in] inPageTableFilePath
	 *		Path to XML page table file.
	 *	\param[in] inFilePathEncoding
	 *		File path text encoding.
	 *	\param[in] inManufacturerID
	 *		\ref AAX_eProperty_ManufacturerID "Manufacturer ID" of the desired plug-in type
	 *	\param[in] inProductID
	 *		\ref AAX_eProperty_ProductID "Product ID" of the desired plug-in type
	 *	\param[in] inPlugInID
	 *		Type ID of the desired plug-in type (\ref AAX_eProperty_PlugInID_Native, \ref AAX_eProperty_PlugInID_TI)
	 *	\param[in] inTableType
	 *		Four-char type identifier for the requested table type (e.g. \c 'PgTL', \c 'Av81', etc.)
	 *	\param[in] inTablePageSize
	 *		Page size for the requested table. Some tables support multiple page sizes.
	 *	\param[out] oPageTable
	 *		The page table object to which the page table data should be copied. \p oPageTable must support \ref AAX_IACFPageTable
	 *
	 *	\sa AAX_IController::CreateTableCopyForEffect()
	 */
	virtual
	AAX_Result
	CopyTableForEffectFromFile(
		const char* inPageTableFilePath,
		AAX_ETextEncoding inFilePathEncoding,
		AAX_CPropertyValue inManufacturerID,
		AAX_CPropertyValue inProductID,
		AAX_CPropertyValue inPlugInID,
		uint32_t inTableType,
		int32_t inTablePageSize,
		IACFUnknown* oPageTable) const = 0;

	/** \copybrief AAX_IACFPageTableController::CopyTableOfLayoutForEffect()
	 *
	 *	\returns \ref AAX_ERROR_NULL_ARGUMENT if \p inPageTableFilePath, \p inLayoutName, or \p oPageTable is null
	 *
	 *	\returns \ref AAX_ERROR_UNSUPPORTED_ENCODING if \p inFilePathEncoding has unsupported encoding value
	 *
	 *	\returns \ref AAX_ERROR_INVALID_ARGUMENT if no valid page table mapping can be created due to the specified arguments
	 *
	 *	\param[in] inPageTableFilePath
	 *		Path to XML page table file.
	 *	\param[in] inFilePathEncoding
	 *		File path text encoding.
	 *	\param[in] inLayoutName
	 *		Page table layout name ("name" attribute of the \c PTLayout XML tag)
	 *	\param[in] inTableType
	 *		Four-char type identifier for the requested table type (e.g. \c 'PgTL', \c 'Av81', etc.)
	 *	\param[in] inTablePageSize
	 *		Page size for the requested table. Some tables support multiple page sizes.
	 *	\param[out] oPageTable
	 *		The page table object to which the page table data should be copied. \p oPageTable must support \ref AAX_IACFPageTable
	 *
	 *	\sa AAX_IController::CreateTableCopyForLayout()
	 */
	virtual
	AAX_Result
	CopyTableOfLayoutFromFile(
		const char* inPageTableFilePath,
		AAX_ETextEncoding inFilePathEncoding,
		const char* inLayoutName,
		uint32_t inTableType,
		int32_t inTablePageSize,
		IACFUnknown* oPageTable) const = 0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif
