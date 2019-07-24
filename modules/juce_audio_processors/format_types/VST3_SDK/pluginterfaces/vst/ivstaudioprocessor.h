//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Interfaces
// Filename    : pluginterfaces/vst/ivstaudioprocessor.h
// Created by  : Steinberg, 10/2005
// Description : VST Audio Processing Interfaces
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses. 
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "ivstcomponent.h"
#include "vstspeaker.h"

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpush.h"
//------------------------------------------------------------------------

//------------------------------------------------------------------------
/** Class Category Name for Audio Processor Component */
//------------------------------------------------------------------------
#ifndef kVstAudioEffectClass
#define kVstAudioEffectClass "Audio Module Class"
#endif

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
class IEventList;
class IParameterChanges;
struct ProcessContext;

//------------------------------------------------------------------------
/** Component Types used as subCategories in PClassInfo2 */
//------------------------------------------------------------------------
namespace PlugType
{
/**
\defgroup plugType Plug-in Type used for subCategories */
/*@{*/
//------------------------------------------------------------------------
const CString kFxAnalyzer			= "Fx|Analyzer";	///< Scope, FFT-Display, Loudness Processing...
const CString kFxDelay				= "Fx|Delay";		///< Delay, Multi-tap Delay, Ping-Pong Delay...
const CString kFxDistortion			= "Fx|Distortion";	///< Amp Simulator, Sub-Harmonic, SoftClipper...
const CString kFxDynamics			= "Fx|Dynamics";	///< Compressor, Expander, Gate, Limiter, Maximizer, Tape Simulator, EnvelopeShaper...
const CString kFxEQ					= "Fx|EQ";			///< Equalization, Graphical EQ...
const CString kFxFilter				= "Fx|Filter";		///< WahWah, ToneBooster, Specific Filter,...
const CString kFx					= "Fx";				///< others type (not categorized)
const CString kFxInstrument			= "Fx|Instrument";	///< Fx which could be loaded as Instrument too
const CString kFxInstrumentExternal	= "Fx|Instrument|External";	///< Fx which could be loaded as Instrument too and is external (wrapped Hardware)
const CString kFxSpatial			= "Fx|Spatial";		///< MonoToStereo, StereoEnhancer,...
const CString kFxGenerator			= "Fx|Generator";	///< Tone Generator, Noise Generator...
const CString kFxMastering			= "Fx|Mastering";	///< Dither, Noise Shaping,...
const CString kFxModulation			= "Fx|Modulation";	///< Phaser, Flanger, Chorus, Tremolo, Vibrato, AutoPan, Rotary, Cloner...
const CString kFxPitchShift			= "Fx|Pitch Shift";	///< Pitch Processing, Pitch Correction, Vocal Tuning...
const CString kFxRestoration		= "Fx|Restoration";	///< Denoiser, Declicker,...
const CString kFxReverb				= "Fx|Reverb";		///< Reverberation, Room Simulation, Convolution Reverb...
const CString kFxSurround			= "Fx|Surround";	///< dedicated to surround processing: LFE Splitter, Bass Manager...
const CString kFxTools				= "Fx|Tools";		///< Volume, Mixer, Tuner...
const CString kFxNetwork			= "Fx|Network";		///< using Network

const CString kInstrument			= "Instrument";			///< Effect used as instrument (sound generator), not as insert
const CString kInstrumentDrum		= "Instrument|Drum";	///< Instrument for Drum sounds
const CString kInstrumentExternal	= "Instrument|External";///< External Instrument (wrapped Hardware)
const CString kInstrumentPiano		= "Instrument|Piano";	///< Instrument for Piano sounds
const CString kInstrumentSampler	= "Instrument|Sampler";	///< Instrument based on Samples
const CString kInstrumentSynth		= "Instrument|Synth";	///< Instrument based on Synthesis
const CString kInstrumentSynthSampler = "Instrument|Synth|Sampler";	///< Instrument based on Synthesis and Samples

const CString kSpatial				= "Spatial";		///< used for SurroundPanner
const CString kSpatialFx			= "Spatial|Fx";		///< used for SurroundPanner and as insert effect
const CString kOnlyRealTime			= "OnlyRT";			///< indicates that it supports only realtime process call, no processing faster than realtime
const CString kOnlyOfflineProcess	= "OnlyOfflineProcess";	///< used for Plug-in offline processing  (will not work as normal insert Plug-in)
const CString kNoOfflineProcess		= "NoOfflineProcess";	///< will be NOT used for Plug-in offline processing (will work as normal insert Plug-in)
const CString kUpDownMix			= "Up-Downmix";		///< used for Mixconverter/Up-Mixer/Down-Mixer
const CString kAnalyzer			    = "Analyzer";	    ///< Meter, Scope, FFT-Display, not selectable as insert plugin
const CString kAmbisonics			= "Ambisonics";		///< used for Ambisonics channel (FX or Panner/Mixconverter/Up-Mixer/Down-Mixer when combined with other category)

const CString kMono					= "Mono";			///< used for Mono only Plug-in [optional]
const CString kStereo				= "Stereo";			///< used for Stereo only Plug-in [optional]
const CString kSurround				= "Surround";		///< used for Surround only Plug-in [optional]

//------------------------------------------------------------------------
/*@}*/
}

