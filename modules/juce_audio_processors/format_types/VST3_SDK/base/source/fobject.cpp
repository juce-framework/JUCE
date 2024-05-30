//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
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
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
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
#define SMTG_VALIDATE_DEPENDENCY_COUNT DEVELOPMENT // validating dependencyCount

#if SMTG_DEPENDENCY_COUNT
#include "base/source/updatehandler.h"
#define SMTG_DEPENDENCY_CHECK_LEVEL 1 // 1 => minimal assert, 2 => full assert
#endif // SMTG_DEPENDENCY_COUNT

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
	FObjectIIDInitializer () { const_cast<FUID&> (FObject::iid).generate (); }
} gFObjectIidInitializer;

//------------------------------------------------------------------------
FObject::~FObject ()
{
#if SMTG_DEPENDENCY_COUNT && DEVELOPMENT
	static bool localNeverDebugger = false;
#endif

#if DEVELOPMENT
	if (refCount > 1)
		FDebugPrint ("Refcount is %d when trying to delete %s\n", refCount, isA ());
#endif

#if SMTG_DEPENDENCY_COUNT
#if SMTG_DEPENDENCY_CHECK_LEVEL >= 1
	if (gUpdateHandler)
	{
#if DEVELOPMENT
		SMTG_ASSERT (dependencyCount == 0 || localNeverDebugger);
#endif // DEVELOPMENT
	}
#endif
#endif // SMTG_DEPENDENCY_COUNT

#if SMTG_VALIDATE_DEPENDENCY_COUNT
	if (!gUpdateHandler || gUpdateHandler != UpdateHandler::instance (false))
		return;

	auto updateHandler = UpdateHandler::instance ();
	if (!updateHandler || updateHandler == this)
		return;

	SMTG_ASSERT ((updateHandler->checkDeferred (this) == false || localNeverDebugger) &&
	             "'this' has scheduled a deferUpdate that was not yet delivered");

	if (updateHandler->hasDependencies (this))
	{
		SMTG_ASSERT (
		    (false || localNeverDebugger) &&
		    "Another object is still dependent on 'this'. This leads to zombie entries in the dependency map that can later crash.");
		FDebugPrint ("Object still has dependencies %x %s\n", this, this->isA ());
		updateHandler->printForObject (this);
	}
#endif // SMTG_VALIDATE_DEPENDENCY_COUNT
}

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
	if (!gUpdateHandler)
		return;

	gUpdateHandler->addDependent (unknownCast (), dep);
#if SMTG_DEPENDENCY_COUNT
	dependencyCount++;
#endif
}

//------------------------------------------------------------------------
void FObject::removeDependent (IDependent* dep)
{
#if SMTG_DEPENDENCY_COUNT && DEVELOPMENT
	static bool localNeverDebugger = false;
#endif

	if (!gUpdateHandler)
		return;

#if SMTG_DEPENDENCY_COUNT
	if (gUpdateHandler != UpdateHandler::instance (false))
	{
		gUpdateHandler->removeDependent (unknownCast (), dep);
		dependencyCount--;
		return;
	}
#if SMTG_DEPENDENCY_CHECK_LEVEL > 1
	SMTG_ASSERT ((dependencyCount > 0 || localNeverDebugger) &&
	             "All dependencies have already been removed - mmichaelis 7/2021");
#endif
	size_t removeCount;
	UpdateHandler::instance ()->removeDependent (unknownCast (), dep, removeCount);
	if (removeCount == 0)
	{
#if SMTG_DEPENDENCY_CHECK_LEVEL > 1
		SMTG_ASSERT (localNeverDebugger && "No dependency to remove - ygrabit 8/2021");
#endif
	}
	else
	{
		SMTG_ASSERT ((removeCount == 1 || localNeverDebugger) &&
		             "Duplicated dependencies established - mmichaelis 7/2021");
	}
	dependencyCount -= (int16)removeCount;
#else
	gUpdateHandler->removeDependent (unknownCast (), dep);
#endif // SMTG_DEPENDENCY_COUNT
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
namespace Singleton {
using ObjectVector = std::vector<FObject**>;
ObjectVector* singletonInstances = nullptr;
bool singletonsTerminated = false;
Steinberg::Base::Thread::FLock* singletonsLock;

bool isTerminated ()
{
	return singletonsTerminated;
}

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
			for (Steinberg::FObject** obj : *singletonInstances)
			{
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
