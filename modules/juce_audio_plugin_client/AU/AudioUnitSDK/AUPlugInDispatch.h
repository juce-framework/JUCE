/*!
	@file		AudioUnitSDK/AUPlugInDispatch.h
	@copyright	© 2000-2021 Apple Inc. All rights reserved.
*/
#ifndef AudioUnitSDK_AUPlugInDispatch_h
#define AudioUnitSDK_AUPlugInDispatch_h

#include <AudioUnitSDK/ComponentBase.h>

namespace ausdk {

/// Method lookup for a basic AUBase subclass.
struct AUBaseLookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for a basic AUBase subclass.
template <class Implementor>
class AUBaseFactory : public APFactory<AUBaseLookup, Implementor> {
};

/// Method lookup for a AUBase subclass which implements I/O methods (Start, Stop).
struct AUOutputLookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for an AUBase subclass which implements I/O methods (Start, Stop).
template <class Implementor>
class AUOutputBaseFactory : public APFactory<AUOutputLookup, Implementor> {
};

/// Method lookup for an AUBase subclass which implements I/O methods (Start, Stop) and
/// ComplexRender.
struct AUComplexOutputLookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for an AUBase subclass which implements I/O methods (Start, Stop) and ComplexRender.
template <class Implementor>
class AUOutputComplexBaseFactory : public APFactory<AUComplexOutputLookup, Implementor> {
};

/// Method lookup for an AUBase subclass which implements Process.
struct AUBaseProcessLookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for an AUBase subclass which implements Process.
template <class Implementor>
class AUBaseProcessFactory : public APFactory<AUBaseProcessLookup, Implementor> {
};

/// Method lookup for an AUBase subclass which implements ProcessMultiple.
struct AUBaseProcessMultipleLookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for an AUBase subclass which implements ProcessMultiple.
template <class Implementor>
class AUBaseProcessMultipleFactory : public APFactory<AUBaseProcessMultipleLookup, Implementor> {
};

/// Method lookup for an AUBase subclass which implements Process and ProcessMultiple.
struct AUBaseProcessAndMultipleLookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for an AUBase subclass which implements Process and ProcessMultiple.
template <class Implementor>
class AUBaseProcessAndMultipleFactory
	: public APFactory<AUBaseProcessAndMultipleLookup, Implementor> {
};

/// Method lookup for an AUBase subclass which implements MusicDevice methods (MIDIEvent and SysEx).
struct AUMIDILookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for an AUBase subclass which implements MusicDevice methods (MIDIEvent and SysEx).
template <class Implementor>
class AUMIDIEffectFactory : public APFactory<AUMIDILookup, Implementor> {
};

/// Method lookup for an AUBase subclass which implements Process and MusicDevice methods (MIDIEvent
/// and SysEx).
struct AUMIDIProcessLookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for an AUBase subclass which implements Process and MusicDevice methods (MIDIEvent
/// and SysEx).
template <class Implementor>
class AUMIDIProcessFactory : public APFactory<AUMIDIProcessLookup, Implementor> {
};

/// Method lookup for an AUBase subclass which implements the full set of MusicDevice methods
/// (MIDIEvent, SysEx, StartNote, StopNote).
struct AUMusicLookup {
	static AudioComponentMethod Lookup(SInt16 selector);
};

/// Factory for an AUBase subclass which implements the full set of MusicDevice methods
/// (MIDIEvent, SysEx, StartNote, StopNote).
template <class Implementor>
class AUMusicDeviceFactory : public APFactory<AUMusicLookup, Implementor> {
};

} // namespace ausdk

#endif // AudioUnitSDK_AUPlugInDispatch_h