//------------------------------------------------------------------------
/** Component Flags used as classFlags in PClassInfo2 */
//------------------------------------------------------------------------
enum ComponentFlags
{
//------------------------------------------------------------------------
	kDistributable			= 1 << 0,	///< Component can be run on remote computer
	kSimpleModeSupported	= 1 << 1	///< Component supports simple IO mode (or works in simple mode anyway) see \ref vst3IoMode
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/** Symbolic sample size.
\see ProcessSetup, ProcessData */
//------------------------------------------------------------------------
enum SymbolicSampleSizes
{
	kSample32,		///< 32-bit precision
	kSample64		///< 64-bit precision
};

//------------------------------------------------------------------------
/** Processing mode informs the Plug-in about the context and at which frequency the process call is called.
VST3 defines 3 modes:
- kRealtime: each process call is called at a realtime frequency (defined by [numSamples of ProcessData] / samplerate).
             The Plug-in should always try to process as fast as possible in order to let enough time slice to other Plug-ins.
- kPrefetch: each process call could be called at a variable frequency (jitter, slower / faster than realtime),
             the Plug-in should process at the same quality level than realtime, Plug-in must not slow down to realtime
			 (e.g. disk streaming)!
			 The host should avoid to process in kPrefetch mode such sampler based Plug-in.
- kOffline:  each process call could be faster than realtime or slower, higher quality than realtime could be used.
             Plug-ins using disk streaming should be sure that they have enough time in the process call for streaming,
			 if needed by slowing down to realtime or slower.
.
Note about Process Modes switching:
	-Switching between kRealtime and kPrefetch process modes are done in realtime thread without need of calling
	 IAudioProcessor::setupProcessing, the Plug-in should check in process call the member processMode of ProcessData
	 in order to know in which mode it is processed.
	-Switching between kRealtime (or kPrefetch) and kOffline requires that the host calls IAudioProcessor::setupProcessing
	 in order to inform the Plug-in about this mode change.
.
\see ProcessSetup, ProcessData */
//------------------------------------------------------------------------
enum ProcessModes
{
	kRealtime,		///< realtime processing
	kPrefetch,		///< prefetch processing
	kOffline		///< offline processing
};

//------------------------------------------------------------------------
/** kNoTail
 *
 * to be returned by getTailSamples when no tail is wanted
 \see IAudioProcessor::getTailSamples */
//------------------------------------------------------------------------
static const uint32 kNoTail = 0;
//------------------------------------------------------------------------
/** kInfiniteTail
 *
 * to be returned by getTailSamples when infinite tail is wanted
 \see IAudioProcessor::getTailSamples */
//------------------------------------------------------------------------
static const uint32 kInfiniteTail = kMaxInt32u;

//------------------------------------------------------------------------
/** Audio processing setup.
\see IAudioProcessor::setupProcessing */
//------------------------------------------------------------------------
struct ProcessSetup
{
//------------------------------------------------------------------------
	int32 processMode;			///< \ref ProcessModes
	int32 symbolicSampleSize;	///< \ref SymbolicSampleSizes
	int32 maxSamplesPerBlock;	///< maximum number of samples per audio block
	SampleRate sampleRate;		///< sample rate
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/** Processing buffers of an audio bus.
This structure contains the processing buffer for each channel of an audio bus.
- The number of channels (numChannels) must always match the current bus arrangement.
  It could be set to value '0' when the host wants to flush the parameters (when the Plug-in is not processed).
- The size of the channel buffer array must always match the number of channels. So the host
  must always supply an array for the channel buffers, regardless if the
  bus is active or not. However, if an audio bus is currently inactive, the actual sample
  buffer addresses are safe to be null.
- The silence flag is set when every sample of the according buffer has the value '0'. It is
  intended to be used as help for optimizations allowing a Plug-in to reduce processing activities.
  But even if this flag is set for a channel, the channel buffers must still point to valid memory!
  This flag is optional. A host is free to support it or not.
.
\see ProcessData */
//------------------------------------------------------------------------
struct AudioBusBuffers
{
	AudioBusBuffers () : numChannels (0), silenceFlags (0), channelBuffers64 (0) {}

//------------------------------------------------------------------------
	int32 numChannels;		///< number of audio channels in bus
	uint64 silenceFlags;	///< Bitset of silence state per channel
	union
	{
		Sample32** channelBuffers32;	///< sample buffers to process with 32-bit precision
		Sample64** channelBuffers64;	///< sample buffers to process with 64-bit precision
	};
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/** Any data needed in audio processing.
	The host prepares AudioBusBuffers for each input/output bus,
	regardless of the bus activation state. Bus buffer indices always match
	with bus indices used in IComponent::getBusInfo of media type kAudio.
\see AudioBusBuffers, IParameterChanges, IEventList, ProcessContext */
//------------------------------------------------------------------------
struct ProcessData
{
	ProcessData ()
	: processMode (0), symbolicSampleSize (kSample32), numSamples (0), numInputs (0)
	, numOutputs (0), inputs (0), outputs (0), inputParameterChanges (0), outputParameterChanges (0)
	, inputEvents (0), outputEvents (0), processContext (0) {}

//------------------------------------------------------------------------
	int32 processMode;			///< processing mode - value of \ref ProcessModes
	int32 symbolicSampleSize;   ///< sample size - value of \ref SymbolicSampleSizes
	int32 numSamples;			///< number of samples to process
	int32 numInputs;			///< number of audio input buses
	int32 numOutputs;			///< number of audio output buses
	AudioBusBuffers* inputs;	///< buffers of input buses
	AudioBusBuffers* outputs;	///< buffers of output buses

