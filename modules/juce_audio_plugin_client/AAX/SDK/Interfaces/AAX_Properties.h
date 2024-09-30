/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_Properties.h
 *	
 *	\brief Contains IDs for properties that can be added to an AAX_IPropertyMap
 *
 */ 
/*================================================================================================*/


/// @cond ignore
#pragma once
#ifndef AAX_PROPERTIES_H
#define AAX_PROPERTIES_H
/// @endcond

#ifndef _AAX_H_
#include "AAX.h"
#endif


// Add new values only at the end of existing sections!

/** \brief The list of properties that can be added to an \ref AAX_IPropertyMap
 *
 *	\details
 *  See \ref AAX_IPropertyMap::AddProperty() for more information
 *
 *	<H2>Sections </H2>
 *	- \ref AAX_eProperty_PlugInSpecPropsBase "Plug-In spec properties"
 *	- \ref AAX_eProperty_ProcessProcPropsBase "ProcessProc properties"
 *	- \ref AAX_eProperty_GeneralPropsBase "General properties"
 *	- \ref AAX_eProperty_TI_SharedCycleCount "TI-specific properties"
 *	- \ref AAX_eProperty_AudiosuitePropsBase "Offline (AudioSuite) properties"
 *	- \ref AAX_eProperty_GUIBase "GUI properties"
 *	- \ref AAX_eProperty_MeterBase "Meter properties"
 *	- \ref AAX_eProperty_ConstraintBase "Plug-in management constraints"
 *	
 *
 *  \legacy These property IDs are somewhat analogous to the pluginGestalt
 *  system in the legacy SDK, and several \ref AAX_EProperty values correlate
 *  directly with a corresponding legacy plug-in gestalt.
 *  
 *  \legacy To ensure session
 *  interchange compatibility, make sure the 4 character IDs for
 *  \ref AAX_eProperty_ManufacturerID, \ref AAX_eProperty_ProductID,
 *  \ref AAX_eProperty_PlugInID_Native, and
 *  \ref AAX_eProperty_PlugInID_AudioSuite are identical to the legacy SDK's
 *  counterpart.
 *
 */ 
