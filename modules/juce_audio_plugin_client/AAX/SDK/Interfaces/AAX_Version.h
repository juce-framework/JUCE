/*================================================================================================*/
/*

 *	Copyright 2013-2017, 2019, 2021-2024 Avid Technology, Inc.
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
 *	\file   AAX_Version.h
 *	
 *	\brief Version stamp header for the %AAX SDK
 *
 *  This file defines a unique number that can be used to identify the version of the %AAX SDK
 */ 
/*================================================================================================*/


#pragma once

#ifndef _AAX_VERSION_H_
#define _AAX_VERSION_H_


/** \brief The SDK's version number
 *  
 *	\details
 *  This version number is generally updated only when changes have been
 *  made to the %AAX binary interface
 *  
 *  - The first byte is the major version number
 *  - The second byte is the minor version number
 *
 *  For example:
 *  - SDK 1.0.5  > \c 0x0100
 *  - SDK 10.2.1 > \c 0x0A02
 *
 */
#define AAX_SDK_VERSION ( 0x0206 )

/** \brief An atomic revision number for the source included in this SDK
 */
#define AAX_SDK_CURRENT_REVISION ( 20207000 )


#define AAX_SDK_1p0p1_REVISION ( 3712639 )
#define AAX_SDK_1p0p2_REVISION ( 3780585 )
#define AAX_SDK_1p0p3_REVISION ( 3895859 )
#define AAX_SDK_1p0p4_REVISION ( 4333589 )
#define AAX_SDK_1p0p5_REVISION ( 4598560 )
#define AAX_SDK_1p0p6_REVISION ( 5051497 )
#define AAX_SDK_1p5p0_REVISION ( 5740047 )
#define AAX_SDK_2p0b1_REVISION ( 6169787 )
#define AAX_SDK_2p0p0_REVISION ( 6307708 )
#define AAX_SDK_2p0p1_REVISION ( 6361692 )
#define AAX_SDK_2p1p0_REVISION ( 7820991 )
#define AAX_SDK_2p1p1_REVISION ( 8086416 )
#define AAX_SDK_2p2p0_REVISION ( 9967334 )
#define AAX_SDK_2p2p1_REVISION ( 10693954 )
#define AAX_SDK_2p2p2_REVISION ( 11819832 )
#define AAX_SDK_2p3p0_REVISION ( 12546840 )
#define AAX_SDK_2p3p1_REVISION ( 13200373 )
#define AAX_SDK_2p3p2_REVISION ( 14017972 )
#define AAX_SDK_2p4p0_REVISION ( 20204000 )
#define AAX_SDK_2p4p1_REVISION ( 20204010 )
#define AAX_SDK_2p5p0_REVISION ( 20205000 )
#define AAX_SDK_2p6p0_REVISION ( 20206000 )
#define AAX_SDK_2p6p1_REVISION ( 20206001 )
#define AAX_SDK_2p7p0_REVISION ( 20207000 )
//CURREVSTAMP < do not remove this comment



#endif // #ifndef _AAX_VERSION_H_