	IParameterChanges* inputParameterChanges;	///< incoming parameter changes for this block
	IParameterChanges* outputParameterChanges;	///< outgoing parameter changes for this block (optional)
	IEventList* inputEvents;				///< incoming events for this block (optional)
	IEventList* outputEvents;				///< outgoing events for this block (optional)
	ProcessContext* processContext;			///< processing context (optional, but most welcome)
//------------------------------------------------------------------------
};

//------------------------------------------------------------------------
/** Audio Processing Interface.
\ingroup vstIPlug vst300
- [plug imp]
- [extends IComponent]
- [released: 3.0.0]
- [mandatory]

This interface must always be supported by audio processing Plug-ins. */
//------------------------------------------------------------------------
class IAudioProcessor: public FUnknown
{
public:
//------------------------------------------------------------------------
	/** Try to set (from host) a predefined arrangement for inputs and outputs.
	    The host should always deliver the same number of input and output buses than the Plug-in needs 
		(see \ref IComponent::getBusCount).
		The Plug-in returns kResultFalse if wanted arrangements are not supported.
		If the Plug-in accepts these arrangements, it should modify its buses to match the new arrangements
		(asked by the host with IComponent::getInfo () or IAudioProcessor::getBusArrangement ()) and then return kResultTrue.
		If the Plug-in does not accept these arrangements, but can adapt its current arrangements (according to the wanted ones),
		it should modify its buses arrangements and return kResultFalse. */
	virtual tresult PLUGIN_API setBusArrangements (SpeakerArrangement* inputs, int32 numIns,
												   SpeakerArrangement* outputs, int32 numOuts) = 0;

	/** Gets the bus arrangement for a given direction (input/output) and index.
		Note: IComponent::getInfo () and IAudioProcessor::getBusArrangement () should be always return the same 
		information about the buses arrangements. */
	virtual tresult PLUGIN_API getBusArrangement (BusDirection dir, int32 index, SpeakerArrangement& arr) = 0;

