/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019-2024 Avid Technology, Inc.
 *	All rights reserved.
 *	
 *	This file is part of the Avid AAX SDK.
 *	
 *	The AAX SDK is subject to commercial or open-source licensing.
 *	
 *	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
 *	Agreement and Avid Privacy Policy.
 *	
 *	AAX SDK License: https://developer.avid.com/aax
 *	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement
 *	
 *	Or: You may also use this code under the terms of the GPL v3 (see
 *	www.gnu.org/licenses).
 *	
 *	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 *	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 *	DISCLAIMED.
 *
 */

/**  
 *	\file AAX_Enums.h
 *
 *	\brief Utility functions for byte-swapping.  Used by AAX_CChunkDataParser.
 *
 */ 
/*================================================================================================*/


/// @cond ignore
#ifndef AAX_ENUMS_H
#define AAX_ENUMS_H
/// @endcond

#include <stdint.h>

#define AAX_INT32_MIN    (-2147483647 - 1) /** minimum signed 32 bit value */
#define AAX_INT32_MAX      2147483647      /** maximum signed 32 bit value */
#define AAX_UINT32_MIN     0U              /** minimum unsigned 32 bit value */
#define AAX_UINT32_MAX     4294967295U     /** maximum unsigned 32 bit value */
#define AAX_INT16_MIN    (-32767 - 1)      /** minimum signed 16 bit value */
#define AAX_INT16_MAX      32767           /** maximum signed 16 bit value */
#define AAX_UINT16_MIN     0U              /** minimum unsigned 16 bit value */
#define AAX_UINT16_MAX     65535U          /** maximum unsigned 16 bit value */

/** \brief Macro to ensure enum type consistency across binaries
 */
#ifndef _TMS320C6X
#define AAX_ENUM_SIZE_CHECK(x) extern int __enumSizeCheck[ 2*(sizeof(uint32_t)==sizeof(x)) - 1]
#else
#define AAX_ENUM_SIZE_CHECK(x)
#endif


//******************************************************************
// ENUM: AAX_EHighlightColor
//******************************************************************
/**	\brief Highlight color selector
	
	\details
	\sa AAX_IEffectGUI::SetControlHighlightInfo()
 */
enum AAX_EHighlightColor
{
	AAX_eHighlightColor_Red = 0,
	AAX_eHighlightColor_Blue = 1,
	AAX_eHighlightColor_Green = 2,
	AAX_eHighlightColor_Yellow = 3,

	AAX_eHighlightColor_Num
}; AAX_ENUM_SIZE_CHECK( AAX_EHighlightColor );


/**	\brief Platform-specific tracing priorities
	
	\details
	Use the generic \c EAAX_Trace_Priority in plug-ins for 
	cross-platform tracing (see AAX_Assert.h)
 */
enum AAX_ETracePriorityHost
{
	AAX_eTracePriorityHost_None		= 0,
	AAX_eTracePriorityHost_Critical	= 0x10000000,
	AAX_eTracePriorityHost_High		= 0x08000000,
	AAX_eTracePriorityHost_Normal	= 0x04000000,
	AAX_eTracePriorityHost_Low		= 0x02000000,
	AAX_eTracePriorityHost_Lowest	= 0x01000000
}; AAX_ENUM_SIZE_CHECK( AAX_ETracePriorityHost );

/**	\brief Platform-specific tracing priorities
	
	\details
	Use the generic EAAX_Trace_Priority in plug-ins for 
	cross-platform tracing (see AAX_Assert.h)
 */
enum AAX_ETracePriorityDSP
{
	AAX_eTracePriorityDSP_None = 0,
	AAX_eTracePriorityDSP_Assert = 1, 
	AAX_eTracePriorityDSP_High = 2,
	AAX_eTracePriorityDSP_Normal = 3,
	AAX_eTracePriorityDSP_Low = 4
}; AAX_ENUM_SIZE_CHECK( AAX_ETracePriorityDSP );

/**	\brief Modifier key definitions used by %AAX API
 */
enum AAX_EModifiers
{
	AAX_eModifiers_None				= 0,
	
	AAX_eModifiers_Shift			= ( 1 << 0 ), ///< Shift
	AAX_eModifiers_Control			= ( 1 << 1 ), ///< Control on Mac, Winkey/Start on PC
	AAX_eModifiers_Option			= ( 1 << 2 ), ///< Option on Mac, Alt on PC
	AAX_eModifiers_Command			= ( 1 << 3 ), ///< Command on Mac, Ctrl on PC
	AAX_eModifiers_SecondaryButton	= ( 1 << 4 ), ///< Secondary mouse button
	
	AAX_eModifiers_Alt				= AAX_eModifiers_Option,  ///< Option on Mac, Alt on PC
	AAX_eModifiers_Cntl				= AAX_eModifiers_Command, ///< Command on Mac, Cntl on PC
	AAX_eModifiers_WINKEY			= AAX_eModifiers_Control ///< Control on Mac, WINKEY on PC
}; AAX_ENUM_SIZE_CHECK( AAX_EModifiers );

/** \brief Generic buffer length definitions
 
 	\details
	These enum values can be used to calculate literal
	values as powers of two:

	\code
	(1 << AAX_eAudioBufferLength_16) == 16;
	\endcode

	\sa \ref AAX_EAudioBufferLengthDSP
	\sa \ref AAE_EAudioBufferLengthNative
 */
enum AAX_EAudioBufferLength
{
	AAX_eAudioBufferLength_Undefined = -1,
	AAX_eAudioBufferLength_1 = 0,
	AAX_eAudioBufferLength_2 = 1,
	AAX_eAudioBufferLength_4 = 2,
	AAX_eAudioBufferLength_8 = 3,
	AAX_eAudioBufferLength_16 = 4,
	AAX_eAudioBufferLength_32 = 5,
	AAX_eAudioBufferLength_64 = 6,
	AAX_eAudioBufferLength_128 = 7,
	AAX_eAudioBufferLength_256 = 8,
	AAX_eAudioBufferLength_512 = 9,
	AAX_eAudioBufferLength_1024 = 10,
	
	/** \brief Maximum buffer length for ProcessProc processing buffers
	 	
	 	\details
		Audio buffers for other methods, such as the high-latency render
		callback for \ref additionalFeatures_Hybrid "AAX Hybrid" or the
		offline render callback for
		\ref AuxInterface_HostProcessor "Host Processor" effects, may
		contain more samples than AAX_eAudioBufferLength_Max.
	 */
	AAX_eAudioBufferLength_Max = AAX_eAudioBufferLength_1024
}; AAX_ENUM_SIZE_CHECK( AAX_EAudioBufferLength );

/** \brief Currently supported processing buffer length definitions for %AAX DSP hosts

	\details
	%AAX DSP decks must support at least these buffer lengths.  All %AAX DSP
	algorithm ProcessProcs must support exactly one of these buffer lengths.

	\sa AAX_eProperty_DSP_AudioBufferLength
 */
enum AAX_EAudioBufferLengthDSP
{
	AAX_eAudioBufferLengthDSP_Default = AAX_eAudioBufferLength_4,
	AAX_eAudioBufferLengthDSP_4 = AAX_eAudioBufferLength_4,
	AAX_eAudioBufferLengthDSP_16 = AAX_eAudioBufferLength_16,
	AAX_eAudioBufferLengthDSP_32 = AAX_eAudioBufferLength_32,
	AAX_eAudioBufferLengthDSP_64 = AAX_eAudioBufferLength_64,
	
	AAX_eAudioBufferLengthDSP_Max = AAX_eAudioBufferLengthDSP_64
}; AAX_ENUM_SIZE_CHECK( AAX_EAudioBufferLengthDSP );

/** \brief Processing buffer length definitions for Native %AAX hosts
	
	\details
	All %AAX Native plug-ins must support variable buffer lengths.  The
	buffer lengths that a host will use are constrained by the values
	in this enum.  All Native buffer lengths will be powers of two, as
	per \ref AAX_EAudioBufferLength

	\sa AAX_eProperty_DSP_AudioBufferLength
 */
enum AAE_EAudioBufferLengthNative
{
	AAX_eAudioBufferLengthNative_Min = AAX_eAudioBufferLength_32,   ///< Minimum Native buffer length
	AAX_eAudioBufferLengthNative_Max = AAX_eAudioBufferLength_Max      ///< Maximum Native buffer length
}; AAX_ENUM_SIZE_CHECK( AAE_EAudioBufferLengthNative );

