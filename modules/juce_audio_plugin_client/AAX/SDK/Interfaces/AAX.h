/*================================================================================================*/
/*

 *	Copyright 2013-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX.h
 *	
 *	\brief Various utility definitions for %AAX
 *
 *	\internal
 *		\todo Here and elsewhere: headers are included that include this header.  Is this something that
 *		should be addressed?
 *
 *		\todo Break up AAX.h into a series of targeted utility headers
 *	\endinternal
 */ 
/*================================================================================================*/


#pragma once

/// @cond ignore
#ifndef _AAX_H_
#define _AAX_H_
/// @endcond

#include <stdint.h>
#include <stddef.h>

#include "AAX_Version.h"
#include "AAX_Enums.h"
#include "AAX_Errors.h"
#include "AAX_Properties.h"



/** @name C++ compiler macros
 */
//@{
/** @def TI_VERSION
 @brief Preprocessor flag indicating compilation for TI
 */
/** @def AAX_CPP11_SUPPORT
 @brief Preprocessor toggle for code which requires C++11 compiler support
 */
//@} C++ compiler macros

#ifndef TI_VERSION
	#if defined _TMS320C6X
		#define TI_VERSION 1
	#elif defined DOXYGEN_PREPROCESSOR
		#define TI_VERSION 0
	#endif
#endif


#ifndef AAX_CPP11_SUPPORT
	#if (defined __cplusplus) && (__cplusplus >= 201103L)
		#define AAX_CPP11_SUPPORT	1
	// VS2015 supports all features except expression SFINAE
	#elif ((defined _MSVC_LANG) && (_MSVC_LANG >= 201402))
		#define AAX_CPP11_SUPPORT	1
	// Let Doxygen see the C++11 version of all code
	#elif defined DOXYGEN_PREPROCESSOR
		#define AAX_CPP11_SUPPORT	1
	#endif
#endif


/** @name C++ keyword macros
 
 Use these macros for keywords which may not be supported on all compilers
 
 \warning Be careful when using these macros; they are a workaround and the
 fallback versions of the macros are not guaranteed to provide identical
 behavior to the fully-supported versions. Always consider the code which
 will be generated in each case!
 
 \warning If your code is protected with PACE Fusion and you are using a PACE
 SDK prior to v4 then you must explicitly define <tt>AAX_CPP11_SUPPORT 0</tt>
 in your project's preprocessor settings to avoid encountering source
 failover caused by %AAX header includes with exotic syntax.
 
 \internal
 \warning Never use keyword macros which could impact the binary
 representation of an interface in any interface which will be
 passed across the library boundary. This mostly applies to
 macros which apply to implicitly-defined constructors:
 - \ref AAX_DEFAULT_CTOR
 - \ref AAX_DEFAULT_COPY_CTOR
 - \ref AAX_DEFAULT_MOVE_CTOR
 - \ref AAX_DEFAULT_ASGN_OPER
 - \ref AAX_DEFAULT_MOVE_OPER
 - \ref AAX_DELETE
 \endinternal
 */
//@{
/** @def AAX_OVERRIDE
 @brief \c override keyword macro
 */
/** @def AAX_FINAL
 @brief \c final keyword macro
 */
/** @def AAX_DEFAULT_CTOR
 @brief \c default keyword macro for a class default constructor
 */
/** @def AAX_DEFAULT_COPY_CTOR
 @brief \c default keyword macro for a class copy constructor
 */
/** @def AAX_DEFAULT_MOVE_CTOR
 @brief \c default keyword macro for a class move constructor
 */
/** @def AAX_DEFAULT_ASGN_OPER
 @brief \c default keyword macro for a class assignment operator
 */
/** @def AAX_DEFAULT_MOVE_OPER
 @brief \c default keyword macro for a class move-assignment operator
 */
/** @def AAX_DELETE
 @brief \c delete keyword macro
 
 \warning The non-C++11 version of this macro assumes \p public declaration access
 */
/** @def AAX_CONSTEXPR
 @brief \c constexpr keyword macro
 */
//@} C++ keyword macros

