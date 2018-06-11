//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 2.4
//
// Category    : VST 2.x Interfaces
// Filename    : pluginterfaces/vst2.x/aeffect.h
// Created by  : Steinberg, 01/2004
// Description : Definition of AEffect structure (VST 1.0)
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

#ifndef __aeffect__
#define __aeffect__

//-------------------------------------------------------------------------------------------------------
// gcc based compiler, or CodeWarrior on Mac OS X
#if ((defined(__GNUC__) && (defined(__APPLE_CPP__) || defined(__APPLE_CC__))) || (defined (__MWERKS__) && defined (__MACH__)))
	#ifndef TARGET_API_MAC_CARBON
		#define TARGET_API_MAC_CARBON 1
	#endif
	#if __ppc__
		#ifndef VST_FORCE_DEPRECATED
			#define VST_FORCE_DEPRECATED 0
		#endif
	#endif
#endif

#ifdef _WIN32
	#ifndef WIN32
	#define WIN32	1
	#endif
#endif

#if TARGET_API_MAC_CARBON
	#ifdef __LP64__
		#pragma options align=power
	#else
		#pragma options align=mac68k
	#endif
	#define VSTCALLBACK
#elif defined __BORLANDC__
	#pragma -a8
	#pragma options push -a8
#elif defined(__GNUC__)
    #pragma pack(push,8)
    #define VSTCALLBACK __cdecl
#elif defined(WIN32) || defined(__FLAT__) || defined CBUILDER
	#pragma pack(push)
	#pragma pack(8)
	#define VSTCALLBACK __cdecl
#else
	#define VSTCALLBACK
#endif
//-------------------------------------------------------------------------------------------------------

#include <string.h>	// for strncpy

//-------------------------------------------------------------------------------------------------------
// VST Version
//-------------------------------------------------------------------------------------------------------

/** Define SDK Version (you can generate different versions (from 2.0 to 2.4) of this SDK by setting the unwanted extensions to 0). */
#define VST_2_1_EXTENSIONS 1 ///< Version 2.1 extensions (08-06-2000)
#define VST_2_2_EXTENSIONS 1 ///< Version 2.2 extensions (08-06-2001)
#define VST_2_3_EXTENSIONS 1 ///< Version 2.3 extensions (20-05-2003)
#ifndef VST_2_4_EXTENSIONS
#define VST_2_4_EXTENSIONS 1 ///< Version 2.4 extensions (01-01-2006)
#endif

/** Current VST Version */
#if VST_2_4_EXTENSIONS
	#define kVstVersion 2400
#elif VST_2_3_EXTENSIONS
	#define kVstVersion 2300
#elif VST_2_2_EXTENSIONS
	#define kVstVersion 2200
#elif VST_2_1_EXTENSIONS
	#define kVstVersion 2100
#else
	#define kVstVersion 2
#endif

/** Disable for Hosts to serve Plug-ins below VST 2.4 */
#ifndef VST_FORCE_DEPRECATED
#define VST_FORCE_DEPRECATED VST_2_4_EXTENSIONS
#endif

/** Declares identifier as deprecated. */
#if VST_FORCE_DEPRECATED
#define DECLARE_VST_DEPRECATED(identifier) __##identifier##Deprecated
#else
#define DECLARE_VST_DEPRECATED(identifier) identifier
#endif

/** Define for 64 Bit Platform. */
#ifndef VST_64BIT_PLATFORM
#define VST_64BIT_PLATFORM _WIN64 || __LP64__
#endif

//-------------------------------------------------------------------------------------------------------
// Integral Types
//-------------------------------------------------------------------------------------------------------

typedef char VstInt8;				///< 8 bit integer type

#ifdef WIN32
	typedef short VstInt16;			///< 16 bit integer type
	typedef int VstInt32;			///< 32 bit integer type
	typedef __int64 VstInt64;		///< 64 bit integer type
