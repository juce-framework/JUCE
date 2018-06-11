//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 2.4
//
// Category    : VST 2.x Interfaces
// Filename    : pluginterfaces/vst2.x/aeffectx.h
// Created by  : Steinberg, 01/2004
// Description : Definition of auxiliary structures, extensions from VST 1.0 to VST 2.4
// 
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// This Software Development Kit may not be distributed in parts or its entirety  
// without prior written agreement by Steinberg Media Technologies GmbH. 
// This SDK must not be used to re-engineer or manipulate any technology used  
// in any Steinberg or Third-party application or software module, 
// unless permitted by law.
// Neither the name of the Steinberg Media Technologies nor the names of its
// contributors may be used to endorse or promote products derived from this 
// software without specific prior written permission.
// 
// THIS SDK IS PROVIDED BY STEINBERG MEDIA TECHNOLOGIES GMBH "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL STEINBERG MEDIA TECHNOLOGIES GMBH BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------------------------

#ifndef __aeffectx__
#define __aeffectx__

// VST 1.0 is included
#ifndef __aeffect__
#include "aeffect.h"
#endif

//-------------------------------------------------------------------------------------------------------
// Define some compiler flags
#if TARGET_API_MAC_CARBON
	#ifdef __LP64__
	#pragma options align=power
	#else
	#pragma options align=mac68k
	#endif
#elif defined __BORLANDC__
	#pragma -a8
	#pragma options push -a8
#elif defined(__GNUC__)
    #pragma pack(push,8)
#elif defined(WIN32) || defined(__FLAT__)
	#pragma pack(push)
	#pragma pack(8)
#endif
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
/** String length limits (in characters excl. 0 byte). */
//-------------------------------------------------------------------------------------------------------
enum Vst2StringConstants
{
//-------------------------------------------------------------------------------------------------------
	kVstMaxNameLen       = 64,	///< used for #MidiProgramName, #MidiProgramCategory, #MidiKeyName, #VstSpeakerProperties, #VstPinProperties
	kVstMaxLabelLen      = 64,	///< used for #VstParameterProperties->label, #VstPinProperties->label
	kVstMaxShortLabelLen = 8,	///< used for #VstParameterProperties->shortLabel, #VstPinProperties->shortLabel
	kVstMaxCategLabelLen = 24,	///< used for #VstParameterProperties->label
	kVstMaxFileNameLen   = 100	///< used for #VstAudioFile->name
//-------------------------------------------------------------------------------------------------------
};
//-------------------------------------------------------------------------------------------------------
// VstEvent
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
/** A generic timestamped event. */
//-------------------------------------------------------------------------------------------------------
typedef struct VstEvent
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 type;			///< @see VstEventTypes
	VstInt32 byteSize;		///< size of this event, excl. type and byteSize
	VstInt32 deltaFrames;	///< sample frames related to the current block start sample position
	VstInt32 flags;			///< generic flags, none defined yet

	char data[16];			///< data size may vary, depending on event type
//-------------------------------------------------------------------------------------------------------
} VstEvent;

