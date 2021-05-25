//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/vsttypes.h
// Created by  : Steinberg, 12/2005
// Description : Common Defines
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses. 
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/fstrdefs.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
//------------------------------------------------------------------------
#ifndef kVstVersionString
#define kVstVersionString	"VST 3.6.13"	///< SDK version for PClassInfo2
#endif

#define kVstVersionMajor	3
#define kVstVersionMinor	6
#define kVstVersionSub		13

// this allows to write things like: #if VST_VERSION >= 0x030500 // note that 3.10.0 is 0x030a00
#define VST_VERSION ((kVstVersionMajor << 16) | (kVstVersionMinor << 8) | kVstVersionSub)

//------------------------------------------------------------------------
/** \defgroup vst3typedef VST 3 Data Types
*/
/*@{*/
//------------------------------------------------------------------------
// String Types
//------------------------------------------------------------------------
typedef char16 TChar;			///< UTF-16 character
typedef TChar String128[128];	///< 128 character UTF-16 string
typedef const char8* CString;	///< C-String

//------------------------------------------------------------------------
// General
//------------------------------------------------------------------------
typedef int32 MediaType;		///< media type (audio/event)
typedef int32 BusDirection;		///< bus direction (in/out)
typedef int32 BusType;			///< bus type (main/aux)
typedef int32 IoMode;			///< I/O mode (see \ref vst3IoMode)
typedef int32 UnitID;			///< unit identifier
typedef double ParamValue;		///< parameter value type
typedef uint32 ParamID;			///< parameter identifier
typedef int32 ProgramListID;	///< program list identifier
typedef int16 CtrlNumber;		///< MIDI controller number (see \ref ControllerNumbers for allowed values)

typedef double TQuarterNotes;	///< time expressed in quarter notes
typedef int64 TSamples;			///< time expressed in audio samples

typedef uint32 ColorSpec;		///< color defining by 4 component ARGB value (Alpha/Red/Green/Blue)

//------------------------------------------------------------------------
static const ParamID kNoParamId = 0xffffffff;	///< default for uninitialized parameter ID
// static const ParamID kNoParamId = std::numeric_limits<ParamID>::max ();

//------------------------------------------------------------------------
// Audio Types
//------------------------------------------------------------------------
typedef float Sample32;				///< 32-bit precision audio sample
typedef double Sample64;			///< 64-bit precision audio sample
typedef double SampleRate;			///< sample rate

//------------------------------------------------------------------------
// Speaker Arrangements Types
//------------------------------------------------------------------------
typedef uint64 SpeakerArrangement;	///< Bitset of speakers
typedef uint64 Speaker;	            ///< Bit for one speaker

/*@}*/

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