#else
	#include <stdint.h>
	typedef int16_t VstInt16;		///< 16 bit integer type
	typedef int32_t VstInt32;		///< 32 bit integer type
	typedef int64_t VstInt64;		///< 64 bit integer type
#endif

//-------------------------------------------------------------------------------------------------------
// Generic Types
//-------------------------------------------------------------------------------------------------------

#if VST_64BIT_PLATFORM
typedef VstInt64 VstIntPtr;			///< platform-dependent integer type, same size as pointer
#else
typedef VstInt32 VstIntPtr;			///< platform-dependent integer type, same size as pointer
#endif

//-------------------------------------------------------------------------------------------------------
// Misc. Definition
//-------------------------------------------------------------------------------------------------------
#undef CCONST
typedef struct AEffect AEffect;

//-------------------------------------------------------------------------------------------------------
/// @cond ignore
typedef	VstIntPtr (VSTCALLBACK *audioMasterCallback) (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
typedef VstIntPtr (VSTCALLBACK *AEffectDispatcherProc) (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
typedef void (VSTCALLBACK *AEffectProcessProc) (AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames);
typedef void (VSTCALLBACK *AEffectProcessDoubleProc) (AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames);
typedef void (VSTCALLBACK *AEffectSetParameterProc) (AEffect* effect, VstInt32 index, float parameter);
typedef float (VSTCALLBACK *AEffectGetParameterProc) (AEffect* effect, VstInt32 index);
/// @endcond

/** Four Character Constant (for AEffect->uniqueID) */
#define CCONST(a, b, c, d) \
	 ((((VstInt32)a) << 24) | (((VstInt32)b) << 16) | (((VstInt32)c) << 8) | (((VstInt32)d) << 0))

/** AEffect magic number */
#define kEffectMagic CCONST ('V', 's', 't', 'P')

//-------------------------------------------------------------------------------------------------------
/** Basic VST Effect "C" Interface. */
//-------------------------------------------------------------------------------------------------------
struct AEffect
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 magic;			///< must be #kEffectMagic ('VstP')

	/** Host to Plug-in dispatcher @see AudioEffect::dispatcher */
	AEffectDispatcherProc dispatcher;

	/** \deprecated Accumulating process mode is deprecated in VST 2.4! Use AEffect::processReplacing instead! */
	AEffectProcessProc DECLARE_VST_DEPRECATED (process);

	/** Set new value of automatable parameter @see AudioEffect::setParameter */
	AEffectSetParameterProc setParameter;

	/** Returns current value of automatable parameter @see AudioEffect::getParameter*/
	AEffectGetParameterProc getParameter;

	VstInt32 numPrograms;   ///< number of programs
	VstInt32 numParams;		///< all programs are assumed to have numParams parameters
	VstInt32 numInputs;		///< number of audio inputs
	VstInt32 numOutputs;	///< number of audio outputs

	VstInt32 flags;			///< @see VstAEffectFlags

	VstIntPtr resvd1;		///< reserved for Host, must be 0
	VstIntPtr resvd2;		///< reserved for Host, must be 0

	VstInt32 initialDelay;	///< for algorithms which need input in the first place (Group delay or latency in Samples). This value should be initialized in a resume state.

	VstInt32 DECLARE_VST_DEPRECATED (realQualities);	///< \deprecated unused member
	VstInt32 DECLARE_VST_DEPRECATED (offQualities);		///< \deprecated unused member
	float    DECLARE_VST_DEPRECATED (ioRatio);			///< \deprecated unused member

	void* object;			///< #AudioEffect class pointer
	void* user;				///< user-defined pointer

	VstInt32 uniqueID;		///< registered unique identifier (register it at Steinberg 3rd party support Web). This is used to identify a plug-in during save+load of preset and project.
	VstInt32 version;		///< plug-in version (example 1100 for version 1.1.0.0)

	/** Process audio samples in replacing mode @see AudioEffect::processReplacing */
	AEffectProcessProc processReplacing;

#if VST_2_4_EXTENSIONS
	/** Process double-precision audio samples in replacing mode @see AudioEffect::processDoubleReplacing */
	AEffectProcessDoubleProc processDoubleReplacing;

	char future[56];		///< reserved for future use (please zero)
#else
	char future[60];		///< reserved for future use (please zero)
#endif // VST_2_4_EXTENSIONS
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** AEffect flags */
//-------------------------------------------------------------------------------------------------------
enum VstAEffectFlags
{
//-------------------------------------------------------------------------------------------------------
	effFlagsHasEditor     = 1 << 0,			///< set if the plug-in provides a custom editor
	effFlagsCanReplacing  = 1 << 4,			///< supports replacing process mode (which should the default mode in VST 2.4)
	effFlagsProgramChunks = 1 << 5,			///< program data is handled in formatless chunks
	effFlagsIsSynth       = 1 << 8,			///< plug-in is a synth (VSTi), Host may assign mixer channels for its outputs
	effFlagsNoSoundInStop = 1 << 9,			///< plug-in does not produce sound when input is all silence

#if VST_2_4_EXTENSIONS
	effFlagsCanDoubleReplacing = 1 << 12,	///< plug-in supports double precision processing
#endif

	DECLARE_VST_DEPRECATED (effFlagsHasClip) = 1 << 1,			///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (effFlagsHasVu)   = 1 << 2,			///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (effFlagsCanMono) = 1 << 3,			///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (effFlagsExtIsAsync)   = 1 << 10,	///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (effFlagsExtHasBuffer) = 1 << 11		///< \deprecated deprecated in VST 2.4
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Basic dispatcher Opcodes (Host to Plug-in) */
//-------------------------------------------------------------------------------------------------------
enum AEffectOpcodes
{
	effOpen = 0,		///< no arguments  @see AudioEffect::open
	effClose,			///< no arguments  @see AudioEffect::close

	effSetProgram,		///< [value]: new program number  @see AudioEffect::setProgram
	effGetProgram,		///< [return value]: current program number  @see AudioEffect::getProgram
	effSetProgramName,	///< [ptr]: char* with new program name, limited to #kVstMaxProgNameLen  @see AudioEffect::setProgramName
	effGetProgramName,	///< [ptr]: char buffer for current program name, limited to #kVstMaxProgNameLen  @see AudioEffect::getProgramName

	effGetParamLabel,	///< [ptr]: char buffer for parameter label, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterLabel
	effGetParamDisplay,	///< [ptr]: char buffer for parameter display, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterDisplay
	effGetParamName,	///< [ptr]: char buffer for parameter name, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterName

	DECLARE_VST_DEPRECATED (effGetVu),	///< \deprecated deprecated in VST 2.4

	effSetSampleRate,	///< [opt]: new sample rate for audio processing  @see AudioEffect::setSampleRate
	effSetBlockSize,	///< [value]: new maximum block size for audio processing  @see AudioEffect::setBlockSize
	effMainsChanged,	///< [value]: 0 means "turn off", 1 means "turn on"  @see AudioEffect::suspend @see AudioEffect::resume

	effEditGetRect,		///< [ptr]: #ERect** receiving pointer to editor size  @see ERect @see AEffEditor::getRect
	effEditOpen,		///< [ptr]: system dependent Window pointer, e.g. HWND on Windows  @see AEffEditor::open
	effEditClose,		///< no arguments @see AEffEditor::close

	DECLARE_VST_DEPRECATED (effEditDraw),	///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (effEditMouse),	///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (effEditKey),	///< \deprecated deprecated in VST 2.4

	effEditIdle,		///< no arguments @see AEffEditor::idle

	DECLARE_VST_DEPRECATED (effEditTop),	///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (effEditSleep),	///< \deprecated deprecated in VST 2.4
	DECLARE_VST_DEPRECATED (effIdentify),	///< \deprecated deprecated in VST 2.4

	effGetChunk,		///< [ptr]: void** for chunk data address [index]: 0 for bank, 1 for program  @see AudioEffect::getChunk
	effSetChunk,		///< [ptr]: chunk data [value]: byte size [index]: 0 for bank, 1 for program  @see AudioEffect::setChunk

	effNumOpcodes
};

//-------------------------------------------------------------------------------------------------------
/** Basic dispatcher Opcodes (Plug-in to Host) */
//-------------------------------------------------------------------------------------------------------
enum AudioMasterOpcodes
{
//-------------------------------------------------------------------------------------------------------
	audioMasterAutomate = 0,	///< [index]: parameter index [opt]: parameter value  @see AudioEffect::setParameterAutomated
	audioMasterVersion,			///< [return value]: Host VST version (for example 2400 for VST 2.4) @see AudioEffect::getMasterVersion
	audioMasterCurrentId,		///< [return value]: current unique identifier on shell plug-in  @see AudioEffect::getCurrentUniqueId
	audioMasterIdle,			///< no arguments  @see AudioEffect::masterIdle
	DECLARE_VST_DEPRECATED (audioMasterPinConnected) ///< \deprecated deprecated in VST 2.4 r2
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** String length limits (in characters excl. 0 byte) */
//-------------------------------------------------------------------------------------------------------
enum VstStringConstants
{
//-------------------------------------------------------------------------------------------------------
	kVstMaxProgNameLen   = 24,	///< used for #effGetProgramName, #effSetProgramName, #effGetProgramNameIndexed
	kVstMaxParamStrLen   = 8,	///< used for #effGetParamLabel, #effGetParamDisplay, #effGetParamName
	kVstMaxVendorStrLen  = 64,	///< used for #effGetVendorString, #audioMasterGetVendorString
	kVstMaxProductStrLen = 64,	///< used for #effGetProductString, #audioMasterGetProductString
	kVstMaxEffectNameLen = 32	///< used for #effGetEffectName
//-------------------------------------------------------------------------------------------------------
};

#ifdef  __cplusplus
#define VST_INLINE inline
#else
#define VST_INLINE
#endif

//-------------------------------------------------------------------------------------------------------
/** String copy taking care of null terminator. */
//-------------------------------------------------------------------------------------------------------
VST_INLINE char* vst_strncpy (char* dst, const char* src, size_t maxLen)
{
	char* result = strncpy (dst, src, maxLen);
	dst[maxLen] = 0;
	return result;
}

//-------------------------------------------------------------------------------------------------------
/** String concatenation taking care of null terminator. */
//-------------------------------------------------------------------------------------------------------
VST_INLINE char* vst_strncat (char* dst, const char* src, size_t maxLen)
{
	char* result = strncat (dst, src, maxLen);
	dst[maxLen] = 0;
	return result;
}

#ifdef  __cplusplus
//-------------------------------------------------------------------------------------------------------
/** Cast #VstIntPtr to pointer. */
//-------------------------------------------------------------------------------------------------------
template <class T> inline T* FromVstPtr (VstIntPtr& arg)
{
	T** address = (T**)&arg;
	return *address;
}

//-------------------------------------------------------------------------------------------------------
/** Cast pointer to #VstIntPtr. */
//-------------------------------------------------------------------------------------------------------
template <class T> inline VstIntPtr ToVstPtr (T* ptr)
{
	VstIntPtr* address = (VstIntPtr*)&ptr;
	return *address;
}
#endif // __cplusplus

//-------------------------------------------------------------------------------------------------------
/** Structure used for #effEditGetRect. */
//-------------------------------------------------------------------------------------------------------
struct ERect
{
//-------------------------------------------------------------------------------------------------------
	VstInt16 top;		///< top coordinate
	VstInt16 left;		///< left coordinate
	VstInt16 bottom;	///< bottom coordinate
	VstInt16 right;		///< right coordinate
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

#endif // __aeffect__