#if AAX_CPP11_SUPPORT
#	define AAX_OVERRIDE override
#	define AAX_FINAL final
#	define AAX_DEFAULT_DTOR(X) ~X() = default
#	define AAX_DEFAULT_DTOR_OVERRIDE(X) ~X() override = default
#	define AAX_DEFAULT_CTOR(X) X() = default
#	define AAX_DEFAULT_COPY_CTOR(X) X(const X&) = default
#	define AAX_DEFAULT_ASGN_OPER(X) X& operator=(const X&) = default
#	define AAX_DELETE(X) X = delete
#	define AAX_DEFAULT_MOVE_CTOR(X) X(X&&) = default
#	define AAX_DEFAULT_MOVE_OPER(X) X& operator=(X&&) = default
#	define AAX_CONSTEXPR constexpr
#	define AAX_UNIQUE_PTR(X) std::unique_ptr<X>
#else
#	define AAX_OVERRIDE
#	define AAX_FINAL
#	define AAX_DEFAULT_DTOR(X) ~X() {}
#	define AAX_DEFAULT_DTOR_OVERRIDE(X) ~X() {}
#	define AAX_DEFAULT_CTOR(X) X() {}
#	define AAX_DEFAULT_COPY_CTOR(X)
#	define AAX_DEFAULT_MOVE_CTOR(X)
#	define AAX_DEFAULT_ASGN_OPER(X)
#	define AAX_DEFAULT_MOVE_OPER(X)
// Assumes public access in the declaration scope where AAX_DELETE is used
#	define AAX_DELETE(X) private: X; public:
#	define AAX_CONSTEXPR const
#	define AAX_UNIQUE_PTR(X) std::auto_ptr<X>
#endif


/** @name Pointer definitions
 */
//@{
/** @def AAXPointer_32bit
 @brief When AAX_PointerSize == AAXPointer_32bit this is a 32-bit build
 */
/** @def AAXPointer_64bit
 @brief When AAX_PointerSize == AAXPointer_64bit this is a 64-bit build
 */
/** @def AAX_PointerSize
 @brief Use this definition to check the pointer size in the current build.
 
 @sa @ref AAXPointer_32bit
 @sa @ref AAXPointer_64bit
 */
//@} Pointer definitions
#define AAXPointer_32bit	1
#define AAXPointer_64bit	2

#if !defined(AAX_PointerSize)
	#if defined(_M_X64) || defined (__LP64__)
		#define AAX_PointerSize AAXPointer_64bit
	#else
		#define AAX_PointerSize AAXPointer_32bit
	#endif
#endif

// ensure that preprocessor comparison logic gives the correct result
#if ((AAX_PointerSize == AAXPointer_32bit) && (defined(_M_X64) || defined (__LP64__)))
	#error incorrect result of AAX_PointerSize check!
#elif ((AAX_PointerSize == AAXPointer_64bit) && !(defined(_M_X64) || defined (__LP64__)))
	#error incorrect result of AAX_PointerSize check!
#endif


/** @name Alignment macros
 *
 *  Use these macros to define struct packing alignment for data structures that will be
 *	sent across binary or platform boundaries.
 *
\code
 	#include AAX_ALIGN_FILE_BEGIN
	#include AAX_ALIGN_FILE_HOST
 	#include AAX_ALIGN_FILE_END
	    // Structure definition
 	#include AAX_ALIGN_FILE_BEGIN
	#include AAX_ALIGN_FILE_RESET
 	#include AAX_ALIGN_FILE_END
\endcode 
 *
 * See the documentation for each macro for individual usage notes and warnings
 */
//@{
/** @def AAX_ALIGN_FILE_HOST
 @brief Macro to set alignment for data structures that are shared with the host
 
 @details
 This macro is used to set alignment for data structures that are part of the %AAX ABI.
 You should not need to use this macro for any custom data structures in your plug-in.
 */

/** @def AAX_ALIGN_FILE_ALG
 @brief Macro to set alignment for data structures that are used in the alg
 
 @details
 IMPORTANT: Be very careful to maintain correct data alignment when sending data
 structures between platforms.
 
 \warning
 \li This macro does not guarantee data alignment compatibility for data structures
 which include base classes/structs or virtual functions. The MSVC, GCC and LLVM/clang,
 and CCS (TI) compilers do not support data structure cross-compatibility for these types
 of structures. clang will now present a warning when these macros are used on any such
 structures: <TT>\#pragma ms_struct can not be used with dynamic classes or structures</TT>
 \li Struct Member Alignment (/Zp) on Microsoft compilers must be set to a minimum of
 8-byte packing in order for this macro to function properly. For more information, see
 this MSDN article:<BR>http://msdn.microsoft.com/en-us/library/ms253935.aspx
 */

