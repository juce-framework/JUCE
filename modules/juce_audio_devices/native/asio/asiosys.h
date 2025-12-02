//------------------------------------------------------------------------
// Project     : ASIO SDK
//
// Category    : Interfaces
// Filename    : common/asiosys.h
// Created by  : Steinberg, 05/1996
// Description : Steinberg Audio Stream I/O
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses. 
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#ifndef __asiosys__
	#define __asiosys__

	#if defined(_WIN32) || defined(_WIN64)
		#undef MAC 
		#define PPC 0
		#define WINDOWS 1
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0

		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
	
	#elif BEOS
		#define MAC 0
		#define PPC 0
		#define WINDOWS 0
		#define PC 0
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		
		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
		
		#ifndef DEBUG
			#define DEBUG 0
		 	#if DEBUG
		 		void DEBUGGERMESSAGE(char *string);
		 	#else
		  		#define DEBUGGERMESSAGE(a)
			#endif
		#endif

	#elif SGI
		#define MAC 0
		#define PPC 0
		#define WINDOWS 0
		#define PC 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0
		
		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
		
		#ifndef DEBUG
			#define DEBUG 0
		 	#if DEBUG
		 		void DEBUGGERMESSAGE(char *string);
		 	#else
		  		#define DEBUGGERMESSAGE(a)
			#endif
		#endif

	#else	// MAC
		#define MAC 1
		#define PPC 1
		#define WINDOWS 0
		#define PC 0
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0

		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1

		#ifndef DEBUG
			#define DEBUG 0
			#if DEBUG
				void DEBUGGERMESSAGE(char *string);
			#else
				#define DEBUGGERMESSAGE(a)
			#endif
		#endif
	#endif

#endif
