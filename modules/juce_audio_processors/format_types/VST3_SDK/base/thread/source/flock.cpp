//------------------------------------------------------------------------
// Project     : SDK Base
// Version     : 1.0
//
// Category    : Helpers
// Filename    : base/thread/source/flock.cpp
// Created by  : Steinberg, 1995
// Description : Locks
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
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

#define DEBUG_LOCK 0

#include "base/thread/include/flock.h"
#include "base/source/fdebug.h"

//------------------------------------------------------------------------
#if SMTG_OS_WINDOWS
//------------------------------------------------------------------------
#ifndef WINVER
#define WINVER 0x0500
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT WINVER
#endif

#include <windows.h>
#include <objbase.h>
#define INIT_CS(cs) \
	InitializeCriticalSection ((LPCRITICAL_SECTION)&cs);

#endif

namespace Steinberg {
namespace Base {
namespace Thread {


//------------------------------------------------------------------------
//	FLock implementation
//------------------------------------------------------------------------
FLock::FLock (const char8* /*name*/)
{
#if SMTG_PTHREADS
	pthread_mutexattr_t mutexAttr;
	pthread_mutexattr_init (&mutexAttr);
	pthread_mutexattr_settype (&mutexAttr, PTHREAD_MUTEX_RECURSIVE);
	if (pthread_mutex_init (&mutex, &mutexAttr) != 0)
		{SMTG_WARNING("mutex_init failed")}
	pthread_mutexattr_destroy (&mutexAttr);

#elif SMTG_OS_WINDOWS
	INIT_CS (section)
#else
#warning implement FLock!
#endif
}

//------------------------------------------------------------------------
FLock::~FLock ()
{
#if SMTG_PTHREADS
	pthread_mutex_destroy (&mutex);

#elif SMTG_OS_WINDOWS
	DeleteCriticalSection ((LPCRITICAL_SECTION)&section);
		
#endif
}

//------------------------------------------------------------------------
void FLock::lock ()
{
#if DEBUG_LOCK
	FDebugPrint ("FLock::lock () %x\n", this);
#endif

#if SMTG_PTHREADS
	pthread_mutex_lock (&mutex);

#elif SMTG_OS_WINDOWS
	EnterCriticalSection ((LPCRITICAL_SECTION)&section);

#endif
}

//------------------------------------------------------------------------
void FLock::unlock ()
{
#if DEBUG_LOCK
	FDebugPrint ("FLock::unlock () %x\n", this);
#endif
	
#if SMTG_PTHREADS
	pthread_mutex_unlock (&mutex);

#elif SMTG_OS_WINDOWS
	LeaveCriticalSection ((LPCRITICAL_SECTION)&section);

#endif 
}

//------------------------------------------------------------------------
bool FLock::trylock ()
{
#if SMTG_PTHREADS
	return pthread_mutex_trylock (&mutex) == 0;

#elif SMTG_OS_WINDOWS
	return TryEnterCriticalSection ((LPCRITICAL_SECTION)&section) != 0 ? true : false;

#else
	return false;
#endif 
}

} // Thread
} // Base
} // Steinberg