/** @brief	The maximum number of tracks that an %AAX host will
 	process in a non-real-time context
 
	\sa \ref AAX_eProperty_NumberOfInputs and \ref AAX_eProperty_NumberOfOutputs
 */
enum AAX_EMaxAudioSuiteTracks
{
	AAX_eMaxAudioSuiteTracks = 48
}; AAX_ENUM_SIZE_CHECK( AAX_EMaxAudioSuiteTracks );

// The channel count ternary here will issue a warning due to a
// signed/unsigned mismatch if anyone tries to create an
// AAX_STEM_FORMAT definition with a negative channel count.
#define AAX_STEM_FORMAT( aIndex, aChannelCount )		( static_cast<uint32_t>( ( static_cast<uint16_t>(aIndex) << 16 ) | ( (aChannelCount >= AAX_UINT16_MIN) && (aChannelCount <= 0xFFFF) ? aChannelCount & 0xFFFF : 0x0000 ) ) )
#define AAX_STEM_FORMAT_CHANNEL_COUNT( aStemFormat )	( static_cast<uint16_t>( aStemFormat & 0xFFFF ) )
#define AAX_STEM_FORMAT_INDEX( aStemFormat )			( static_cast<int16_t>( ( aStemFormat >> 16 ) & 0xFFFF ) )

/** @brief Stem format definitions
 
 @details
 A stem format combines a channel count with a semantic meaning for each
 channel. Usually this is the speaker or speaker position associated with
 the data in the channel. The meanings of each channel in each stem format
 (i.e. channel orders) are listed below.
 
 Not all stem formats are supported by all %AAX plug-in hosts. An effect
 may describe support for any stem format combination which it supports
 and the host will ignore any configurations which it cannot support.
 
 \note When defining stem format support in \ref AAX_IHostProcessor
 effects do not use stem format properties or values. Instead, use
 \ref AAX_eProperty_NumberOfInputs and \ref AAX_eProperty_NumberOfOutputs
 with integer channel count values.
 
 \sa - \ref AAX_eProperty_InputStemFormat
 \sa - \ref AAX_eProperty_OutputStemFormat
 \sa - \ref AAX_eProperty_HybridInputStemFormat
 \sa - \ref AAX_eProperty_HybridOutputStemFormat
 \sa - \ref AAX_eProperty_SideChainStemFormat
 */
enum AAX_EStemFormat
{
	// Point source stem formats
	AAX_eStemFormat_Mono		= AAX_STEM_FORMAT ( 0,	 1 ),	///<  M
	AAX_eStemFormat_Stereo		= AAX_STEM_FORMAT ( 1,	 2 ),	///<  L     R
	AAX_eStemFormat_LCR			= AAX_STEM_FORMAT ( 2,	 3 ),	///<  L  C  R
	AAX_eStemFormat_LCRS		= AAX_STEM_FORMAT ( 3,	 4 ),	///<  L  C  R  S
	AAX_eStemFormat_Quad		= AAX_STEM_FORMAT ( 4,	 4 ),	///<  L     R          Ls      Rs
	AAX_eStemFormat_5_0			= AAX_STEM_FORMAT ( 5,	 5 ),	///<  L  C  R          Ls      Rs
	AAX_eStemFormat_5_1			= AAX_STEM_FORMAT ( 6,	 6 ),	///<  L  C  R          Ls      Rs  LFE
	AAX_eStemFormat_6_0			= AAX_STEM_FORMAT ( 7,	 6 ),	///<  L  C  R          Ls  Cs  Rs
	AAX_eStemFormat_6_1			= AAX_STEM_FORMAT ( 8,	 7 ),	///<  L  C  R          Ls  Cs  Rs  LFE
	AAX_eStemFormat_7_0_SDDS	= AAX_STEM_FORMAT ( 9,	 7 ),	///<  L  Lc C  Rc  R   Ls      Rs
	AAX_eStemFormat_7_1_SDDS	= AAX_STEM_FORMAT ( 10,	 8 ),	///<  L  Lc C  Rc  R   Ls      Rs  LFE
	AAX_eStemFormat_7_0_DTS		= AAX_STEM_FORMAT ( 11,	 7 ),	///<  L  C  R          Lss Rss Lsr Rsr
	AAX_eStemFormat_7_1_DTS		= AAX_STEM_FORMAT ( 12,	 8 ),	///<  L  C  R          Lss Rss Lsr Rsr LFE
	AAX_eStemFormat_7_0_2		= AAX_STEM_FORMAT ( 20,	 9 ),	///<  L  C  R          Lss Rss Lsr Rsr     Lts Rts
	AAX_eStemFormat_7_1_2		= AAX_STEM_FORMAT ( 13,	 10 ),	///<  L  C  R          Lss Rss Lsr Rsr LFE Lts Rts
	AAX_eStemFormat_5_0_2		= AAX_STEM_FORMAT ( 21,	 7 ),	///<  L  C  R  Ls  Rs          Ltm Rtm
	AAX_eStemFormat_5_1_2		= AAX_STEM_FORMAT ( 22,	 8 ),	///<  L  C  R  Ls  Rs  LFE     Ltm Rtm
	AAX_eStemFormat_5_0_4		= AAX_STEM_FORMAT ( 23,	 9 ),	///<  L  C  R  Ls  Rs                      Ltf Rtf         Ltr Rtr
	AAX_eStemFormat_5_1_4		= AAX_STEM_FORMAT ( 24,	 10 ),	///<  L  C  R  Ls  Rs  LFE                 Ltf Rtf         Ltr Rtr
	AAX_eStemFormat_7_0_4		= AAX_STEM_FORMAT ( 25,	 11 ),	///<  L  C  R          Lss Rss Lsr Rsr     Ltf Rtf         Ltr Rtr
	AAX_eStemFormat_7_1_4		= AAX_STEM_FORMAT ( 26,	 12 ),	///<  L  C  R          Lss Rss Lsr Rsr LFE Ltf Rtf         Ltr Rtr
	AAX_eStemFormat_7_0_6		= AAX_STEM_FORMAT ( 35,	 13 ),	///<  L  C  R          Lss Rss Lsr Rsr     Ltf Rtf Ltm Rtm Ltr Rtr
	AAX_eStemFormat_7_1_6		= AAX_STEM_FORMAT ( 36,	 14 ),	///<  L  C  R          Lss Rss Lsr Rsr LFE Ltf Rtf Ltm Rtm Ltr Rtr
	AAX_eStemFormat_9_0_4		= AAX_STEM_FORMAT ( 27,	 13 ),	///<  L  C  R  Lw  Rw  Lss Rss Lsr Rsr     Ltf Rtf         Ltr Rtr
	AAX_eStemFormat_9_1_4		= AAX_STEM_FORMAT ( 28,	 14 ),	///<  L  C  R  Lw  Rw  Lss Rss Lsr Rsr LFE Ltf Rtf         Ltr Rtr
	AAX_eStemFormat_9_0_6		= AAX_STEM_FORMAT ( 29,	 15 ),	///<  L  C  R  Lw  Rw  Lss Rss Lsr Rsr     Ltf Rtf Ltm Rtm Ltr Rtr
	AAX_eStemFormat_9_1_6		= AAX_STEM_FORMAT ( 30,	 16 ),	///<  L  C  R  Lw  Rw  Lss Rss Lsr Rsr LFE Ltf Rtf Ltm Rtm Ltr Rtr
	
	// Ambisonics stem formats
	AAX_eStemFormat_Ambi_1_ACN = AAX_STEM_FORMAT ( 14, 4 ),   ///< Ambisonics: first-order with ACN channel order and SN3D (AmbiX) normalization
	AAX_eStemFormat_Ambi_2_ACN = AAX_STEM_FORMAT ( 18, 9 ),   ///< Ambisonics: second-order with ACN channel order and SN3D (AmbiX) normalization
	AAX_eStemFormat_Ambi_3_ACN = AAX_STEM_FORMAT ( 19, 16 ),  ///< Ambisonics: third-order with ACN channel order and SN3D (AmbiX) normalization
	AAX_eStemFormat_Ambi_4_ACN = AAX_STEM_FORMAT ( 31, 25 ),  ///< Ambisonics: fourth-order with ACN channel order and SN3D (AmbiX) normalization
	AAX_eStemFormat_Ambi_5_ACN = AAX_STEM_FORMAT ( 32, 36 ),  ///< Ambisonics: fifth-order with ACN channel order and SN3D (AmbiX) normalization
	AAX_eStemFormat_Ambi_6_ACN = AAX_STEM_FORMAT ( 33, 49 ),  ///< Ambisonics: sixth-order with ACN channel order and SN3D (AmbiX) normalization
	AAX_eStemFormat_Ambi_7_ACN = AAX_STEM_FORMAT ( 34, 64 ),  ///< Ambisonics: seventh-order with ACN channel order and SN3D (AmbiX) normalization
	
	

	
	AAX_eStemFormatNum			= 37, // One greater than the highest available AAX_STEM_FORMAT_INDEX value. This needs to increase as stem types are added.
	
