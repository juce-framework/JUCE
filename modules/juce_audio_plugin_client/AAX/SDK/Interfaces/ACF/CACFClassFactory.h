/***********************************************************************

	This file is part of the Avid AAX SDK.

	The AAX SDK is subject to commercial or open-source licensing.

	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
	Agreement and Avid Privacy Policy.

	AAX SDK License: https://developer.avid.com/aax
	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement

	Or: You may also use this code under the terms of the GPL v3 (see
	www.gnu.org/licenses).

	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
	DISCLAIMED.

	Copyright (c) 2004, 2018, 2021, 2024 Avid Technology, Inc. All rights reserved.

************************************************************************/




#ifndef CACFClassFactory_h
#define CACFClassFactory_h

#ifndef CACFUnknown_h
#include "CACFUnknown.h"
#endif

/*!
	\file CACFClassFactory.h
	\brief Define the object creation callback function that should be
	implemented as a static method for every concrete ACF object.
*/

typedef ACFRESULT (*ACFCreateObjectProc)(IACFUnknown *, IACFUnknown *, const acfIID& iid, void** ppOut); 

class CACFClassFactory : 
	public IACFClassFactory,
	public CACFUnknown
{
public:
    // Default factory method for creating class factories.
    static ACFRESULT Create (ACFCreateObjectProc pfnCreate, const acfIID& iid, void** ppOut);

	// IACFUnknown methods  
	ACF_DECLARE_STANDARD_UNKNOWN()
	
protected:
	CACFClassFactory(ACFCreateObjectProc createProc);

	virtual ~CACFClassFactory();

	// ACFUnknown override
	ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppv) ACF_OVERRIDE;

public:
	// IACFClassFactory methods 
	ACFMETHOD(CreateInstance)(IACFUnknown * pUnkHost, IACFUnknown * pUnkOuter, const acfIID & riid, void ** ppv) ACF_OVERRIDE;
  
private:
    CACFClassFactory(); // Default constructor is prohibited.
    CACFClassFactory(const CACFClassFactory&); // non-copyable

	ACFCreateObjectProc m_pfnCreate;
};


#endif // CACFClassFactory_h
