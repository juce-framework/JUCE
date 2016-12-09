/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

 ==============================================================================
*/

#ifndef JUCE_VSTINTERFACE_H_INCLUDED
#define JUCE_VSTINTERFACE_H_INCLUDED

#include "../../juce_core/juce_core.h"

using namespace juce;

#if JUCE_MSVC
 #define VSTINTERFACECALL __cdecl
 #pragma pack(push)
 #pragma pack(8)
#elif JUCE_MAC || JUCE_IOS
 #define VSTINTERFACECALL
 #if JUCE_64BIT
  #pragma options align=power
 #else
  #pragma options align=mac68k
 #endif
#else
 #define VSTINTERFACECALL
 #pragma pack(push, 8)
#endif

const int32 juceVstInterfaceVersion = 2400;
const int32 juceVstInterfaceIdentifier = 0x56737450;    // The "magic" identifier in the SDK is 'VstP'.

//==============================================================================
struct VstEffectInterface
{
    int32 interfaceIdentifier;
    pointer_sized_int (VSTINTERFACECALL* dispatchFunction)          (VstEffectInterface*, int32 op, int32 index, pointer_sized_int value, void* ptr, float opt);
    void              (VSTINTERFACECALL* processAudioFunction)      (VstEffectInterface*, float** inputs, float** outputs, int32 numSamples);
    void              (VSTINTERFACECALL* setParameterValueFunction) (VstEffectInterface*, int32 parameterIndex, float value);
    float             (VSTINTERFACECALL* getParameterValueFunction) (VstEffectInterface*, int32 parameterIndex);
    int32 numPrograms;
    int32 numParameters;
    int32 numInputChannels;
    int32 numOutputChannels;
    int32 flags;
    pointer_sized_int hostSpace1;
    pointer_sized_int hostSpace2;
    int32 latency;
    int32 deprecated1;
    int32 deprecated2;
    float deprecated3;
    void* effectPointer;
    void* userPointer;
    int32 plugInIdentifier;
    int32 plugInVersion;
    void (VSTINTERFACECALL* processAudioInplaceFunction)       (VstEffectInterface*, float**  inputs, float**  outputs, int32 numSamples);
    void (VSTINTERFACECALL* processDoubleAudioInplaceFunction) (VstEffectInterface*, double** inputs, double** outputs, int32 numSamples);
    char emptySpace[56];
};

typedef pointer_sized_int (VSTINTERFACECALL* VstHostCallback) (VstEffectInterface*, int32 op, int32 index, pointer_sized_int value, void* ptr, float opt);

enum VstEffectInterfaceFlags
{
    vstEffectFlagHasEditor          = 1,
    vstEffectFlagInplaceAudio       = 16,
    vstEffectFlagDataInChunks       = 32,
    vstEffectFlagIsSynth            = 256,
    vstEffectFlagInplaceDoubleAudio = 4096
};