	AAX_eStemFormat_None		= AAX_STEM_FORMAT ( -100, 0 ),
	AAX_eStemFormat_Any			= AAX_STEM_FORMAT ( -1, 0 ),

	AAX_eStemFormat_INT32_MAX = AAX_INT32_MAX
}; AAX_ENUM_SIZE_CHECK( AAX_EStemFormat );

/** @brief Effect category definitions

	@details
	Used with \ref AAX_IEffectDescriptor::AddCategory() to categorize an Effect.

	These values are bitwise-exclusive and may be used in a bitmask to define multiple categories:

	\code
	myCategory = AAX_ePlugInCategory_EQ | AAX_ePlugInCategory_Dynamics;
	\endcode

	\note The host may handle plug-ins with different categories in different manners, e.g.
	replacing "analyze" with "reverse" for offline processing of delays and reverbs.
 
 */
enum AAX_EPlugInCategory							
{
	AAX_ePlugInCategory_None			= 0x00000000,
	AAX_ePlugInCategory_EQ				= 0x00000001,	///<  Equalization
	AAX_ePlugInCategory_Dynamics		= 0x00000002,	///<  Compressor, expander, limiter, etc.
	AAX_ePlugInCategory_PitchShift		= 0x00000004,	///<  Pitch processing
	AAX_ePlugInCategory_Reverb			= 0x00000008,	///<  Reverberation and room/space simulation
	AAX_ePlugInCategory_Delay			= 0x00000010,	///<  Delay and echo
	AAX_ePlugInCategory_Modulation		= 0x00000020,	///<  Phasing, flanging, chorus, etc.
	AAX_ePlugInCategory_Harmonic		= 0x00000040,	///<  Distortion, saturation, and harmonic enhancement
	AAX_ePlugInCategory_NoiseReduction	= 0x00000080,	///<  Noise reduction
	AAX_ePlugInCategory_Dither			= 0x00000100,	///<  Dither, noise shaping, etc.
	AAX_ePlugInCategory_SoundField		= 0x00000200,  	///<  Pan, auto-pan, upmix and downmix, and surround handling
	AAX_ePlugInCategory_HWGenerators	= 0x00000400,	///<  Fixed hardware audio sources such as SampleCell
	AAX_ePlugInCategory_SWGenerators	= 0x00000800,	///<  Virtual instruments, metronomes, and other software audio sources
	AAX_ePlugInCategory_WrappedPlugin	= 0x00001000,	///<  All plug-ins wrapped by a thrid party wrapper (i.e. VST to RTAS wrapper), except for VI plug-ins which should be mapped to AAX_PlugInCategory_SWGenerators
	AAX_EPlugInCategory_Effect			= 0x00002000,	///<  Special effects
	
// HACK: 32-bit hosts do not have support for AAX_ePlugInCategory_Example
#if ( defined(_WIN64) || defined(__LP64__) )
	AAX_ePlugInCategory_Example			= 0x00004000,	///<  SDK example plug-ins \compatibility \ref AAX_ePlugInCategory_Example is compatible with Pro Tools 11 and higher. Effects with this category will not appear in Pro Tools 10.
#else
	AAX_ePlugInCategory_Example			= AAX_EPlugInCategory_Effect,
#endif

	AAX_EPlugInCategory_MIDIEffect		= 0x00010000,	///<  MIDI effects

	AAX_ePlugInCategory_INT32_MAX = AAX_INT32_MAX
}; AAX_ENUM_SIZE_CHECK( AAX_EPlugInCategory );

/** @brief Effect string identifiers
	
	@details
	The %AAX host may associate certain plug-in display strings
	with these identifiers.

	\sa \ref AAX_IEffectGUI::GetCustomLabel()
 
 */
enum AAX_EPlugInStrings
{
	AAX_ePlugInStrings_Analysis = 0,					///< "Analyze" button label (AudioSuite)					\legacy Was pluginStrings_Analysis in the RTAS/TDM SDK
	AAX_ePlugInStrings_MonoMode = 1,					///< "Mono Mode" selector label (AudioSuite)				\legacy Was pluginStrings_MonoMode in the RTAS/TDM SDK
	AAX_ePlugInStrings_MultiInputMode = 2,				///< "Multi-Input Mode" selector label (AudioSuite)			\legacy Was pluginStrings_MultiInputMode in the RTAS/TDM SDK
	AAX_ePlugInStrings_RegionByRegionAnalysis = 3,		///< "Clip-by-Clip Analysis" selector label (AudioSuite)	\legacy Was pluginStrings_RegionByRegionAnalysis in the RTAS/TDM SDK
	AAX_ePlugInStrings_AllSelectedRegionsAnalysis = 4,	///< "Whole File Analysis" selector label (AudioSuite)		\legacy Was pluginStrings_AllSelectedRegionsAnalysis in the RTAS/TDM SDK
	AAX_ePlugInStrings_RegionName = 5,					///< \deprecated
	AAX_ePlugInStrings_ClipName = 5,					///< Clip name label (AudioSuite). This value will replace the clip's name. \sa AAX_ePlugInStrings_ClipNameSuffix  \legacy Was pluginStrings_RegionName in the RTAS/TDM SDK
	AAX_ePlugInStrings_Progress = 6,					///< Progress bar label (AudioSuite)						\compatibility Not currently supported by Pro Tools \legacy Was pluginStrings_Progress in the RTAS/TDM SDK
	AAX_ePlugInStrings_PlugInFileName = 7,				///< \deprecated
	AAX_ePlugInStrings_Preview = 8,						///< \deprecated
	AAX_ePlugInStrings_Process = 9,						///< "Render" button label (AudioSuite)						\legacy Was pluginStrings_Process in the RTAS/TDM SDK
	AAX_ePlugInStrings_Bypass = 10,						///< "Bypass" button label (AudioSuite)						\legacy Was pluginStrings_Bypass in the RTAS/TDM SDK
	AAX_ePlugInStrings_ClipNameSuffix = 11,				///< Clip name label suffix (AudioSuite). This value will be appended to the clip's name, vs \ref AAX_ePlugInStrings_ClipName which will replace the clip's name completely.
	
	AAX_ePlugInStrings_INT32_MAX = AAX_INT32_MAX
}; AAX_ENUM_SIZE_CHECK( AAX_EPlugInStrings );

/** @brief Meter orientation
 *
 *  @details
 *	Use this enum in conjunction with the \ref AAX_eProperty_Meter_Orientation property
 *
 *  For more information about meters in AAX, see \ref AdditionalFeatures_Meters
 */
enum AAX_EMeterOrientation
{
	AAX_eMeterOrientation_Default = 0,
	AAX_eMeterOrientation_BottomLeft = AAX_eMeterOrientation_Default,  		///<  the default orientation
	AAX_eMeterOrientation_TopRight = 1, 		 	///<  Some dynamics plug-in orient their gain reduction like so
	AAX_eMeterOrientation_Center = 2, 			///<  A plug-in that does gain increase and decrease may want this. meter values less than 0x40000000 would display downward from the mid-point. meter values greater than 0x40000000 would display upward from the mid-point
	AAX_eMeterOrientation_PhaseDot = 3			///<  linear scale, displays 2 dots around the value ( currently D-Control only )
}; AAX_ENUM_SIZE_CHECK( AAX_EMeterOrientation );


/** @brief Meter ballistics type
 *
 *  @details
 *	Use this enum in conjunction with the \ref AAX_eProperty_Meter_Ballistics property
 *
 *  For more information about meters in AAX, see \ref AdditionalFeatures_Meters
 */
enum AAX_EMeterBallisticType
{
	AAX_eMeterBallisticType_Host = 0,			///< The ballistics follow the host settings.
	AAX_eMeterBallisticType_NoDecay = 1		///< No decay ballistics.
}; AAX_ENUM_SIZE_CHECK( AAX_EMeterBallisticType );