/** @def AAX_ALIGN_FILE_RESET
 @brief Macro to reset alignment back to default
 */

/** @def AAX_ALIGN_FILE_BEGIN
 @brief Wrapper macro used for warning suppression
 
 @details This wrapper is required in llvm 10.0 and later due to the addition of the
 @c -Wpragma-pack warning. This is a useful compiler warning but it is awkward to properly
 suppress in cases where we are intentionally including only part of the push/pop
 sequence in a single file, as with the @c AAX_ALIGN_FILE_XXX macros.
 */

 /** @def AAX_ALIGN_FILE_END
 @copydoc AAX_ALIGN_FILE_BEGIN
 */
//@} Alignment macros

#if ( defined(_WIN64) || defined(__LP64__) )
	#define AAX_ALIGN_FILE_HOST		"AAX_Push8ByteStructAlignment.h"
#elif ( defined(_TMS320C6X) )
	// AAX_ALIGN_FILE_HOST is not compatible with this compiler
	// We don't use an #error here b/c that causes Doxygen to get confused
#else
    #define AAX_ALIGN_FILE_HOST		"AAX_Push2ByteStructAlignment.h"
#endif
#define AAX_ALIGN_FILE_ALG		"AAX_Push8ByteStructAlignment.h"
#define AAX_ALIGN_FILE_RESET	"AAX_PopStructAlignment.h"
#define AAX_ALIGN_FILE_BEGIN		"AAX_PreStructAlignmentHelper.h"
#define AAX_ALIGN_FILE_END		"AAX_PostStructAlignmentHelper.h"


#ifndef AAX_CALLBACK
#	ifdef _MSC_VER
#		define AAX_CALLBACK		__cdecl
#	else
#		define AAX_CALLBACK
#	endif
#endif // AAX_CALLBACK


#ifdef _MSC_VER
#	define AAX_RESTRICT
#elif defined(_TMS320C6X) // TI
#	define AAX_RESTRICT	restrict
#elif defined (__GNUC__)// Mac
#   define AAX_RESTRICT	__restrict__
#endif // _MSC_VER


// preprocessor helper macros
#define AAX_PREPROCESSOR_CONCAT_HELPER(X,Y) X ## Y
#define AAX_PREPROCESSOR_CONCAT(X,Y) AAX_PREPROCESSOR_CONCAT_HELPER(X,Y)



#ifdef _MSC_VER
// Disable unknown pragma warning for TI pragmas under VC++
#pragma warning( disable : 4068 )
#endif


/** @brief	Compute the index used to address a context field.
			
	@details
	This macro expands to a constant expression suitable for use in enumerator
	definitions and case labels so int32_t as @p aMember is a constant specifier.
	
	@param[in]	aContextType
				The name of context type
	
	@param[in]	aMember
				The name or other specifier of a field of that context type
*/
#define AAX_FIELD_INDEX( aContextType, aMember ) \
	((AAX_CFieldIndex) (offsetof (aContextType, aMember) / sizeof (void *)))


typedef int32_t		AAX_CIndex;        //!< \todo Not used by %AAX plug-ins (except as \ref AAX_CFieldIndex)
typedef AAX_CIndex	AAX_CCount;        //!< \todo Not used by %AAX plug-ins
typedef uint8_t		AAX_CBoolean;      //!< Cross-compiler boolean type used by %AAX interfaces
typedef uint32_t	AAX_CSelector;     //!< \todo Clean up usage; currently used for a variety of ID-related values
typedef int64_t		AAX_CTimestamp;    //!< Time stamp value.  Measured against the DAE clock (see \ref AAX_IComponentDescriptor::AddClock() )
typedef int64_t		AAX_CTimeOfDay;    //!< Hardware running clock value.  MIDI packet time stamps are measured against this clock.  This is actually the same as TransportCounter, but kept for compatibility.
typedef int64_t     AAX_CTransportCounter;  //!< Offset of samples from transport start. Same as TimeOfDay, but added for new interfaces as TimeOfDay is a confusing name.
typedef float		AAX_CSampleRate;   //!< Literal sample rate value used by the \ref AAX_IComponentDescriptor::AddSampleRate() "sample rate field".  For \ref AAX_eProperty_SampleRate, use a mask of \ref AAX_ESampleRateMask.  \sa sampleRateInMask

