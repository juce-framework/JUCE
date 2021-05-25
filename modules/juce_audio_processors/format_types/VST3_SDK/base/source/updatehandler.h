//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/source/updatehandler.h
// Created by  : Steinberg, 2008
// Description :
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2019, Steinberg Media Technologies GmbH, All Rights Reserved
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

#pragma once

#include "base/source/fobject.h"
#include "base/thread/include/flock.h"
#include "pluginterfaces/base/iupdatehandler.h"

namespace Steinberg {

/// @cond ignore
namespace Update { struct Table; }
/// @endcond

//------------------------------------------------------------------------
/** Handle Send and Cancel pending message for a given object*/
//------------------------------------------------------------------------
class IUpdateManager : public FUnknown
{
public:
//------------------------------------------------------------------------
	/** cancel pending messages send by \param object or by any if object is 0 */
	virtual tresult PLUGIN_API cancelUpdates (FUnknown* object) = 0;
	/** send pending messages send by \param object or by any if object is 0 */
	virtual tresult PLUGIN_API triggerDeferedUpdates (FUnknown* object = 0) = 0;
	static const FUID iid;
};

DECLARE_CLASS_IID (IUpdateManager, 0x030B780C, 0xD6E6418D, 0x8CE00BC2, 0x09C834D4)

//------------------------------------------------------------------------------
/**
UpdateHandler implements IUpdateManager and IUpdateHandler to handle dependencies
between objects to store and forward messages to dependent objects.

This implementation is thread save, so objects can send message, add or remove
dependents from different threads.
Do do so it uses mutex, so be aware of locking.
*/
//------------------------------------------------------------------------------
class UpdateHandler : public FObject, public IUpdateHandler, public IUpdateManager
{
public:
//------------------------------------------------------------------------------
	UpdateHandler ();
	~UpdateHandler ();

	using FObject::addDependent;
	using FObject::removeDependent;
	using FObject::deferUpdate;

	// IUpdateHandler
	/** register \param dependent to get messages from \param object */
	virtual tresult PLUGIN_API addDependent (FUnknown* object, IDependent* dependent) SMTG_OVERRIDE;
	/** unregister \param dependent to get no messages from \param object */
	virtual tresult PLUGIN_API removeDependent (FUnknown* object,
	                                            IDependent* dependent) SMTG_OVERRIDE;
	/** send \param message to all dependents of \param object immediately */
	virtual tresult PLUGIN_API triggerUpdates (FUnknown* object, int32 message) SMTG_OVERRIDE;
	/** send \param message to all dependents of \param object when idle */
	virtual tresult PLUGIN_API deferUpdates (FUnknown* object, int32 message) SMTG_OVERRIDE;

	// IUpdateManager
	/** cancel pending messages send by \param object or by any if object is 0 */
	virtual tresult PLUGIN_API cancelUpdates (FUnknown* object) SMTG_OVERRIDE;
	/** send pending messages send by \param object or by any if object is 0 */
	virtual tresult PLUGIN_API triggerDeferedUpdates (FUnknown* object = 0) SMTG_OVERRIDE;

	/// @cond ignore
	// obsolete functions kept for compatibility
	void checkUpdates (FObject* object = nullptr) { triggerDeferedUpdates (object->unknownCast ()); }
	void flushUpdates (FObject* object) { cancelUpdates (object->unknownCast ()); }
	void deferUpdate (FObject* object, int32 message)
	{
		deferUpdates (object->unknownCast (), message);
	}
	void signalChange (FObject* object, int32 message, bool suppressUpdateDone = false)
	{
		doTriggerUpdates (object->unknownCast (), message, suppressUpdateDone);
	}
#if DEVELOPMENT
	bool checkDeferred (FUnknown* object);
	bool hasDependencies (FUnknown* object);
	void printForObject (FObject* object) const;
#endif
	/// @endcond
	size_t countDependencies (FUnknown* object = nullptr);
	
	OBJ_METHODS (UpdateHandler, FObject)
	FUNKNOWN_METHODS2 (IUpdateHandler, IUpdateManager, FObject)
	SINGLETON (UpdateHandler)
//------------------------------------------------------------------------------
private:
	tresult doTriggerUpdates (FUnknown* object, int32 message, bool suppressUpdateDone);

	Steinberg::Base::Thread::FLock lock;
	Update::Table* table = nullptr;
	friend struct LockUpdateDependencies;
	static bool lockUpdates;
};


//------------------------------------------------------------------------
} // namespace Steinberg
