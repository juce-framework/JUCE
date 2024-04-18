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
 * \file  AAX_CStringDataBuffer.h
 */ 
/*================================================================================================*/

#pragma once

#ifndef AAX_CStringDataBuffer_H
#define AAX_CStringDataBuffer_H

#include "AAX_IDataBuffer.h"
#include "AAX.h"

#include <string>
#include <limits>
#include <type_traits>


/**
 * \brief A convenience class for string data buffers
 * 
 * The data payload is a \c char* C string
 */
template <AAX_CTypeID T>
class AAX_CStringDataBufferOfType : public AAX_IDataBuffer
{
public:
	explicit AAX_CStringDataBufferOfType (std::string const & inData) : mData{inData} {}
	explicit AAX_CStringDataBufferOfType (std::string && inData) : mData{inData} {}
	explicit AAX_CStringDataBufferOfType (const char * inData) : mData{inData ? std::string{inData} : std::string{}} {}

	AAX_CStringDataBufferOfType(AAX_CStringDataBufferOfType const &) = delete;
	AAX_CStringDataBufferOfType(AAX_CStringDataBufferOfType &&) = delete;

	~AAX_CStringDataBufferOfType (void) AAX_OVERRIDE = default;
	
	AAX_CStringDataBufferOfType& operator= (AAX_CStringDataBufferOfType const & other) = delete;
	AAX_CStringDataBufferOfType& operator= (AAX_CStringDataBufferOfType && other) = delete;

	AAX_Result Type(AAX_CTypeID * oType) const AAX_OVERRIDE {
		if (!oType) { return AAX_ERROR_NULL_ARGUMENT; }
		*oType = T;
		return AAX_SUCCESS;
	}
	AAX_Result Size(int32_t * oSize) const AAX_OVERRIDE {
		if (!oSize) { return AAX_ERROR_NULL_ARGUMENT; }
		auto const size = mData.size() + 1; // null termination
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
		*oBuffer = mData.c_str();
		return AAX_SUCCESS;
	}
private:
	std::string mData;
};

/**
 * \copydoc AAX_CStringDataBufferOfType
 */
class AAX_CStringDataBuffer : public AAX_IDataBuffer
{
public:
	AAX_CStringDataBuffer (AAX_CTypeID inType, std::string const & inData) : mType{inType}, mData{inData} {}
	AAX_CStringDataBuffer (AAX_CTypeID inType, std::string && inData) : mType{inType}, mData{inData} {}
	AAX_CStringDataBuffer (AAX_CTypeID inType, const char * inData) : mType{inType}, mData{inData ? std::string{inData} : std::string{}} {}

	AAX_CStringDataBuffer(AAX_CStringDataBuffer const &) = delete;
	AAX_CStringDataBuffer(AAX_CStringDataBuffer &&) = delete;

	~AAX_CStringDataBuffer (void) AAX_OVERRIDE = default;
	
	AAX_CStringDataBuffer& operator= (AAX_CStringDataBuffer const & other) = delete;
	AAX_CStringDataBuffer& operator= (AAX_CStringDataBuffer && other) = delete;

	AAX_Result Type(AAX_CTypeID * oType) const AAX_OVERRIDE {
		if (!oType) { return AAX_ERROR_NULL_ARGUMENT; }
		*oType = mType;
		return AAX_SUCCESS;
	}
	AAX_Result Size(int32_t * oSize) const AAX_OVERRIDE {
		if (!oSize) { return AAX_ERROR_NULL_ARGUMENT; }
		auto const size = mData.size() + 1; // null termination
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
		*oBuffer = mData.c_str();
		return AAX_SUCCESS;
	}
private:
	AAX_CTypeID const mType;
	std::string mData;
};

#endif