	/** Asks if a given sample size is supported see \ref SymbolicSampleSizes. */
	virtual tresult PLUGIN_API canProcessSampleSize (int32 symbolicSampleSize) = 0;

	/** Gets the current Latency in samples.
		The returned value defines the group delay or the latency of the Plug-in. For example, if the Plug-in internally needs
		to look in advance (like compressors) 512 samples then this Plug-in should report 512 as latency.
		If during the use of the Plug-in this latency change, the Plug-in has to inform the host by
		using IComponentHandler::restartComponent (kLatencyChanged), this could lead to audio playback interruption
		because the host has to recompute its internal mixer delay compensation.
		Note that for player live recording this latency should be zero or small. */
	virtual uint32 PLUGIN_API getLatencySamples () = 0;

	/** Called in disable state (not active) before processing will begin. */
	virtual tresult PLUGIN_API setupProcessing (ProcessSetup& setup) = 0;

	/** Informs the Plug-in about the processing state. This will be called before any process calls start with true and after with false.
		Note that setProcessing (false) may be called after setProcessing (true) without any process calls.
		In this call the Plug-in should do only light operation (no memory allocation or big setup reconfiguration), 
		this could be used to reset some buffers (like Delay line or Reverb). */
	virtual tresult PLUGIN_API setProcessing (TBool state) = 0;

	/** The Process call, where all information (parameter changes, event, audio buffer) are passed. */
	virtual tresult PLUGIN_API process (ProcessData& data) = 0;

	/** Gets tail size in samples. For example, if the Plug-in is a Reverb Plug-in and it knows that
		the maximum length of the Reverb is 2sec, then it has to return in getTailSamples() 
		(in VST2 it was getGetTailSize ()): 2*sampleRate.
		This information could be used by host for offline processing, process optimization and 
		downmix (avoiding signal cut (clicks)).
		It should return:
		 - kNoTail when no tail
		 - x * sampleRate when x Sec tail.
		 - kInfiniteTail when infinite tail. */
	virtual uint32 PLUGIN_API getTailSamples () = 0;

//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IAudioProcessor, 0x42043F99, 0xB7DA453C, 0xA569E79D, 0x9AAEC33D)

//------------------------------------------------------------------------
/** Extended IAudioProcessor interface for a component.
\ingroup vstIPlug vst310
- [plug imp]
- [extends IAudioProcessor]
- [released: 3.1.0]
- [optional]

Inform the Plug-in about how long from the moment of generation/acquiring (from file or from Input)
it will take for its input to arrive, and how long it will take for its output to be presented (to output or to Speaker).

Note for Input Presentation Latency: when reading from file, the first Plug-in will have an input presentation latency set to zero.
When monitoring audio input from a Audio Device, then this initial input latency will be the input latency of the Audio Device itself.

Note for Output Presentation Latency: when writing to a file, the last Plug-in will have an output presentation latency set to zero.
When the output of this Plug-in is connected to a Audio Device then this initial output latency will be the output
latency of the Audio Device itself.

A value of zero means either no latency or an unknown latency.

Each Plug-in adding a latency (returning a none zero value for IAudioProcessor::getLatencySamples) will modify the input 
presentation latency of the next Plug-ins in the mixer routing graph and will modify the output presentation latency 
of the previous Plug-ins.

\n
\image html "iaudiopresentationlatency_usage.png"
\n
\see IAudioProcessor
\see IComponent*/
//------------------------------------------------------------------------
class IAudioPresentationLatency: public FUnknown
{
public:
	//------------------------------------------------------------------------
	/** Informs the Plug-in about the Audio Presentation Latency in samples for a given direction (kInput/kOutput) and bus index. */
	virtual tresult PLUGIN_API setAudioPresentationLatencySamples (BusDirection dir, int32 busIndex, uint32 latencyInSamples) = 0;

	//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IAudioPresentationLatency, 0x309ECE78, 0xEB7D4fae, 0x8B2225D9, 0x09FD08B6)

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg

//------------------------------------------------------------------------
#include "pluginterfaces/base/falignpop.h"
//------------------------------------------------------------------------