typedef uint32_t	AAX_CTypeID;  //!< Matches type of OSType used in classic plugins.
typedef int32_t		AAX_Result;
typedef int32_t		AAX_CPropertyValue; //!< \brief 32-bit property values \details Use this property value type for all properties unless otherwise specified by the property documentation
typedef int64_t		AAX_CPropertyValue64; //!< \brief 64-bit property values \details Do not use this value type unless specified explicitly in the property documentation
#if AAX_PointerSize == AAXPointer_32bit
	typedef AAX_CPropertyValue AAX_CPointerPropertyValue; //!< \brief Pointer-sized property values \details Do not use this value type unless specified explicitly in the property documentation
#elif AAX_PointerSize == AAXPointer_64bit
	typedef AAX_CPropertyValue64 AAX_CPointerPropertyValue; //!< \brief Pointer-sized property values \details Do not use this value type unless specified explicitly in the property documentation
#else
	#error unexpected pointer size
#endif
typedef int32_t		AAX_CTargetPlatform;  //!< Matches type of \ref AAX_ETargetPlatform "target platform

typedef AAX_CIndex		AAX_CFieldIndex;    //!< Not used by %AAX plug-ins (except in \ref AAX_FIELD_INDEX macro)
typedef AAX_CSelector	AAX_CComponentID;   //!< \todo Not used by %AAX plug-ins
typedef AAX_CSelector	AAX_CMeterID;       //!< \todo Not used by %AAX plug-ins
typedef const char *	AAX_CParamID;		//!< Parameter identifier \note While this is a string, it must be less than 32 characters in length.  (strlen of 31 or less) \sa \ref kAAX_ParameterIdentifierMaxSize
typedef AAX_CParamID	AAX_CPageTableParamID; //!< \brief Parameter identifier used in a page table \details May be a parameter ID or a parameter name string depending on the page table formatting. Must be less than 32 characters in length (strlen of 31 or less.) \sa \ref subsection_parameter_identifiers in the \ref AAX_Page_Table_Guide
typedef const char *	AAX_CEffectID;      //!< URL-style Effect identifier.  Must be unique among all registered effects in the collection.

// Forward declarations required for AAX_Feature_UID typedef (the "real" typedef is in AAX_UIDs.h)
struct _acfUID;
typedef _acfUID acfUID;

/** Identifier for %AAX features
 
 See \ref AAX_IDescriptionHost::AcquireFeatureProperties() and \ref AAX_IFeatureInfo
 */
typedef acfUID AAX_Feature_UID;

/** Maximum size for a \ref AAX_CParamID including the null-terminating character */
AAX_CONSTEXPR size_t		kAAX_ParameterIdentifierMaxSize = 32;

static const AAX_CTimestamp		kAAX_Never = (AAX_CTimestamp) ~0ULL;


/** @brief	A cross-platform alignment macro to ensure a data type is aligned properly. */
#ifdef _TMS320C6X
	// TI's C compiler defaults to 8 byte alignment of doubles
	#define AAX_ALIGNED(v)
#elif defined(__GNUC__)
	#define AAX_ALIGNED(v) __attribute__((aligned(v)))
#elif defined(_MSC_VER)
	#define AAX_ALIGNED(v) __declspec(align(v))
#else
	#error Teach me to align data types with this compiler.
#endif


/**
 
 \todo Not used by %AAX plug-ins - remove?
 
 */
static
inline
int32_t
AAX_GetStemFormatChannelCount (
							   AAX_EStemFormat		inStemFormat)
{
	return AAX_STEM_FORMAT_CHANNEL_COUNT (inStemFormat);
}


/*! \brief %AAX algorithm audio input port data type
 *
 *	\details
 *	Audio input ports are provided with a pointer to an array
 *	of const audio buffers, with one buffer provided per input or
 *	side chain channel.
 *
 *	\todo Not used directly by %AAX plug-ins
 */
typedef	const float * const *		AAX_CAudioInPort;


/*! \brief %AAX algorithm audio output port data type
 *
 *	\details
 *	Audio output ports are provided with a pointer to an array
 *	of audio buffers, with one buffer provided per output or
 *	auxiliary output channel.
 *
 *	\todo Not used directly by %AAX plug-ins
 */
