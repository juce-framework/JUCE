//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : SDK Core Interfaces
// Filename    : pluginterfaces/base/falignpop.h
// Created by  : Steinberg, 01/2004
// Description : Restore alignment settings
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
#if SMTG_OS_MACOS
	#if TARGET_API_MAC_CARBON
		#if SMTG_PLATFORM_64
			#pragma pack(pop)
		#else
			#pragma options align=reset
		#endif
	#endif
#elif defined __BORLANDC__
	#pragma -a-
#elif SMTG_OS_WINDOWS
	#pragma pack(pop)
#endif
//---------------------------------------------------------------------------------------------------