//==============================================================================
enum VstHostToPlugInOpcodes
{
    plugInOpcodeOpen,
    plugInOpcodeClose,
    plugInOpcodeSetCurrentProgram,
    plugInOpcodeGetCurrentProgram,
    plugInOpcodeSetCurrentProgramName,
    plugInOpcodeGetCurrentProgramName,
    plugInOpcodeGetParameterLabel,
    plugInOpcodeGetParameterText,
    plugInOpcodeGetParameterName,
    plugInOpcodeSetSampleRate = plugInOpcodeGetParameterName + 2,
    plugInOpcodeSetBlockSize,
    plugInOpcodeResumeSuspend,
    plugInOpcodeGetEditorBounds,
    plugInOpcodeOpenEditor,
    plugInOpcodeCloseEditor,
    plugInOpcodeDrawEditor,
    plugInOpcodeGetMouse,
    plugInOpcodeEditorIdle = plugInOpcodeGetMouse + 2,
    plugInOpcodeeffEditorTop,
    plugInOpcodeSleepEditor,
    plugInOpcodeIdentify,
    plugInOpcodeGetData,
    plugInOpcodeSetData,
    plugInOpcodePreAudioProcessingEvents,
    plugInOpcodeIsParameterAutomatable,
    plugInOpcodeParameterValueForText,
    plugInOpcodeGetProgramName = plugInOpcodeParameterValueForText + 2,
    plugInOpcodeConnectInput = plugInOpcodeGetProgramName + 2,
    plugInOpcodeConnectOutput,
    plugInOpcodeGetInputPinProperties,
    plugInOpcodeGetOutputPinProperties,
    plugInOpcodeGetPlugInCategory,
    plugInOpcodeSetSpeakerConfiguration = plugInOpcodeGetPlugInCategory + 7,
    plugInOpcodeSetBypass = plugInOpcodeSetSpeakerConfiguration + 2,
    plugInOpcodeGetPlugInName,
    plugInOpcodeGetManufacturerName = plugInOpcodeGetPlugInName + 2,
    plugInOpcodeGetManufacturerProductName,
    plugInOpcodeGetManufacturerVersion,
    plugInOpcodeManufacturerSpecific,
    plugInOpcodeCanPlugInDo,
    plugInOpcodeGetTailSize,
    plugInOpcodeIdle,
    plugInOpcodeKeyboardFocusRequired = plugInOpcodeIdle + 4,
    plugInOpcodeGetVstInterfaceVersion,
    plugInOpcodeGetCurrentMidiProgram = plugInOpcodeGetVstInterfaceVersion + 5,
    plugInOpcodeGetSpeakerArrangement = plugInOpcodeGetCurrentMidiProgram + 6,
    plugInOpcodeNextPlugInUniqueID,
    plugInOpcodeStartProcess,
    plugInOpcodeStopProcess,
    plugInOpcodeSetNumberOfSamplesToProcess,
    plugInOpcodeSetSampleFloatType = plugInOpcodeSetNumberOfSamplesToProcess + 4,
    plugInOpcodeMaximum = plugInOpcodeSetSampleFloatType
};


enum VstPlugInToHostOpcodes
{
    hostOpcodeParameterChanged,
    hostOpcodeVstVersion,
    hostOpcodeCurrentId,
    hostOpcodeIdle,
    hostOpcodePinConnected,
    hostOpcodePlugInWantsMidi = hostOpcodePinConnected + 2,
    hostOpcodeGetTimingInfo,
    hostOpcodePreAudioProcessingEvents,
    hostOpcodeSetTime,
    hostOpcodeTempoAt,
    hostOpcodeGetNumberOfAutomatableParameters,
    hostOpcodeGetParameterInterval,
    hostOpcodeIOModified,
    hostOpcodeNeedsIdle,
    hostOpcodeWindowSize,
    hostOpcodeGetSampleRate,
    hostOpcodeGetBlockSize,
    hostOpcodeGetInputLatency,
    hostOpcodeGetOutputLatency,
    hostOpcodeGetPreviousPlugIn,
    hostOpcodeGetNextPlugIn,
    hostOpcodeWillReplace,
    hostOpcodeGetCurrentAudioProcessingLevel,
    hostOpcodeGetAutomationState,
    hostOpcodeOfflineStart,
    hostOpcodeOfflineReadSource,
    hostOpcodeOfflineWrite,
    hostOpcodeOfflineGetCurrentPass,
    hostOpcodeOfflineGetCurrentMetaPass,
    hostOpcodeSetOutputSampleRate,
    hostOpcodeGetOutputSpeakerConfiguration,
    hostOpcodeGetManufacturerName,
    hostOpcodeGetProductName,
    hostOpcodeGetManufacturerVersion,
    hostOpcodeManufacturerSpecific,
    hostOpcodeSetIcon,
    hostOpcodeCanHostDo,
    hostOpcodeGetLanguage,
    hostOpcodeOpenEditorWindow,
    hostOpcodeCloseEditorWindow,
    hostOpcodeGetDirectory,
    hostOpcodeUpdateView,
    hostOpcodeParameterChangeGestureBegin,
    hostOpcodeParameterChangeGestureEnd,
};

//==============================================================================
enum VstProcessingSampleType
{
    vstProcessingSampleTypeFloat,
    vstProcessingSampleTypeDouble
};

