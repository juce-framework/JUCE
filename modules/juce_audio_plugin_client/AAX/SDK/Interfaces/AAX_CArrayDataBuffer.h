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
 * \file  AAX_CArrayDataBuffer.h
 */ 
/*================================================================================================*/

#pragma once

#ifndef AAX_CArrayDataBuffer_H
#define AAX_CArrayDataBuffer_H

#include "AAX_IDataBuffer.h"
#include "AAX.h"

#include <string>
#include <limits>
#include <type_traits>


/**
 * \brief A convenience class for array data buffers
 * 
 * The data payload is an array of \c D
 */
template <AAX_CTypeID T, class D>
class AAX_CArrayDataBufferOfType : public AAX_IDataBuffer
{
public:
	explicit AAX_CArrayDataBufferOfType (std::vector<D> const & inData) : mData{inData} {}
	explicit AAX_CArrayDataBufferOfType (std::vector<D> && inData) : mData{inData} {}

	AAX_CArrayDataBufferOfType(AAX_CArrayDataBufferOfType const &) = default;
	AAX_CArrayDataBufferOfType(AAX_CArrayDataBufferOfType &&) = default;

	~AAX_CArrayDataBufferOfType (void) AAX_OVERRIDE = default;
	
	AAX_CArrayDataBufferOfType& operator= (AAX_CArrayDataBufferOfType const & other) = default;
	AAX_CArrayDataBufferOfType& operator= (AAX_CArrayDataBufferOfType && other) = default;

	AAX_Result Type(AAX_CTypeID * oType) const AAX_OVERRIDE {
		if (!oType) { return AAX_ERROR_NULL_ARGUMENT; }
		*oType = T;
		return AAX_SUCCESS;
	}
	AAX_Result Size(int32_t * oSize) const AAX_OVERRIDE {
		if (!oSize) { return AAX_ERROR_NULL_ARGUMENT; }
		auto const size = mData.size() * sizeof(D);
		static_assert(std::numeric_limits<decltype(size)>::max() >= std::numeric_limits<std::remove_pointer<decltype(oSize)>::type>::max(),
			"size variable may not represent all positive values of oSize");
		if (size > std::numeric_limits<std::remove_pointer<decltype(oSize)>::type>::max()) {
			return AAX_ERROR_SIGNED_INT_OVERFLOW;
		}
		*oSize = static_cast<std::remove_pointer<decltype(oSize)>::type>(size);
		return AAX_SUCCESS;
	}
	AAX_Result Data(void const ** oBuffer) const AAX_OVERRIDE {
		if (!oBuffer) { return AAX_ERROR_NULL_ARGUMENT; }
		*oBuffer = mData.data();
		return AAX_SUCCESS;
	}
private:
	std::vector<D> const mData;
};

/**
 * \copydoc AAX_CArrayDataBufferOfType
 */
template <class D>
class AAX_CArrayDataBuffer : public AAX_IDataBuffer
{
public:
	AAX_CArrayDataBuffer (AAX_CTypeID inType, std::vector<D> const & inData) : mType{inType}, mData{inData} {}
	AAX_CArrayDataBuffer (AAX_CTypeID inType, std::vector<D> && inData) : mType{inType}, mData{inData} {}

	AAX_CArrayDataBuffer(AAX_CArrayDataBuffer const &) = default;
	AAX_CArrayDataBuffer(AAX_CArrayDataBuffer &&) = default;

	~AAX_CArrayDataBuffer (void) AAX_OVERRIDE = default;
	
	AAX_CArrayDataBuffer& operator= (AAX_CArrayDataBuffer const & other) = default;
	AAX_CArrayDataBuffer& operator= (AAX_CArrayDataBuffer && other) = default;

	AAX_Result Type(AAX_CTypeID * oType) const AAX_OVERRIDE {
		if (!oType) { return AAX_ERROR_NULL_ARGUMENT; }
		*oType = mType;
		return AAX_SUCCESS;
	}
	AAX_Result Size(int32_t * oSize) const AAX_OVERRIDE {
		if (!oSize) { return AAX_ERROR_NULL_ARGUMENT; }
		auto const size = mData.size() * sizeof(D);
		static_assert(std::numeric_limits<decltype(size)>::max() >= std::numeric_limits<std::remove_pointer<decltype(oSize)>::type>::max(),
			"size variable may not represent all positive values of oSize");
		if (size > std::numeric_limits<std::remove_pointer<decltype(oSize)>::type>::max()) {
			return AAX_ERROR_SIGNED_INT_OVERFLOW;
		}
		*oSize = static_cast<std::remove_pointer<decltype(oSize)>::type>(size);
		return AAX_SUCCESS;
	}
	AAX_Result Data(void const ** oBuffer) const AAX_OVERRIDE {
		if (!oBuffer) { return AAX_ERROR_NULL_ARGUMENT; }
		*oBuffer = mData.data();
		return AAX_SUCCESS;
	}
private:
	AAX_CTypeID const mType;
	std::vector<D> const mData;
};

#endif