/** @brief Meter type
 *
 *  @details
 *	Use this enum in conjunction with the \ref AAX_eProperty_Meter_Type property
 *
 *  For more information about meters in AAX, see \ref AdditionalFeatures_Meters
 */
enum AAX_EMeterType
{
	AAX_eMeterType_Input = 0,		 		///<  e.g. Your typical input meter (possibly after an input gain stage)
	AAX_eMeterType_Output = 1,				///<  e.g. Your typical output meter (possibly after an output gain stage)
	AAX_eMeterType_CLGain = 2,				///<  e.g. Compressor/Limiter gain reduction
	AAX_eMeterType_EGGain = 3,				///<  e.g. Expander/Gate gain reduction
	AAX_eMeterType_Analysis = 4,			///<  e.g. multi-band amplitude from a Spectrum analyzer
	AAX_eMeterType_Other = 5,				///<  e.g. a meter that does not fit in any of the above categories
	AAX_eMeterType_None = 31				///<  For internal host use only
}; AAX_ENUM_SIZE_CHECK( AAX_EMeterType );

/*!	@brief Different Curve Types that can be queried from the Host.
 	
 	@details
	\note All 'AX__' IDs are reserved for host messages
	
	\sa \ref AAX_IEffectParameters::GetCurveData()
	\sa \ref AAX_IEffectParameters::GetCurveDataMeterIds()
	\sa \ref AAX_IEffectParameters::GetCurveDataDisplayRange()
 
	@ingroup AdditionalFeatures_CurveDisplays
 */
enum 	AAX_ECurveType
{
	AAX_eCurveType_None = 0,
	
	/** \brief EQ Curve, input values are in Hz, output values are in dB
		
		\compatibility Pro Tools requests this curve type for
		\ref AAX_ePlugInCategory_EQ "EQ" plug-ins only
	 */
	AAX_eCurveType_EQ = 'AXeq',
	/** \brief Dynamics Curve showing input vs. output, input and output values are in dB
		 
		\compatibility Pro Tools requests this curve type for
		\ref AAX_ePlugInCategory_Dynamics "Dynamics" plug-ins only
	 */
	AAX_eCurveType_Dynamics = 'AXdy',
	/**	\brief Gain-reduction curve showing input vs. gain reduction, input and output values are in dB
		
		\compatibility Pro Tools requests this curve type for
		\ref AAX_ePlugInCategory_Dynamics "Dynamics" plug-ins only
	 */
	AAX_eCurveType_Reduction = 'AXdr'
}; AAX_ENUM_SIZE_CHECK( AAX_ECurveType );

/*!	@brief Types of resources that can be added to an Effect's description
 	
	@details
	\sa AAX_IEffectDescriptor::AddResourceInfo()
 */
enum AAX_EResourceType
{
	AAX_eResourceType_None = 0,
	/** The file name of the page table xml file
	 */
	AAX_eResourceType_PageTable,
	/** The absolute path to the directory containing the plug-in's page table xml file(s)
	 
	 Defaults to *.aaxplugin/Contents/Resources
	 */
	AAX_eResourceType_PageTableDir
}; AAX_ENUM_SIZE_CHECK( AAX_EResourceType );

/*!	@brief Events IDs for %AAX notifications
	
	@details
	- Notifications listed with <em>Sent by: Host</em> are dispatched by the
	%AAX host and may be received in one or more of

	\li \ref AAX_IEffectParameters::NotificationReceived()
	\li \ref AAX_IEffectGUI::NotificationReceived()
	\li \ref AAX_IEffectDirectData::NotificationReceived()
	
	The host will choose which components are registered to receive each event type.
	See the documentation for each event type for more information.

	\note All 'AX__' four-char IDs are reserved for the %AAX specification
 */
enum AAX_ENotificationEvent
{
	/** \brief (not currently sent) The zero-indexed insert position
	 of this plug-in instance within its track
	 
	 <em>Data: \c int32_t</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_InsertPositionChanged = 'AXip',
	/** \brief  (const AAX_IString) The current name of
	 this plug-in instance's track
	 
	 \compatibility Supported in Pro Tools 11.2 and higher
	 \compatibility Not supported by Media Composer
	 
	 <em>Data: \c const \ref AAX_IString</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_TrackNameChanged = 'AXtn',
	/** \brief  (not currently sent) The current UID of
	 this plug-in instance's track
	 
	 <em>Data: <tt>const uint8_t[16]</tt></em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_TrackUIDChanged = 'AXtu',
	/** \brief  (not currently sent) The current position index of
	 this plug-in instance's track
	 
	 <em>Data: \c int32_t</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_TrackPositionChanged = 'AXtp',
	/** \brief  Not currently sent
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_AlgorithmMoved = 'AXam',
	/** \brief  Not currently sent
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_GUIOpened = 'AXgo',
	/** \brief  Not currently sent
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_GUIClosed = 'AXgc',
	/** \brief  AudioSuite processing state change notification. One
	 of \ref AAX_EProcessingState.
	 
	 \compatibility Supported in Pro Tools 11 and higher
	 \compatibility Not supported by Media Composer
	 
	 <em>Data: \c int32_t</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_ASProcessingState = 'AXPr',
	/** \brief  AudioSuite preview state change notification. One of
	 \ref AAX_EPreviewState. \legacy Replacement for \c SetPreviewState()
	 
	 \compatibility Supported in Pro Tools 11 and higher
	 \compatibility Not supported by Media Composer
	 
	 <em>Data: \c int32_t</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_ASPreviewState = 'ASPv',
	/** \brief  Tell the plug-in that chunk data is coming from a PTX
		
	 \compatibility Supported in Pro Tools 11 and higher
	 \compatibility Not supported by Media Composer
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_SessionBeingOpened = 'AXso',
	/** \brief  Tell the plug-in that chunk data is coming from a TFX
		
	 \compatibility Supported in Pro Tools 11 and higher
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_PresetOpened = 'AXpo',
	/** \brief  Entering offline processing mode (i.e. offline bounce)
	 
	 \compatibility Supported in Pro Tools 11 and higher
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_EnteringOfflineMode = 'AXof',
	/** \brief  Exiting offline processing mode (i.e. offline bounce)
	 
	 \compatibility Supported in Pro Tools 11 and higher
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_ExitingOfflineMode = 'AXox',
	/** \brief  A string representing the path of the
	 current session
	 
	 \compatibility Supported in Pro Tools 11.1 and higher
	 
	 <em>Data: \c const \ref AAX_IString</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_SessionPathChanged = 'AXsp',
	/** \brief  The host has changed its latency compensation for this
	 plug-in instance.
	 
	 \note This notification may be sent redundantly just after plug-in
	 instantiation when the \ref AAX_eProperty_LatencyContribution property is
	 described.
	 
	 \compatibility Supported in Pro Tools 11.1 and higher
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_SignalLatencyChanged = 'AXsl',
	/** \brief The host's delay compensation state has changed
	 
	 This notification refers to the host's delay compensation feature as a
	 whole, rather than the specific delay compensation state for the plug-in.
	 
	 Possible values: 0 (disabled), 1 (enabled)
	 
	 Plug-ins may need to monitor the host's delay compensation state because,
	 while delay compensation is disabled, the host will never change the
	 plug-in's accounted latency and, therefore, will never dispatch
	 \ref AAX_eNotificationEvent_SignalLatencyChanged to the plug-in following
	 a call to \ref AAX_IController::SetSignalLatency().
	 
	 \compatibility Supported in Pro Tools 12.6 and higher
	 
	 <em>Data: \c int32_t</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_DelayCompensationState = 'AXdc',
	/** \brief  (not currently sent) The host has changed its DSP cycle
	 allocation for this plug-in instance
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_CycleCountChanged = 'AXcc',
	/** \brief  Tell the plug-in the maximum allowed GUI dimensions
	 
	 Delivered to the plugin's \ref AAX_IEffectGUI::NotificationReceived()
	 
	 \compatibility Supported in Pro Tools 11.1 and higher
	 
	 <em>Data: \c const \ref AAX_Point</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_MaxViewSizeChanged = 'AXws',
	/** \brief  Tell the plug-in about connection of the sidechain input
	 
	 \compatibility Supported in Pro Tools 11.1 and higher
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_SideChainBeingConnected = 'AXsc',
	/** \brief  Tell the plug-in about disconnection of the sidechain
	 input
	 
	 \compatibility Supported in Pro Tools 11.1 and higher
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_SideChainBeingDisconnected = 'AXsd',
	/** \brief The plug-in's noise floor level
	 
	 The notification data is the new absolute noise floor level generated by the
	 plug-in, as amplitude. For example, a plug-in generating a noise floor at -80
	 dB (amplitude) would provide 0.0001 in the notification data.
	 
	 Signal below the level of the plug-in's noise floor may be ignored by host
	 features such as Dynamic Plug-In Processing, which detect whether or not
	 there is any signal being generated by the plug-in
	 
	 <em>Data: \c double</em> <br />
	 <em>Sent by: Plug-in</em>
	 */
	AAX_eNotificationEvent_NoiseFloorChanged = 'AXnf',
	/** \brief Notify the host that some aspect of the parameters' mapping has changed
	
	 To respond to this notification, the host will call
	 \ref AAX_IEffectParameters::UpdatePageTable() to update its cached page tables.
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Plug-in</em>
	 */
	AAX_eNotificationEvent_ParameterMappingChanged = 'AXpm',
	/** \brief Notify the host that one or more parameters' display names have changed
	 
	 The payload is the parameter's ID. The payload size must be at least as large as
	 the ID string, including the null termination character, and no larger than the
	 size of the buffer containing  the \ref AAX_CParamID .

	\compatibility Supported in Pro Tools 2023.3 and higher

	 <em>Data: \c const \ref AAX_CParamID</em> <br />
	 <em>Sent by: Plug-in</em>
	 */
	AAX_eNotificationEvent_ParameterNameChanged = 'AXpn',
	/** \brief  Notify the plug-in about Host mode changing

	\compatibility Supported in Venue 5.6 and higher

	<em>Data: AAX_EHostModeBits</em> <br />
	<em>Sent by: Host</em>
	*/
	AAX_eNotificationEvent_HostModeChanged = 'AXHm',
	/** \brief Previously-saved settings may no longer restore the captured state
	 
	 Use this notification when a change occurs which may cause a different state to
	 be restored by saved settings, and in particular by a saved setting representing
	 the plug-in's state just prior to the change.
	 
	 For example, a plug-in which restricts certain types of state changes when the
	 host is in \ref AAX_eHostModeBits_Live mode should post an
	 \ref AAX_eNotificationEvent_PriorSettingsInvalid notification when this part of
	 the plug-in state is changed manually by the user; if plug-in settings captured
	 prior to this manual change are later set on the plug-in while the host is in
	 live mode then some part of the settings change will be blocked and the captured
	 state will not be perfectly restored.
	 
	 \compatibility Supported in Venue 5.6 and higher
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Plug-in</em>
	 */
	AAX_eNotificationEvent_PriorSettingsInvalid = 'AXps',
	/** \brief  Notify plug-in to log current state
	 
	 Plug-in implementation specific
	 
	 \compatibility Pro Tools currently only sends this notification to the Direct
	 Data object in the plug-in
	 
	 <em>Data: none</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_LogState = 'AXls',
	/** \brief  Notify plug-in that the TransportState was changed.
	 
	 \compatibility Supported in Pro Tools 2021.10 and higher

	 <em>Data: \ref AAX_TransportStateInfo_V1</em> <br />
	 <em>Sent by: Host</em>
	 */
	AAX_eNotificationEvent_TransportStateChanged = 'AXts',
	/** \brief  Tell the plug-in the current host language setting

	 Data is sent as a string. The format is a two-part code based on RFC 4646. The values
	 follow Microsoft's formatting for CultureInfo culture names as described in
	 http://msdn.microsoft.com/en-us/library/system.globalization.cultureinfo%28VS.80%29.aspx
	 
	 Examples:
	 - en-US: English (US)
	 - ja-JP: Japanese
	 - ko-KR: Korean
	 - fr-FR: French
	 - it-IT: Italian
	 - de-DE: German
	 - es-ES: Spanish

	 These exceptions to the specification are used by Pro Tools:
	 - zh-CHS: Simplified Chinese
	 - zh-CN: Traditional Chinese

	 \note Currently in Pro Tools the language setting will remain consistent throughout
	 the lifetime of the plugin instance.

	 Delivered to the plugin's \ref AAX_IEffectGUI::NotificationReceived() and \ref AAX_IEffectParameters::NotificationReceived()

	 \compatibility Supported in Pro Tools 2024.3 and higher

	 <em>Data: \c const \ref AAX_IString</em> <br />
	 <em>Sent by: Host</em>

	 */
	AAX_eNotificationEvent_HostLocale = 'AXLc',
}; AAX_ENUM_SIZE_CHECK( AAX_ENotificationEvent );