//==============================================================================
// These names must be identical to the Steinberg SDK so JUCE users can set
// exactly what they want.
enum VstPlugInCategory
{
    kPlugCategUnknown,
    kPlugCategEffect,
    kPlugCategSynth,
    kPlugCategAnalysis,
    kPlugCategMastering,
    kPlugCategSpacializer,
    kPlugCategRoomFx,
    kPlugSurroundFx,
    kPlugCategRestoration,
    kPlugCategOfflineProcess,
    kPlugCategShell,
    kPlugCategGenerator
};

//==============================================================================
struct VstEditorBounds
{
    int16 upper;
    int16 leftmost;
    int16 lower;
    int16 rightmost;
};

//==============================================================================
enum VstMaxStringLengths
{
    vstMaxNameLength                     = 64,
    vstMaxParameterOrPinLabelLength      = 64,
    vstMaxParameterOrPinShortLabelLength = 8,
    vstMaxCategoryLength                 = 24,
    vstMaxManufacturerStringLength       = 64,
    vstMaxPlugInNameStringLength         = 64
};

//==============================================================================
struct VstPinInfo
{
    char text[vstMaxParameterOrPinLabelLength];
    int32 flags;
    int32 configurationType;
    char shortText[vstMaxParameterOrPinShortLabelLength];
    char unused[48];
};

enum VstPinInfoFlags
{
    vstPinInfoFlagIsActive = 1,
    vstPinInfoFlagIsStereo = 2,
    vstPinInfoFlagValid    = 4
};

//==============================================================================
struct VstEvent
{
    int32 type;
    int32 size;
    int32 sampleOffset;
    int32 flags;
    char content[16];
};

enum VstEventTypes
{
    vstMidiEventType  = 1,
    vstSysExEventType = 6
};

struct VstEventBlock
{
    int32 numberOfEvents;
    pointer_sized_int future;
    VstEvent* events[2];
};

struct VstMidiEvent
{
    int32 type;
    int32 size;
    int32 sampleOffset;
    int32 flags;
    int32 noteSampleLength;
    int32 noteSampleOffset;
    char midiData[4];
    char tuning;
    char noteVelocityOff;
    char future1;
    char future2;
};

enum VstMidiEventFlags
{
    vstMidiEventIsRealtime = 1
};

struct VstSysExEvent
{
    int32 type;
    int32 size;
    int32 offsetSamples;
    int32 flags;
    int32 sysExDumpSize;
    pointer_sized_int future1;
    char* sysExDump;
    pointer_sized_int future2;
};

//==============================================================================
struct VstTimingInformation
{
    double samplePosition;
    double sampleRate;
    double systemTimeNanoseconds;
    double musicalPosition;
    double tempoBPM;
    double lastBarPosition;
    double loopStartPosition;
    double loopEndPosition;
    int32 timeSignatureNumerator;
    int32 timeSignatureDenominator;
    int32 smpteOffset;
    int32 smpteRate;
    int32 samplesToNearestClock;
    int32 flags;
};

enum VstTimingInformationFlags
{
    vstTimingInfoFlagTransportChanged          = 1,
    vstTimingInfoFlagCurrentlyPlaying          = 2,
    vstTimingInfoFlagLoopActive                = 4,
    vstTimingInfoFlagCurrentlyRecording        = 8,
    vstTimingInfoFlagAutomationWriteModeActive = 64,
    vstTimingInfoFlagAutomationReadModeActive  = 128,
    vstTimingInfoFlagNanosecondsValid          = 256,
    vstTimingInfoFlagMusicalPositionValid      = 512,
    vstTimingInfoFlagTempoValid                = 1024,
    vstTimingInfoFlagLastBarPositionValid      = 2056,
    vstTimingInfoFlagLoopPositionValid         = 4096,
    vstTimingInfoFlagTimeSignatureValid        = 8192,
    vstTimingInfoFlagSmpteValid                = 16384,
    vstTimingInfoFlagNearestClockValid         = 32768
};

//==============================================================================
enum VstSmpteRates
{
    vstSmpteRateFps24,
    vstSmpteRateFps25,
    vstSmpteRateFps2997,
    vstSmpteRateFps30,
    vstSmpteRateFps2997drop,
    vstSmpteRateFps30drop,

    vstSmpteRate16mmFilm,
    vstSmpteRate35mmFilm,