typedef float * const *		AAX_CAudioOutPort;


/*! \brief %AAX algorithm meter port data type
 *
 *	\details
 *	Meter output ports are provided with a pointer to an array
 *	of floats, with one float provided per meter tap.  The
 *	algorithm is responsible for setting these to the
 *	corresponding per-buffer peak sample values.
 *
 *	\todo Not used directly by %AAX plug-ins
 */
typedef  float * const		AAX_CMeterPort;


/*! \brief Determines whether a particular \ref AAX_CSampleRate is present
	in a given mask of \ref AAX_ESampleRateMask.
	
 	\details
	\sa kAAX_Property_SampleRate
 */
inline AAX_CBoolean sampleRateInMask(AAX_CSampleRate inSR, uint32_t iMask)
{
	return	static_cast<AAX_CBoolean>(
		(44100.0 == inSR) ? ((iMask & AAX_eSampleRateMask_44100) != 0) :
		(48000.0 == inSR) ?  ((iMask & AAX_eSampleRateMask_48000)  != 0) :
		(88200.0 == inSR) ?  ((iMask & AAX_eSampleRateMask_88200)  != 0) :
		(96000.0 == inSR) ?  ((iMask & AAX_eSampleRateMask_96000)  != 0) :
		(176400.0 == inSR) ? ((iMask & AAX_eSampleRateMask_176400) != 0) :
		(192000.0 == inSR) ? ((iMask & AAX_eSampleRateMask_192000) != 0) : false
	);
}

/*!	\brief Converts from a mask of \ref AAX_ESampleRateMask to the lowest
	supported \ref AAX_CSampleRate value in Hz
	
 */
inline AAX_CSampleRate getLowestSampleRateInMask(uint32_t iMask)
{
	return (
		((iMask & AAX_eSampleRateMask_44100)  != 0) ? 44100.0f : // AAX_eSamplRateMask_All returns 44100
		((iMask & AAX_eSampleRateMask_48000)  != 0) ? 48000.0f :
		((iMask & AAX_eSampleRateMask_88200)  != 0) ? 88200.0f :
		((iMask & AAX_eSampleRateMask_96000)  != 0) ? 96000.0f :
		((iMask & AAX_eSampleRateMask_176400) != 0) ? 176400.0f :
		((iMask & AAX_eSampleRateMask_192000) != 0) ? 192000.0f : 0.0f
	);
}

/*!	\brief Returns the \ref AAX_ESampleRateMask selector for a literal
	sample rate.
	
	The given rate must be an exact match with one of the available
	selectors. If no exact match is found then
	\ref AAX_eSampleRateMask_No is returned.
 */
inline uint32_t getMaskForSampleRate(float inSR)
{
	return (
	  (44100.0 == inSR) ?  AAX_eSampleRateMask_44100 :
	  (48000.0 == inSR) ?  AAX_eSampleRateMask_48000 :
	  (88200.0 == inSR) ?  AAX_eSampleRateMask_88200 :
	  (96000.0 == inSR) ?  AAX_eSampleRateMask_96000 :
	  (176400.0 == inSR) ? AAX_eSampleRateMask_176400 :
	  (192000.0 == inSR) ? AAX_eSampleRateMask_192000 : AAX_eSampleRateMask_No
	);
}


#ifndef _TMS320C6X

#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_HOST
#include AAX_ALIGN_FILE_END

#endif

/** \brief Plug-in chunk header
 *
 *  \legacy To ensure compatibility with TDM/RTAS plug-ins whose implementation requires \c fSize to be equal
 *  to the size of the chunk's header plus its data, AAE performs some behind-the-scenes record keeping.
 *  <BR>
 *  <BR>
 *  The following actions are only taken for %AAX plug-ins, so, e.g., if a chunk is stored by an RTAS or TDM
 *  plug-in that reports data+header size in \c fSize and this chunk is then loaded by the %AAX version of the
 *  plug-in, the header size will be cached as-is from the legacy plug-in and will be subtracted out before
 *  the chunk data is passed to the %AAX plug-in.  If a chunk is stored by an %AAX plug-in and is then loaded
 *  by a legacy plug-in, the legacy plug-in will receive the cached plug-in header with \c fSize equal to the
 *  data+header size.
 *  <BR>
 *  <BR>
 *  These are the special actions that AAE takes to ensure backwards-compatibility when handling %AAX chunk data:
 *  - When AAE retrieves the size of a chunk from an %AAX plug-in using
 *    \ref AAX_IACFEffectParameters::GetChunkSize() "GetChunkSize()", it adds the chunk header size to the
 *    amount of memory that it allocates for the chunk <BR>
 *  - When AAE retrieves a chunk from an %AAX plug-in using \ref AAX_IACFEffectParameters::GetChunk() "GetChunk()",
 *    it adds the chunk header size to \c fChunkSize before caching the chunk <BR>
 *  - Before calling \ref AAX_IACFEffectParameters::SetChunk() "SetChunk()" or
 *    \ref AAX_IACFEffectParameters::CompareActiveChunk() "CompareActiveChunk()", AAE subtracts the chunk header
 *    size from the cached chunk's header's \c fChunkSize member
 *
 */
