/*================================================================================================*/
/*
 *	Copyright 2011-2015, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_CMutex.cpp
 *	
 *	\author Viktor Iarovyi
 *
 */ 
/*================================================================================================*/

#include "AAX_CMutex.h"

#if defined(__GNUC__)
#include <pthread.h>
#include <errno.h>
#elif defined(WIN32)
#include <windows.h>
#else
#error AAX_CMutex not implemented
#endif

struct opaque_aax_mutex_t
{
#if defined(__GNUC__)
	pthread_t		mOwner;
	pthread_mutex_t	mSysMutex;	
#elif defined(WIN32)
	DWORD			mOwner;
	HANDLE			mSysMutex;	
#endif
};

// ******************************************************************************************
// METHOD:	AAX_CMutex
// ******************************************************************************************
AAX_CMutex::AAX_CMutex()
{
	mMutex = new opaque_aax_mutex_t;
	mMutex->mOwner = 0;
#if defined(__GNUC__)
	if (::pthread_mutex_init(&mMutex->mSysMutex, NULL) != 0)
#elif defined(WIN32)			
	mMutex->mSysMutex = ::CreateMutex(NULL, false, NULL);
	if (0 == mMutex->mSysMutex)
#endif
	{
		delete mMutex;
		mMutex = 0;
	}
}
	
// ******************************************************************************************
// METHOD:	~AAX_CMutex
// ******************************************************************************************
AAX_CMutex::~AAX_CMutex()
{
	if (mMutex)
	{
#if defined(__GNUC__)
		::pthread_mutex_destroy(&mMutex->mSysMutex);
#elif defined(WIN32)	
		::CloseHandle(mMutex->mSysMutex);
#endif
		delete mMutex;		
		mMutex = 0;
	}
}

// ******************************************************************************************
// METHOD:	Lock
// ******************************************************************************************
bool AAX_CMutex::Lock()
{
	bool result = false;
	if (mMutex)
	{			
#if defined(__GNUC__)
		pthread_t curThread = ::pthread_self();
		if(! ::pthread_equal(curThread, mMutex->mOwner))
		{
			::pthread_mutex_lock(&mMutex->mSysMutex);
			mMutex->mOwner = curThread;
			result = true;
		}
#elif defined(WIN32)
		DWORD curThread = ::GetCurrentThreadId();
		if(mMutex->mOwner != curThread)
		{
			::WaitForSingleObject(mMutex->mSysMutex, INFINITE);
			mMutex->mOwner = curThread;
			result = true;
		}
#endif
	}
	return result;
}

// ******************************************************************************************
// METHOD:	Unlock
// ******************************************************************************************
void AAX_CMutex::Unlock()
{
	if (mMutex)
	{					
#if defined(__GNUC__)
		if(::pthread_equal(::pthread_self(), mMutex->mOwner))
		{
			mMutex->mOwner = 0;
			::pthread_mutex_unlock(&mMutex->mSysMutex);
		}
#elif defined(WIN32)
		if(mMutex->mOwner == ::GetCurrentThreadId())
		{
			mMutex->mOwner = 0;
			::ReleaseMutex(mMutex->mSysMutex);
		}
#endif
	}
}
	
// ******************************************************************************************
// METHOD:	Try_Lock
// ******************************************************************************************
bool AAX_CMutex::Try_Lock()
{
	bool result = false;
	
	if (mMutex)
	{							
#if defined(__GNUC__)
		pthread_t curThread = ::pthread_self();
		if(! ::pthread_equal(curThread, mMutex->mOwner))
		{
			// current thread doesn't own the Lock - try lock to see if we can lock it
			int err = ::pthread_mutex_trylock(&mMutex->mSysMutex);
			if (0 == err)
			{
				// we locked the lock
				mMutex->mOwner = curThread;
				result = true;
			}
			else if (EBUSY == err)
			{
				// the Lock was already locked by another thread
				result = false;
			}
			else
			{
				// it's bad
				result = false;
			}
		}
		else
		{
			// current thread already owns the lock
			result = true;
		}
#elif defined(WIN32)
		if(mMutex->mOwner != ::GetCurrentThreadId())
		{
			// try to acquire the mutex
			DWORD err = ::WaitForSingleObject(mMutex->mSysMutex, 0);
			if (WAIT_OBJECT_0 == err)
			{
				// we locked the lock
				mMutex->mOwner = ::GetCurrentThreadId();
				result = true;
			}
			else if (WAIT_TIMEOUT == err)
			{
				// lock was already locked by another thread
				result = false;
			}
			else
			{
				// it's bad
				result = false;
			}
		}
		else
		{
			// current thread already owns the lock
			result = true;
		}
#endif	
	}
	return result;
}