/**	@brief Host mode
 \compatibility Supported in Venue 5.6 and higher
 */
enum AAX_EHostModeBits
{
	AAX_eHostModeBits_None = 0,			///< No special host mode, e.g. Pro Tools normal operation, Venue Config mode
	AAX_eHostModeBits_Live = (1 << 0)	///< The host is in a live playback mode, e.g. Venue Show mode - inserts are live and must not allow state changes which interrupt audio processing
	
}; AAX_ENUM_SIZE_CHECK(AAX_EHostModeBits);

/**	@brief DEPRECATED
 
 Use \ref AAX_EHostModeBits
 
 \warning The values of these modes have changed as of %AAX SDK 2.3.1 from the definitions originally published in %AAX SDK 2.3.0
 
 \deprecated This enum is deprecated and will be removed in a future release.
*/
enum AAX_EHostMode
{
	AAX_eHostMode_Show = AAX_eHostModeBits_Live, ///< \deprecated Use \ref AAX_eHostModeBits_Live
	AAX_eHostMode_Config = AAX_eHostModeBits_None ///< \deprecated Use \ref AAX_eHostModeBits_None

}; AAX_ENUM_SIZE_CHECK(AAX_EHostMode);

/**	@brief Options for algorithm private data fields
 */
enum AAX_EPrivateDataOptions
{
	AAX_ePrivateDataOptions_DefaultOptions	= 0,
	AAX_ePrivateDataOptions_KeepOnReset		= (1 << 0),			///< Retain data upon plug-in reset \warning Not currently implemented. If this functionality is desired, the recommended workaround is to cache the desired private data to be set during \ref AAX_IEffectParameters::ResetFieldData().
	AAX_ePrivateDataOptions_External		= (1 << 1),			///< Place the block in external memory (internal by default)
	AAX_ePrivateDataOptions_Align8			= (1 << 2),			///< Place the block in mem aligned by 64 bits
	
	AAX_ePrivateDataOptions_INT32_MAX = AAX_INT32_MAX
}; AAX_ENUM_SIZE_CHECK( AAX_EPrivateDataOptions );


/*! \brief Property values to describe location constraints placed on
 	the plug-in's algorithm component (\c ProcessProc)
 	
 	\details
	\sa AAX_eProperty_Constraint_Location
 */
enum AAX_EConstraintLocationMask
{
	/** \brief No constraint placed on component's location
	 */
	AAX_eConstraintLocationMask_None            = 0,
	/** \brief This \c ProcessProc must be co-located with the plug-in's data model object
	 */
	AAX_eConstraintLocationMask_DataModel       = (1 << 0),
	/** \brief This \c ProcessProc should be instantiated on the same chip as other effects that use the same DLL.
	 *
	 *  \li This constraint is only applicable to DSP algorithms
	 * 
	 *  This property should only be used when absolutely required, as it will constrain the DSP manager and reduce
	 *  overall DSP plug-in instance counts on the system.
	 * 
	 *  \compatibility This constraint is supported in Pro Tools 10.2 and higher
	 */
	AAX_eConstraintLocationMask_DLLChipAffinity = (1 << 1),
}; AAX_ENUM_SIZE_CHECK( AAX_EConstraintLocationMask );

/*! \brief Property values to describe the topology of the plug-in's
	modules (e.g. data model, GUI.)
 	
 	\details
	\sa AAX_eProperty_Constraint_Topology
 */
enum AAX_EConstraintTopology
{
	AAX_eConstraintTopology_None			= 0,	///< No constraint placed on plug-in's topology
	AAX_eConstraintTopology_Monolithic		= 1		///< All plug-in modules (e.g. data model, GUI) must be co-located and non-relocatable
}; AAX_ENUM_SIZE_CHECK( AAX_EConstraintTopology );


/*! \brief Selector indicating the action that occurred to prompt a component initialization callback
 	
 	\details
 	\sa \ref AAX_CInstanceInitProc
 */
