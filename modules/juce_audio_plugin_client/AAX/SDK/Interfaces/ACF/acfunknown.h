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

*
* Copyright 2004, 2008, 2010, 2013, 2024 Avid Technology, Inc.
*
************************************************************************/

#ifndef acfunknown_h
#define acfunknown_h

/*!
    \file acfunknown.h
	\brief Defines the base class, IACFUnknown, for all ACF interfaces.

    \remarks Define common symbols so that each compiler can produce
    runtime compatible vtables, methods and function signatures.
 */


#include "acfbasetypes.h"
#include "defineacfuid.h" // now includes DEFINE_ACFUID

/*!
   \remarks Define common symbols so that each compiler can produce
   runtime compatible vtables, methods and function signatures.
*/

#ifndef __cplusplus
#error "ERROR acfunknown.h must be compiled with a C++ compiler!"
#endif


/*!
  \def ACFBOOL
  \brief Define COM compatible BOOL type.

  We need the following definition for a bool type since
  the Win32 used BOOL as an int and ActiveX SDK, MacOLE use
  unsigned long for OLEBOOL.

  \internal We may have to move this definition to acfbasetypes.h.
 */
#if defined(ACF_MAC)
#define ACFBOOL acfUInt32
#else
#define ACFBOOL acfSInt32
#endif


#if defined(ACF_PLATFORM_MWERKS_POWERPC_MACOS)
  	#define ACFMETHODCALLTYPE
	#define ACFAPICALLTYPE          pascal
    #define ACFPLUGINAPI            ACFEXTERN_C ACFEXPORT ACFRESULT ACFAPICALLTYPE
	#define ACFAPI                  ACFEXTERN_C ACFAPICALLTYPE ACFRESULT
	#define ACFAPI_(type)           ACFEXTERN_C ACFAPICALLTYPE type

	#define ACFMETHODVCALLTYPE
	#define ACFAPIVCALLTYPE

#elif defined(ACF_PLATFORM_MWERKS_POWERPC_MACOS10) || defined(ACF_PLATFORM_GCC_POWERPC_MACOS10) || defined(ACF_PLATFORM_GCC_INTEL_MACOS10) || defined(ACF_PLATFORM_INTEL_INTEL_MACOS10) \
	|| defined(ACF_OS_LINUX) || defined(ACF_OS_FREEBSD) || defined(ACF_PLATFORM_ARM_IOS) || defined(ACF_PLATFORM_GCC_ARM_MACOS11)
  	#define ACFMETHODCALLTYPE
	#define ACFAPICALLTYPE
    #define ACFPLUGINAPI            ACFEXTERN_C ACFEXPORT ACFRESULT ACFAPICALLTYPE
	#define ACFAPI                  ACFEXTERN_C ACFRESULT ACFAPICALLTYPE
	#define ACFAPI_(type)           ACFEXTERN_C type ACFAPICALLTYPE

	#define ACFMETHODVCALLTYPE
	#define ACFAPIVCALLTYPE


#elif defined(ACF_PLATFORM_MSC_INTEL_WINDOWS) || defined(ACF_PLATFORM_INTEL_INTEL_WINDOWS) || defined(ACF_PLATFORM_MWERKS_INTEL_WINDOWS)
/*!
\brief Specifies the calling convention used for methods.
*/
	#define ACFMETHODCALLTYPE     __stdcall

/*!
\brief Specifies the calling convention used for functions.
*/
	#define ACFAPICALLTYPE        __stdcall

/*!
 \brief Specifies the calling convension for a variable argument ACF function.
 */
    #define ACFPLUGINAPI          ACFEXTERN_C ACFEXPORT ACFRESULT ACFAPICALLTYPE

/*!
\brief Specifies the a standard ACF function that returns an ACFRESULT.
*/
	#define ACFAPI                ACFEXTERN_C ACFRESULT ACFAPICALLTYPE

/*!
\brief Specifies an ACF function that returns a type result.
*/
	#define ACFAPI_(type)         ACFEXTERN_C type ACFAPICALLTYPE

/*!
\brief Specifies the calling convension for a variable argument ACF method.
*/
	#define ACFMETHODVCALLTYPE    __cdecl

/*!
\brief Specifies the calling convension for a variable argument ACF function.
*/
	#define ACFAPIVCALLTYPE       __cdecl


#else
	#error Unsupported configuration of OS and compiler!
#endif

/*!
\brief Specifies an ACF virtual method that returns an ACFRESULT.
*/
#define ACFMETHOD(method)         virtual ACFRESULT ACFMETHODCALLTYPE method

