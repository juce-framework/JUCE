//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/fobject.cpp
// Created by  : Steinberg, 2008
// Description : Basic Object implementing FUnknown
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2021, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "base/source/fobject.h"
#include "base/thread/include/flock.h"

#include <vector>

namespace Steinberg {

IUpdateHandler* FObject::gUpdateHandler = nullptr;

//------------------------------------------------------------------------
const FUID FObject::iid;

//------------------------------------------------------------------------
struct FObjectIIDInitializer
{
	// the object iid is always generated so that different components
	// only can cast to their own objects
	// this initializer must be after the definition of FObject::iid, otherwise
	//  the default constructor of FUID will clear the generated iid
	FObjectIIDInitializer () 
	{ 
		const_cast<FUID&> (FObject::iid).generate (); 
	}
} gFObjectIidInitializer;

//------------------------------------------------------------------------
uint32 PLUGIN_API FObject::addRef ()  
{                                      
	return FUnknownPrivate::atomicAdd (refCount, 1);
}                                             

//------------------------------------------------------------------------
uint32 PLUGIN_API FObject::release () 
{
	if (FUnknownPrivate::atomicAdd (refCount, -1) == 0)
	{
		refCount = -1000;
		delete this;
		return 0;
	}                                   
	return refCount;                 
}

//------------------------------------------------------------------------
tresult PLUGIN_API FObject::queryInterface (const TUID _iid, void** obj)
{
	QUERY_INTERFACE (_iid, obj, FUnknown::iid, FUnknown)             
	QUERY_INTERFACE (_iid, obj, IDependent::iid, IDependent)             
	QUERY_INTERFACE (_iid, obj, FObject::iid, FObject)             
	*obj = nullptr;
	return kNoInterface; 
}

//------------------------------------------------------------------------
void FObject::addDependent (IDependent* dep)
{
	if (gUpdateHandler)
		gUpdateHandler->addDependent (unknownCast (), dep);
}

//------------------------------------------------------------------------
void FObject::removeDependent (IDependent* dep)
{
	if (gUpdateHandler)
		gUpdateHandler->removeDependent (unknownCast (), dep);
}

//------------------------------------------------------------------------
void FObject::changed (int32 msg)
{
	if (gUpdateHandler)
		gUpdateHandler->triggerUpdates (unknownCast (), msg);
	else
		updateDone (msg);
}

//------------------------------------------------------------------------
void FObject::deferUpdate (int32 msg)
{
	if (gUpdateHandler)
		gUpdateHandler->deferUpdates (unknownCast (), msg);
	else
		updateDone (msg);
}

//------------------------------------------------------------------------
/** Automatic creation and destruction of singleton instances. */
//------------------------------------------------------------------------
namespace Singleton 
{
	using ObjectVector = std::vector<FObject**>;
	ObjectVector* singletonInstances = nullptr;
	bool singletonsTerminated = false;
	Steinberg::Base::Thread::FLock* singletonsLock;

	bool isTerminated () {return singletonsTerminated;}

	void lockRegister ()
	{
		if (!singletonsLock) // assume first call not from multiple threads
			singletonsLock = NEW Steinberg::Base::Thread::FLock;
		singletonsLock->lock ();
	}
	void unlockRegister () 
	{ 
		singletonsLock->unlock ();
	}

	void registerInstance (FObject** o)
	{
		SMTG_ASSERT (singletonsTerminated == false)
		if (singletonsTerminated == false)
		{
			if (singletonInstances == nullptr)
				singletonInstances = NEW std::vector<FObject**>;
			singletonInstances->push_back (o);
		}
	}

	struct Deleter
	{
		~Deleter ()
		{
			singletonsTerminated = true;
			if (singletonInstances)
			{
				for (ObjectVector::iterator it = singletonInstances->begin (),
											end = singletonInstances->end ();
					 it != end; ++it)
				{
					FObject** obj = (*it);
					(*obj)->release ();
					*obj = nullptr;
					obj = nullptr;
				}

				delete singletonInstances;
				singletonInstances = nullptr;
			}
			delete singletonsLock;
			singletonsLock = nullptr;
		}
	} deleter;
}

//------------------------------------------------------------------------
} // namespace Steinberg
