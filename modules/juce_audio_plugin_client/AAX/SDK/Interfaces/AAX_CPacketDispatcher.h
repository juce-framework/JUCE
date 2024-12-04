/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_CPacketDispatcher.h
 *
 *	\brief Helper classes related to posting %AAX packets and handling parameter update events
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CPACKETDISPATCHER_H
#define AAX_CPACKETDISPATCHER_H

#include "AAX.h"
#include "AAX_IController.h"
#include "AAX_CMutex.h"

#include <string>
#include <map>


/** \brief Container for packet-related data
 	
 	\details
	This class collects a number of packet-related data into the same
	object and provides a facility for tracking when the parameter
	is "dirty", i.e. after its value has been updated and before an
	associated packet has not been posted.
*/
class AAX_CPacket
{
public:
	AAX_CPacket(AAX_CFieldIndex inFieldIndex) : mID(inFieldIndex), mDirty(true), mDataSize(0) {}
	~AAX_CPacket() {}
	
	template<typename DataType>
	DataType* GetPtr()
	{
		mDataSize = sizeof(DataType);
		void * data = mPacketData.Get(mDataSize);
		return reinterpret_cast<DataType*> (data);
	}
	
	void			SetDirty(bool iDirty)	{ mDirty = iDirty; };
	bool			IsDirty() const			{ return mDirty; };
	
	AAX_CFieldIndex	GetID() const			{ return mID; };
	uint32_t		GetSize() const			{ return mDataSize; }
	
private:	
	AAX_CFieldIndex	mID;
	bool			mDirty;
	uint32_t		mDataSize;

private:
	struct SPacketData
	{
	public:
		SPacketData();
		~SPacketData();
		const void* Get() const;
		void* Get(size_t newSize) const;
	private:	
		mutable void*	mData;
	} mPacketData;
};

// GetPtr() specialization for void*
template <>
inline const void*
AAX_CPacket::GetPtr<const void>()
{
	return mPacketData.Get();
}


/** \brief Callback container used by \ref AAX_CPacketDispatcher
*/
struct AAX_IPacketHandler
{
	virtual						~AAX_IPacketHandler() {};
	virtual	AAX_IPacketHandler*	Clone() const = 0;
	virtual AAX_Result			Call( AAX_CParamID inParamID, AAX_CPacket& ioPacket ) const = 0;
};

/** \brief Callback container used by \ref AAX_CPacketDispatcher
 */
template<class TWorker>
class AAX_CPacketHandler : public AAX_IPacketHandler
{
	typedef AAX_Result(TWorker::*fPt2Fn)(AAX_CPacket&);
	typedef AAX_Result(TWorker::*fPt2FnEx)(AAX_CParamID, AAX_CPacket&);

public:
	AAX_CPacketHandler( TWorker* iPt2Object, fPt2Fn infPt )
		: pt2Object(iPt2Object), fpt(infPt), fptEx(NULL) {}
	
	AAX_CPacketHandler( TWorker* iPt2Object, fPt2FnEx infPt )
		: pt2Object(iPt2Object), fpt(NULL), fptEx(infPt) {}
	
	AAX_IPacketHandler* Clone() const
	{
		return new AAX_CPacketHandler(*this);
	}
	
	AAX_Result Call( AAX_CParamID inParamID, AAX_CPacket& ioPacket ) const
	{
		if (fptEx)
			return (*pt2Object.*fptEx)( inParamID, ioPacket);
		else if (fpt)
			return (*pt2Object.*fpt)( ioPacket);
		else
			return AAX_ERROR_NULL_OBJECT;
	}
	
protected:	
	TWorker *	pt2Object;	// pointer to object
	fPt2Fn		fpt ;		// pointer to member function
	fPt2FnEx	fptEx ;		// pointer to member function
};


class AAX_IEffectParameters;

/** \brief Helper class for managing %AAX packet posting
 	
 	\details
	This optional class can be used to associate individual parameters with custom update callbacks.
	The update callbacks for all "dirty" parameters are triggered whenever
	\ref AAX_CPacketDispatcher::Dispatch() is called.  The resulting coefficient data is then posted
	to the AAX_IController automatically by the packet dispatcher.
 
	The packet dispatcher supports many-to-one relationships between parameters and handler
	callbacks, so a single callback may be registered for several related parameters.
 
	\sa \ref AAX_CEffectParameters::EffectInit()
 */
class AAX_CPacketDispatcher
{
	typedef std::map<AAX_CFieldIndex, AAX_CPacket*>	PacketsHolder;	
	typedef std::multimap<std::string, std::pair<AAX_CPacket*, AAX_IPacketHandler*> > PacketsHandlersMap;

public:
	AAX_CPacketDispatcher();
	~AAX_CPacketDispatcher();
	
	void Initialize( AAX_IController* iPlugIn, AAX_IEffectParameters* iEffectParameters);
	
	AAX_Result RegisterPacket( AAX_CParamID paramID, AAX_CFieldIndex portID, const AAX_IPacketHandler* iHandler);
	
	template <class TWorker, typename Func>
	AAX_Result RegisterPacket( AAX_CParamID paramID, AAX_CFieldIndex portID,
							  TWorker* iPt2Object, Func infPt)
	{
		AAX_CPacketHandler<TWorker> handler(iPt2Object, infPt);
		return RegisterPacket(paramID, portID, &handler);
	}
	
	AAX_Result RegisterPacket( AAX_CParamID paramID, AAX_CFieldIndex portID)
	{
		AAX_CPacketHandler<AAX_CPacketDispatcher> handler(this, &AAX_CPacketDispatcher::GenerateSingleValuePacket);
		return RegisterPacket(paramID, portID, &handler);
	}
	
	AAX_Result SetDirty(AAX_CParamID paramID, bool iDirty = true);
	
	AAX_Result Dispatch();
	
	AAX_Result	GenerateSingleValuePacket( AAX_CParamID iParam, AAX_CPacket& ioPacket);
	
private:
	PacketsHolder			mPacketsHolder;
	PacketsHandlersMap		mPacketsHandlers;
	AAX_IController*		mController;
	AAX_IEffectParameters*	mEffectParameters;

	AAX_CMutex				mLockGuard;
};


#endif // AAX_CPACKETDISPATCHER_H
