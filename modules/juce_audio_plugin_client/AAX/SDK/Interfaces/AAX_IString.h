/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_IString.h
 *	
 *	\brief	An %AAX string interface
 *
 */ 
/*================================================================================================*/


#ifndef AAX_ISTRING_H
#define AAX_ISTRING_H

#include "AAX.h"	//for types


/** 
*	\brief	A simple string container that can be passed across a binary boundary.  This class, for simplicity, is not versioned and thus can never change.
*
*	\details
*	For a real string implementation, see AAX_CString, which inherits from this interface, but provides a much richer string interface.
*
*   This object is not versioned with ACF for a variety of reasons, but the biggest implication of that is that THIS INTERFACE CAN NEVER CHANGE!
*
*/
class AAX_IString
{
public:
	/** Virtual Destructor */
	virtual ~AAX_IString ()	{}
	
	/** Length methods */
	virtual uint32_t		Length () const = 0;
	virtual uint32_t		MaxLength () const = 0;
	
	/** C string methods */
	virtual const char *	Get ()	const = 0;
	virtual void			Set ( const char * iString ) = 0;		
	
	/** Assignment operators */
	virtual AAX_IString &	operator=(const AAX_IString & iOther) = 0;
	virtual AAX_IString &	operator=(const char * iString) = 0;			
};




#endif //AAX_ISTRING_H