/*!
\brief Specifies an ACF virtual method that returns a type result.
*/
#define ACFMETHOD_(type,method)   virtual type ACFMETHODCALLTYPE method

/*!
\brief Specifies an ACF method that returns an ACFRESULT for an implementation class.
*/
#define ACFMETHODIMP              ACFRESULT ACFMETHODCALLTYPE

/*!
\brief Specifies an ACF method that returns a type result in an implementation class.
*/
#define ACFMETHODIMP_(type)       type ACFMETHODCALLTYPE


#define acfinterface struct

#define DECLARE_ACFINTERFACE(iface)    acfinterface iface : public IACFUnknown
#define DECLARE_ACFINTERFACE_(iface, baseiface)    acfinterface iface : public baseiface

#if ACF_WIN
#if defined(_MPPC_) && \
    ( (defined(_MSC_VER) || defined(__SC__) || defined(__MWERKS__)) && \
    !defined(NO_NULL_VTABLE_ENTRY) )
   #define BEGIN_ACFINTERFACE virtual void a() {}
   #define END_ACFINTERFACE

#else
   #define BEGIN_ACFINTERFACE
   #define END_ACFINTERFACE
#endif

#elif ACF_MACH
	#define BEGIN_ACFINTERFACE
	#define END_ACFINTERFACE

#elif ACF_UNIX
	#define BEGIN_ACFINTERFACE
	#define END_ACFINTERFACE

#elif ACF_IOS
    #define BEGIN_ACFINTERFACE
    #define END_ACFINTERFACE

#else
	#error Unsupported configuration of OS and compiler!

#endif


#if defined(ACF_PLATFORM_MWERKS_POWERPC_MACOS) || (defined(ACF_PLATFORM_MWERKS_POWERPC_MACOS10) && (__MWERKS__ <= 0x31FF /* CW 8 or earlier */))
  // jjo For testing with CW9 with __comobject compiler path from Andreas Hommel

  // fyi: comment snatched from the Mac ActiveX SDK (no longer supported by MS).
#define ACFCOMBASE  : __comobject
#else
/*!
\def ACFCOMBASE
\brief Specifies the compiler specific base class for IACFUnknown.
*/
#define ACFCOMBASE
#endif

/*!
	\b IID_IACFUnknown
	\remarks
    The interface identifier for IACFUnknown.
    \note For compatibility with COM IID_IACFUnknown == IID_IUnknown .
	\n <b> type: </b> UID
	\n <b> context: </b> global
	\n <b> ACFNamespace name: </b> none
	\n {00000000-0000-0000-C000-000000000046}
*/
DEFINE_ACFUID(acfIID, IID_IACFUnknown, 0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif // __clang__

/*!
    \interface IACFUnknown
    \brief COM compatible IUnknown C++ interface.
    \remarks
     The methods of the IACFUnknown interface, implemented by all ACF objects, supports
     general inter-object protocol negotiation via the QueryInterface method, and object
     lifetime management with the AddRef and Release methods.

     \note Because AddRef and Release are not required to return accurate values, callers
     of these methods must not use the return values to determine if an object is still
     valid or has been destroyed. (Standard M*cr*s*ft disclaimer)

     For further information please refer to the Microsoft documentation for IUnknown.

     \note This class will work only with compilers that can produce COM-compatible object
     layouts for C++ classes.  egcs can not do this.  Metrowerks can do this (if you
     subclass from __comobject).
 */
class IACFUnknown ACFCOMBASE
{
public:
	BEGIN_ACFINTERFACE

//	virtual ~IACFUnknown() {}

/*!
    \brief Returns pointers to supported interfaces.
    \remarks
    The QueryInterface method gives a client access to alternate interfaces implemented by
    an object. The returned interface pointer will have already had its reference count
	incremented so the caller will be required to call the Release method.
    \param iid Identifier of the requested interface
    \param ppOut Address of variable that receives the interface pointer associated with iid.
 */
	virtual ACFRESULT ACFMETHODCALLTYPE QueryInterface (const acfIID & iid, void ** ppOut) = 0;

/*!
    \brief Increments reference count.
    \remarks
     The AddRef method should be called every time a new copy of an interface is made. When
     this copy is no longer referenced it must be released with the Release method.
 */
	virtual acfUInt32 ACFMETHODCALLTYPE AddRef (void) = 0;

/*!
    \brief Decrements reference count.
    \remarks
	Use this method to decrement the reference count. When the reference count reaches zero the
	object that implements the interface will be deleted.
 */
	virtual acfUInt32 ACFMETHODCALLTYPE Release (void) = 0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__


#endif // acfunknown_h