enum AAX_EComponentInstanceInitAction
{
	AAX_eComponentInstanceInitAction_AddingNewInstance = 0,
	AAX_eComponentInstanceInitAction_RemovingInstance = 1,
	AAX_eComponentInstanceInitAction_ResetInstance = 2
}; AAX_ENUM_SIZE_CHECK( AAX_EComponentInstanceInitAction );

/*! \brief Property values to describe various sample rates.
	
	\details 
	These values may be used as a bitmask, so e.g. a particular Effect
	may declare compatibility with
	<TT>AAX_eSampleRateMask_44100 | AAX_eSampleRateMask_48000</TT>

	\sa \ref AAX_eProperty_SampleRate
 */
enum AAX_ESampleRateMask
{
	AAX_eSampleRateMask_No = 0,
	
	AAX_eSampleRateMask_44100 = (1 << 0),
	AAX_eSampleRateMask_48000 = (1 << 1),
	AAX_eSampleRateMask_88200 = (1 << 2),
	AAX_eSampleRateMask_96000 = (1 << 3),
	AAX_eSampleRateMask_176400 = (1 << 4),
	AAX_eSampleRateMask_192000 = (1 << 5),
	
	AAX_eSampleRateMask_All = AAX_INT32_MAX
}; AAX_ENUM_SIZE_CHECK( AAX_ESampleRateMask );

/**	@brief FIC stuff that I can't include without DAE library dependence
	
	@details
	\legacy Values must match unnamed type enum in FicTDMControl.h

	\todo FLAGGED FOR REMOVAL
 
 */
typedef enum AAX_EParameterType
{
	AAX_eParameterType_Discrete,         ///< \legacy Matches \c kDAE_DiscreteValues
	AAX_eParameterType_Continuous        ///< \legacy Matches \c kDAE_ContinuousValues
} AAX_EParameterType;  AAX_ENUM_SIZE_CHECK( AAX_EParameterType );

/**	@brief Visual Orientation of a parameter
	
	@details
	\todo FLAGGED FOR REVISION
	 
 */
enum AAX_EParameterOrientationBits {
	AAX_eParameterOrientation_Default = 0,
	
	AAX_eParameterOrientation_BottomMinTopMax = 0,			// Choose this...
	AAX_eParameterOrientation_TopMinBottomMax = 1,			// or this.
	
	AAX_eParameterOrientation_LeftMinRightMax = 0,			// AND this...
	AAX_eParameterOrientation_RightMinLeftMax = 2,			// or this.
	
	// Rotary multi-Segment Display Choices
	AAX_eParameterOrientation_RotarySingleDotMode = 0,		// AND this...
	AAX_eParameterOrientation_RotaryBoostCutMode = 4,		// or this.
	AAX_eParameterOrientation_RotaryWrapMode = 8,			// or this.
	AAX_eParameterOrientation_RotarySpreadMode = 12,			// or this.
	
	// Rotary multi-Segment Display Polarity
	AAX_eParameterOrientation_RotaryLeftMinRightMax = 0,		// AND this...
	AAX_eParameterOrientation_RotaryRightMinLeftMax = 16		// or this.
}; AAX_ENUM_SIZE_CHECK( AAX_EParameterOrientationBits );

/** @brief Typedef for a bitfield of \ref AAX_EParameterOrientationBits values
 */
typedef int32_t		AAX_EParameterOrientation;

/**	@brief Query type selectors for use with \ref AAX_IEffectParameters::GetParameterValueInfo()
 *
 *	@details
 *  \sa \ref AAX_EEQBandTypes
 *  \sa \ref AAX_EEQInCircuitPolarity
 *  \sa \ref AAX_EUseAlternateControl
 *
 *	\legacy converted from \c EControlValueInfo in the legacy SDK
 */
enum AAX_EParameterValueInfoSelector
{
	/** \brief EQ filter band type
	 *	
	 *	\details
	 *  Possible response values are listed in \ref AAX_EEQBandTypes
	 *
	 *	\legacy converted from \c eDigi_PageTable_EQ_Band_Type in the legacy SDK
	 *
	 */
	AAX_ePageTable_EQ_Band_Type = 0,
	/** \brief Description of whether a particular EQ band is active
	 *
	 *  \details
	 *  Possible response values are listed in \ref AAX_EEQInCircuitPolarity
	 *
	 *	\legacy converted from \c eDigi_PageTable_EQ_InCircuitPolarity in the legacy SDK
	 *
	 */
	AAX_ePageTable_EQ_InCircuitPolarity = 1,
	/** \brief Description of whether an alternate parameter should be used for a
	 *  given slot
	 *
	 *  \details
	 *  For example, some control surfaces support Q/Slope encoders.  Using an
	 *  alternate control mechanism, plug-ins mapped to these devices can
	 *  assign a different slope control to the alternate slot and have
	 *  it coexist with a Q control for each band.  This is only applicable
	 *  when mapping separate parameters to the same encoder; if the Q and
	 *  Slope controls are implemented as the same parameter object in the
	 *  plug-in then customization is not needed.
	 *
	 *  Possible response values are listed in \ref AAX_EUseAlternateControl
	 *
	 *	\legacy converted from \c eDigi_PageTable_UseAlternateControl in the legacy SDK
	 *
	 */
	AAX_ePageTable_UseAlternateControl = 2
}; AAX_ENUM_SIZE_CHECK( AAX_EParameterValueInfoSelector );

/** @brief Definitions of band types for EQ page table
 *
 *  @details
 *  For the \ref AAX_ePageTable_EQ_Band_Type parameter value info selector
 */
enum AAX_EEQBandTypes
{
	AAX_eEQBandType_HighPass = 0,			/*!< Freq,       Slope    */
	AAX_eEQBandType_LowShelf = 1,			/*!< Freq, Gain, Slope    */
	AAX_eEQBandType_Parametric = 2,			/*!< Freq, Gain,       Q  */
	AAX_eEQBandType_HighShelf = 3,			/*!< Freq, Gain, Slope    */
	AAX_eEQBandType_LowPass = 4,			/*!< Freq,       Slope    */
	AAX_eEQBandType_Notch = 5				/*!< Freq,             Q  */
}; AAX_ENUM_SIZE_CHECK( AAX_EEQBandTypes );

/** @brief Definitions for band in/out for EQ page table.
 *
 *	@details
 *  For the AAX_ePageTable_EQ_InCircuitPolarity parameter value selector
 */
enum AAX_EEQInCircuitPolarity
{
	AAX_eEQInCircuitPolarity_Enabled = 0,		/*!< EQ band is in the signal path and enabled	*/	
	AAX_eEQInCircuitPolarity_Bypassed = 1,      /*!< EQ band is in the signal path but bypassed/off */
	AAX_eEQInCircuitPolarity_Disabled = 2		/*!< EQ band is completely removed from signal path */
}; AAX_ENUM_SIZE_CHECK( AAX_EEQInCircuitPolarity );

/** @brief Definitions for Use Alternate Control parameter
 *
 *  @details
 *	For the AAX_ePageTable_UseAlternateControl parameter value info selector
 */
enum AAX_EUseAlternateControl
{
	AAX_eUseAlternateControl_No = 0,
	AAX_eUseAlternateControl_Yes = 1
}; AAX_ENUM_SIZE_CHECK( AAX_EUseAlternateControl );

/*! \brief MIDI node types
 	
 	\details
	\sa \ref AAX_IComponentDescriptor::AddMIDINode()
 */
