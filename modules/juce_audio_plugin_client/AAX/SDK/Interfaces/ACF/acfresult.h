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

	Copyright (c) 2004, 2024 Avid Technology, Inc. All rights reserved.

************************************************************************/



#ifndef acfresult_h
#define acfresult_h
 
/*!
    \file acfresult.h
    \brief Defines the common result codes.
    \remarks
	Result codes contain embedded information describing 
	the severity, status and factors in the result. Note 
	A successful result is not defined as 1. The ACFSUCCEEDED() 
	and ACFFAILED() macros should be used for determining 
	the result.
	\note Define ACF equivalent of COM HRESULT.
 */

#include "acfbasetypes.h"

/*!
\defgroup ResultCodes Result Codes
Result codes are values returned from methods in an interface. Result codes are bitfield that provide a richer set of information
than a simple true or false return value. The result code can be used to determine the exact nature of a failure in the API.
*/

//@{


/*!
Macro that creates an ACFRESULT value from component pieces
*/
#define MAKE_ACFRESULT(sev,fac,code) \
    ((ACFRESULT) (((acfUInt32)(sev)<<31) | ((acfUInt32)(fac)<<16) | ((acfUInt32)(code))) )

/*!
Macro that creates a custom ACFRESULT
*/
#define MAKE_CUSTOM_ACFRESULT(sev,fac,code) \
    ((ACFRESULT) (((acfUInt32)(sev)<<31) | (1 << 29) | ((acfUInt32)(fac)<<16) | ((acfUInt32)(code))) )


/*!
Macro that checks for the success of an ACFRESULT. Note that general success or failure should not be tested
against binary operators. Use the ACFSUCCEEDED and ACFFAILED macros instead.
*/
#define ACFSUCCEEDED(Status) ((ACFRESULT)(Status) >= 0)

/*!
Macro that checks for the failure of a ACFRESULT. Note that general success or failure should not be tested
against binary operators. Use the ACFSUCCEEDED and ACFFAILED macros instead.
*/
#define ACFFAILED(Status) ((ACFRESULT)(Status)<0)



/*!
   \brief <b>Success:</b>  A successful return result.
   \remarks The function succeeded and returned a Boolean true.
   ACF_OK is defined as 0.
*/

#define ACF_OK                  0L


/*!
   \def ACF_FALSE
   \brief <b>Success:</b> A false return result.
   \remarks The function succeeded and returned a Boolean false.
   ACF_FALSE is defined as 1.
*/

#define ACF_FALSE               MAKE_ACFRESULT(0, 0, 1)


/*!
    \def ACF_E_UNEXPECTED
    \brief <b>Error:</b> unexpected return result.
	\remarks Should be returned as a generic error
	when an unanticipated problem has arisen. The result
	is generally a catastrophic error.
 */
#define ACF_E_UNEXPECTED        MAKE_ACFRESULT(1,   0, 0xffff)
                                 // relatively catastrophic failure
/*!
    \def ACF_E_NOTIMPL
    \brief <b>Error:</b> the call is not implemented result.
	\remarks Used for forward thinking and future features.
*/
#define ACF_E_NOTIMPL           MAKE_ACFRESULT(1,   0, 1)
                                 // not implemented
/*!
    \def ACF_E_OUTOFMEMORY
    \brief <b>Error:</b> out of memory.
	\remarks Out of memory error, if there were more 
	memory, you would could forget the error.
*/
#define ACF_E_OUTOFMEMORY       MAKE_ACFRESULT(1,   0, 2)
                                 // ran out of memory
/*!
    \def ACF_E_INVALIDARG
    \brief <b>Error:</b> an argument passed to the function is invalid.
	\remarks Not to be confused with an invalid pointer argument.
	An invalid argument is given when the wrong or inconsistent 
	information is passed to the function. 
*/
#define ACF_E_INVALIDARG        MAKE_ACFRESULT(1,   0, 3)
                                 // one or more arguments are invalid
/*!
	\def ACF_E_NOINTERFACE
	\brief <b>Error:</b> there is no such interface
	\remarks A compatible interface can not be found.
*/
#define ACF_E_NOINTERFACE       MAKE_ACFRESULT(1,   0, 4)
                                 // no such interface supported

/*! 
	\def ACF_E_POINTER
	\brief <b>Error:</b> the pointer is invalid
	\remarks The pointer is most likely not initialized properly.
	It could also be of the wrong type.
*/
#define ACF_E_POINTER           MAKE_ACFRESULT(1,   0, 5)
                                 // invalid pointer

/*! 
	\def ACF_E_HANDLE
	\brief <b>Error:</b> the handle is invalid
	\remarks The handle could be of the wrong type, no longer valid
	or closed.
*/
#define ACF_E_HANDLE            MAKE_ACFRESULT(1,   0, 6)
                                 // invalid handle