// Current CCS doesn't support C++11
#ifdef _TMS320C6X
enum AAX_EProperty
#else
enum AAX_EProperty : int32_t
#endif
{
	AAX_eProperty_NoID = 0,
	AAX_eProperty_MinProp = 10, // MUST BE EQUAL TO MINIMUM PROPERTY VALUE
	
//---------------------------------------------------------------------	
#if 0
#pragma mark Plug-In spec properties
#endif
	/** @name Plug-In spec properties
	 *
	 */
	//@{
	/** <HR> */
	AAX_eProperty_PlugInSpecPropsBase = 10,
	/** \brief Four-character osid-style manufacturer identifier
	 *	
	 *	Should be registered with Avid, and must be identical for all plug-ins from the same
	 *	manufacturer.
	 *
	 *  \li Apply this property at the \b ProcessProc level for plug-ins that support audio
	 *	processing using a \b ProcessProc callback, or at the \b Effect level for all other
	 *	plug-ins.
     *
     *  \legacy For legacy plug-in session compatibility, this ID should match the Manufacturer ID
     *	used in the corresponding legacy plug-ins.
     */
    AAX_eProperty_ManufacturerID = 11,
	/** \brief Four-character osid-style Effect identifier
	 *	
	 *	Must be identical for all \b ProcessProcs within a single
	 *	\ref AAX_IEffectDescriptor "Effect".
	 *
	 *  \li Apply this property at the \b ProcessProc level for plug-ins that support audio
	 *	processing using a \b ProcessProc callback, or at the \b Effect level for all other
	 *	plug-ins.
     *
     *  \legacy For legacy plug-in session compatibility, this ID should match the Product ID used
     *	in the corresponding legacy plug-in.
     */
    AAX_eProperty_ProductID = 12,
	/** \brief Four-character osid-style plug-in type identifier for real-time native audio Effects
	 *	
	 *	All registered plug-in type IDs (\ref AAX_eProperty_PlugInID_Native,
	 *	\ref AAX_eProperty_PlugInID_AudioSuite, \ref AAX_eProperty_PlugInID_TI, etc.) must be
	 *	unique across all ProcessProcs registered within a single
	 *	\ref AAX_IEffectDescriptor "Effect".
     *  
	 *  \warning As with all plug-in ID properties, this value must remain constant across all
	 *  releases of the plug-in which support this Effect configuration. The value of this
	 *  property should be stored in a constant rather than being calculated at run-time in order
	 *  to avoid unresolvable compatibility issues with saved sessions which can occur of an ID
	 *  value is accidentally changed between two plug-in version releases.
	 *
     *  \li Apply this property at the \b ProcessProc level
     *
     *  \legacy For legacy plug-in session compatibility, this ID should match the Type ID used in
     *	the corresponding legacy RTAS plug-in Types.
     */
    AAX_eProperty_PlugInID_Native = 13,
	/** \brief \deprecated Use \ref AAX_eProperty_PlugInID_Native
     */
    AAX_eProperty_PlugInID_RTAS = AAX_eProperty_PlugInID_Native,
	/** \brief Four-character osid-style plug-in type identifier for offline native audio Effects
	 *
	 *	All registered plug-in type IDs (\ref AAX_eProperty_PlugInID_Native,
	 *	\ref AAX_eProperty_PlugInID_AudioSuite, \ref AAX_eProperty_PlugInID_TI, etc.) must be
	 *	unique across all ProcessProcs registered within a single
	 *	\ref AAX_IEffectDescriptor "Effect".
	 *
	 *  \li Apply this property at the \b ProcessProc level for plug-ins that support audio
	 *	processing using a \b ProcessProc callback, or at the \b Effect level for all other
	 *	AudioSuite plug-ins (e.g. those that use the \ref AAX_IHostProcessor interface.)
     *
     *  \legacy For legacy plug-in session compatibility, this ID should match the Type ID used in
     *	the corresponding legacy AudioSuite plug-in Types.
     */
    AAX_eProperty_PlugInID_AudioSuite = 14,
	/** \brief Four-character osid-style plug-in type identifier for real-time TI-accelerated audio
	 *	Effect types
	 *
	 *	All registered plug-in type IDs (\ref AAX_eProperty_PlugInID_Native,
	 *	\ref AAX_eProperty_PlugInID_AudioSuite, \ref AAX_eProperty_PlugInID_TI, etc.) must be
	 *	unique across all ProcessProcs registered within a single
	 *	\ref AAX_IEffectDescriptor "Effect".
     *
	 *  \warning As with all plug-in ID properties, this value must remain constant across all
	 *  releases of the plug-in which support this Effect configuration. The value of this
	 *  property should be stored in a constant rather than being calculated at run-time in order
	 *  to avoid unresolvable compatibility issues with saved sessions which can occur of an ID
	 *  value is accidentally changed between two plug-in version releases.
     *
     *  \li Apply this property at the \b ProcessProc level
     *
     *  \legacy For legacy plug-in session compatibility, this ID should match the Type ID
     *	used in the corresponding legacy TDM plug-in Types.
     */
    AAX_eProperty_PlugInID_TI = 15,
	/** \brief Four-character osid-style plug-in type identifier for Effect types that do not
	 *	process audio
	 *
	 *	All registered plug-in type IDs (\ref AAX_eProperty_PlugInID_Native,
	 *	\ref AAX_eProperty_PlugInID_AudioSuite, \ref AAX_eProperty_PlugInID_TI, etc.) must be
	 *	unique across all ProcessProcs registered within a single
	 *	\ref AAX_IEffectDescriptor "Effect".
	 *
	 *  \warning As with all plug-in ID properties, this value must remain constant across all
	 *  releases of the plug-in which support this Effect configuration. The value of this
	 *  property should be stored in a constant rather than being calculated at run-time in order
	 *  to avoid unresolvable compatibility issues with saved sessions which can occur of an ID
	 *  value is accidentally changed between two plug-in version releases.
	 *
	 *  \li Apply this property at the \b Effect level
	 */
	AAX_eProperty_PlugInID_NoProcessing = 16,
	/** \brief Four-character osid-style plug-in type identifier for a corresponding deprecated type
	 *
	 *	Only one deprecated effect ID may correspond to each valid (non-deprecated) effect ID.
     *  To associate a plug-in type with more than one deprecated type, use the following properties instead:
     *  - @ref AAX_eProperty_Deprecated_DSP_Plugin_List
     *  - @ref AAX_eProperty_Deprecated_Native_Plugin_List
     *
     *  \li Apply this property at the \b ProcessProc level
     */
    AAX_eProperty_PlugInID_Deprecated = 18,
    /** \brief \deprecated Use \ref AAX_eProperty_Deprecated_Native_Plugin_List and \ref AAX_eProperty_Deprecated_DSP_Plugin_List
     *  See AAX_eProperty_PlugInID_RTAS for an example.
     */
    AAX_eProperty_Deprecated_Plugin_List = 21,
	/** \brief Specify a list of DSP plug-ins that are related to a plug-in type.  
     *
	 *  \li For example, use this property inside a Native process to tell the host that this plug-in can be used
	 *      in place of a DSP version.
     *  \li This property must be applied at the ProcessProc level and used with the
	 *      \ref AAX_IPropertyMap::AddPropertyWithIDArray method, which takes a list of full plug-in identifier
     *      specification triads (ManufacturerID, ProductID, PluginID)
     */
    AAX_eProperty_Related_DSP_Plugin_List = 22,
	/** \brief Specify a list of Native plug-ins that are related to a plug-in type.  
     *
	 *  \li This property must be applied at the ProcessProc level and used with the 
	 *      \ref AAX_IPropertyMap::AddPropertyWithIDArray method, which takes a list of full plug-in identifier
	 *      specification triads (ManufacturerID, ProductID, PluginID)
     */
    AAX_eProperty_Related_Native_Plugin_List = 23,
    /** \brief Specify a list of DSP plug-ins that are deprecated by a new plug-in type.
     *
     *  \li This property must be applied at the ProcessProc level and used with the AddPropertyWithIDArray, which
     *      is a list of full plug-in specs (ManufacturerID, ProductID, PluginID)
     */
    AAX_eProperty_Deprecated_DSP_Plugin_List = 24,
    /** \brief Specify a list of Native plug-ins that are deprecated by a new plug-in type.
     *
     *  \li This property must be applied at the ProcessProc level and used with the AddPropertyWithIDArray, which
     *      is a list of full plug-in specs (ManufacturerID, ProductID, PluginID)
     */
    AAX_eProperty_Deprecated_Native_Plugin_List = AAX_eProperty_Deprecated_Plugin_List,
	/** \brief Four-character osid-style plug-in type identifier for audio effects rendered on
	 *	external hardware.
	 *
	 *	\note This property is not currently used by any %AAX plug-in host software
	 *
	 *	All registered plug-in type IDs must be unique across all ProcessProcs registered within a
	 *	single \ref AAX_IEffectDescriptor "Effect".
     *
	 *  \warning As with all plug-in ID properties, this value must remain constant across all
	 *  releases of the plug-in which support this Effect configuration. The value of this
	 *  property should be stored in a constant rather than being calculated at run-time in order
	 *  to avoid unresolvable compatibility issues with saved sessions which can occur of an ID
	 *  value is accidentally changed between two plug-in version releases.
     *
     *  \li Apply this property at the \b ProcessProc level
     */
	AAX_eProperty_PlugInID_ExternalProcessor = 25,
	/** \brief Identifier for the type of the external processor hardware
	 *
	 *	\sa \ref AAX_eProperty_PlugInID_ExternalProcessor
	 *
	 *	The value of this property will be specific to the external processor hardware. Currently
	 *	there are no public external processor hardware type IDs.
     *
     *  \li Apply this property at the \b ProcessProc level
	 */
	AAX_eProperty_ExternalProcessorTypeID = 26,
	//@}end Plug-In spec properties
	
//---------------------------------------------------------------------	
#if 0
#pragma mark ProcessProc properties
#endif
	/** @name ProcessProc properties
	 *
	 */
	//@{
	/** <HR> */
	AAX_eProperty_ProcessProcPropsBase = 35,
	/** Address of a native effect's ProcessProc callback
	 *	
	 *	Data type: \ref AAX_CProcessProc
	 *
	 *	For use with \ref AAX_IComponentDescriptor::AddProcessProc()
	 */
	AAX_eProperty_NativeProcessProc = 36,
	/** Address of a native effect's instance initialization callback
	 *
	 *	Data type: \ref AAX_CInstanceInitProc
	 *
	 *	For use with \ref AAX_IComponentDescriptor::AddProcessProc()
	 */
	AAX_eProperty_NativeInstanceInitProc = 37,
	/** Address of a native effect's background callback
	 *
	 *	Data type: \ref AAX_CBackgroundProc
	 *
	 *	For use with \ref AAX_IComponentDescriptor::AddProcessProc()
	 */
	AAX_eProperty_NativeBackgroundProc = 38,
	/** Name of the DLL for a TI effect
	 *
	 *	Data type: UTF-8 C-string
	 *
	 *	For use with \ref AAX_IComponentDescriptor::AddProcessProc()
	 */
	AAX_eProperty_TIDLLFileName = 39,
	/** Name of a TI effect's ProcessProc callback
	 *
	 *	Data type: C-string
	 *
	 *	For use with \ref AAX_IComponentDescriptor::AddProcessProc()
	 */
	AAX_eProperty_TIProcessProc = 40,
	/** Name of a TI effect's instance initialization callback
	 *
	 *	Data type: C-string
	 *
	 *	For use with \ref AAX_IComponentDescriptor::AddProcessProc()
	 */
	AAX_eProperty_TIInstanceInitProc = 41,
	/** Name of a TI effect's background callback
	 *
	 *	Data type: C-string
	 *
	 *	For use with \ref AAX_IComponentDescriptor::AddProcessProc()
	 */
	AAX_eProperty_TIBackgroundProc = 42,
	//@}end Plug-In spec properties
	
//---------------------------------------------------------------------	
#if 0
#pragma mark General properties
#endif
	/** @name General properties
	 *
	 */
	//@{
	/** <HR> */
	AAX_eProperty_GeneralPropsBase = 50,
	/** \brief Input stem format.  One of \ref AAX_EStemFormat
	 *
     *  \li Apply this property at the \b ProcessProc level
	 *
	 *	For offline processing, use \ref AAX_eProperty_NumberOfInputs
     */
    AAX_eProperty_InputStemFormat = 51,
	/** \brief Output stem format.  One of \ref AAX_EStemFormat
     *
     *  \li Apply this property at the \b ProcessProc level
	 *
	 *	For offline processing, use \ref AAX_eProperty_NumberOfOutputs
     */
    AAX_eProperty_OutputStemFormat = 52,
	/** \brief Audio buffer length for DSP processing callbacks.  One of
	 *	\ref AAX_EAudioBufferLengthDSP
     *
     *  \li Apply this property at the \b ProcessProc level
     *  \li This property is only applicable to DSP algorithms
     */
	AAX_eProperty_DSP_AudioBufferLength = 54,
	/** \brief \deprecated Use \ref AAX_eProperty_DSP_AudioBufferLength
     */
    AAX_eProperty_AudioBufferLength = AAX_eProperty_DSP_AudioBufferLength,
	/** \brief Default latency contribution of a given processing callback, in samples
     *
     *  \li Apply this property at the \b ProcessProc level
     *
     *  Unlike most properties, an Effect's latency contribution may also be changed dynamically at
     *	runtime.  This is done via \ref AAX_IController::SetSignalLatency().  Dynamic latency
     *	reporting may not be recognized by the host application in all circumstances, however, so
     *	Effects should \em always define any nonzero initial latency value using
     *	\ref AAX_eProperty_LatencyContribution
	 *
	 *	\compatibility Maximum delay compensation limits will vary from host to host. If your
	 *	plug-in exceeds the delay compensation sample limit for a given %AAX host then you should
	 *	note this limitation in your user documentation. Example limits:
	 *	- Pro Tools 9 and higher: 16,383 samples at 44.1/48 kHz, 32,767 samples at 88.2/96 kHz, or 65,534 samples at 176.4/192 kHz
	 *	- Media Composer 8.1 and higher: 16,383 samples at 44.1/48 kHz, 32,767 samples at 88.2/96 kHz
     */
    AAX_eProperty_LatencyContribution = 56,
	/** \brief Specifies which sample rates the Effect supports.  A mask of \ref AAX_ESampleRateMask
     *
     *  \li Apply this property at the \b ProcessProc level
     *
     *  \sa \ref AAX_IComponentDescriptor::AddSampleRate()
     */
    AAX_eProperty_SampleRate = 58,
    /** \brief The plug-in supports a Master Bypass control
     *
     *  \li Apply this property at the \b ProcessProc level
	 *  
	 *  Nearly all %AAX plug-ins should set this property to \c true
	 *
	 *  Set this property to \c false (0) to disable Master Bypass for plug-ins that cannot be bypassed,
	 *  such as fold-down plug-ins that convert to a narrower channel format.
     *
     *  \legacy Was pluginGestalt_CanBypass.
     */
    AAX_eProperty_CanBypass = 60,
	/** \brief Side chain stem format.  One of \ref AAX_EStemFormat
	 *
	 *	\compatibility Currently Pro Tools supports only \ref AAX_eStemFormat_Mono side chain inputs
	 *
     *  \li Apply this property at the \b ProcessProc level
     *
     *  \compatibility AAX_eProperty_SideChainStemFormat is not currently implemented in DAE or AAE
     */
    AAX_eProperty_SideChainStemFormat = 61,
	//@}end General properties
	
//---------------------------------------------------------------------	
#if 0
#pragma mark TI-specific properties
#endif
	/** @name TI-specific properties
	 *
	 */
	//@{
	/** \brief Shared cycle count (outer, per clump, loop overhead)
     *
     *  \li Apply this property at the \b ProcessProc level
     *  \li This property is only applicable to DSP algorithms
     */
    AAX_eProperty_TI_SharedCycleCount = 62,
	/** \brief Instance cycle count (inner, per instance, loop overhead)
     *
     *  \li Apply this property at the \b ProcessProc level
     *  \li This property is only applicable to DSP algorithms
     */
    AAX_eProperty_TI_InstanceCycleCount = 63,
	/** \brief Maximum number of instances of this plug-in that can be loaded on a chip. This
	 *	property is only used for DMA and background thread-enabled plug-ins.
     *
     *  \li Apply this property at the \b ProcessProc level
     *  \li This property is only applicable to DSP algorithms
     */
    AAX_eProperty_TI_MaxInstancesPerChip = 64,
	/** \brief Allow different plug-in types to share the same DSP even if
	 *	\ref AAX_eProperty_TI_MaxInstancesPerChip is declared
	 *
	 *	In general, this is not desired behavior. However, this can be useful if your plug-in
	 *	instance counts are bound by a system constraint other than CPU usage and you require
	 *	chip-sharing between instances of different types of the plug-in.
	 *
	 *	\note In addition to defining this property, the types which will share allocations on
	 *	the same DSP chip must be compiled into the same ELF DLL file.
	 *
	 *	\li Apply this property at the \b ProcessProc level
	 *	\li This property is only applicable to DSP algorithms
	 */
	AAX_eProperty_TI_ForceAllowChipSharing = 65,
	//@}end TI-specific properties
	
//---------------------------------------------------------------------	
#if 0
#pragma mark General properties (continued)
#endif
	/** @name General properties
	 *
	 *    <!-- CONTINUED -->
	 *
	 */
	//@{
	/** \brief The plug-in never alters its audio signal, audio output is always equal to audio input
	 *
	 *  \li Apply this property at the \b ProcessProc level
	 *
	 *	Setting this property allows host to optimize audio routing and reduce audio latency.
	 */
	AAX_eProperty_AlwaysBypass = 75,
	/** \brief Indicates whether or not the plug-in should be shown in insert menus.
	 *
	 *  \li Apply this property to show or hide the plug-in from the Pro Tools insert menus.
	 *  \li This property value is \c true by default.
	 */
	AAX_eProperty_ShowInMenus = 76,
	//@}end General properties (continued)

//---------------------------------------------------------------------	
#if 0
#pragma mark AAX Hybrid properties
#endif
	/** @name %AAX Hybrid properties
	 *
	 */
	//@{
	/** \brief Hybrid Output stem format.  One of \ref AAX_EStemFormat
     *
	 *	This property represents the stem format for the audio channels that are sent from the
	 *	ProcessProc callback to the \ref AAX_IEffectParameters::RenderAudio_Hybrid() method
	 *
     *  \li Apply this property at the \b ProcessProc level
	 *	\li Normally plugins will set this to the same thing as \ref AAX_eProperty_InputStemFormat
	 *
	 *	\ingroup additionalFeatures_Hybrid
     */
    AAX_eProperty_HybridOutputStemFormat = 90,
    /** \brief Hybrid Input stem format.  One of \ref AAX_EStemFormat
     *
	 *	This property represents the stem format for the audio channels that are sent from the
	 *	\ref AAX_IEffectParameters::RenderAudio_Hybrid() method to the ProcessProc callback
	 *
     *  \li Apply this property at the \b ProcessProc level
	 *	\li Normally plugins will set this to the same thing as \ref AAX_eProperty_OutputStemFormat
	 *
	 *	\ingroup additionalFeatures_Hybrid
     */
    AAX_eProperty_HybridInputStemFormat = 91,
	//@}end %AAX Hybrid properties
	
//---------------------------------------------------------------------	
#if 0
#pragma mark Offline (AudioSuite) properties
#endif
	/** @name Offline (AudioSuite) properties
	 *
	 */
	//@{
	/** <HR> */
	AAX_eProperty_AudiosuitePropsBase = 100,
	/** \brief The Effect requires random access to audio data
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to \ref AuxInterface_HostProcessor "Host Processor"
     *	algorithms
     *
     *  \legacy Was pluginGestalt_UsesRandomAccess
     */
    AAX_eProperty_UsesRandomAccess = 101,
	/** \brief The Effect requires an analysis pass
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     *
     *  \legacy Was pluginGestalt_RequiresAnalysis
     */
    AAX_eProperty_RequiresAnalysis = 102,
	/** \brief The Effect supports an analysis pass, but does not require it
     *
	 *  \compatibility In Media Composer, optional analysis will also be performed automatically
	 *  before each channel is rendered. See \ref MCDEV-2904
	 *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     *
     *  \legacy Was pluginGestalt_OptionalAnalysis
     */
    AAX_eProperty_OptionalAnalysis = 103,
	/** \brief The Effect requires analysis, but is also allowed to preview
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     *
     *  \legacy Was pluginGestalt_AnalyzeOnTheFly
     */
    AAX_eProperty_AllowPreviewWithoutAnalysis = 104,
	/** \brief Informs the host application to reassign output to a different track
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     *
     *	\compatibility This property is not supported on Media Composer
     *
     *  \legacy Was pluginGestalt_DestinationTrack
     */
    AAX_eProperty_DestinationTrack = 105,
	/** \brief The host should make all of the processed track's data available to the Effect
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to \ref AuxInterface_HostProcessor "Host Processor"
     *	algorithms
     *
     *  \legacy Was pluginGestalt_RequestsAllTrackData
     */
    AAX_eProperty_RequestsAllTrackData = 106,
	/** \brief The Effect only processes on continuous data and does not support 'clip by clip'
	 *	rendering
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     *
     *  \legacy Was pluginGestalt_ContinuousOnly
     */
    AAX_eProperty_ContinuousOnly = 107,
    /** \brief The Effect wants multi-input mode only (no mono mode option)
     *
     *  \note See bug \ref PT-258560
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     *
     *  \legacy Was pluginGestalt_MultiInputModeOnly
     */
    AAX_eProperty_MultiInputModeOnly = 108,
	/** \brief The Effect does not support preview
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     *
     *  \legacy Was pluginGestalt_DisablePreview
     */
    AAX_eProperty_DisablePreview = 110,
	/** \brief The Effect may not increment its output sample during some rendering calls
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to \ref AuxInterface_HostProcessor "Host Processor"
     *	algorithms
     *
     *  \legacy Was pluginGestalt_DoesntIncrOutputSample
     */
    AAX_eProperty_DoesntIncrOutputSample = 112,
	/** \brief The number of input channels that the plug-in supports
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to \ref AuxInterface_HostProcessor "Host Processor"
     *	algorithms
	 *
	 *	For real-time processing, use \ref AAX_eProperty_InputStemFormat
     */
    AAX_eProperty_NumberOfInputs = 113,
	/** \brief The number of output channels that the plug-in supports
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to \ref AuxInterface_HostProcessor "Host Processor"
     *	algorithms
	 *
	 *	For real-time processing, use \ref AAX_eProperty_OutputStemFormat
     */
    AAX_eProperty_NumberOfOutputs = 114,
	/** \brief Prevents the application of rendered region handles by the host
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     */
    AAX_eProperty_DisableHandles = 115,
	/** \brief Tells the host that the plug-in supports side chain inputs
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     */
    AAX_eProperty_SupportsSideChainInput = 116,
	/** \brief Requests that the host apply dithering to the Effect's output
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     *
     *  \legacy Was pluginGestalt_NeedsOutputDithered
     */
    AAX_eProperty_NeedsOutputDithered = 117,
    /** \brief The plug-in supports audiosuite reverse.  By default, all reverb and delay 
     *   plug-ins support this feature.  If a plug-in needs to opt out of this feature, they
     *   can set this property to true.
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *  \li This property is only applicable to offline processing
     */
    AAX_eProperty_DisableAudioSuiteReverse = 118,

	AAX_eProperty_MaxASProp, // Intentionally given no explicit value
	//@}end Offline (AudioSuite) properties
	
//---------------------------------------------------------------------	
#if 0
#pragma mark GUI properties
#endif
	/** @name GUI properties
	*
	*/
	//@{
	/** <HR> */
	AAX_eProperty_GUIBase = 150,
	/** \brief Requests a host-generated GUI based on the Effect's parameters
     *
	 *  Use this property while your plug-in is in development to test the plug-in's
	 *  data model and algorithm before its GUI has been created, or when troubleshooting
	 *  problems to isolate the data model and algorithm operation from the plug-in's GUI.
	 *
     *  \li Apply this property at the \b ProcessProc level
	 *
	 *  \compatibility Currently supported by Pro Tools only
	 *
	 *  \note See \ref PTSW-189725
     */
    AAX_eProperty_UsesClientGUI = 151,
	
	AAX_eProperty_MaxGUIProp, // Intentionally given no explicit value
	//@}end GUI properties
	
//---------------------------------------------------------------------	
#if 0
#pragma mark Meter properties
#endif
	/** @name Meter properties
	 *
	 *	These properties define the behavior of individual meters
	 *
	 *	For more information about meters in AAX, see \ref AdditionalFeatures_Meters
	 */
	//@{
	/** <HR> */
	AAX_eProperty_MeterBase = 199,
	/** \brief Indicates meter type as one of \ref AAX_EMeterType
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor::AddMeterDescription() level
     */
    AAX_eProperty_Meter_Type = 200,
	/** \brief Indicates meter orientation as one of \ref AAX_EMeterOrientation
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor::AddMeterDescription() level
     */
    AAX_eProperty_Meter_Orientation = 201,
	/** \brief Indicates meter ballistics preference as one of \ref AAX_EMeterBallisticType
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor::AddMeterDescription() level
     */
    AAX_eProperty_Meter_Ballistics = 202,
	
	AAX_eProperty_MaxMeterProp, // Intentionally given no explicit value
	//@}end GUI properties
	
//---------------------------------------------------------------------	
#if 0
#pragma mark Plug-in management constraints
#endif
	/** @name Plug-in management constraints
	 *
	 *	These properties define constraints on how the host may manage
	 *	the plug-in's various components and modules, as well as its
	 *	overall configuration.
	 */
	//@{
	/** <HR> */
	AAX_eProperty_ConstraintBase = 299,
	/** \brief Constraint on the algorithm's location, as a mask of \ref AAX_EConstraintLocationMask
     *
     *  \li Apply this property at the \b ProcessProc level
     */
    AAX_eProperty_Constraint_Location = 300,
	/** \brief Constraint on the topology of the Effect's modules, as one of
	 *	\ref AAX_EConstraintTopology
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     */
    AAX_eProperty_Constraint_Topology = 301,
	/** \brief Tells the host that it should never unload the plug-in binary
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     *
     *  \compatibility AAX_eProperty_Constraint_NeverUnload is not currently implemented in DAE or AAE
     */
    AAX_eProperty_Constraint_NeverUnload = 302,
	/** \brief Tells the host that it should never cache the plug-in binary.  Only use this if required as
     *   there is a performance penalty on launch to not use the Cache.  Set this property to 1, if you really need
     *   to not cache.  Default is 0.
	 *
	 *   The most common reason for a plug-in to require this constraint is if the plug-in's configuration can
	 *   change based on external conditions. Most of the data contained in the plug-in's description routine is
	 *   cached, so if the plug-in description can change between launches of the host application then the plug-in
	 *   should apply this constraint to prevent the host from using stale description information.
     *
     *   This property should be applied at the collection level as it affects the entire bundle.
     *
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
     */
    AAX_eProperty_Constraint_NeverCache = 303,
	/** \brief Indicates whether or not the plug-in supports multi-mono configurations
	 *	(\c true/\c false)
	 *
	 *	\note Multi-mono mode may not work as expected for VIs and other plug-ins which rely on non-global
	 *	MIDI input. Depending on the host, multi-mono instances may not all be automatically connected to
	 *	the same MIDI port upon instantiation. Therefore it is recommended to set this property to \c 0 for
	 *	any plug-ins if this lack of automatic connection may confuse users.
     *
     *  \li Apply this property at the \b ProcessProc level
     */
    AAX_eProperty_Constraint_MultiMonoSupport = 304,
	
	AAX_eProperty_MaxConstraintProp, // Intentionally given no explicit value
	//@} end Plug-in management constraints (NOTE: CONTINUED BELOW)
	
//---------------------------------------------------------------------	
#if 0
#pragma mark Plug-in features
#endif
	/** @name Plug-in features
	 *
	 *	These properties declare plug-in support (or lack of support) for certain
	 *	host features.
	 */
	//@{
	AAX_eProperty_FeaturesBase = 305, // No room was given, so this equals AAX_eProperty_SupportsSaveRestore
	/** \brief Indicates whether or not the plug-in supports Save/Restore features.
     *	(\c true/\c false)
     *
     *  \li Apply this property to show or hide the Settings section in the plug-in window.
     *  \li This property value is true by default.
     *
     *  \legacy Was pluginGestalt_SupportsSaveRestore
	 */
	AAX_eProperty_SupportsSaveRestore = 305,
	/** \brief Indicates whether or not the plug-in uses transport requests.
     *	(\c true/\c false)
     *
	 *  \li Apply this property if your plug-in uses AAX_ITransport class.
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
	 */
	AAX_eProperty_UsesTransport = 306,
    /** \brief This property specifies whether the plug-in bundle contains an XML file per plug-in type.
     *
	 *  \details
	 *  %AAX plug-ins always provide XML page table data via external files referenced by
	 *  \ref AAX_eResourceType_PageTable. If \ref AAX_eProperty_StoreXMLPageTablesByEffect is not defined
	 *  or is set to 0 (the default) then the host may assume that all Effects in the collection use the same XML
	 *  page table file. If this property is set to a non-zero value, the plug-in may describe a different
	 *  \ref AAX_eResourceType_PageTable for each separate Effect.
     *
	 *  This property needs to be set at the collection level.
     */
	AAX_eProperty_StoreXMLPageTablesByEffect = 307,
	/** \brief \deprecated Use \ref AAX_eProperty_StoreXMLPageTablesByEffect
     */
    AAX_eProperty_StoreXMLPageTablesByType = AAX_eProperty_StoreXMLPageTablesByEffect,
    /** \brief Indicates whether the plug-in supports SetChunk and GetChunk calls on threads other than the main thread.
    *   It is actually important for plug-ins to support these calls on non-main threads, so that is the default.  
    *   However, in response to a few companies having issues with this, we have decided to support this constraint 
    *   for now.
    *
    *   property value should be set to true if you need Chunk calls on the main thread.
    * 
    *   Values: 0 (off, default), 1 (on)
    *
    *   \li Apply this property at the \ref AAX_IEffectDescriptor level
    */
    AAX_eProperty_RequiresChunkCallsOnMainThread = 308,
	/** \brief Indicates whether the plug-in subscribes to the \ref AAX_eNotificationEvent_TransportStateChanged "TransportStateChanged" notification 
	* to receive transport info.
	*
	*   property value should be set to true if you need subscribe to the TransportStateNotification.
	*
	*   Values: 0 (off, default), 1 (on)
	*
	*   \li Apply this property at the \ref AAX_IEffectDescriptor level
	*/
	AAX_eProperty_ObservesTransportState = 309,
	/** \brief Indicates whether or not the plug-in uses transport control requests.
     *	(\c true/\c false)
     *
	 *  \li Apply this property if your plug-in uses \ref AAX_IACFTransportControl methods in the \ref AAX_ITransport class.
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
	 */
	AAX_eProperty_UsesTransportControl = 311,

	AAX_eProperty_MaxFeaturesProp, // Intentionally given no explicit value
	//@} end Plug-in features

//---------------------------------------------------------------------	
#if 0
#pragma mark Plug-in management constraints (continued)
#endif
	/** @name Plug-in management constraints
	 *
	 *	<!-- CONTINUED -->
	 */
	//@{
	AAX_eProperty_ConstraintBase_2 = 350,
	
	/** \brief Indicates that the plug-in's processing should never be disabled by the host
	 *	(\c true/\c false)
	 *
	 *	Some hosts will disable processing for plug-in chains in certain circumstances to conserve system resources, e.g.
	 *	when the chains' output drops to silence for an extended period.
	 *	
	 *	\note This property may impact performance of other plug-ins. For example, the Dynamic Plug-In Processing feature
	 *	in Pro Tools operates over chains of plug-ins rather than single instances; any plug-in that defines
	 *	\ref AAX_eProperty_Constraint_AlwaysProcess will force its entire signal chain to continue processing.
	 *	Therefore it is important to avoid using this property unless features such as Dynamic Plug-In Processing are
	 *	actually interfering in some way with the operation of the plug-in.
	 *
     *  \li This property value is false by default.
     *  \li Apply this property at the \ref AAX_IEffectDescriptor level
	 */
	AAX_eProperty_Constraint_AlwaysProcess = 351,
	
	/** \brief Requests that the host does not send default settings chunks to the plug-in
	 *	after instantiation (\c true/\c false)
	 *
	 *	Some hosts will apply the plug-in's default settings via chunks after creating a
	 *	new plug-in instance as a way to ensure that the all new plug-in instances are
	 *	initialized to the same state.
	 *
	 *	If a plug-in can make this guarantee itself and does not wish to receive any
	 *	default settings chunks from the host after instantiation then it may set this
	 *	property.
	 *
	 *	Support for this property is not guaranteed; the plug-in must be able to handle
	 *	default settings chunk application even if this property is set, or clearly
	 *	document the plug-in's host compatibility.
	 *
	 *	\note See bug \ref PT-284916
	 *
	 *	\li Apply this property at the \ref AAX_IEffectDescriptor level
	 #

	 */
	AAX_eProperty_Constraint_DoNotApplyDefaultSettings = 352,
	
	AAX_eProperty_MaxConstraintProp_2, // Intentionally given no explicit value
	//@} end Plug-in management constraints
	
//---------------------------------------------------------------------	
#if 0
#pragma mark Debug properties
#endif
	/** @name Debug properties
	 */
	//@{
	AAX_eProperty_DebugPropertiesBase = 400,
    /** \brief Enables host debug logging for this plug-in.
	 *
	 *  This logging is made via DigiTrace using the DTF_AAXHOST facility, generally at DTP_LOW priority
	 *
	 *	\li It is recommended to set this property to \c 1 for debug builds and to \c 0 for release builds of a plug-in
	 *  \li Apply this property at the \ref AAX_IEffectDescriptor level
	 */
	AAX_eProperty_EnableHostDebugLogs = 401,
	//@} end Debug properties
	
	AAX_eProperty_MaxProp,			// ALWAYS LEAVE AS LAST PROPERTY VALUE
	AAX_eProperty_MaxCap = 10000	// Maximum possible property value over the lifetime of AAX
}; AAX_ENUM_SIZE_CHECK(AAX_EProperty);

/// @cond ignore
#endif	// AAX_PROPERTIES_H
/// @endcond