enum AAX_EMIDINodeType
{
	/** \brief Local MIDI input
	 *
	 *	\details
	 *	Local MIDI input nodes receive MIDI by accessing \ref AAX_CMidiStream buffers filled with MIDI
	 *	messages. These buffers of MIDI data are available within the algorithm context with data
	 *	corresponding to the current audio buffer being computed. The Effect can step through this
	 *	buffer like a "script" to respond to MIDI events within the audio callback.
	 *
	 *	\legacy Corresponds to RTAS Buffered MIDI input nodes in the legacy SDK
	 */
	AAX_eMIDINodeType_LocalInput = 0,
	/**	\brief Local MIDI output
	 *
	 *	\details
	 *	Local MIDI output nodes send MIDI by filling buffers with MIDI messages.  Messages posted to
	 *	MIDI output nodes will be available in the host as MIDI streams, routable to MIDI track inputs
	 *	and elsewhere.
	 *
	 *	Data posted to a MIDI output buffer will be timed to correspond with the current audio buffer
	 *	being processed.  MIDI outputs support custom timestamping relative to the first sample of the
	 *	audio buffer.
	 *
	 *	The delivery of variable length SysEx messages is also supported. There are no buffer size
	 *	limitations for output of SysEx messages.
	 *
	 *	To post a MIDI output buffer, an Effect must construct a series of \ref AAX_CMidiPacket objects
	 *	and place them in the output buffer provided in the port's \ref AAX_CMidiStream
	 *
	 *	\legacy Corresponds to RTAS Buffered MIDI output nodes in the legacy SDK
	 */
	AAX_eMIDINodeType_LocalOutput = 1,
	/** \brief Global MIDI node
	 *
	 *	\details
	 *	Global MIDI nodes allow an Effect to receive streaming global MIDI data like MIDI Time Code,
	 *	MIDI Beat Clock, and host-specific message formats such as the Click messages used in Pro Tools.
	 *
	 *  The specific kind of data that will be received by a Global MIDI node is specified using a mask
	 *  of \ref AAX_EMidiGlobalNodeSelectors values.
	 *
	 *	Global MIDI nodes are like local MIDI nodes, except they do not show up as assignable outputs in
	 *	the host. Instead the MIDI data is automatically routed to the plug-in, without the user making
	 *	any connections.
	 *
	 *	The buffer of data provided via a Global MIDI node may be shared between all currently active
	 *	Effect instances, and this node may include both explicitly requested data and data not requested
	 *	by the current Effect. For example, if one plug-in requests MTC and another plug-in requests
	 *	Click, all plug-ins connected to this global node will get both MTC and Click messages in the
	 *	shared buffer.
	 *
	 *	\legacy Corresponds to RTAS Shared Buffer global nodes in the legacy SDK
	 */
	AAX_eMIDINodeType_Global = 2,
	/** \brief Transport node
	 *
	 *	\details
	 *	Call \ref AAX_IMIDINode::GetTransport() on this node to access the \ref AAX_ITransport interface. 
	 *	
	 *	\warning See warning at \ref AAX_IMIDINode::GetTransport() regarding use of this interface
	 */
	AAX_eMIDINodeType_Transport = 3
}; AAX_ENUM_SIZE_CHECK( AAX_EMIDINodeType );


/** @brief Source for values passed into
	\ref AAX_IACFEffectParameters::UpdateParameterNormalizedValue() "UpdateParameterNormalizedValue()".
 */
enum AAX_EUpdateSource
{
	AAX_eUpdateSource_Unspecified = 0,			///< Parameter updates of unknown / unspecified origin, currently including all updates from control surfaces, GUI edit events, and edits originating in the plug-in outside of the context of \ref AAX_IACFEffectParameters::UpdateParameterNormalizedValue() "UpdateParameterNormalizedValue()" or \ref AAX_IACFEffectParameters::SetChunk() "SetChunk()"
	AAX_eUpdateSource_Parameter = 1,			///< Parameter updates originating (via \ref AAX_IAutomationDelegate::PostSetValueRequest() ) within the scope of \ref AAX_IACFEffectParameters::UpdateParameterNormalizedValue() "UpdateParameterNormalizedValue()"
	AAX_eUpdateSource_Chunk = 2,				///< Parameter updates originating (via \ref AAX_IAutomationDelegate::PostSetValueRequest() ) within the scope of \ref AAX_IACFEffectParameters::SetChunk() "SetChunk()"
	AAX_eUpdateSource_Delay = 3					///< @notusedbyaax
}; AAX_ENUM_SIZE_CHECK( AAX_EUpdateSource );

/*! \brief Property value for whether a data in port should be
 buffered or not.
 
 \details
 \sa AAX_IComponentDescriptor::AddDataInPort()
 */
enum AAX_EDataInPortType
{
	/** Data port is unbuffered; the most recently posted packet is always delivered to the alg proc
	 */
	AAX_eDataInPortType_Unbuffered	= 0,
	/** Data port is buffered both on the host and DSP and packets are updated to the current timestamp
	 *	with every alg proc call
	 *
	 *	Data delivered to alg proc always reflects the latest posted packet that has a timestamp at or
	 *	before the current processing buffer
	 */
	AAX_eDataInPortType_Buffered	= 1,
	/** Data port is buffered both on the host and DSP and packets are updated only once per alg proc call
	 *
	 *	Since only one packet is delivered at a time, all packets will be delivered to the alg proc unless
	 *	an internal buffer overflow occurs
	 *
	 *	@note If multiple packets are posted to this port \em before the initial call to the alg proc, only
	 *	the latest packet will be delivered to the first call to the alg proc. Thereafter, all packets will
	 *	be delivered incrementally.
	 *
	 *	@compatibility Supported in Pro Tools 12.5 and higher; when \ref AAX_eDataInPortType_Incremental is
	 *	not supported the port will be treated as \ref AAX_eDataInPortType_Unbuffered
	 */
	AAX_eDataInPortType_Incremental = 2
}; AAX_ENUM_SIZE_CHECK( AAX_EDataInPortType );

/*! \brief FrameRate types
	
	\details 
	\sa AAX_ITransport::GetTimeCodeInfo()
 	\sa AAX_ITransport::GetHDTimeCodeInfo()
 */
enum AAX_EFrameRate
{
	AAX_eFrameRate_Undeclared = 0,
	AAX_eFrameRate_24Frame = 1,
	AAX_eFrameRate_25Frame = 2,
	AAX_eFrameRate_2997NonDrop = 3,
	AAX_eFrameRate_2997DropFrame = 4,
	AAX_eFrameRate_30NonDrop = 5,
	AAX_eFrameRate_30DropFrame = 6,
	AAX_eFrameRate_23976 = 7,
	AAX_eFrameRate_47952 = 8,
	AAX_eFrameRate_48Frame = 9,
	AAX_eFrameRate_50Frame = 10,
	AAX_eFrameRate_5994NonDrop = 11,
	AAX_eFrameRate_5994DropFrame = 12,
	AAX_eFrameRate_60NonDrop = 13,
	AAX_eFrameRate_60DropFrame = 14,
	AAX_eFrameRate_100Frame = 15,
	AAX_eFrameRate_11988NonDrop = 16,
	AAX_eFrameRate_11988DropFrame = 17,
	AAX_eFrameRate_120NonDrop = 18,
	AAX_eFrameRate_120DropFrame = 19
}; AAX_ENUM_SIZE_CHECK( AAX_EFrameRate );

/*! \brief FeetFramesRate types
	
	\details 
	\sa AAX_ITransport::GetFeetFramesInfo()
 */
enum AAX_EFeetFramesRate
{
	AAX_eFeetFramesRate_23976 = 0,
	AAX_eFeetFramesRate_24 = 1,
	AAX_eFeetFramesRate_25 = 2
}; AAX_ENUM_SIZE_CHECK( AAX_EFeetFramesRate );


/*!	\brief The Global MIDI Node Selectors 
 * 	
 *	\details
 *	These selectors are used in the \a channelMask argument of 
 *	\ref AAX_IComponentDescriptor::AddMIDINode() and \ref AAX_IEffectDescriptor::AddControlMIDINode()
 *	to request one or more kinds of global data. 
 */ 
enum AAX_EMidiGlobalNodeSelectors
{
	/*!
	 *	\brief Selector to request click messages
	 *
	 *	\details
	 *	The click messages are special 2-byte messages encoded as follows: 
	 *  \li Accented click: Note on pitch 0 (<tt>0x90 0x00</tt>)
	 *  \li Unaccented click: Note on pitch 1 (<tt>0x90 0x01</tt>)
	 *  \note No <em> Note Off </em> messages are ever sent. This isn't up-to-spec MIDI data, just a way of encoding click events.
	 */	
	AAX_eMIDIClick        = 1 << 0, 
	/*!
	 *	\brief Selector to request MIDI Time Code (MTC) data. 
	 *	
	 *	\details
	 *	The Standard MIDI Time Code format.
	 */
	AAX_eMIDIMtc          = 1 << 1, 
	/*!
	 *	\brief Selector to request MIDI Beat Clock (MBC) messages. 
	 *	
	 *	\details
	 *	This includes Song Position Pointer, Start/Stop/Continue, and Midi Clock (F8).
	 */
	AAX_eMIDIBeatClock    = 1 << 2  

}; AAX_ENUM_SIZE_CHECK( AAX_EMidiGlobalNodeSelectors );

/** @brief Offline preview states for use with \ref AAX_eNotificationEvent_ASPreviewState
 	
 	@details
	\note Do not perform any non-trivial processing within the notification handler. Instead,
	use the processing state notification to inform the processing that is performed in
	methods such as \ref AAX_CHostProcessor::PreRender() "PreRender()".
 */