struct AAX_SPlugInChunkHeader {
	int32_t				fSize;				///< The size of the chunk's \ref AAX_SPlugInChunk::fData "fData" member
	int32_t				fVersion;			///< The chunk's version.
	AAX_CTypeID			fManufacturerID;	///< The Plug-In's manufacturer ID
	AAX_CTypeID			fProductID;			///< The Plug-In file's product ID
	AAX_CTypeID			fPlugInID;			///< The ID of a particular Plug-In within the file
	AAX_CTypeID			fChunkID;			///< The ID of a particular Plug-In chunk.
	unsigned char		fName[32];			///< A user defined name for this chunk.
};
typedef struct AAX_SPlugInChunkHeader AAX_SPlugInChunkHeader;

/** \brief Plug-in chunk header + data
 *
 *	\sa \ref AAX_SPlugInChunkHeader
 */
struct AAX_SPlugInChunk {
	int32_t				fSize;				///< The size of the chunk's \ref AAX_SPlugInChunk::fData "fData" member
	int32_t				fVersion;			///< The chunk's version.
	AAX_CTypeID			fManufacturerID;	///< The Plug-In's manufacturer ID
	AAX_CTypeID			fProductID;			///< The Plug-In file's product ID
	AAX_CTypeID			fPlugInID;			///< The ID of a particular Plug-In within the file
	AAX_CTypeID			fChunkID;			///< The ID of a particular Plug-In chunk.
	unsigned char		fName[32];			///< A user defined name for this chunk.
	char				fData[1];			///< The chunk's data. \note The fixed-size array definition here is historical, but misleading.  Plug-ins actually write off the end of this block and are allowed to as long as they don't exceed their reported size.
};
typedef struct AAX_SPlugInChunk AAX_SPlugInChunk, *AAX_SPlugInChunkPtr;

/** \brief Plug-in Identifier Triad
 *
 *	\details
 *  This set of identifiers are what uniquely identify a particular plug-in type.
 */
struct AAX_SPlugInIdentifierTriad {
    AAX_CTypeID			mManufacturerID;	///< The Plug-In's manufacturer ID
	AAX_CTypeID			mProductID;			///< The Plug-In's product (Effect) ID
	AAX_CTypeID			mPlugInID;			///< The ID of a specific type in the product (Effect)
};
typedef struct AAX_SPlugInIdentifierTriad AAX_SPlugInIdentifierTriad, *AAX_SPlugInIdentifierTriadPtr;

#ifndef _TMS320C6X
#include AAX_ALIGN_FILE_BEGIN
#include AAX_ALIGN_FILE_RESET
#include AAX_ALIGN_FILE_END
#endif

#ifndef TI_VERSION
static inline bool operator==(const AAX_SPlugInIdentifierTriad& v1, const AAX_SPlugInIdentifierTriad& v2)
{
    return ((v1.mManufacturerID == v2.mManufacturerID) &&
			(v1.mProductID      == v2.mProductID) &&
			(v1.mPlugInID       == v2.mPlugInID));
}

static inline bool operator!=(const AAX_SPlugInIdentifierTriad& v1, const AAX_SPlugInIdentifierTriad& v2)
{
    return false == operator==(v1, v2);
}

static inline bool operator<(const AAX_SPlugInIdentifierTriad & lhs, const AAX_SPlugInIdentifierTriad & rhs)
{
    if (lhs.mManufacturerID < rhs.mManufacturerID)
        return true;
    
    if (lhs.mManufacturerID == rhs.mManufacturerID)
    {
        if (lhs.mProductID < rhs.mProductID)
            return true;
        
        if (lhs.mProductID == rhs.mProductID)
            if (lhs.mPlugInID < rhs.mPlugInID)
                return true;
    }
    return false;
}