    vstSmpteRateFps239 = vstSmpteRate35mmFilm + 3,
    vstSmpteRateFps249 ,
    vstSmpteRateFps599,
    vstSmpteRateFps60
};

//==============================================================================
struct VstIndividualSpeakerInfo
{
    float azimuthalAngle;
    float elevationAngle;
    float radius;
    float reserved;
    char label[vstMaxNameLength];
    int32 type;
    char unused[28];
};

enum VstIndividualSpeakerType
{
    vstIndividualSpeakerTypeUndefined = 0x7fffffff,
    vstIndividualSpeakerTypeMono = 0,
    vstIndividualSpeakerTypeLeft,
    vstIndividualSpeakerTypeRight,
    vstIndividualSpeakerTypeCentre,
    vstIndividualSpeakerTypeLFE,
    vstIndividualSpeakerTypeLeftSurround,
    vstIndividualSpeakerTypeRightSurround,
    vstIndividualSpeakerTypeLeftCentre,
    vstIndividualSpeakerTypeRightCentre,
    vstIndividualSpeakerTypeSurround,
    vstIndividualSpeakerTypeCentreSurround = vstIndividualSpeakerTypeSurround,
    vstIndividualSpeakerTypeLeftRearSurround,
    vstIndividualSpeakerTypeRightRearSurround,
    vstIndividualSpeakerTypeTopMiddle,
    vstIndividualSpeakerTypeTopFrontLeft,
    vstIndividualSpeakerTypeTopFrontCentre,
    vstIndividualSpeakerTypeTopFrontRight,
    vstIndividualSpeakerTypeTopRearLeft,
    vstIndividualSpeakerTypeTopRearCentre,
    vstIndividualSpeakerTypeTopRearRight,
    vstIndividualSpeakerTypeLFE2
};

struct VstSpeakerConfiguration
{
    int32 type;
    int32 numberOfChannels;
    VstIndividualSpeakerInfo speakers[8];
};

enum VstSpeakerConfigurationType
{
    vstSpeakerConfigTypeUser  = -2,
    vstSpeakerConfigTypeEmpty = -1,
    vstSpeakerConfigTypeMono  = 0,
    vstSpeakerConfigTypeLR,
    vstSpeakerConfigTypeLsRs,
    vstSpeakerConfigTypeLcRc,
    vstSpeakerConfigTypeSlSr,
    vstSpeakerConfigTypeCLfe,
    vstSpeakerConfigTypeLRC,
    vstSpeakerConfigTypeLRS,
    vstSpeakerConfigTypeLRCLfe,
    vstSpeakerConfigTypeLRLfeS,
    vstSpeakerConfigTypeLRCS,
    vstSpeakerConfigTypeLRLsRs,
    vstSpeakerConfigTypeLRCLfeS,
    vstSpeakerConfigTypeLRLfeLsRs,
    vstSpeakerConfigTypeLRCLsRs,
    vstSpeakerConfigTypeLRCLfeLsRs,
    vstSpeakerConfigTypeLRCLsRsCs,
    vstSpeakerConfigTypeLRLsRsSlSr,
    vstSpeakerConfigTypeLRCLfeLsRsCs,
    vstSpeakerConfigTypeLRLfeLsRsSlSr,
    vstSpeakerConfigTypeLRCLsRsLcRc,
    vstSpeakerConfigTypeLRCLsRsSlSr,
    vstSpeakerConfigTypeLRCLfeLsRsLcRc,
    vstSpeakerConfigTypeLRCLfeLsRsSlSr,
    vstSpeakerConfigTypeLRCLsRsLcRcCs,
    vstSpeakerConfigTypeLRCLsRsCsSlSr,
    vstSpeakerConfigTypeLRCLfeLsRsLcRcCs,
    vstSpeakerConfigTypeLRCLfeLsRsCsSlSr,
    vstSpeakerConfigTypeLRCLfeLsRsTflTfcTfrTrlTrrLfe2
};

#if JUCE_MSVC
 #pragma pack(pop)
#elif JUCE_MAC || JUCE_IOS
 #pragma options align=reset
#else
 #pragma pack(pop)
#endif

#endif // JUCE_VSTINTERFACE_H_INCLUDED
