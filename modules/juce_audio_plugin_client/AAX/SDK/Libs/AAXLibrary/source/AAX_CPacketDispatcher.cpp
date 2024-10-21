/*================================================================================================*/
/*
 *	Copyright 2011-2015, 2018, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_CPacketDispatcher.cpp
 *	
 *	\author Viktor Iarovyi
 *
 */ 
/*================================================================================================*/
#include "AAX_CPacketDispatcher.h"
#include "AAX_IEffectParameters.h"
#include "AAX_IParameter.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////
// class AAX_CPacket implementation

const size_t kDefaultPacketSize = 8;

AAX_CPacket::SPacketData::SPacketData()
{
	mData = new std::vector<char>(kDefaultPacketSize);
}

AAX_CPacket::SPacketData::~SPacketData()
{
	std::vector<char>* storage = static_cast<std::vector<char>*>(mData);
	delete storage;
}

const void*
AAX_CPacket::SPacketData::Get() const
{
	std::vector<char>* storage = static_cast<std::vector<char>*>(mData);
	return &storage->at(0);
}

void*
AAX_CPacket::SPacketData::Get(size_t maxSize) const
{
	std::vector<char>* storage = static_cast<std::vector<char>*>(mData);
	storage->resize(maxSize);
	return &storage->at(0);
}


////////////////////////////////////////////////////////////////////////////////
// class AAX_CPacketHandler

// *******************************************************************************
// METHOD:	AAX_CPacketDispatcher
// *******************************************************************************
AAX_CPacketDispatcher::AAX_CPacketDispatcher()
:	mPacketsHolder(),
	mPacketsHandlers(),
	mController(NULL),
	mEffectParameters(NULL),
	mLockGuard()
{
}

// *******************************************************************************
// METHOD:	~AAX_CPacketDispatcher
// *******************************************************************************
AAX_CPacketDispatcher::~AAX_CPacketDispatcher()
{
	AAX_StLock_Guard guard(this->mLockGuard); 

	// delete registered packets
	for (PacketsHolder::iterator each = mPacketsHolder.begin();  each != mPacketsHolder.end (); each++) 
	{
		AAX_CPacket* packet = each->second;
		delete packet;
	}
		
	// delete registered handlers
	for (PacketsHandlersMap::iterator each = mPacketsHandlers.begin();  each != mPacketsHandlers.end (); each++) 
	{
		AAX_IPacketHandler* handler = each->second.second;
		delete handler;
	}
}

// *******************************************************************************
// METHOD:	Initialize
// *******************************************************************************
void AAX_CPacketDispatcher::Initialize( AAX_IController* inController, AAX_IEffectParameters* inEffectParameters)
{
	mController = inController;
	mEffectParameters = inEffectParameters;
}

// *******************************************************************************
// METHOD:	RegisterTarget
// *******************************************************************************
AAX_Result AAX_CPacketDispatcher::RegisterPacket( AAX_CParamID paramID, AAX_CFieldIndex portID, const AAX_IPacketHandler* inHandler)
{
	AAX_StLock_Guard guard(this->mLockGuard); 
	
	AAX_CPacket* packet = NULL;
	
	// find whether the packet was registered
	if (0 <= portID)
	{
		PacketsHolder::iterator found (mPacketsHolder.find (portID));
		if (found == mPacketsHolder.end())
		{
			packet = new AAX_CPacket(portID);
			mPacketsHolder.insert(std::make_pair(portID, packet));
		}
		else
		{
			packet = found->second;
		}
	}

	// register handler for parameter
	std::pair<AAX_CPacket*, AAX_IPacketHandler* > myPair( packet, inHandler->Clone() );
	mPacketsHandlers.insert(std::make_pair(std::string(paramID), myPair));
	
	return AAX_SUCCESS;
}
	
// *******************************************************************************
// METHOD:	SetDirty
// *******************************************************************************
AAX_Result AAX_CPacketDispatcher::SetDirty(AAX_CParamID paramID, bool inDirty)
{
	AAX_StLock_Guard guard(this->mLockGuard); 

	PacketsHandlersMap::iterator found (mPacketsHandlers.find (paramID));
	for (PacketsHandlersMap::iterator each = found;  each != mPacketsHandlers.end (); each++) 
	{
		if (each->first != paramID)
			break;
		
		AAX_CPacket* packet = each->second.first;
		packet->SetDirty(inDirty);
	}
	
	return AAX_SUCCESS;
}
	
// *******************************************************************************
// METHOD:	Dispatch
// *******************************************************************************
AAX_Result AAX_CPacketDispatcher::Dispatch()
{
	AAX_StLock_Guard guard(this->mLockGuard); 

	AAX_Result	result = AAX_SUCCESS;
	
	for (PacketsHandlersMap::iterator each = mPacketsHandlers.begin();  each != mPacketsHandlers.end (); each++) 
	{
		AAX_CPacket* packet = each->second.first;
		if ( packet->IsDirty() )
		{
			AAX_IPacketHandler* handler = each->second.second;
			if (AAX_SUCCESS == handler->Call( each->first.c_str(), *packet ))
			{	   
				result = mController->PostPacket( packet->GetID(), packet->GetPtr<const void>(), packet->GetSize() );
			}
				
			packet->SetDirty(false);
		}
	}
	
	return result;
}


// *******************************************************************************
// METHOD:	GenerateSingleValuePacket  (simple convenience for single values packets)
// *******************************************************************************
AAX_Result AAX_CPacketDispatcher::GenerateSingleValuePacket( AAX_CParamID inParam, AAX_CPacket& ioPacket)
{
	AAX_IParameter*		parameter;
	AAX_Result			result = mEffectParameters->GetParameter ( inParam, &parameter );

	if ((result == AAX_SUCCESS) && (parameter != 0))
	{
		bool boolValue;
		if(parameter->GetValueAsBool(&boolValue))
		{	
			*ioPacket.GetPtr<int32_t>() = int32_t(boolValue);
			return AAX_SUCCESS;
		}
		
		float floatValue;
		if (parameter->GetValueAsFloat(&floatValue))
		{	
			*ioPacket.GetPtr<float>() = floatValue;
			return AAX_SUCCESS;
		}
		
		int32_t intValue;
		if(parameter->GetValueAsInt32(&intValue))
		{			
			*ioPacket.GetPtr<int32_t>() = intValue;
			return AAX_SUCCESS;
		}
		
		double doubleValue;
		if(parameter->GetValueAsDouble(&doubleValue))
		{			
			*ioPacket.GetPtr<double>() = doubleValue;
			return AAX_SUCCESS;
		}
	}
	
	return AAX_ERROR_NULL_OBJECT;
}

