//************************************************************************************************
//
// PreSonus Plug-In Extensions
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.
//
// Filename    : pslauextensions.h
// Created by  : PreSonus Software Ltd., 08/2017, last updated 10/2017
// Description : PreSonus-specific AU API Extensions
//
//************************************************************************************************
/*
	DISCLAIMER:
	The PreSonus Plug-In Extensions are host-specific extensions of existing proprietary technologies,
	provided to the community on an AS IS basis. They are not part of any official 3rd party SDK and
	PreSonus is not affiliated with the owner of the underlying technology in any way.
 */
//************************************************************************************************

#ifndef _pslauextensions_h
#define _pslauextensions_h

#ifdef __cplusplus
namespace Presonus {
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
// Property IDs
//////////////////////////////////////////////////////////////////////////////////////////////////

/** This AU property in the global scope is of type CFArrayRef and is writable by the host. 
	The elements of the array are of type CFDataRef which encapsulate SlaveMode structures.
	For more details, please check the documentation of Presonus::ISlaveControllerHandler. */
static const AudioUnitPropertyID kSlaveEffectsPropID = 0x50534C01;

//////////////////////////////////////////////////////////////////////////////////////////////////
// Data types
//////////////////////////////////////////////////////////////////////////////////////////////////

enum SlaveMode
{
	kSlaveModeNormal,            ///< plug-in used in different location following given master
	kSlaveModeLowLatencyClone    ///< plug-in used as hidden slave for low latency processing following given master
};

//////////////////////////////////////////////////////////////////////////////////////////////////

struct SlaveEffect
{
	AudioUnit unit;				///< Audio Unit reference
	SInt32 mode;				///< SlaveMode
};

#ifdef __cplusplus
}
#endif

#endif // _pslauextensions_h