/*!
	\def ACF_E_ABORT
	\brief <b>Error:</b> The call is being aborted.
*/
#define ACF_E_ABORT             MAKE_ACFRESULT(1,   0, 7)
                                 // operation aborted
/*!
	\def ACF_E_FAIL
	\brief <b>Error:</b>  Unspecified error.
*/
#define ACF_E_FAIL              MAKE_ACFRESULT(1,   0, 8)
                                 // unspecified error
/*!
	\def ACF_E_ACCESSDENIED
	\brief <b>Error:</b>  Access to a resource was denied.
	\remarks Check permissions and serialization.
*/
#define ACF_E_ACCESSDENIED      MAKE_ACFRESULT(1,   0, 9)

/*!
	\def ACF_E_ATTRIBUTEUNDEFINED
	\brief <b>Error:</b>  The specified attribute could not be found.
	\remarks Ensure the attribute was registered with DefineAttribute.
*/
#define ACF_E_ATTRIBUTEUNDEFINED MAKE_CUSTOM_ACFRESULT(1,  0, 10)
                                 // general access denied error
/*!
	\def ACF_E_WRONGTYPE
	\brief <b>Error:</b>  The specified type id does not match the expected type ID
	\remarks Ensure the type is correct
*/
#define ACF_E_WRONGTYPE MAKE_CUSTOM_ACFRESULT(1,  0, 11)


/*! 
	\def ACF_E_OUT_OF_RANGE
	\brief <b>Error:</b>  Out of range
	\remarks The index was inaccurate or out of range
*/
#define ACF_E_OUT_OF_RANGE      MAKE_ACFRESULT(1, 0, 12 )


/*!
\def ACF_E_UNKNOWNDEFINITION
\brief <b>Error:</b> a requested definition can not be found.
\remarks The definition does not exist or is not registered with this host.
*/
#define ACF_E_UNKNOWNDEFINITION MAKE_ACFRESULT( 1, 0, 13 ) 



/*!
	\def ACF_E_CLASSNOTREG
	\brief <b>Error:</b>  The specified type id could not be found.
	\remarks Ensure the type is valid. See acfbasetypes.h for valid types and avx2uids.h
				for a list of standard types and thier corresponding uids.
*/
#define ACF_E_CLASSNOTREG MAKE_ACFRESULT(1,  0x0004, 0x0154)

/*!
	\def ACF_E_BUFFERTOOSMALL
	\brief <b>Error:</b>  The specified buffer is invalid.
	\remarks The buffer's is inappropriate. For a fixed size object, it could mean
			there is a mismatch between the defined size and the arguments buffer size.
*/
#define ACF_E_BUFFERTOOSMALL           MAKE_ACFRESULT(1,  0x0002, 0x8016)


/*!
	\def ACF_CLASS_E_NOAGGREGATION
	\brief <b>Error:</b>  There is no aggregate class
	\remarks The class does not support aggregation (or class object is remote).
*/
#define ACF_CLASS_E_NOAGGREGATION MAKE_ACFRESULT(1,   0x0004, 0x0110)
                                 // class does not support aggregation (or class object is remote)
/*!
	\def ACF_CLASS_E_CLASSNOTAVAILABLE
	\brief <b>Error:</b> a requested class is not available.
	\remarks The dll doesn't support that class (returned from DllGetClassObject)
*/
#define ACF_CLASS_E_CLASSNOTAVAILABLE   MAKE_ACFRESULT(1,   0x0004, 0x0111)
                                 // dll doesn't support that class (returned from DllGetClassObject)


/*!
	\def ACF_E_OUTOFRESOURCES
	\brief <b>Error:</b> out of system resources
	\remarks There were not enough system resources to satisfy the request.
*/
#define ACF_E_OUTOFRESOURCES MAKE_CUSTOM_ACFRESULT(1,  0, 101)

/*!
	\def ACF_E_ALREADYINITIALIZED
	\brief <b>Error:</b> entity has already been initialized
	\remarks This result indicates that the internal state of some entity has already been initialized.
*/
#define ACF_E_ALREADYINITIALIZED MAKE_CUSTOM_ACFRESULT(1,  0, 102)

/*!
	\def ACF_E_BUSY
	\brief <b>Error:</b> entity is already used by another component (or thread)
	\remarks This result indicates that the internal state of some entity is being used by another component (or thread).
*/
#define ACF_E_BUSY MAKE_CUSTOM_ACFRESULT(1,  0, 103)

/*!
	\def ACF_E_NOTINITIALIZED
	\brief <b>Error:</b> entity has not been initialized properly.
	\remarks This result indicates that the internal state of some entity has not been initialize.
*/
#define ACF_E_NOTINITIALIZED MAKE_CUSTOM_ACFRESULT(1,  0, 104)

