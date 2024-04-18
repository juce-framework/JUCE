/*================================================================================================*/
/*
 *
 *	Copyright 2014-2015, 2023-2024 Avid Technology, Inc.
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

#ifndef AAX_CMUTEX_H
#define AAX_CMUTEX_H

/**  
 *	\file  AAX_CMutex.h
 *
 *	\brief Mutex
 *
 */ 
/*================================================================================================*/

/**	\brief Mutex with try lock functionality
 */
class AAX_CMutex
{
public:
	AAX_CMutex();
	~AAX_CMutex();
	
    bool Lock();
    void Unlock();
    bool Try_Lock();
	
private:
    AAX_CMutex(const AAX_CMutex&);
    AAX_CMutex& operator=(const AAX_CMutex&);

	typedef struct opaque_aax_mutex_t * aax_mutex_t;	
	aax_mutex_t		mMutex;	
};

/** \brief Helper class for working with mutex
 */
class AAX_StLock_Guard
{
public:
    explicit AAX_StLock_Guard(AAX_CMutex& iMutex) : mMutex(iMutex) { mNeedsUnlock = mMutex.Lock(); }
    ~AAX_StLock_Guard() { if (mNeedsUnlock) mMutex.Unlock(); }
	
private:	
    AAX_StLock_Guard(AAX_StLock_Guard const&);
    AAX_StLock_Guard& operator=(AAX_StLock_Guard const&);

	AAX_CMutex &	mMutex;
	bool			mNeedsUnlock;
};

#endif // AAX_CMUTEX_H