static inline bool operator>=(const AAX_SPlugInIdentifierTriad & lhs, const AAX_SPlugInIdentifierTriad & rhs)
{
    return false == operator<(lhs, rhs);
}

static inline bool operator>(const AAX_SPlugInIdentifierTriad & lhs, const AAX_SPlugInIdentifierTriad & rhs)
{
	return operator>=(lhs, rhs) && operator!=(lhs, rhs);
}

static inline bool operator<=(const AAX_SPlugInIdentifierTriad & lhs, const AAX_SPlugInIdentifierTriad & rhs)
{
	return false == operator>(lhs, rhs);
}
#endif //TI_VERSION


//<DMT> For historical compatibility with PT10, we have to make the MIDI structures DEFAULT aligned instead of ALG aligned.  With PT11 and 64 bit, these will now be ALG aligned.
#if ( defined(_WIN64) || defined(__LP64__) || defined(_TMS320C6X) )
	#include AAX_ALIGN_FILE_BEGIN
	#include AAX_ALIGN_FILE_ALG
	#include AAX_ALIGN_FILE_END
#else
    #if defined (__GNUC__)
        #pragma options align=power // To maintain backwards-compatibility with pre-10 versions of Pro Tools
    #else // Windows, other
		#include AAX_ALIGN_FILE_BEGIN
        #include AAX_ALIGN_FILE_HOST
		#include AAX_ALIGN_FILE_END
    #endif
#endif

/*! \brief Packet structure for MIDI data

 	\details
	\sa AAX_CMidiStream
 
	\legacy Corresponds to DirectMidiPacket in the legacy SDK
 */
struct AAX_CMidiPacket 
{
	uint32_t			mTimestamp;     //!< This is the playback time at which the MIDI event should occur, relative to the beginning of the current audio buffer.
	uint32_t			mLength;        //!< The length of MIDI message, in terms of bytes.
	unsigned char       mData[4];		//!< The MIDI message itself. Each array element is one byte of the message, with the 0th element being the first byte.
	AAX_CBoolean        mIsImmediate;   //!< Indicates that the message is to be sent as soon as possible.	\compatibility This value is not currently set. Use <tt>mTimestamp == 0</tt> to detect immediate packets
};

/*! \brief MIDI stream data structure used by \ref AAX_IMIDINode
	
 	\details
	For \ref AAX_eMIDINodeType_LocalInput "MIDI input", mBufferSize is set by the %AAX host
	when the buffer is filled.  
	
	For \ref AAX_eMIDINodeType_LocalOutput "MIDI output", the plug-in sets mBufferSize with
	the number of \ref AAX_CMidiPacket objects it has filled mBuffer with.  The %AAX host
	will reset mBufferSize to 0 after it has received the buffer of MIDI.
 
	System Exclusive (SysEx) messages that are greater than 4 bytes in length can be
	transmitted via a series of concurrent \ref AAX_CMidiPacket objects in mBuffer. In
	accordance with the MIDI Specification, \c 0xF0 indicates the beginning of a SysEx
	message and \c 0xF7 indicates its end.

	\legacy Corresponds to DirectMidiNode in the legacy SDK
 */
struct AAX_CMidiStream
{
	uint32_t			mBufferSize;		//!< The number of \ref AAX_CMidiPacket objects contained in the node's buffer. 
	AAX_CMidiPacket*		mBuffer;		//!< Pointer to the first element of the node's buffer.
};	

#if ( defined(_WIN64) || defined(__LP64__) || defined(_TMS320C6X) )
	#include AAX_ALIGN_FILE_BEGIN
	#include AAX_ALIGN_FILE_RESET
	#include AAX_ALIGN_FILE_END
#else
    #if defined (__GNUC__)
        #pragma pack()  // To maintian backwards-compatibility with pre-10 versions of Pro Tools
    #else
		#include AAX_ALIGN_FILE_BEGIN
        #include AAX_ALIGN_FILE_RESET
		#include AAX_ALIGN_FILE_END
    #endif
#endif

/// @cond ignore
#endif // #ifndef _AAX_H_
/// @endcond