/*!
	\def ACF_E_DATANOTAVAILABLE
	\brief <b>Error:</b> entity could not retrieve the data.
	\remarks This result indicates that the system was too busy to retrieve the data.
*/
#define ACF_E_DATANOTAVAILABLE MAKE_CUSTOM_ACFRESULT(1,  0, 105)

/*!
	\def ACF_E_PARAMETERNOTAVAILABLE
	\brief <b>Error:</b> entity could not retrieve the data.
	\remarks This result indicates that the requested parameter is not available.
*/
#define ACF_E_PARAMETERNOTAVAILABLE MAKE_CUSTOM_ACFRESULT(1,  0, 106)

/*!
	\def ACF_E_UNKNOWNTYPE
	\brief <b>Error:</b> type is not known.
	\remarks This result indicates that the corresponding type has not been completed defined in the current host.
*/
#define ACF_E_UNKNOWNTYPE MAKE_CUSTOM_ACFRESULT(1,  0, 107)

/*!
	\def ACF_E_ALREADYDEFINED
	\brief <b>Error:</b>  The Definition already exists.
	\remarks .
*/
#define ACF_E_ALREADYDEFINED MAKE_CUSTOM_ACFRESULT(1,  0, 108)

/*!
	\def ACF_E_LAYOUTNOTAVAILABLE
	\brief <b>Error:</b> entity could not retrieve the data.
	\remarks This result indicates the the requested layout is not available.
*/
#define ACF_E_LAYOUTNOTAVAILABLE MAKE_CUSTOM_ACFRESULT(1,  0, 109)


/*! 
\def ACF_E_INVALIDTYPESIZE
\brief <b>Error:</b>  Invalid type size
\remarks The type size is invalid, the buffer size does not match the type.
This can happen when the type ID does not match the data type. 
*/
#define ACF_E_INVALIDTYPESIZE      MAKE_ACFRESULT(1, 0, 110 )

/*!
\def ACF_E_NODATA
\brief <b>Error:</b> entity could not retrieve the data because there is no data.
\remarks This result indicates that the system was unable to retrieve the data because there is none.
*/
#define ACF_E_NODATA MAKE_CUSTOM_ACFRESULT(1,  0, 111)


/*!
\def ACF_E_TIMEOUT
\brief <b>Error:</b> entity's processing took longer that it was prepared to wait. The connection is timed out.
\remarks 
*/
#define ACF_E_TIMEOUT MAKE_CUSTOM_ACFRESULT(1,  0, 112)

/*!
\def ACF_E_ENTITLEMENT
\brief <b>Error:</b> Plug-in could not be loaded because it is not entitled
\remarks 
*/
#define ACF_E_ENTITLEMENT MAKE_CUSTOM_ACFRESULT(1,  0, 113)

/*!
 \def ACF_E_CACHEHASHMISSING
 \brief <b>Error:</b> Plug-in cache file is missing the hash data.
 \remarks
 */
#define ACF_E_CACHEHASHMISSING MAKE_CUSTOM_ACFRESULT(1,  0, 114)

/*!
 \def ACF_E_CACHEHASHMISSMATCH
 \brief <b>Error:</b> Plug-in cache file is stale and needs to be recreated.
 \remarks
 */
#define ACF_E_CACHEHASHMISSMATCH MAKE_CUSTOM_ACFRESULT(1,  0, 115)

/*!
 \def ACF_E_NOTCOMPATIBLE
 \brief <b>Error:</b> Plug-in module could not be loaded or register because it is incompatible with another plugin or application.
 \remarks
 */
#define ACF_E_NOTCOMPATIBLE MAKE_CUSTOM_ACFRESULT(1,  0, 116)

/*!
 \def ACF_E_DISABLED
 \brief <b>Error:</b> Plug-in module could not be loaded because it has been explicitely disabled.
 \remarks
 */
#define ACF_E_DISABLED MAKE_CUSTOM_ACFRESULT(1,  0, 117)

/*!
 \def ACF_E_ACFCACHEREGISTER
 \brief <b>Error:</b> Plug-in module could not be loaded or registered by the acfcacheregister process.
 \remarks
 */
#define ACF_E_ACFCACHEREGISTER MAKE_CUSTOM_ACFRESULT(1,  0, 118)

/*!
 \def ACF_E_ACFCACHEREGISTERMISSING
 \brief <b>Error:</b> The acfcacheregister process was not installed with the plugin host process.
 \remarks
 */
#define ACF_E_ACFCACHEREGISTERMISSING MAKE_CUSTOM_ACFRESULT(1,  0, 119)

/*!
 \def ACF_E_PLUGINCACHENOTSUPPORTED
 \brief <b>Error:</b> The plugin host has not enabled plugin caching (\sa ACFATTR_Host_SupportsPluginCache).
 \remarks
 */
#define ACF_E_PLUGINCACHENOTSUPPORTED MAKE_CUSTOM_ACFRESULT(1,  0, 120)

//@} // End ResultCodes
#endif // acfresult_h
