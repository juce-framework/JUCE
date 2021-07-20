/*!
	@file		AudioUnitSDK/AUMIDIUtility.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUMIDIUtility_h
#define AudioUnitSDK_AUMIDIUtility_h

// OS
#if defined __has_include && __has_include(<AvailabilityVersions.h>)
#include <AvailabilityVersions.h>
#endif
#if defined(__MAC_12_0) || defined(__IPHONE_15_0)
#define AUSDK_MIDI2_AVAILABLE 1
#endif

#if AUSDK_MIDI2_AVAILABLE
#include <CoreMIDI/MIDIServices.h>
#endif

#endif // AudioUnitSDK_AUMIDIUtility_h