//-------------------------------------------------------------------------------------------------------
/** VstEvent Types used by #VstEvent. */
//-------------------------------------------------------------------------------------------------------
enum VstEventTypes
{
//-------------------------------------------------------------------------------------------------------
	kVstMidiType = 1,		///< MIDI event  @see VstMidiEvent
	DECLARE_VST_DEPRECATED (kVstAudioType),		///< \deprecated unused event type
	DECLARE_VST_DEPRECATED (kVstVideoType),		///< \deprecated unused event type
	DECLARE_VST_DEPRECATED (kVstParameterType),	///< \deprecated unused event type
	DECLARE_VST_DEPRECATED (kVstTriggerType),	///< \deprecated unused event type
	kVstSysExType			///< MIDI system exclusive  @see VstMidiSysexEvent
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** A block of events for the current processed audio block. */
//-------------------------------------------------------------------------------------------------------
struct VstEvents
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 numEvents;		///< number of Events in array
	VstIntPtr reserved;		///< zero (Reserved for future use)
	VstEvent* events[2];	///< event pointer array, variable size
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** MIDI Event (to be casted from VstEvent). */
//-------------------------------------------------------------------------------------------------------
struct VstMidiEvent
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 type;			///< #kVstMidiType
	VstInt32 byteSize;		///< sizeof (VstMidiEvent)
	VstInt32 deltaFrames;	///< sample frames related to the current block start sample position
	VstInt32 flags;			///< @see VstMidiEventFlags
	VstInt32 noteLength;	///< (in sample frames) of entire note, if available, else 0
	VstInt32 noteOffset;	///< offset (in sample frames) into note from note start if available, else 0
	char midiData[4];		///< 1 to 3 MIDI bytes; midiData[3] is reserved (zero)
	char detune;			///< -64 to +63 cents; for scales other than 'well-tempered' ('microtuning')
	char noteOffVelocity;	///< Note Off Velocity [0, 127]
	char reserved1;			///< zero (Reserved for future use)
	char reserved2;			///< zero (Reserved for future use)
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Flags used in #VstMidiEvent. */
//-------------------------------------------------------------------------------------------------------
enum VstMidiEventFlags
{
//-------------------------------------------------------------------------------------------------------
	kVstMidiEventIsRealtime = 1 << 0	///< means that this event is played life (not in playback from a sequencer track).\n This allows the Plug-In to handle these flagged events with higher priority, especially when the Plug-In has a big latency (AEffect::initialDelay)
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** MIDI Sysex Event (to be casted from #VstEvent). */
//-------------------------------------------------------------------------------------------------------
struct VstMidiSysexEvent
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 type;			///< #kVstSysexType
	VstInt32 byteSize;		///< sizeof (VstMidiSysexEvent)
	VstInt32 deltaFrames;	///< sample frames related to the current block start sample position
	VstInt32 flags;			///< none defined yet (should be zero)
	VstInt32 dumpBytes;		///< byte size of sysexDump
	VstIntPtr resvd1;		///< zero (Reserved for future use)
	char* sysexDump;		///< sysex dump
	VstIntPtr resvd2;		///< zero (Reserved for future use)
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
// VstTimeInfo
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
/** VstTimeInfo requested via #audioMasterGetTime.  @see AudioEffectX::getTimeInfo 

\note VstTimeInfo::samplePos :Current Position. It must always be valid, and should not cost a lot to ask for. The sample position is ahead of the time displayed to the user. In sequencer stop mode, its value does not change. A 32 bit integer is too small for sample positions, and it's a double to make it easier to convert between ppq and samples.
\note VstTimeInfo::ppqPos : At tempo 120, 1 quarter makes 1/2 second, so 2.0 ppq translates to 48000 samples at 48kHz sample rate.
.25 ppq is one sixteenth note then. if you need something like 480ppq, you simply multiply ppq by that scaler.
\note VstTimeInfo::barStartPos : Say we're at bars/beats readout 3.3.3. That's 2 bars + 2 q + 2 sixteenth, makes 2 * 4 + 2 + .25 = 10.25 ppq. at tempo 120, that's 10.25 * .5 = 5.125 seconds, times 48000 = 246000 samples (if my calculator servers me well :-). 
\note VstTimeInfo::samplesToNextClock : MIDI Clock Resolution (24 per Quarter Note), can be negative the distance to the next midi clock (24 ppq, pulses per quarter) in samples. unless samplePos falls precicely on a midi clock, this will either be negative such that the previous MIDI clock is addressed, or positive when referencing the following (future) MIDI clock.
*/
//-------------------------------------------------------------------------------------------------------
struct VstTimeInfo
{
//-------------------------------------------------------------------------------------------------------
	double samplePos;				///< current Position in audio samples (always valid)
	double sampleRate;				///< current Sample Rate in Herz (always valid)
	double nanoSeconds;				///< System Time in nanoseconds (10^-9 second)
	double ppqPos;					///< Musical Position, in Quarter Note (1.0 equals 1 Quarter Note)
	double tempo;					///< current Tempo in BPM (Beats Per Minute)
	double barStartPos;				///< last Bar Start Position, in Quarter Note
	double cycleStartPos;			///< Cycle Start (left locator), in Quarter Note
	double cycleEndPos;				///< Cycle End (right locator), in Quarter Note
	VstInt32 timeSigNumerator;		///< Time Signature Numerator (e.g. 3 for 3/4)
	VstInt32 timeSigDenominator;	///< Time Signature Denominator (e.g. 4 for 3/4)
	VstInt32 smpteOffset;			///< SMPTE offset (in SMPTE subframes (bits; 1/80 of a frame)). The current SMPTE position can be calculated using #samplePos, #sampleRate, and #smpteFrameRate.
	VstInt32 smpteFrameRate;		///< @see VstSmpteFrameRate
	VstInt32 samplesToNextClock;	///< MIDI Clock Resolution (24 Per Quarter Note), can be negative (nearest clock)
	VstInt32 flags;					///< @see VstTimeInfoFlags
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Flags used in #VstTimeInfo. */
//-------------------------------------------------------------------------------------------------------
enum VstTimeInfoFlags
{
//-------------------------------------------------------------------------------------------------------
	kVstTransportChanged     = 1,		///< indicates that play, cycle or record state has changed
	kVstTransportPlaying     = 1 << 1,	///< set if Host sequencer is currently playing
	kVstTransportCycleActive = 1 << 2,	///< set if Host sequencer is in cycle mode
	kVstTransportRecording   = 1 << 3,	///< set if Host sequencer is in record mode
	kVstAutomationWriting    = 1 << 6,	///< set if automation write mode active (record parameter changes)
	kVstAutomationReading    = 1 << 7,	///< set if automation read mode active (play parameter changes)
	kVstNanosValid           = 1 << 8,	///< VstTimeInfo::nanoSeconds valid
	kVstPpqPosValid          = 1 << 9,	///< VstTimeInfo::ppqPos valid
	kVstTempoValid           = 1 << 10,	///< VstTimeInfo::tempo valid
	kVstBarsValid            = 1 << 11,	///< VstTimeInfo::barStartPos valid
	kVstCyclePosValid        = 1 << 12,	///< VstTimeInfo::cycleStartPos and VstTimeInfo::cycleEndPos valid
	kVstTimeSigValid         = 1 << 13,	///< VstTimeInfo::timeSigNumerator and VstTimeInfo::timeSigDenominator valid
	kVstSmpteValid           = 1 << 14,	///< VstTimeInfo::smpteOffset and VstTimeInfo::smpteFrameRate valid
	kVstClockValid           = 1 << 15	///< VstTimeInfo::samplesToNextClock valid
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** SMPTE Frame Rates. */
//-------------------------------------------------------------------------------------------------------
enum VstSmpteFrameRate
{
//-------------------------------------------------------------------------------------------------------
	kVstSmpte24fps    = 0,		///< 24 fps
	kVstSmpte25fps    = 1,		///< 25 fps
	kVstSmpte2997fps  = 2,		///< 29.97 fps
	kVstSmpte30fps    = 3,		///< 30 fps
	kVstSmpte2997dfps = 4,		///< 29.97 drop
	kVstSmpte30dfps   = 5,		///< 30 drop

	kVstSmpteFilm16mm = 6, 		///< Film 16mm
	kVstSmpteFilm35mm = 7, 		///< Film 35mm
	kVstSmpte239fps   = 10,		///< HDTV: 23.976 fps
	kVstSmpte249fps   = 11,		///< HDTV: 24.976 fps
	kVstSmpte599fps   = 12,		///< HDTV: 59.94 fps
	kVstSmpte60fps    = 13		///< HDTV: 60 fps
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Variable IO for Offline Processing. */
//-------------------------------------------------------------------------------------------------------
struct VstVariableIo
{
//-------------------------------------------------------------------------------------------------------
	float** inputs;								///< input audio buffers
	float** outputs;							///< output audio buffers
	VstInt32 numSamplesInput;					///< number of incoming samples
	VstInt32 numSamplesOutput;					///< number of outgoing samples
	VstInt32* numSamplesInputProcessed;			///< number of samples actually processed of input
	VstInt32* numSamplesOutputProcessed;		///< number of samples actually processed of output
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Language code returned by audioMasterGetLanguage. */
//-------------------------------------------------------------------------------------------------------
enum VstHostLanguage
{
//-------------------------------------------------------------------------------------------------------
	kVstLangEnglish = 1,	///< English
	kVstLangGerman,			///< German
	kVstLangFrench,			///< French
	kVstLangItalian,		///< Italian
	kVstLangSpanish,		///< Spanish
	kVstLangJapanese		///< Japanese
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** VST 2.x dispatcher Opcodes (Plug-in to Host). Extension of #AudioMasterOpcodes */
//-------------------------------------------------------------------------------------------------------
enum AudioMasterOpcodesX
{
//-------------------------------------------------------------------------------------------------------
	DECLARE_VST_DEPRECATED (audioMasterWantMidi) = DECLARE_VST_DEPRECATED (audioMasterPinConnected) + 2,	///< \deprecated deprecated in VST 2.4

	audioMasterGetTime,				///< [return value]: #VstTimeInfo* or null if not supported [value]: request mask  @see VstTimeInfoFlags @see AudioEffectX::getTimeInfo
	audioMasterProcessEvents,		///< [ptr]: pointer to #VstEvents  @see VstEvents @see AudioEffectX::sendVstEventsToHost

	DECLARE_VST_DEPRECATED (audioMasterSetTime),	///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (audioMasterTempoAt),	///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (audioMasterGetNumAutomatableParameters),	///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (audioMasterGetParameterQuantization),		///< \deprecated deprecated in VST 2.4

	audioMasterIOChanged,			///< [return value]: 1 if supported  @see AudioEffectX::ioChanged

	DECLARE_VST_DEPRECATED (audioMasterNeedIdle),	///< \deprecated deprecated in VST 2.4
	
	audioMasterSizeWindow,			///< [index]: new width [value]: new height [return value]: 1 if supported  @see AudioEffectX::sizeWindow
	audioMasterGetSampleRate,		///< [return value]: current sample rate  @see AudioEffectX::updateSampleRate
	audioMasterGetBlockSize,		///< [return value]: current block size  @see AudioEffectX::updateBlockSize
	audioMasterGetInputLatency,		///< [return value]: input latency in audio samples  @see AudioEffectX::getInputLatency
	audioMasterGetOutputLatency,	///< [return value]: output latency in audio samples  @see AudioEffectX::getOutputLatency

	DECLARE_VST_DEPRECATED (audioMasterGetPreviousPlug),			///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (audioMasterGetNextPlug),				///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (audioMasterWillReplaceOrAccumulate),	///< \deprecated deprecated in VST 2.4

	audioMasterGetCurrentProcessLevel,	///< [return value]: current process level  @see VstProcessLevels
	audioMasterGetAutomationState,		///< [return value]: current automation state  @see VstAutomationStates

	audioMasterOfflineStart,			///< [index]: numNewAudioFiles [value]: numAudioFiles [ptr]: #VstAudioFile*  @see AudioEffectX::offlineStart
	audioMasterOfflineRead,				///< [index]: bool readSource [value]: #VstOfflineOption* @see VstOfflineOption [ptr]: #VstOfflineTask*  @see VstOfflineTask @see AudioEffectX::offlineRead
	audioMasterOfflineWrite,			///< @see audioMasterOfflineRead @see AudioEffectX::offlineRead
	audioMasterOfflineGetCurrentPass,	///< @see AudioEffectX::offlineGetCurrentPass
	audioMasterOfflineGetCurrentMetaPass,	///< @see AudioEffectX::offlineGetCurrentMetaPass

	DECLARE_VST_DEPRECATED (audioMasterSetOutputSampleRate),			///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (audioMasterGetOutputSpeakerArrangement),	///< \deprecated deprecated in VST 2.4

	audioMasterGetVendorString,			///< [ptr]: char buffer for vendor string, limited to #kVstMaxVendorStrLen  @see AudioEffectX::getHostVendorString
	audioMasterGetProductString,		///< [ptr]: char buffer for vendor string, limited to #kVstMaxProductStrLen  @see AudioEffectX::getHostProductString
	audioMasterGetVendorVersion,		///< [return value]: vendor-specific version  @see AudioEffectX::getHostVendorVersion
	audioMasterVendorSpecific,			///< no definition, vendor specific handling  @see AudioEffectX::hostVendorSpecific
	
	DECLARE_VST_DEPRECATED (audioMasterSetIcon),		///< \deprecated deprecated in VST 2.4
	
	audioMasterCanDo,					///< [ptr]: "can do" string [return value]: 1 for supported
	audioMasterGetLanguage,				///< [return value]: language code  @see VstHostLanguage

	DECLARE_VST_DEPRECATED (audioMasterOpenWindow),		///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (audioMasterCloseWindow),	///< \deprecated deprecated in VST 2.4

	audioMasterGetDirectory,			///< [return value]: FSSpec on MAC, else char*  @see AudioEffectX::getDirectory
	audioMasterUpdateDisplay,			///< no arguments	
	audioMasterBeginEdit,               ///< [index]: parameter index  @see AudioEffectX::beginEdit
	audioMasterEndEdit,                 ///< [index]: parameter index  @see AudioEffectX::endEdit
	audioMasterOpenFileSelector,		///< [ptr]: VstFileSelect* [return value]: 1 if supported  @see AudioEffectX::openFileSelector
	audioMasterCloseFileSelector,		///< [ptr]: VstFileSelect*  @see AudioEffectX::closeFileSelector
	
	DECLARE_VST_DEPRECATED (audioMasterEditFile),		///< \deprecated deprecated in VST 2.4
	
	DECLARE_VST_DEPRECATED (audioMasterGetChunkFile),	///< \deprecated deprecated in VST 2.4 [ptr]: char[2048] or sizeof (FSSpec) [return value]: 1 if supported  @see AudioEffectX::getChunkFile

	DECLARE_VST_DEPRECATED (audioMasterGetInputSpeakerArrangement)	///< \deprecated deprecated in VST 2.4
};

//-------------------------------------------------------------------------------------------------------
/** VST 2.x dispatcher Opcodes (Host to Plug-in). Extension of #AEffectOpcodes */
//-------------------------------------------------------------------------------------------------------
enum AEffectXOpcodes
{
//-------------------------------------------------------------------------------------------------------
	effProcessEvents = effSetChunk + 1		///< [ptr]: #VstEvents*  @see AudioEffectX::processEvents

	, effCanBeAutomated						///< [index]: parameter index [return value]: 1=true, 0=false  @see AudioEffectX::canParameterBeAutomated
	, effString2Parameter					///< [index]: parameter index [ptr]: parameter string [return value]: true for success  @see AudioEffectX::string2parameter

	, DECLARE_VST_DEPRECATED (effGetNumProgramCategories)	///< \deprecated deprecated in VST 2.4

	, effGetProgramNameIndexed				///< [index]: program index [ptr]: buffer for program name, limited to #kVstMaxProgNameLen [return value]: true for success  @see AudioEffectX::getProgramNameIndexed
	
	, DECLARE_VST_DEPRECATED (effCopyProgram)	///< \deprecated deprecated in VST 2.4
	, DECLARE_VST_DEPRECATED (effConnectInput)	///< \deprecated deprecated in VST 2.4
	, DECLARE_VST_DEPRECATED (effConnectOutput)	///< \deprecated deprecated in VST 2.4
	
	, effGetInputProperties					///< [index]: input index [ptr]: #VstPinProperties* [return value]: 1 if supported  @see AudioEffectX::getInputProperties
	, effGetOutputProperties				///< [index]: output index [ptr]: #VstPinProperties* [return value]: 1 if supported  @see AudioEffectX::getOutputProperties
	, effGetPlugCategory					///< [return value]: category  @see VstPlugCategory @see AudioEffectX::getPlugCategory

	, DECLARE_VST_DEPRECATED (effGetCurrentPosition)	///< \deprecated deprecated in VST 2.4
	, DECLARE_VST_DEPRECATED (effGetDestinationBuffer)	///< \deprecated deprecated in VST 2.4

	, effOfflineNotify						///< [ptr]: #VstAudioFile array [value]: count [index]: start flag  @see AudioEffectX::offlineNotify
	, effOfflinePrepare						///< [ptr]: #VstOfflineTask array [value]: count  @see AudioEffectX::offlinePrepare
	, effOfflineRun							///< [ptr]: #VstOfflineTask array [value]: count  @see AudioEffectX::offlineRun

	, effProcessVarIo						///< [ptr]: #VstVariableIo*  @see AudioEffectX::processVariableIo
	, effSetSpeakerArrangement				///< [value]: input #VstSpeakerArrangement* [ptr]: output #VstSpeakerArrangement*  @see AudioEffectX::setSpeakerArrangement

	, DECLARE_VST_DEPRECATED (effSetBlockSizeAndSampleRate)	///< \deprecated deprecated in VST 2.4

	, effSetBypass							///< [value]: 1 = bypass, 0 = no bypass  @see AudioEffectX::setBypass
	, effGetEffectName						///< [ptr]: buffer for effect name, limited to #kVstMaxEffectNameLen  @see AudioEffectX::getEffectName

	, DECLARE_VST_DEPRECATED (effGetErrorText)	///< \deprecated deprecated in VST 2.4

	, effGetVendorString					///< [ptr]: buffer for effect vendor string, limited to #kVstMaxVendorStrLen  @see AudioEffectX::getVendorString
	, effGetProductString					///< [ptr]: buffer for effect vendor string, limited to #kVstMaxProductStrLen  @see AudioEffectX::getProductString
	, effGetVendorVersion					///< [return value]: vendor-specific version  @see AudioEffectX::getVendorVersion
	, effVendorSpecific						///< no definition, vendor specific handling  @see AudioEffectX::vendorSpecific
	, effCanDo								///< [ptr]: "can do" string [return value]: 0: "don't know" -1: "no" 1: "yes"  @see AudioEffectX::canDo
	, effGetTailSize						///< [return value]: tail size (for example the reverb time of a reverb plug-in); 0 is default (return 1 for 'no tail')

	, DECLARE_VST_DEPRECATED (effIdle)				///< \deprecated deprecated in VST 2.4
	, DECLARE_VST_DEPRECATED (effGetIcon)			///< \deprecated deprecated in VST 2.4
	, DECLARE_VST_DEPRECATED (effSetViewPosition)	///< \deprecated deprecated in VST 2.4

	, effGetParameterProperties				///< [index]: parameter index [ptr]: #VstParameterProperties* [return value]: 1 if supported  @see AudioEffectX::getParameterProperties

	, DECLARE_VST_DEPRECATED (effKeysRequired)	///< \deprecated deprecated in VST 2.4

	, effGetVstVersion						///< [return value]: VST version  @see AudioEffectX::getVstVersion

#if VST_2_1_EXTENSIONS
	, effEditKeyDown						///< [index]: ASCII character [value]: virtual key [opt]: modifiers [return value]: 1 if key used  @see AEffEditor::onKeyDown
	, effEditKeyUp							///< [index]: ASCII character [value]: virtual key [opt]: modifiers [return value]: 1 if key used  @see AEffEditor::onKeyUp
	, effSetEditKnobMode					///< [value]: knob mode 0: circular, 1: circular relativ, 2: linear (CKnobMode in VSTGUI)  @see AEffEditor::setKnobMode

	, effGetMidiProgramName					///< [index]: MIDI channel [ptr]: #MidiProgramName* [return value]: number of used programs, 0 if unsupported  @see AudioEffectX::getMidiProgramName
	, effGetCurrentMidiProgram				///< [index]: MIDI channel [ptr]: #MidiProgramName* [return value]: index of current program  @see AudioEffectX::getCurrentMidiProgram
	, effGetMidiProgramCategory				///< [index]: MIDI channel [ptr]: #MidiProgramCategory* [return value]: number of used categories, 0 if unsupported  @see AudioEffectX::getMidiProgramCategory
	, effHasMidiProgramsChanged				///< [index]: MIDI channel [return value]: 1 if the #MidiProgramName(s) or #MidiKeyName(s) have changed  @see AudioEffectX::hasMidiProgramsChanged
	, effGetMidiKeyName						///< [index]: MIDI channel [ptr]: #MidiKeyName* [return value]: true if supported, false otherwise  @see AudioEffectX::getMidiKeyName
	
	, effBeginSetProgram					///< no arguments  @see AudioEffectX::beginSetProgram
	, effEndSetProgram						///< no arguments  @see AudioEffectX::endSetProgram
#endif // VST_2_1_EXTENSIONS

#if VST_2_3_EXTENSIONS
	, effGetSpeakerArrangement				///< [value]: input #VstSpeakerArrangement* [ptr]: output #VstSpeakerArrangement*  @see AudioEffectX::getSpeakerArrangement
	, effShellGetNextPlugin					///< [ptr]: buffer for plug-in name, limited to #kVstMaxProductStrLen [return value]: next plugin's uniqueID  @see AudioEffectX::getNextShellPlugin

	, effStartProcess						///< no arguments  @see AudioEffectX::startProcess
	, effStopProcess						///< no arguments  @see AudioEffectX::stopProcess
	, effSetTotalSampleToProcess		    ///< [value]: number of samples to process, offline only!  @see AudioEffectX::setTotalSampleToProcess
	, effSetPanLaw							///< [value]: pan law [opt]: gain  @see VstPanLawType @see AudioEffectX::setPanLaw
	
	, effBeginLoadBank						///< [ptr]: #VstPatchChunkInfo* [return value]: -1: bank can't be loaded, 1: bank can be loaded, 0: unsupported  @see AudioEffectX::beginLoadBank
	, effBeginLoadProgram					///< [ptr]: #VstPatchChunkInfo* [return value]: -1: prog can't be loaded, 1: prog can be loaded, 0: unsupported  @see AudioEffectX::beginLoadProgram
#endif // VST_2_3_EXTENSIONS

#if VST_2_4_EXTENSIONS
	, effSetProcessPrecision				///< [value]: @see VstProcessPrecision  @see AudioEffectX::setProcessPrecision
	, effGetNumMidiInputChannels			///< [return value]: number of used MIDI input channels (1-15)  @see AudioEffectX::getNumMidiInputChannels
	, effGetNumMidiOutputChannels			///< [return value]: number of used MIDI output channels (1-15)  @see AudioEffectX::getNumMidiOutputChannels
#endif // VST_2_4_EXTENSIONS
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Symbolic precision constants used for effSetProcessPrecision. */
//-------------------------------------------------------------------------------------------------------
enum VstProcessPrecision
{
	kVstProcessPrecision32 = 0,		///< single precision float (32bits)
	kVstProcessPrecision64			///< double precision (64bits)
};

//-------------------------------------------------------------------------------------------------------
/** Parameter Properties used in #effGetParameterProperties. */
//-------------------------------------------------------------------------------------------------------
struct VstParameterProperties
{
//-------------------------------------------------------------------------------------------------------
	float stepFloat;			///< float step
	float smallStepFloat;		///< small float step
	float largeStepFloat;		///< large float step
	char label[kVstMaxLabelLen];///< parameter label
	VstInt32 flags;				///< @see VstParameterFlags
	VstInt32 minInteger;		///< integer minimum
	VstInt32 maxInteger;		///< integer maximum
	VstInt32 stepInteger;		///< integer step
	VstInt32 largeStepInteger;	///< large integer step
	char shortLabel[kVstMaxShortLabelLen];	///< short label, recommended: 6 + delimiter

	// The following are for remote controller display purposes.
	// Note that the kVstParameterSupportsDisplayIndex flag must be set.
	// Host can scan all parameters, and find out in what order
	// to display them:

	VstInt16 displayIndex;		///< index where this parameter should be displayed (starting with 0)

	// Host can also possibly display the parameter group (category), such as...
	// ---------------------------
	// Osc 1
	// Wave  Detune  Octave  Mod
	// ---------------------------
	// ...if the plug-in supports it (flag #kVstParameterSupportsDisplayCategory)

	VstInt16 category;			///< 0: no category, else group index + 1
	VstInt16 numParametersInCategory;			///< number of parameters in category
	VstInt16 reserved;			///< zero
	char categoryLabel[kVstMaxCategLabelLen];	///< category label, e.g. "Osc 1" 

	char future[16];			///< reserved for future use
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Flags used in #VstParameterProperties. */
//-------------------------------------------------------------------------------------------------------
enum VstParameterFlags
{
//-------------------------------------------------------------------------------------------------------
	kVstParameterIsSwitch				 = 1 << 0,	///< parameter is a switch (on/off)
	kVstParameterUsesIntegerMinMax		 = 1 << 1,	///< minInteger, maxInteger valid
	kVstParameterUsesFloatStep			 = 1 << 2,	///< stepFloat, smallStepFloat, largeStepFloat valid
	kVstParameterUsesIntStep			 = 1 << 3,	///< stepInteger, largeStepInteger valid
	kVstParameterSupportsDisplayIndex 	 = 1 << 4,	///< displayIndex valid
	kVstParameterSupportsDisplayCategory = 1 << 5,	///< category, etc. valid
	kVstParameterCanRamp				 = 1 << 6	///< set if parameter value can ramp up/down
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Pin Properties used in #effGetInputProperties and #effGetOutputProperties. */
//-------------------------------------------------------------------------------------------------------
struct VstPinProperties
{
//-------------------------------------------------------------------------------------------------------
	char label[kVstMaxLabelLen];	///< pin name
	VstInt32 flags;					///< @see VstPinPropertiesFlags
	VstInt32 arrangementType;		///< @see VstSpeakerArrangementType
	char shortLabel[kVstMaxShortLabelLen];	///< short name (recommended: 6 + delimiter)

	char future[48];				///< reserved for future use
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Flags used in #VstPinProperties. */
//-------------------------------------------------------------------------------------------------------
enum VstPinPropertiesFlags
{
//-------------------------------------------------------------------------------------------------------
	kVstPinIsActive   = 1 << 0,		///< pin is active, ignored by Host
	kVstPinIsStereo   = 1 << 1,		///< pin is first of a stereo pair
	kVstPinUseSpeaker = 1 << 2		///< #VstPinProperties::arrangementType is valid and can be used to get the wanted arrangement
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Plug-in Categories. */
//-------------------------------------------------------------------------------------------------------
enum VstPlugCategory
{
//-------------------------------------------------------------------------------------------------------
    kPlugCategUnknown = 0,		///< Unknown, category not implemented
    kPlugCategEffect,			///< Simple Effect
    kPlugCategSynth,			///< VST Instrument (Synths, samplers,...)
    kPlugCategAnalysis,			///< Scope, Tuner, ...
    kPlugCategMastering,		///< Dynamics, ...
	kPlugCategSpacializer,		///< Panners, ...
	kPlugCategRoomFx,			///< Delays and Reverbs
	kPlugSurroundFx,			///< Dedicated surround processor
	kPlugCategRestoration,		///< Denoiser, ...
	kPlugCategOfflineProcess,	///< Offline Process
	kPlugCategShell,			///< Plug-in is container of other plug-ins  @see effShellGetNextPlugin
	kPlugCategGenerator,		///< ToneGenerator, ...

	kPlugCategMaxCount			///< Marker to count the categories
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
// MIDI Programs
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
/** MIDI Program Description. */
//-------------------------------------------------------------------------------------------------------
struct MidiProgramName 
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 thisProgramIndex;		///< 0 or greater: fill struct for this program index
	char name[kVstMaxNameLen];		///< program name
	char midiProgram;				///< -1:off, 0-127
	char midiBankMsb;				///< -1:off, 0-127
	char midiBankLsb;				///< -1:off, 0-127
	char reserved;					///< zero
	VstInt32 parentCategoryIndex;	///< -1:no parent category
	VstInt32 flags;					///< omni etc. @see VstMidiProgramNameFlags
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Flags used in MidiProgramName. */
//-------------------------------------------------------------------------------------------------------
enum VstMidiProgramNameFlags
{
//-------------------------------------------------------------------------------------------------------
	kMidiIsOmni = 1	///< default is multi. for omni mode, channel 0 is used for inquiries and program changes
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** MIDI Program Category. */
//-------------------------------------------------------------------------------------------------------
struct MidiProgramCategory 
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 thisCategoryIndex;		///< 0 or greater:  fill struct for this category index.
	char name[kVstMaxNameLen];		///< name
	VstInt32 parentCategoryIndex;	///< -1:no parent category
	VstInt32 flags;					///< reserved, none defined yet, zero.
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** MIDI Key Description. */
//-------------------------------------------------------------------------------------------------------
struct MidiKeyName 
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 thisProgramIndex;		///< 0 or greater:  fill struct for this program index.
	VstInt32 thisKeyNumber;			///< 0 - 127. fill struct for this key number.
	char keyName[kVstMaxNameLen];	///< key name, empty means regular key names
	VstInt32 reserved;				///< zero
	VstInt32 flags;					///< reserved, none defined yet, zero.
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
// Surround Setup
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
/** Speaker Properties.
	The origin for azimuth is right (as by math conventions dealing with radians).
	The elevation origin is also right, visualizing a rotation of a circle across the
	-pi/pi axis of the horizontal circle. Thus, an elevation of -pi/2 corresponds
	to bottom, and a speaker standing on the left, and 'beaming' upwards would have
	an azimuth of -pi, and an elevation of pi/2.
	For user interface representation, grads are more likely to be used, and the
	origins will obviously 'shift' accordingly. */
//-------------------------------------------------------------------------------------------------------
typedef struct VstSpeakerProperties
{
//-------------------------------------------------------------------------------------------------------
	float azimuth;		///< unit: rad, range: -PI...PI, exception: 10.f for LFE channel
	float elevation;	///< unit: rad, range: -PI/2...PI/2, exception: 10.f for LFE channel
	float radius;		///< unit: meter, exception: 0.f for LFE channel
	float reserved;		///< zero (reserved for future use)
	char name[kVstMaxNameLen];	///< for new setups, new names should be given (L/R/C... won't do)
	VstInt32 type;		///< @see VstSpeakerType

	char future[28];	///< reserved for future use
//-------------------------------------------------------------------------------------------------------
} VstSpeakerProperties;

//-------------------------------------------------------------------------------------------------------
/** Speaker Arrangement. */
//-------------------------------------------------------------------------------------------------------
struct VstSpeakerArrangement
{	
//-------------------------------------------------------------------------------------------------------
	VstInt32 type;						///< e.g. #kSpeakerArr51 for 5.1  @see VstSpeakerArrangementType
	VstInt32 numChannels;				///< number of channels in this speaker arrangement
	VstSpeakerProperties speakers[8];	///< variable sized speaker array
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Speaker Types. */
//-------------------------------------------------------------------------------------------------------
enum VstSpeakerType
{
//-------------------------------------------------------------------------------------------------------
	kSpeakerUndefined = 0x7fffffff,	///< Undefined
	kSpeakerM = 0,					///< Mono (M)
	kSpeakerL,						///< Left (L)
	kSpeakerR,						///< Right (R)
	kSpeakerC,						///< Center (C)
	kSpeakerLfe,					///< Subbass (Lfe)
	kSpeakerLs,						///< Left Surround (Ls)
	kSpeakerRs,						///< Right Surround (Rs)
	kSpeakerLc,						///< Left of Center (Lc)
	kSpeakerRc,						///< Right of Center (Rc)
	kSpeakerS,						///< Surround (S)
	kSpeakerCs = kSpeakerS,			///< Center of Surround (Cs) = Surround (S)
	kSpeakerSl,						///< Side Left (Sl)
	kSpeakerSr,						///< Side Right (Sr)
	kSpeakerTm,						///< Top Middle (Tm)
	kSpeakerTfl,					///< Top Front Left (Tfl)
	kSpeakerTfc,					///< Top Front Center (Tfc)
	kSpeakerTfr,					///< Top Front Right (Tfr)
	kSpeakerTrl,					///< Top Rear Left (Trl)
	kSpeakerTrc,					///< Top Rear Center (Trc)
	kSpeakerTrr,					///< Top Rear Right (Trr)
	kSpeakerLfe2					///< Subbass 2 (Lfe2)
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** User-defined speaker types, to be extended in the negative range.
	Will be handled as their corresponding speaker types with abs values:
	e.g abs(#kSpeakerU1) == #kSpeakerL, abs(#kSpeakerU2) == #kSpeakerR) */
//-------------------------------------------------------------------------------------------------------
enum VstUserSpeakerType
{
//-------------------------------------------------------------------------------------------------------
	kSpeakerU32 = -32,	
	kSpeakerU31,			
	kSpeakerU30,			
	kSpeakerU29,			
	kSpeakerU28,			
	kSpeakerU27,			
	kSpeakerU26,			
	kSpeakerU25,			
	kSpeakerU24,			
	kSpeakerU23,			
	kSpeakerU22,			
	kSpeakerU21,			
	kSpeakerU20,			///< == #kSpeakerLfe2
	kSpeakerU19,			///< == #kSpeakerTrr
	kSpeakerU18,			///< == #kSpeakerTrc
	kSpeakerU17,			///< == #kSpeakerTrl
	kSpeakerU16,			///< == #kSpeakerTfr
	kSpeakerU15,			///< == #kSpeakerTfc
	kSpeakerU14,			///< == #kSpeakerTfl
	kSpeakerU13,			///< == #kSpeakerTm
	kSpeakerU12,			///< == #kSpeakerSr
	kSpeakerU11,			///< == #kSpeakerSl
	kSpeakerU10,			///< == #kSpeakerCs
	kSpeakerU9,				///< == #kSpeakerS
	kSpeakerU8,				///< == #kSpeakerRc
	kSpeakerU7,				///< == #kSpeakerLc
	kSpeakerU6,				///< == #kSpeakerRs
	kSpeakerU5,				///< == #kSpeakerLs
	kSpeakerU4,				///< == #kSpeakerLfe
	kSpeakerU3,				///< == #kSpeakerC
	kSpeakerU2,				///< == #kSpeakerR
	kSpeakerU1				///< == #kSpeakerL
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Speaker Arrangement Types*/
//-------------------------------------------------------------------------------------------------------
enum VstSpeakerArrangementType
{
//-------------------------------------------------------------------------------------------------------
	kSpeakerArrUserDefined = -2,///< user defined
	kSpeakerArrEmpty = -1,		///< empty arrangement
	kSpeakerArrMono  =  0,		///< M
	kSpeakerArrStereo,			///< L R
	kSpeakerArrStereoSurround,	///< Ls Rs
	kSpeakerArrStereoCenter,	///< Lc Rc
	kSpeakerArrStereoSide,		///< Sl Sr
	kSpeakerArrStereoCLfe,		///< C Lfe
	kSpeakerArr30Cine,			///< L R C
	kSpeakerArr30Music,			///< L R S
	kSpeakerArr31Cine,			///< L R C Lfe
	kSpeakerArr31Music,			///< L R Lfe S
	kSpeakerArr40Cine,			///< L R C   S (LCRS)
	kSpeakerArr40Music,			///< L R Ls  Rs (Quadro)
	kSpeakerArr41Cine,			///< L R C   Lfe S (LCRS+Lfe)
	kSpeakerArr41Music,			///< L R Lfe Ls Rs (Quadro+Lfe)
	kSpeakerArr50,				///< L R C Ls  Rs 
	kSpeakerArr51,				///< L R C Lfe Ls Rs
	kSpeakerArr60Cine,			///< L R C   Ls  Rs Cs
	kSpeakerArr60Music,			///< L R Ls  Rs  Sl Sr 
	kSpeakerArr61Cine,			///< L R C   Lfe Ls Rs Cs
	kSpeakerArr61Music,			///< L R Lfe Ls  Rs Sl Sr 
	kSpeakerArr70Cine,			///< L R C Ls  Rs Lc Rc 
	kSpeakerArr70Music,			///< L R C Ls  Rs Sl Sr
	kSpeakerArr71Cine,			///< L R C Lfe Ls Rs Lc Rc
	kSpeakerArr71Music,			///< L R C Lfe Ls Rs Sl Sr
	kSpeakerArr80Cine,			///< L R C Ls  Rs Lc Rc Cs
	kSpeakerArr80Music,			///< L R C Ls  Rs Cs Sl Sr
	kSpeakerArr81Cine,			///< L R C Lfe Ls Rs Lc Rc Cs
	kSpeakerArr81Music,			///< L R C Lfe Ls Rs Cs Sl Sr 
	kSpeakerArr102,				///< L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2
	kNumSpeakerArr
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
// Offline Processing
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
/** Offline Task Description. */
//-------------------------------------------------------------------------------------------------------
struct VstOfflineTask
{
//-------------------------------------------------------------------------------------------------------
	char processName[96];			///< set by plug-in

	// audio access
	double readPosition;			///< set by plug-in/Host
	double writePosition;			///< set by plug-in/Host
	VstInt32 readCount;				///< set by plug-in/Host
	VstInt32 writeCount;			///< set by plug-in
	VstInt32 sizeInputBuffer;		///< set by Host
	VstInt32 sizeOutputBuffer;		///< set by Host
	void* inputBuffer;				///< set by Host
	void* outputBuffer;				///< set by Host
	double positionToProcessFrom;	///< set by Host
	double numFramesToProcess;		///< set by Host
	double maxFramesToWrite;		///< set by plug-in

	// other data access
	void* extraBuffer;				///< set by plug-in
	VstInt32 value;					///< set by Host or plug-in
	VstInt32 index;					///< set by Host or plug-in

	// file attributes
	double numFramesInSourceFile;	///< set by Host
	double sourceSampleRate;		///< set by Host or plug-in
	double destinationSampleRate;	///< set by Host or plug-in
	VstInt32 numSourceChannels;		///< set by Host or plug-in
	VstInt32 numDestinationChannels;///< set by Host or plug-in
	VstInt32 sourceFormat;			///< set by Host
	VstInt32 destinationFormat;		///< set by plug-in
	char outputText[512];			///< set by plug-in or Host

	// progress notification
	double progress;				///< set by plug-in
	VstInt32 progressMode;			///< Reserved for future use
	char progressText[100];			///< set by plug-in

	VstInt32 flags;					///< set by Host and plug-in; see enum #VstOfflineTaskFlags
	VstInt32 returnValue;			///< Reserved for future use
	void* hostOwned;				///< set by Host
	void* plugOwned;				///< set by plug-in

	char future[1024];				///< Reserved for future use
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Flags used in #VstOfflineTask. */
//-------------------------------------------------------------------------------------------------------
enum VstOfflineTaskFlags
{
//-------------------------------------------------------------------------------------------------------
	kVstOfflineUnvalidParameter	= 1 << 0,	///< set by Host
	kVstOfflineNewFile			= 1 << 1,	///< set by Host

	kVstOfflinePlugError		= 1 << 10,	///< set by plug-in
	kVstOfflineInterleavedAudio	= 1 << 11,	///< set by plug-in
	kVstOfflineTempOutputFile	= 1 << 12,	///< set by plug-in
	kVstOfflineFloatOutputFile	= 1 << 13,	///< set by plug-in
	kVstOfflineRandomWrite		= 1 << 14,	///< set by plug-in
	kVstOfflineStretch			= 1 << 15,	///< set by plug-in
	kVstOfflineNoThread			= 1 << 16	///< set by plug-in
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Option passed to #offlineRead/#offlineWrite. */
//-------------------------------------------------------------------------------------------------------
enum VstOfflineOption
{
//-------------------------------------------------------------------------------------------------------
   kVstOfflineAudio,		///< reading/writing audio samples
   kVstOfflinePeaks,		///< reading graphic representation
   kVstOfflineParameter,	///< reading/writing parameters
   kVstOfflineMarker,		///< reading/writing marker
   kVstOfflineCursor,		///< reading/moving edit cursor
   kVstOfflineSelection,	///< reading/changing selection
   kVstOfflineQueryFiles	///< to request the Host to call asynchronously #offlineNotify
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Structure passed to #offlineNotify and #offlineStart */
//-------------------------------------------------------------------------------------------------------
struct VstAudioFile
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 flags;					///< see enum #VstAudioFileFlags
	void* hostOwned;				///< any data private to Host
	void* plugOwned;				///< any data private to plug-in
	char name[kVstMaxFileNameLen];	///< file title
	VstInt32 uniqueId;				///< uniquely identify a file during a session
	double sampleRate;				///< file sample rate
	VstInt32 numChannels;			///< number of channels (1 for mono, 2 for stereo...)
	double numFrames;				///< number of frames in the audio file
	VstInt32 format;				///< Reserved for future use
	double editCursorPosition;		///< -1 if no such cursor
	double selectionStart;			///< frame index of first selected frame, or -1
	double selectionSize;			///< number of frames in selection, or 0
	VstInt32 selectedChannelsMask;	///< 1 bit per channel
	VstInt32 numMarkers;			///< number of markers in the file
	VstInt32 timeRulerUnit;			///< see doc for possible values
	double timeRulerOffset;			///< offset in time ruler (positive or negative)
	double tempo;					///< as BPM (Beats Per Minute)
	VstInt32 timeSigNumerator;		///< time signature numerator
	VstInt32 timeSigDenominator;	///< time signature denominator
	VstInt32 ticksPerBlackNote;		///< resolution
	VstInt32 smpteFrameRate;		///< SMPTE rate (set as in #VstTimeInfo)

	char future[64];				///< Reserved for future use
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Flags used in #VstAudioFile. */
//-------------------------------------------------------------------------------------------------------
enum VstAudioFileFlags
{
//-------------------------------------------------------------------------------------------------------
	kVstOfflineReadOnly				= 1 << 0,	///< set by Host (in call #offlineNotify)
	kVstOfflineNoRateConversion		= 1 << 1,	///< set by Host (in call #offlineNotify)
	kVstOfflineNoChannelChange		= 1 << 2,	///< set by Host (in call #offlineNotify)

	kVstOfflineCanProcessSelection	= 1 << 10,	///< set by plug-in (in call #offlineStart)
	kVstOfflineNoCrossfade			= 1 << 11,	///< set by plug-in (in call #offlineStart)
	kVstOfflineWantRead				= 1 << 12,	///< set by plug-in (in call #offlineStart)
	kVstOfflineWantWrite			= 1 << 13,	///< set by plug-in (in call #offlineStart)
	kVstOfflineWantWriteMarker		= 1 << 14,	///< set by plug-in (in call #offlineStart)
	kVstOfflineWantMoveCursor		= 1 << 15,	///< set by plug-in (in call #offlineStart)
	kVstOfflineWantSelect			= 1 << 16	///< set by plug-in (in call #offlineStart)
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Audio file marker. */
//-------------------------------------------------------------------------------------------------------
struct VstAudioFileMarker
{
//-------------------------------------------------------------------------------------------------------
	double position;		///< marker position
	char name[32];			///< marker name
	VstInt32 type;			///< marker type
	VstInt32 id;			///< marker identifier
	VstInt32 reserved;		///< reserved for future use
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
// Others
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
/** \deprecated Structure used for #openWindow and #closeWindow (deprecated in VST 2.4). */
//-------------------------------------------------------------------------------------------------------
struct DECLARE_VST_DEPRECATED (VstWindow)
{
//-------------------------------------------------------------------------------------------------------
	char title[128];
	VstInt16 xPos;
	VstInt16 yPos;
	VstInt16 width;
	VstInt16 height;
	VstInt32 style;
	void* parent;
	void* userHandle;
	void* winHandle;

	char future[104];
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Structure used for keyUp/keyDown. */
//-------------------------------------------------------------------------------------------------------
struct VstKeyCode
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 character;		///< ASCII character
	unsigned char virt;     ///< @see VstVirtualKey
	unsigned char modifier; ///< @see VstModifierKey
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Platform-independent definition of Virtual Keys (used in #VstKeyCode). */
//-------------------------------------------------------------------------------------------------------
enum VstVirtualKey 
{
//-------------------------------------------------------------------------------------------------------
	VKEY_BACK = 1, 
	VKEY_TAB, 
	VKEY_CLEAR, 
	VKEY_RETURN, 
	VKEY_PAUSE, 
	VKEY_ESCAPE, 
	VKEY_SPACE, 
	VKEY_NEXT, 
	VKEY_END, 
	VKEY_HOME, 
	VKEY_LEFT, 
	VKEY_UP, 
	VKEY_RIGHT, 
	VKEY_DOWN, 
	VKEY_PAGEUP, 
	VKEY_PAGEDOWN, 
	VKEY_SELECT, 
	VKEY_PRINT, 
	VKEY_ENTER, 
	VKEY_SNAPSHOT, 
	VKEY_INSERT, 
	VKEY_DELETE, 
	VKEY_HELP, 
	VKEY_NUMPAD0, 
	VKEY_NUMPAD1, 
	VKEY_NUMPAD2, 
	VKEY_NUMPAD3, 
	VKEY_NUMPAD4, 
	VKEY_NUMPAD5, 
	VKEY_NUMPAD6, 
	VKEY_NUMPAD7, 
	VKEY_NUMPAD8, 
	VKEY_NUMPAD9, 
	VKEY_MULTIPLY, 
	VKEY_ADD, 
	VKEY_SEPARATOR, 
	VKEY_SUBTRACT, 
	VKEY_DECIMAL, 
	VKEY_DIVIDE, 
	VKEY_F1, 
	VKEY_F2, 
	VKEY_F3, 
	VKEY_F4, 
	VKEY_F5, 
	VKEY_F6, 
	VKEY_F7, 
	VKEY_F8, 
	VKEY_F9, 
	VKEY_F10, 
	VKEY_F11, 
	VKEY_F12, 
	VKEY_NUMLOCK, 
	VKEY_SCROLL,
	VKEY_SHIFT,
	VKEY_CONTROL,
	VKEY_ALT,
	VKEY_EQUALS
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Modifier flags used in #VstKeyCode. */
//-------------------------------------------------------------------------------------------------------
enum VstModifierKey
{
//-------------------------------------------------------------------------------------------------------
	MODIFIER_SHIFT     = 1<<0, ///< Shift
	MODIFIER_ALTERNATE = 1<<1, ///< Alt
	MODIFIER_COMMAND   = 1<<2, ///< Control on Mac
	MODIFIER_CONTROL   = 1<<3  ///< Ctrl on PC, Apple on Mac
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** File filter used in #VstFileSelect. */
//-------------------------------------------------------------------------------------------------------
typedef struct VstFileType
{
//-------------------------------------------------------------------------------------------------------
	char name[128];				///< display name
	char macType[8];			///< MacOS type
	char dosType[8];			///< Windows file extension
	char unixType[8];			///< Unix file extension
	char mimeType1[128];		///< MIME type
	char mimeType2[128];		///< additional MIME type

#ifdef  __cplusplus
	VstFileType (const char* _name = 0, const char* _macType = 0, const char* _dosType = 0,
				 const char* _unixType = 0, const char* _mimeType1 = 0, const char* _mimeType2 = 0)
	{
		vst_strncpy (name, _name ? _name : "", 127);
		vst_strncpy (macType, _macType ? _macType : "", 7);
		vst_strncpy (dosType, _dosType ? _dosType : "", 7);
		vst_strncpy (unixType, _unixType ? _unixType : "", 7);
		vst_strncpy (mimeType1, _mimeType1 ? _mimeType1 : "", 127);
		vst_strncpy (mimeType2, _mimeType2 ? _mimeType2 : "", 127);
	}
#endif // __cplusplus
//-------------------------------------------------------------------------------------------------------
} VstFileType;

//-------------------------------------------------------------------------------------------------------
/** File Selector Description used in #audioMasterOpenFileSelector. */
//-------------------------------------------------------------------------------------------------------
struct VstFileSelect
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 command;           ///< @see VstFileSelectCommand
	VstInt32 type;              ///< @see VstFileSelectType
	VstInt32 macCreator;        ///< optional: 0 = no creator
	VstInt32 nbFileTypes;       ///< number of fileTypes
	VstFileType* fileTypes;		///< list of fileTypes  @see VstFileType
	char title[1024];			///< text to display in file selector's title
	char* initialPath;			///< initial path
	char* returnPath;			///< use with #kVstFileLoad and #kVstDirectorySelect. null: Host allocates memory, plug-in must call #closeOpenFileSelector!
	VstInt32 sizeReturnPath;	///< size of allocated memory for return paths
	char** returnMultiplePaths; ///< use with kVstMultipleFilesLoad. Host allocates memory, plug-in must call #closeOpenFileSelector!
	VstInt32 nbReturnPath;		///< number of selected paths
	VstIntPtr reserved;			///< reserved for Host application

	char future[116];			///< reserved for future use
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Command constants used in #VstFileSelect structure. */
//-------------------------------------------------------------------------------------------------------
enum VstFileSelectCommand
{
//-------------------------------------------------------------------------------------------------------
	kVstFileLoad = 0,		///< for loading a file
	kVstFileSave,			///< for saving a file
	kVstMultipleFilesLoad,	///< for loading multiple files
	kVstDirectorySelect		///< for selecting a directory/folder
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Types used in #VstFileSelect structure. */
//-------------------------------------------------------------------------------------------------------
enum VstFileSelectType
{
//-------------------------------------------------------------------------------------------------------
	kVstFileType = 0		///< regular file selector
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Structure used for #effBeginLoadBank/#effBeginLoadProgram. */
//-------------------------------------------------------------------------------------------------------
struct VstPatchChunkInfo
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 version;			///< Format Version (should be 1)
	VstInt32 pluginUniqueID;	///< UniqueID of the plug-in
	VstInt32 pluginVersion;		///< Plug-in Version
	VstInt32 numElements;		///< Number of Programs (Bank) or Parameters (Program)

	char future[48];			///< Reserved for future use
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** PanLaw Type. */
//-------------------------------------------------------------------------------------------------------
enum VstPanLawType
{
//-------------------------------------------------------------------------------------------------------
	kLinearPanLaw = 0,	///< L = pan * M; R = (1 - pan) * M;
	kEqualPowerPanLaw	///< L = pow (pan, 0.5) * M; R = pow ((1 - pan), 0.5) * M;
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Process Levels returned by #audioMasterGetCurrentProcessLevel. */
//-------------------------------------------------------------------------------------------------------
enum VstProcessLevels
{
//-------------------------------------------------------------------------------------------------------
	kVstProcessLevelUnknown = 0,	///< not supported by Host
	kVstProcessLevelUser,			///< 1: currently in user thread (GUI)
	kVstProcessLevelRealtime,		///< 2: currently in audio thread (where process is called)
	kVstProcessLevelPrefetch,		///< 3: currently in 'sequencer' thread (MIDI, timer etc)
	kVstProcessLevelOffline			///< 4: currently offline processing and thus in user thread
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Automation States returned by #audioMasterGetAutomationState. */
//-------------------------------------------------------------------------------------------------------
enum VstAutomationStates
{
//-------------------------------------------------------------------------------------------------------
	kVstAutomationUnsupported = 0,	///< not supported by Host
	kVstAutomationOff,				///< off
	kVstAutomationRead,				///< read
	kVstAutomationWrite,			///< write
	kVstAutomationReadWrite			///< read and write
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
#if TARGET_API_MAC_CARBON
	#pragma options align=reset
#elif defined(WIN32) || defined(__FLAT__) || defined(__GNUC__)
	#pragma pack(pop)
#elif defined __BORLANDC__
	#pragma -a-
#endif
//-------------------------------------------------------------------------------------------------------

#endif //__aeffectx__