enum AAX_EPreviewState
{
	/**	\brief Offline preview has ended
		
		\details
		For \ref AuxInterface_HostProcessor "Host Processor" plug-ins, this notification is
		sent just before the final call to \ref AAX_IHostProcessor::PostRender() "PostRender()",
		or after analysis is complete for plug-ins with analysis-only preview.
	 */
	AAX_ePreviewState_Stop = 0,
	/**	\brief Offline preview is beginning
		
		\details
		For \ref AuxInterface_HostProcessor "Host Processor" plug-ins, this notification is
		sent before any calls to \ref AAX_IHostProcessor::PreAnalyze() "PreAnalyze()" or to
		\ref AAX_IHostProcessor::PreRender() "PreRender()".
	 */
	AAX_ePreviewState_Start = 1
}; AAX_ENUM_SIZE_CHECK( AAX_EPreviewState );

/** @brief Offline preview states for use with \ref AAX_eNotificationEvent_ASProcessingState
	
	@details
	\note Do not perform any non-trivial processing within the notification handler. Instead,
	use the processing state notification to inform the processing that is performed in
	methods such as \ref AAX_CHostProcessor::PreRender() "PreRender()".
 */
enum AAX_EProcessingState
{
	/**	\brief A single offline processing pass has ended
	 
	 \details
	 A single offline processing pass is an analysis and/or render applied to a set of
	 channels in parallel.
	 
	 For \ref AuxInterface_HostProcessor "Host Processor" plug-ins, this notification is
	 sent just before the final call to \ref AAX_IHostProcessor::PostRender() "PostRender()",
	 or after analysis is complete for analysis-only offline plug-ins.
	 */
	AAX_eProcessingState_StopPass = 2,
	/**	\brief A single offline processing pass is beginning
	 
	 \details
	 A single offline processing pass is an analysis and/or render applied to a set of
	 channels in parallel.
	 
	 For \ref AuxInterface_HostProcessor "Host Processor" plug-ins, this notification is
	 sent before any calls to \ref AAX_IHostProcessor::PreAnalyze() "PreAnalyze()",
	 \ref AAX_IHostProcessor::PreRender() "PreRender()", or
	 \ref AAX_IHostProcessor::InitOutputBounds() "InitOutputBounds()" for each processing
	 pass.
	 */
	AAX_eProcessingState_StartPass = 3,
	/**	\brief An offline processing pass group has completed
	 
	 \details
	 An offline processing pass group is a full set of analysis and/or render passes
	 applied to the complete set of input channels.
	 
	 \compatibility AudioSuite pass group notifications are supported starting in
	 Pro Tools 12.0
	 */
	AAX_eProcessingState_EndPassGroup = 4,
	/**	\brief An offline processing pass group is beginning
	 
	 \details
	 An offline processing pass group is a full set of analysis and/or render passes
	 applied to the complete set of input channels.
	 
	 \compatibility AudioSuite pass group notifications are supported starting in
	 Pro Tools 12.0
	 */
	AAX_eProcessingState_BeginPassGroup = 5,
	
	AAX_eProcessingState_Stop = AAX_eProcessingState_StopPass, ///< \deprecated
	AAX_eProcessingState_Start = AAX_eProcessingState_StartPass ///< \deprecated
}; AAX_ENUM_SIZE_CHECK( AAX_EProcessingState );

///	@brief Describes what platform the component runs on.
enum AAX_ETargetPlatform
{
	kAAX_eTargetPlatform_None = 0,
	kAAX_eTargetPlatform_Native = 1,		// For host-based components
	kAAX_eTargetPlatform_TI = 2,			// For TI components
	kAAX_eTargetPlatform_External = 3,		// For components running on external hardware
	kAAX_eTargetPlatform_Count = 5
}; AAX_ENUM_SIZE_CHECK( AAX_ETargetPlatform );

/** Feature support indicators
 
 \sa \ref AAX_IDescriptionHost::AcquireFeatureProperties()
 
 \note: There is no value defined for unknown features. Intead, unknown features are
 indicated by
 \ref AAX_IDescriptionHost::AcquireFeatureProperties() "AcquireFeatureProperties()"
 providing a null \ref AAX_IFeatureInfo in response to a request using the unknown
 feature UID
 */
enum AAX_ESupportLevel
{
	/** An uninitialized \ref AAX_ESupportLevel
	 */
	AAX_eSupportLevel_Uninitialized = 0,
	
	/** The feature is known but explicitly not supported
	 */
	AAX_eSupportLevel_Unsupported = 1
	
	/** The feature is at least partially supported
	 */
	,AAX_eSupportLevel_Supported = 2
	
	/** The feature is supported but disabled due to current settings
	 
	 For example, the feature may be supported by the software but not allowed due to
	 a lack of user entitlements.
	 
	 A host is not required to provide information about disabled features. The host
	 may simply provide \ref AAX_eSupportLevel_Supported even for features which are
	 disabled.
	 */
	,AAX_eSupportLevel_Disabled = 3
	
	/** This feature's support level depends on values in the property map
	 
	 For example, to get information about stem format support the property map must
	 be queried for individual stem formats
	 */
	,AAX_eSupportLevel_ByProperty = 4
}; AAX_ENUM_SIZE_CHECK( AAX_ESupportLevel );

/** @brief Host levels
 
 Some %AAX software hosts support different levels which are sold as separate products.
 For example, there may be an entry-level version of a product as well as a full
 version.
 
 The level of a host may impact the user experience, workflows, or the availability of
 certain plug-ins. For example, some entry-level hosts are restricted to loading only
 specific plug-ins.
 
 Typically an %AAX plug-in should not need to query this information or change its
 behavior based on the level of the host.
 
 @sa \ref AAXATTR_Client_Level
 */
enum AAX_EHostLevel
{
	 AAX_eHostLevel_Unknown = 0
	,AAX_eHostLevel_Standard = 1 ///< Standard host level
	,AAX_eHostLevel_Entry = 2 ///< Entry-level host
	,AAX_eHostLevel_Intermediate = 3 ///< Intermediate-level host
}; AAX_ENUM_SIZE_CHECK( AAX_EHostLevel );

///	@brief Describes possible string encodings.
enum AAX_ETextEncoding
{
	 AAX_eTextEncoding_Undefined = -1
	,AAX_eTextEncoding_UTF8 = 0 ///< UTF-8 string encoding
	
	,AAX_eTextEncoding_Num
}; AAX_ENUM_SIZE_CHECK( AAX_ETextEncoding );

/** @brief Flags for use with \ref AAX_IHostServices::HandleAssertFailure()
 */
enum AAX_EAssertFlags
{
	AAX_eAssertFlags_Default = 0, ///< No special handler requested
	AAX_eAssertFlags_Log = 1 << 0, ///< Logging requested
	AAX_eAssertFlags_Dialog = 1 << 1, ///< User-visible modal alert dialog requested
}; AAX_ENUM_SIZE_CHECK( AAX_EAssertFlags );

// ENUM: AAX_ETransportState
/**	\brief Used to indicate the current transport state of the host.
	This is the global transport state; it does not indicate a track-specific state
 */
enum AAX_ETransportState
{
	AAX_eTransportState_Unknown = 0,
	AAX_eTransportState_Stopping = 1,
	AAX_eTransportState_Stop = 2,
	AAX_eTransportState_Paused = 3,
	AAX_eTransportState_Play = 4,
	AAX_eTransportState_FastForward = 5,
	AAX_eTransportState_Rewind = 6,
	AAX_eTransportState_Scrub = 11,
	AAX_eTransportState_Shuttle = 12,

	AAX_eTransportState_Num
}; AAX_ENUM_SIZE_CHECK(AAX_ETransportState);

// ENUM: AAX_ERecordMode
/**	\brief Used to indicate the current record mode of the host.
	This is the global record mode; it does not indicate a track-specific state
 */
enum AAX_ERecordMode
{
	AAX_eRecordMode_Unknown = 0,
	AAX_eRecordMode_None = 1,
	AAX_eRecordMode_Normal = 2,
	AAX_eRecordMode_Destructive = 3,
	AAX_eRecordMode_QuickPunch = 4,
	AAX_eRecordMode_TrackPunch = 5,

	AAX_eRecordMode_Num
}; AAX_ENUM_SIZE_CHECK(AAX_ERecordMode);

/// @cond ignore
#endif // include guard
/// @endcond
