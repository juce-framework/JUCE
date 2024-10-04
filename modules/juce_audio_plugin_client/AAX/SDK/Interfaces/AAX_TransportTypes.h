/*================================================================================================*/
/*
 *
 *	Copyright 2020-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_TransportTypes.h
 *
 *	\brief Structures, enums and other definitions used in transport
 *
 */ 
/*================================================================================================*/

#ifndef AAX_TransportTypes_h_
#define AAX_TransportTypes_h_
#pragma once

// AAX Includes
#include "AAX.h"

// Standard Library Includes
#include <string>
#include <sstream>

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_HOST
#include AAX_ALIGN_FILE_END

/**
 Helper structure for payload data described transport state information.
 */
struct AAX_TransportStateInfo_V1
{
	AAX_ETransportState	mTransportState;
	AAX_ERecordMode	mRecordMode;
	AAX_CBoolean mIsRecordEnabled;
	AAX_CBoolean mIsRecording;
	AAX_CBoolean mIsLoopEnabled;

	AAX_TransportStateInfo_V1() :
		mTransportState(AAX_eTransportState_Unknown),
		mRecordMode(AAX_eRecordMode_Unknown), 
		mIsRecordEnabled(false), 
		mIsRecording(false), 
		mIsLoopEnabled(false)
	{
		static_assert(sizeof(AAX_TransportStateInfo_V1) == 12, "Invalid size of AAX_TransportStateInfo_V1 struct during compilation!");
	}

	inline std::string ToString() const
	{
		std::stringstream ss;

		ss << "{" << std::endl;
		ss << "\"transport_state\": " << mTransportState << "," << std::endl;
		ss << "\"record_mode\": " << mRecordMode << "," << std::endl;
		ss << "\"is_record_enabled\": " << mIsRecordEnabled << "," << std::endl;
		ss << "\"is_recording\": " << mIsRecording << "," << std::endl;
		ss << "\"is_loop_enabled\": " << mIsLoopEnabled << std::endl;
		ss << "}";

		return ss.str();
	}
};

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_RESET
#include AAX_ALIGN_FILE_END

inline bool operator==(const AAX_TransportStateInfo_V1& state1, const AAX_TransportStateInfo_V1& state2)
{
	return (state1.mTransportState == state2.mTransportState) && (state1.mRecordMode == state2.mRecordMode) &&
		(state1.mIsRecordEnabled == state2.mIsRecordEnabled) && (state1.mIsRecording == state2.mIsRecording) &&
		(state1.mIsLoopEnabled == state2.mIsLoopEnabled);
}

inline bool operator!=(const AAX_TransportStateInfo_V1& state1, const AAX_TransportStateInfo_V1& state2)
{
	return !(state1 == state2);
}

#endif // #ifndef AAX_TransportTypes_h_
