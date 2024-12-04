/*================================================================================================*/
/*
 *	Copyright 2004-2016, 2023-2024 Avid Technology, Inc.
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
 *  \file AAX_CACFUnknown.cpp
 *
 *  \brief Implementation for various things in the unknown object that supports aggregation.
 *
 */
/*================================================================================================*/

#include "CACFUnknown.h"
#include "acfextras.h"

#include <stddef.h>

// AH/RM mod start - using inlined fast atomic operations with no library dependencies.
#include "AAX_Atomic.h"

// rmajors mod - internal debugging code for acf refcount debugging of aax objects
#if defined AAX_DEBUG_ACF_REFCOUNT
#include "AAX_Assert.h"
#endif

// Using casts here since ACF uses long and stdint uses int on 32-bit systems.  I have
// looked at the assembly for both cast and non-cast approaches and it is identical, so
// this should not incur any performance penalty. -rmajors 10/11
acfUInt32 ACFInterlockedIncrement (acfUInt32 & value)
{
	return (acfUInt32)AAX_Atomic_IncThenGet_32((uint32_t&)value);
}

acfUInt32 ACFInterlockedDecrement (acfUInt32 & value)
{
	return (acfUInt32)AAX_Atomic_DecThenGet_32((uint32_t&)value);
}
// mod end

// Manage total number of outstanding "live" objects. Used by ACFCanUnloadNow.
acfUInt32 CACFUnknown::m_ActiveObjects = 0;

acfUInt32 CACFUnknown::IncrementActiveObjects(void)
{
	return ::ACFInterlockedIncrement(m_ActiveObjects);
}

acfUInt32 CACFUnknown::DecrementActiveObjects(void)
{
	return ::ACFInterlockedDecrement(m_ActiveObjects);
}

acfUInt32 CACFUnknown::GetActiveObjectCount(void)
{
	return m_ActiveObjects;
}


/*!
 // CACFUnknown::CACFUnknown
 // The "outer unknown" is the private non-delegating implementation. 
 // This method also increments the active ACF object count
 // so that ACF Plugin will not attempt to unload (see \ref ACFCanUnloadNow).
 */
CACFUnknown::CACFUnknown() : 
m_cRef(0), 
m_pUnkOuter(0)
{
	m_UnkPrivate.SetParentUnknown(this);
	
	m_pUnkOuter = GetPrivateUnknown();
	
	IncrementActiveObjects();
}

/*!
 // CACFUnknown::CACFUnknown
 // If the given outer unknown is 0 then use the private non-delegating
 // implementation. This method also increments the active ACF object count
 // so that ACF Plugin will not attempt to unload (see \ref ACFCanUnloadNow).
 */
CACFUnknown::CACFUnknown
(
 IACFUnknown *pUnkOuter // Controlling unknown from IACFClassFactory::CreateInstance, 
 // may be 0.
 ) : 
m_cRef(0), 
m_pUnkOuter(0)
{
	m_UnkPrivate.SetParentUnknown(this);
	
	m_pUnkOuter = (pUnkOuter) ? pUnkOuter : GetPrivateUnknown();
	
	IncrementActiveObjects();
}

/*!
 // CACFUnknown::~CACFUnknown
 
 // Decrements the active ACF object count so that the ACF plugin can unload if 
 // the count is zero (see \ref ACFCanUnloadNow)
 */
CACFUnknown::~CACFUnknown()
{
	DecrementActiveObjects();
}

/*!
 */
ACFMETHODIMP_(void) CACFUnknown::ReclaimMemory(void)
{
    delete this;
}

/*!
 // CACFUnknown::InternalQueryInterface
 
 // Objects that are aggregated use this to support additional interfaces.
 // they should call this method on their parent so that any of it's interfaces
 // are queried.
 
 // Returns one of the following values:
 //
 // ACF_OK | 
 // If objects supports the requested interface.
 // ACF_E_NOINTERFACE  | 
 // If object does not implement the requeste interface.
 */
ACFRESULT CACFUnknown::InternalQueryInterface
(
 const acfIID & riid, // [in] interface they want
 void  **ppvObjOut // [out] where they want to put the resulting object ptr.
 )
{
    *ppvObjOut = 0;
	
    // We only support the IID_IACFUnknown (a.k.a. IID_IUnknown) interface 
    //
    if (riid == IID_IACFUnknown )
    { 
        *ppvObjOut = static_cast<IACFUnknown *>(GetPrivateUnknown()); 
        ( static_cast<IACFUnknown *>(*ppvObjOut))->AddRef();
        return ACF_OK;
    }
	
    return ACF_E_NOINTERFACE;
}


/*!
 Adds a tick to the current reference count.
 The new reference count
 */
acfUInt32 CACFUnknown::InternalAddRef
(
 void
 )
{
#if defined AAX_DEBUG_ACF_REFCOUNT
	const acfUInt32 cRef = ::ACFInterlockedIncrement(m_cRef);
	AAX_TRACEORSTACKTRACE(kAAX_Trace_Priority_Low, kAAX_Trace_Priority_Lowest, "CACFUnknown::InternalAddRef - object: %p, new count: %lu", this, (unsigned long)cRef);
	return cRef;
#else
    return ::ACFInterlockedIncrement(m_cRef);
#endif
}

/*! 
 Removes a tick from the count, and delets the object if necessary
 Remaining refs
 */
acfUInt32 CACFUnknown::InternalRelease
(
 void
 )
{
    acfUInt32 cRef = ::ACFInterlockedDecrement(m_cRef);
    if (0 == cRef)
    {
        // Give a subclass a chance to cleanup while this object is still valid.
        FinalRelease();
		
        // Get a subclass a change to reclaim/resuse the object's memory.
        ReclaimMemory();
    }
	
#if defined AAX_DEBUG_ACF_REFCOUNT
	AAX_TRACEORSTACKTRACE(kAAX_Trace_Priority_Low, kAAX_Trace_Priority_Lowest, "CACFUnknown::InternalRelease - object: %p, new count: %lu", this, (unsigned long)cRef);
#endif
	
    return cRef;
}

