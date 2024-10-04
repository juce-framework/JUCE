/*================================================================================================*/
/*
 *
 *	Copyright 2014-2015, 2018, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_Push2ByteStructAlignment.h
 *	
 *	\brief Set the struct alignment to 2-byte. This file will throw an error on platforms that do
 *	not support 2-byte alignment (i.e. TI DSPs)
 *
 *	\details
 *	When setting the alignment for a struct in order to match a particular environment (e.g.
 *	host/plug-in binary compatibility) the following macros are recommended:
 *	\li \ref AAX_ALIGN_FILE_HOST
 *	\li \ref AAX_ALIGN_FILE_ALG
 *	\li \ref AAX_ALIGN_FILE_RESET
 *
 *	 \section AAX_Push2ByteStructAlignment_usagenotes Usage notes
 *	
 *		\li Always follow an inclusion of this file with a matching
 *			inclusion of AAX_PopStructAlignment.h
 *		
 *		\li Do not place other file \c \#include after this file.  For example:
 *	\code
 *	// HeaderFile1.h
 *		#include AAX_Push2ByteStructAlignment.h
 *		#include HeaderFile2.h	// this file now has 2-byte alignment also!!
 *		// HeaderFile1.h definitions...
 *		#include AAX_PopStructAlignment.h
 *	// end HeaderFile1.h
 *	\endcode
 *				This will cause problems if HeaderFile2.h is included elsewhere without the 2-byte 
 *				alignment which will manifest as hard to find run-time bugs. The proper usage is:
 *	\code
 *	// HeaderFile1.h
 *		#include HeaderFile2.h
 *		#include AAX_Push2ByteStructAlignment.h
 *			// HeaderFile1.h definitions...
 *		#include AAX_PopStructAlignment.h
 *	// end HeaderFile1.h
 *	\endcode
 *				
 *	\sa \ref AAX_Push4ByteStructAlignment.h
 *	\sa \ref AAX_Push8ByteStructAlignment.h
 *	\sa \ref AAX_PopStructAlignment.h
 *				
 *	\internal
 *		NOTE: we don't use include guards for this file because it *is* possible to 
 *		include this file multiple times in the same file.
 *	\endinternal
 *
 *
 */ 
/*================================================================================================*/

#ifdef _TMS320C6X
#error "TI structure packing changes not supported"
#elif defined (_MSC_VER)
#pragma warning( disable : 4103 ) // used #pragma pack to change alignment
#pragma pack(push, 2)
#elif defined (__GNUC__)
// Uncomment this warning suppression if you really want to apply packing to a virtual data
// structure, but note that there is no guarantee of cross-platform compatibility for such
// a structure. For more information, see the AAX_ALIGN_FILE_ALG macro documentation
//	#ifdef __clang__
//		#pragma clang diagnostic push
//		#pragma clang diagnostic ignored "-Wno-incompatible-ms-struct"
//	#endif
#pragma ms_struct on
//	#ifdef __clang__
//		#pragma clang diagnostic pop
//	#endif
#pragma pack(push, 2)
#elif defined (__MWERKS__)
#pragma options align=mac68k
#else
#error "You need to supply a pragma here to set structure alignment to 2 bytes"
#endif

// Nesting of struct alignment headers is not allowed
#ifdef __AAX_CUSTOM_STRUCT_ALIGN_IS_SET__
#error "Nested AAX struct alignment directives"
#else
#define __AAX_CUSTOM_STRUCT_ALIGN_IS_SET__
#endif
