/*================================================================================================*/
/*

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

/**  
 *	\file   AAX_CAutoreleasePool.h
 *	
 *	\brief Autorelease pool helper utility
 */ 
/*================================================================================================*/


#pragma once

#ifndef _AAX_CAUTORELEASEPOOL_H_
#define _AAX_CAUTORELEASEPOOL_H_


/*	\brief Creates an autorelease pool for the scope of the stack based class
	to clearn up any autoreleased memory that was allocated in the lifetime of
	the pool.
	
	\details
	This may be used on either Mac or Windows platforms and will not pull in
	any Cocoa dependencies.
	
	usage:
\code
{
	AAX_CAutoreleasePool myAutoReleasePool
	delete myCocoaObject;

	// Pool is released when the AAX_CAutoreleasePool is destroyed
}
\endcode
 */
class AAX_CAutoreleasePool
{
	public:
		AAX_CAutoreleasePool();
		~AAX_CAutoreleasePool();

	private:
		AAX_CAutoreleasePool (const AAX_CAutoreleasePool&);
		AAX_CAutoreleasePool& operator= (const AAX_CAutoreleasePool&);

	private:
		void* mAutoreleasePool; //!< Opaque pool instance
 };


#endif // #ifndef _AAX_CAUTORELEASEPOOL_H_
