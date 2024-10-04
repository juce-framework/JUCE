/*================================================================================================*/
/*
 *
 *	Copyright 2010-2017, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_Errors.h
 *
 *	\brief Definitions of error codes used by %AAX plug-ins
 *
 */ 
/*================================================================================================*/


/// @cond ignore
#ifndef AAX_ERRORS_H
#define AAX_ERRORS_H
/// @endcond

#include "AAX_Enums.h"

/** AAX result codes
 
 \internal Any new codes added here must also be added to AAX::AsStringResult
 \endinternal
 */
enum AAX_EError
{
	AAX_SUCCESS												= 0,
	
	AAX_ERROR_INVALID_PARAMETER_ID							= -20001,
	AAX_ERROR_INVALID_STRING_CONVERSION						= -20002,
	AAX_ERROR_INVALID_METER_INDEX							= -20003,
	AAX_ERROR_NULL_OBJECT									= -20004,
	AAX_ERROR_OLDER_VERSION									= -20005,
	AAX_ERROR_INVALID_CHUNK_INDEX							= -20006,
	AAX_ERROR_INVALID_CHUNK_ID								= -20007,
	AAX_ERROR_INCORRECT_CHUNK_SIZE							= -20008,
	AAX_ERROR_UNIMPLEMENTED									= -20009,
	AAX_ERROR_INVALID_PARAMETER_INDEX						= -20010,
	AAX_ERROR_NOT_INITIALIZED								= -20011,
	AAX_ERROR_ACF_ERROR										= -20012,
	AAX_ERROR_INVALID_METER_TYPE							= -20013,
	AAX_ERROR_CONTEXT_ALREADY_HAS_METERS					= -20014,
	AAX_ERROR_NULL_COMPONENT								= -20015,
	AAX_ERROR_PORT_ID_OUT_OF_RANGE							= -20016,
	AAX_ERROR_FIELD_TYPE_DOES_NOT_SUPPORT_DIRECT_ACCESS		= -20017,
	AAX_ERROR_DIRECT_ACCESS_OUT_OF_BOUNDS					= -20018,
	AAX_ERROR_FIFO_FULL										= -20019,
	AAX_ERROR_INITIALIZING_PACKET_STREAM_THREAD				= -20020,
	AAX_ERROR_POST_PACKET_FAILED							= -20021,
	AAX_RESULT_PACKET_STREAM_NOT_EMPTY						= -20022,
	AAX_RESULT_ADD_FIELD_UNSUPPORTED_FIELD_TYPE				= -20023,
	AAX_ERROR_MIXER_THREAD_FALLING_BEHIND					= -20024,
	AAX_ERROR_INVALID_FIELD_INDEX							= -20025,
	AAX_ERROR_MALFORMED_CHUNK								= -20026,
	AAX_ERROR_TOD_BEHIND									= -20027,
	AAX_RESULT_NEW_PACKET_POSTED							= -20028,
	AAX_ERROR_PLUGIN_NOT_AUTHORIZED                         = -20029,    //return this from EffectInit() if the plug-in doesn't have proper license.
	AAX_ERROR_PLUGIN_NULL_PARAMETER							= -20030,
	AAX_ERROR_NOTIFICATION_FAILED							= -20031,
	AAX_ERROR_INVALID_VIEW_SIZE 							= -20032,
	AAX_ERROR_SIGNED_INT_OVERFLOW							= -20033,
	AAX_ERROR_NO_COMPONENTS									= -20034,
	AAX_ERROR_DUPLICATE_EFFECT_ID							= -20035,
	AAX_ERROR_DUPLICATE_TYPE_ID								= -20036,
	AAX_ERROR_EMPTY_EFFECT_NAME								= -20037,
	AAX_ERROR_UNKNOWN_PLUGIN								= -20038,
	AAX_ERROR_PROPERTY_UNDEFINED							= -20039,
	AAX_ERROR_INVALID_PATH									= -20040,
	AAX_ERROR_UNKNOWN_ID									= -20041,
	AAX_ERROR_UNKNOWN_EXCEPTION								= -20042, ///< An AAX plug-in should return this to the host if an unknown exception is caught. Exceptions should never be passed to the host.
	AAX_ERROR_INVALID_ARGUMENT								= -20043, ///< One or more input parameters are invalid; all output parameters are left unchanged.
	AAX_ERROR_NULL_ARGUMENT									= -20044, ///< One or more required pointer arguments are null
	AAX_ERROR_INVALID_INTERNAL_DATA							= -20045, ///< Some part of the internal data required by the method is invalid. \sa AAX_ERROR_NOT_INITIALIZED
	AAX_ERROR_ARGUMENT_BUFFER_OVERFLOW						= -20046, ///< A buffer argument was not large enough to hold the data which must be placed within it
	AAX_ERROR_UNSUPPORTED_ENCODING							= -20047, ///< Unsupported input argument text encoding
	AAX_ERROR_UNEXPECTED_EFFECT_ID							= -20048, ///< Encountered an effect ID with a different value from what was expected
	AAX_ERROR_NO_ABBREVIATED_PARAMETER_NAME					= -20049, ///< No parameter name abbreviation with the requested properties has been defined
	AAX_ERROR_ARGUMENT_OUT_OF_RANGE							= -20050, ///< One or more input parameters are out of the expected range, e.g. an index argument that is negative or exceeds the number of elements
	AAX_ERROR_PRINT_FAILURE									= -20051, ///< A failure occurred in a "print" library call such as @c printf
	
	
	AAX_ERROR_PLUGIN_BEGIN									= -20600, ///< Custom plug-in error codes may be placed in the range ( \ref AAX_ERROR_PLUGIN_END, \ref AAX_ERROR_PLUGIN_BEGIN ]
	AAX_ERROR_PLUGIN_END									= -21000  ///< Custom plug-in error codes may be placed in the range ( \ref AAX_ERROR_PLUGIN_END, \ref AAX_ERROR_PLUGIN_BEGIN ]
}; AAX_ENUM_SIZE_CHECK( AAX_EError );









/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// AAE and other known AAX host error codes                                //
// Listed here as a reference                                              //
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// FicErrors.h
/*

//
// NOTE: (Undefined) comments for an error code mean that it's
// either no longer supported or returned from another source
// other than DAE.
//
												
//----------------------------------------------------------------------------
// Error codes for all of Fic
//----------------------------------------------------------------------------

enum {
	kFicHostTimeoutErr				= -9003,	// Host Timeout Error.  DSP is not responding.
	kFicHostBusyErr					= -9004,	// (Undefined)
	kFicLowMemoryErr				= -9006,	// DAE was unable to allocate memory.  Memory is low.
	kFicUnimplementedErr			= -9007,	// An unimplemented method was called.
	kFicAllocatedErr				= -9008,	// (Undefined)
	kFicNILObjectErr				= -9013,	// Standard error return when an object is NULL.
	kFicNoDriverDSPErr				= -9014,	// Missing DSPPtr from the SADriver.
	kFicBadIndexErr					= -9015,	// Index to an array or list is invalid.
	kFicAlreadyDeferredErr			= -9017,	// Tried to install a deferred task when the task was already deferred.
	kFicFileSystemBusyErr			= -9019,	// PB chain for an audio file returned an error for a disk task.
	kFicRunningErr					= -9020,	// Tried to execute code when the deck was started.
	kFicTooManyItemsErr				= -9022,	// Number of needed items goes beyond a lists max size.
	kFicItemNotFoundErr				= -9023,	// Unable to find an object in a list of objects.
	kFicWrongTypeErr				= -9024,	// Type value not found or not supported.
	kFicNoDeckErr					= -9025,	// Standard error returned from other objects that require a deck object.
	kFicNoDSPErr					= -9028,	// Required DSP object is NULL.
	kFicNoFeederErr					= -9029,	// (Undefined)
	kFicNoOwnerErr					= -9030,	// Play or record track not owned by a channel.
	kFicPrimedErr					= -9031,	// Tried to execute code when the deck was primed.
	kFicAlreadyAttached				= -9032,	// DAE object already attached to another DAE object.
	kFicTooManyDSPTracksErr			= -9033,	// The user has run out of virtual tracks for a given card or dsp.
	kFicParseErr					= -9035,	// While trying to parse a data structure ran into an error.
	kFicNotAcquiredErr				= -9041,	// Tried to execute code when an object needs to be acquired first.
	kFicNoSSIClockErr				= -9045,	// DSP does not recieve peripheral clock interrupts.
	kFicNotFound					= -9048,	// Missing DAE resource or timeout occured while waiting for DAE to launch.
	kFicCantRecordErr				= -9050,	// Error returned when CanRecord() returns false.  Exp: Recording on scrub channel.
	kFicWrongObjectErr				= -9054,	// Object size or pointers do not match.
	kFicLowVersionErr				= -9055,	// Errors with version number too low.
	kFicNotStartedErr				= -9057,	// Tried to execute code when the deck was not started yet.
	kFicOnly1PunchInErr				= -9059,	// Error when deck can only support a single punch in.
	kFicAssertErr					= -9060,	// Generic error when a format does not match.
	kFicScrubOnlyErr				= -9061,	// Tried to scrub in a non-scrub mode or on a sys axe channel.
	kFicNoSADriverErr				= -9062,	// InitSADriver failed.  Possible missing DigiSystem INIT.
	kFicCantFindDAEFolder			= -9064,	// Unable to find "DAE Folder" in the system folder.
	kFicCantFindDAEApp				= -9065,	// Unable to find DAE app in the DAE Folder.
	kFicNeeds32BitModeErr			= -9066,	// DAE runs only in 32 bit mode.
	kFicHatesVirtualMemErr			= -9068,	// DAE will not run if virtual memory is turned on.
	kFicSCIConnectErr				= -9070,	// Unable to get SCI ports between two dsp's to communicate.
	kFicSADriverVersionErr			= -9071,	// Unable to get DigiSystem INIT version or it's version is too low.
	kFicUserCancelledErr			= -9072,	// User chose to cancel or quit operation from DAE dialog.
	kFicDiskTooSlowErr				= -9073,	// Disk action did not complete in time for next command.
	kFicAudioTrackTooDense1			= -9074,	// Audio playlist is too dense.
	kFicAudioTrackTooDense2			= -9075,	// Audio playlist is too dense for silience play list.
	kFicCantDescribeZone			= -9076,	// Zone description is NULL.
	kFicCantApplyPlayLimits			= -9077,	// Ran out of time regions for a zone.
	kFicCantApplySkipMode			= -9078,	// Ran out of time regions for a zone in skip mode.
	kFicCantApplyLoop				= -9079,	// Ran out of time regions for a zone in loop mode.
	kFicAutoSortErr					= -9084,	// DSP event elements are not sorted in relation to time.
	kFicNoAutoEvent					= -9085,	// No event list for an auto parser.
	kFicAutoTrackTooDense1			= -9086,	// Automation event scripts are too dense.
	kFicAutoTrackTooDense2			= -9087,	// Ran out of free events for the automation parser.
	kFicNothingAllowedErr			= -9088,	// Missing allowed decks for the hw setup dialog.
	kFicHardwareNotFreeErr			= -9089,	// Unable to select a deck because the hardware is allocated or not available.
	kFicUnderrunErr9093				= -9093,	// Under run error from the DSP.
	kFicBadVRefNumErr				= -9095,	// Audio file is not on the propper disk SCSI chain.
	kFicNoPeripheralSelected		= -9096,	// Deck can not be aquired without a peripheral being selected.
	kFicLaunchMemoryErr				= -9097,	// Unable to launch DAE because of a memory error.  DAE does NOT launch.
	kFicGestaltBadSelector			= -9099,	// Gestalt selector not supported.
	kDuplicateWriteFiles			= -9118,	// Writing to the same file multiple times during processing.
	kFicCantGetTempBuffer			= -9121,	// Disk scheduler ran out of temporary buffers.  Playlist is too complex.
	kFicPendingRequestsFull			= -9122,	// (Undefined)
	kFicRequestHandlesFull			= -9123,	// (Undefined)
	kFicAnonymousDrive				= -9124,	// (Win32) Disk scheduler can't use a drive that doesn't have a drive signature.
	kFicComputerNeedsRestart		= -9127,	// DAE state has changed such that the computer needs to restart
	kFicCPUOverload					= -9128,	// Host processing has exceeded its CPU allocation.
	kFicHostInterruptTooLong		= -9129,	// Host processing held off other system interrupts for too long.
	kFicBounceHandlerTooSlow		= -9132,
	kFicBounceHandlerTooSlowToConvertWhileBouncing = -9133,
	kFicMBoxLostConnection			= -9134,		// MBox was disconnected during playback
	kFicMBoxNotConnected			= -9135,		// MBox is not connected
	kFicUSBIsochronousUnderrun		= -9136,		// USB audio streaming underrun
	kFicAlreadyAcquired 			= -9137,		// tried to change coarse sample rate on already acquired deck
	kFicTDM2BusTopologyErr 			= -9138,		// eDsiTDM2BusTopologyErr was returned from DSI.
	kFicDirectIODHSAlreadyOpen		= -9142,			// can't run if a DirectIO client is running DHS right now
	kFicAcquiredButChangedBuffers	= -9143,			// DAE was able to acquire the device but had to change the disk buffers size to do it.
	kFicStreamManagerUnderrun		= -9144,			// received error from StreamManager
	kDirectMidiError				= -9145,			// an error occurred in the DirectMidi subsytem
	kFicResourceForkNotFound		= -9146,			// Could not find the DAE resource fork (i.e. fnfErr)
	kFicInputDelayNotSupported		= -9147,
	kFicInsufficientBounceStreams	= -9148,
	kFicAutoTotalTooDenseForDSP		= -9155,	// (Undefined)
	kBadPlugInSpec					= -9156,	// Default error returned when there's no component object attatched to a spec.
	kFicFarmRequiredErr				= -9157,	// Error returned by objects that require a DSP farm in the system.
	kFicPlugInDidSetCursor				= -9163,	// When returned by FicPlugInEvent, the plug-in DID change the cursor.
	kFicMaxFileCountReached				= -9168,	// Max number of files open has been reached
	kFicCantIncreaseAIOLimits			= -9169,	// Can't increase the AIO kernel limits on OSX. DigiShoeTool is probably not installed correctly.
	kFicGreenOverrunWhileVSOIsOn			= -9170,	// A PIO underrun/overrun occurred while varispeed is on; should probably warn the user this can happen.
	kFicBerlinGreenStreamingError			= -9171,
	kFicHardwareDeadlineMissedErr			= -9172,
	kFicStatePacketUnderrun				= -9173,	// Low-latency engine ran out of state packets sent from high-latency engine
	kFicCannotCompleteRequestError			= -9174,
	kFicNILParameterError           		= -9175,	// Method called with one or more required parameters set to NULL
	kFicMissingOrInvalidAllowedPlugInsListFile	= -9176,	// PT First-specific: could not parse the "Allowed" plug-ins file
	kFicBufferNotLargeEnoughError	= -9177, // Method called with a data buffer that is too small for the requested data
	kFicInitializationFailed        = -9178, // Error caught during FicInit
	kFicPostPacketFailed = -9179, // Error triggered by AAXH_CPlugIn::PostPacket
	
};

// Weird errors preserved here for backwards compatibility (i.e., older DAE's returned these errors, so we should also):

enum {
	kFicBeyondPlayableRange			= -9735		// Session playback passed the signed 32 bit sample number limit ( = kFicParseErr - 700).
};


//----------------------------------------------------------------------------
// Error codes returned from the SADriver/DigiSystem INIT via DAE
//----------------------------------------------------------------------------

enum {
	kFicSADriverErrOffset			= -9200,	// Offset only, should never be returned as a result.
	kSADUnsupported					= -9201,	// Unsupported feature being set from a piece of hardware.
	kSADNoStandardShell				= -9202,	// Unable to load standard shell code resource.
	kSADTooManyPeripherals			= -9203,	// Went beyond the max number of peripherals allowed in the code.
	kSADHostTimeoutErr				= -9204,	// Timeout occured while trying to communicate with the DSP's host port.
	kSADInvalidValue				= -9205,	// Invalid value being set to a hardware feature.
	kSADInvalidObject				= -9206,	// NULL object found when a valid object is required.
	
	kSADNILClient					= -9210,	// Trying to opperate on a NULL client.
	kSADClientRegistered			= -9211,	// Client already registered.
	kSADClientUnregistered			= -9212,	// Trying to remove a client when it's not registered.
	kSADNoListener					= -9213,	// No client to respond to a message from another client.
	
	kSADCardOwned					= -9220,	// A card is owned by a client.
	kSADDSPOwned					= -9230,	// A DSP is owned by a client.
	
	kSADNILShell					= -9240,	// Trying to opperate on a NULL shell.
	kSADShellRegistered				= -9241,	// Shell already registered.
	kSADShellUnregistered			= -9242,	// Trying to remove a shell when it's not registered.
	kSADShellTooSmall				= -9243,	// (Undefined)
	kSADShellTooLarge				= -9244,	// DSP code runs into standard shell or runs out of P memory.
	kSADStandardShell				= -9245,	// Trying to unregister the standard shell.
	
	kSADNoDriverFile				= -9250,	// Unable to open or create the DigiSetup file.
	kSADDriverFileUnused			= -9251,	// Trying to free the DigiSetup file when it hasn't been openned.
	kSADNILResource					= -9252,	// Resource not found in the DigiSetup file.
	kSADBadSize						= -9253,	// Resource size does not match pointer size requested.
	kSADBadSlot						= -9254,	// NuBus slot value is out of range for the system.
	kSADBadIndex					= -9255		// DSP index is out of range for the system.
};


//----------------------------------------------------------------------------
// Error codes for Elastic audio
//----------------------------------------------------------------------------
enum {
	kFicElasticGeneralErr			= -9400,	// don't know what else to do
	kFicElasticUnsupported			= -9401,	// requested op unsupported
	kFicElasticCPUOverload			= -9403,	// Like kFicCPUOverload but for Fela
	kFicElasticOutOfMemory			= -9404,	// you're not going to last long...
	kFicElasticTrackTooDense		= -9405,	// like kFicAudioTrackTooDense1; feeder list too big
	kFicElasticInadequateBuffering	= -9406,	// reserved buffers for Fela data too small
	kFicElasticConnectionErr		= -9408,	// Problem with a plugin connection
	kFicElasticDriftBackwardsErr	= -9411,	// disconnect between DAE (app?) and plugin data consumption rates
	kFicElasticDriftForwardsErr		= -9412,	// disconnect between DAE (app?) and plugin data consumption rates
	kFicElasticPlugInLimitsErr		= -9413,	// problem with plugin drift/lookAhead; too much requested?
	kFicElasticInvalidParameter		= -9415,    // Elastic function was passed a bad parameter
	kFicElasticInvalidState			= -9416,	// Elastic track's internal state is in error.
	kFicElasticPlugInConnected		= -9417,	// Can't change stem format once an elastic plugin is already connected to a track
	kFicElasticEphemeralAllocErr	= -9419,	// ephemeral buffer alloc failure
	kFicElasticDiskTooSlowErr		= -9473,	// Like -9073, but caught in a new way (Elastic needs disk data sooner)
};

//----------------------------------------------------------------------------
// Error codes for Clip Gain RT Fades
//----------------------------------------------------------------------------
enum {
	kFicClipGainRTFadesFadeOutofBounds	= 9480,
};

//----------------------------------------------------------------------------
// Error codes for Disk Cache
//----------------------------------------------------------------------------
enum {
	kFicDiskCachePageOverflow		= -9500,	// not enough pages in the cache to fulfill page request.
	kFicDiskCacheWriteErr			= -9502,	// problem writing to the disk cache.
	kFicDiskCacheDiskWriteErr		= -9503,    // problem writing to disk from the cache.
	kFicDiskCacheInvalidNull		= -9504,    // invalid NULL variable.  NULL and 0 have special meaning in the cache.
	kFicDiskCacheMissingDataErr		= -9506,	// data that's supposed to be in the cache is not.
	kFicDiskCacheGeneralErr			= -9507,	// general error.
	kFicDiskCacheDoubleLRUPageErr	= -9508,	// duplicate page in the LRU.
	kFicDiskCacheDoubleOwnerPageErr = -9509,	// two pages with the same owner.
	kFicDiskCachePageLeakErr		= -9510,	// page leak in the allocator.
	kFicDiskCacheMappingErr			= -9511,	// corruption in mapping of disk cache objects to the page allocator
	kFicDiskCacheUnityFileErr		= -9513,	// Unity and ISIS are incompatible with the disk cache's temporary buffers
	kFicDiskCacheOutOfMemory		= -9514,	// Couldn't allocate the disk cache!  32bits will suffocate us all.
	kFicNativeDiskCacheOutOfMemory	= -9515,	// Couldn't allocate the disk cache on a Native system!
};

//----------------------------------------------------------------------------
// Error codes for FPGA DMA Device(Green and Berlin cards)
//----------------------------------------------------------------------------
enum {
	kFicFpgaDmaDevicePIOOverflow	        	= -9600,	// PIO ring buffer overflowed
	kFicFpgaDmaDevicePIOUnderflow	         	= -9601,	// PIO ring buffer underflow
	kFicFpgaDmaDevicePIOSyncErr			        = -9602,	// PIO sync error
	kFicFpgaDmaDevicePIOClockChange		        = -9603,	// PIO clock change error
	kFicFpgaDmaDevicePIOUnknownErr		        = -9604,	// PIO unknown error
	kFicFpgaDmaDeviceTDMRcvOverflow		        = -9605,	// TDM receive overflow
	kFicFpgaDmaDeviceTDMXmtUnderflow	        = -9606,	// TDM transmit underflow
	kFicFpgaDmaDeviceTDMSyncErr			        = -9607,	// TDM sync error
	kFicFpgaDmaDeviceTDMCRCErr			        = -9608,	// TDM CRC  error
	kFicFpgaDmaDeviceTDM_NO_Xbar_Txdata_error	= -9609,	// TDM NO_Xbar_Txdata_error
	kFicFpgaDmaDeviceTDMUnknownErr		        = -9610,	// TDM unknown error
	kFicFpgaDmaDeviceRegRdTimeoutErr	        = -9611,	// RegRdTimeoutErr
	kFicFpgaDmaDeviceTemperatureErr		        = -9612,	// Temperature error
};

//----------------------------------------------------------------------------
// Various Widget Error Codes
//----------------------------------------------------------------------------

enum {
	
	// External Callback Proc Errors -7000..-7024
	kSelectorNotSupported 			= -7000,	// This selector ID is unknown currently.
	kWidgetNotFound 				= -7001,	// Refnum did not specify a known widget.
	
	// Plug-In Manager Errors -7025..-7049
	kPlugInNotInstantiated 			= -7026,	// A non-instantiated plug-in was asked to do something.
	kNilComponentObject 			= -7027,	// A component-referencing object was NIL.
	kWidgetNotOpen 					= -7028,	// A non-instantiated widget was asked to do something.
	//TIMILEONE ADD
	kDspMgrError					= -7030,	// An error originating in DspMgr returned
	kEffectInstantiateError			= -7032,	// Problem occurred attempting to instantiate a plug-in.
	
	// Plug-In Manager Errors -7050..-7075
	kNotEnoughHardware 				= -7050,	// Not enough hardware available to instantiate a plug-in.
	kNotEnoughTDMSlots 				= -7052,	// Not enough TDM slots available to instantiate a plug-in.
	kCantInstantiatePlugIn 			= -7054,	// Unable to instantiate a plug-in (generic error).
	kCantFindPlugIn 				= -7055,	// Unable to find the specified plug-in.
	kNoPlugInsExist 				= -7056,	// No plug-ins at all exist.
	kPlugInUnauthorized				= -7058,	// To catch uncopyprotected plugins
	kInvalidHostSignalNet			= -7062,	// The signalNet ptr does not correspond to a CHostSignalNet instance
	// The RTAS/TDM plug-in would be disabled because the corresponding AAX plug-in exists.
	//
	// The following lower-level errors can also be converted to kPlugInDisabled:
	//   kAAXH_Result_FailedToRegisterEffectPackageWrongArchitecture
	//   kAAXH_Result_PluginBuiltAgainstIncompatibleSDKVersion
	kPlugInDisabled					= -7063,
	kPlugInNotAllowed				= -7064,	// The plug-in not allowed to load
	
	// Widget errors (returned by calls to widget functions): -7075..-7099.
	kWidgetUnsupportedSampleRate	= -7081,	// Widget cannot instantiate at the current sample rate
	
	// Connection errors: -7100..-7124
	kInputPortInUse 				= -7100,	// Tried to connect to an input that is already connected.
	kOutputPortCannotConnect 		= -7101,	// Specified output port has reached its limit of output connections.
	kInvalidConnection 				= -7103,	// Invalid or freed connection reference passed.
	kBadConnectionInfo 				= -7104,	// TDM talker & listener data not consistent on disconnect.
	kFreeConnectionErr 				= -7105,	// Could not delete connection info.
	kInvalidPortNum 				= -7106,	// Out-of-range or nonexistent port number specified.
	kPortIsDisconnected 			= -7107,	// Tried to disconnect a disconnected port.
	
	kBadStemFormat					= -7110,
	kBadInputStemFormat				= -7111,
	kBadOutputStemFormat			= -7112,
	kBadSideChainStemFormat			= -7113,
	kBadGenericStemFormat			= -7114,
	kBadUnknownStemFormat			= -7115,
	
	kNoFirstRTASDuringPlayback      = -7117,	// can't instantiate the first RTAS plug-in on the fly (TDM decks)
	kNoBridgeConnectionDuringPlayback = -7118, // can't create or free a bridge connection during playback
	
	// Subwidget errs: -7125..-7149
	kInstanceIndexRangeErr 			= -7126,	// Specified instance index doesn't correspond with an instance.
	kEmptySubWidgetList 			= -7129,	// List isn't NULL, but has no elements.
	
	// Instance errs: -7150..-7174
	kNumInstancesWentNegative 		= -7150,	// Somehow a count of instances (in widget or DSP) went < 0.
	kCantChangeNumInputs 			= -7152,	// Plugin does not have variable number of inputs.
	kCantChangeNumOutputs 			= -7153,	// Plugin does not have variable number of outputs.
	kSetNumInputsOutOfRange 		= -7154,	// Number of inputs being set is out of range.
	kSetNumOutputsOutOfRange 		= -7155,	// Number of outputs being set is out of range.
	kChunkRangeErr					= -7157,	// Handle of plugin settings will not work on a plugin.
	
	// driver call errs: -7200..-7249
	kBadDriverRefNum 				= -7200,	// Plugin does not have a valid driver object.
	kBadHardwareRefNum 				= -7201,	// Plugin does not have a valid pointer to a hardware object.  DSPPtr = NULL.
	kBadWidgetRef 					= -7202,	// Widget object is NULL.
	kLoggedExceptionInConnMgr       = -7224,    // Logged  exception caught in Connection Manager
	kUnknownExceptionInConnMgr      = -7225,    // Unknown exception caught in Connection Manager
	
	// Widget control errors: -7300..-7324
	kControlIndexRangeErr 			= -7300,	// Passed control index was out of range (couldn't find control).
	kNotOurControl 					= -7301,	// Passed in control that didn't belong to widget.
	kNullControl 					= -7302,	// Passed in control ref was NULL.
	kControlNumStepsErr             = -7303,	// Control provided an invalid number of steps
	
	// Builtin plugin errors: -7350..-7374
	kUnsupportedBuiltinPlugin 		= -7350,	// Invalid built-in plugin spec.
	kAssertErr						= -7400,
	
	// ASP Processing errors: - 7450..-7499
	kFicProcessStuckInLoop			= -7450,	// Plugin is stuck in a loop for an process pass.
	kFicOutputBoundsNotInited		= -7452,	// Plugin needs to set output connections to valid range within InitOutputBounds.
	kFicConnectionBufferOverwrite	= -7453,	// Plugin overwrote the end of the connection buffer.
	kFicNoASPBounds					= -7454,	// Start and end bounds for an ASP process or analysis were equal.
	kFicASPDoneProcessing			= -7456,	// The ASP terminated processing with no errors.
	kFicASPErrorWritingToDisk		= -7457,	// ASP encountered error while writing audio data to disk.
	kFicASPOutputFileTooLarge       = -7458,	// ASP tried to write a file larger than the 2^31 bytes in size.
	kFicASPOverwriteOnUnity			= -7459,	// ASP tried to write destructively to Unity
	
	// Errors called from Failure Handler routines.
	kUnknownErr			= -7401					// Plugin caught an unknown exception
};

//----------------------------------------------------------------------------
//  Digi Serial Port Errors
//----------------------------------------------------------------------------

enum {
	kFicSerBadParameterPointer			= -7500,
	kFicSerBadRoutineSelector			= -7501,
	kFicSerPortDoesNotExist				= -7502,
	kFicSerPortAlreadyInUse				= -7503,
	kFicSerPortNotOpen					= -7504,
	kFicSerBadPortRefereceNumber		= -7505
};

// Play nice with emacs
// Local variables:
// mode:c++
// End:

*/


// AAXH.h
/*
enum
{
	kAAXH_Result_NoErr = 0,
	kAAXH_Result_Error_Base = -14000,			// ePSError_Base_AAXHost
	//	kAAXH_Result_Error =							kAAXH_Result_Error_Base - 0,
	kAAXH_Result_Warning =							kAAXH_Result_Error_Base - 1,
	kAAXH_Result_UnsupportedPlatform =				kAAXH_Result_Error_Base - 3,
	kAAXH_Result_EffectNotRegistered =				kAAXH_Result_Error_Base - 4,
	kAAXH_Result_IncompleteInstantiationRequest =	kAAXH_Result_Error_Base - 5,
	kAAXH_Result_NoShellMgrLoaded =					kAAXH_Result_Error_Base - 6,
	kAAXH_Result_UnknownExceptionLoadingTIPlugIn =	kAAXH_Result_Error_Base - 7,
	kAAXH_Result_EffectComponentsMissing =			kAAXH_Result_Error_Base - 8,
	kAAXH_Result_BadLegacyPlugInIDIndex =			kAAXH_Result_Error_Base - 9,
	kAAXH_Result_EffectFactoryInitedTooManyTimes =	kAAXH_Result_Error_Base - 10,
	kAAXH_Result_InstanceNotFoundWhenDeinstantiating = kAAXH_Result_Error_Base - 11,
	kAAXH_Result_FailedToRegisterEffectPackage =	kAAXH_Result_Error_Base - 12,
	kAAXH_Result_PlugInSignatureNotValid =			kAAXH_Result_Error_Base - 13,
	kAAXH_Result_ExceptionDuringInstantiation =		kAAXH_Result_Error_Base - 14,
	kAAXH_Result_ShuffleCancelled =					kAAXH_Result_Error_Base - 15,
	kAAXH_Result_NoPacketTargetRegistered =			kAAXH_Result_Error_Base - 16,
	kAAXH_Result_ExceptionReconnectingAfterShuffle = kAAXH_Result_Error_Base - 17,
	kAAXH_Result_EffectModuleCreationFailed =		kAAXH_Result_Error_Base - 18,
	kAAXH_Result_AccessingUninitializedComponent =	kAAXH_Result_Error_Base - 19,
	kAAXH_Result_TIComponentInstantiationPostponed = kAAXH_Result_Error_Base - 20,
	kAAXH_Result_FailedToRegisterEffectPackageNotAuthorized =	kAAXH_Result_Error_Base - 21,
	kAAXH_Result_FailedToRegisterEffectPackageWrongArchitecture =	kAAXH_Result_Error_Base - 22,
    kAAXH_Result_PluginBuiltAgainstIncompatibleSDKVersion = kAAXH_Result_Error_Base - 23,
    kAAXH_Result_RequiredProperyMissing =                   kAAXH_Result_Error_Base - 24,
    kAAXH_Result_ObjectCopyFailed =                 kAAXH_Result_Error_Base - 25,
	kAAXH_Result_CouldNotGetPlugInBundleLoc =       kAAXH_Result_Error_Base - 26,
	kAAXH_Result_CouldNotFindExecutableInBundle =   kAAXH_Result_Error_Base - 27,
	kAAXH_Result_CouldNotGetExecutableLoc =         kAAXH_Result_Error_Base - 28,
    
	kAAXH_Result_InvalidArgumentValue =				kAAXH_Result_Error_Base - 100,	// WARNING: Overlaps with eTISysErrorBase
	kAAXH_Result_NameNotFoundInPageTable =			kAAXH_Result_Error_Base - 101	// WARNING: Overlaps with eTISysErrorNotImpl
};

*/


// PlatformSupport_Error.h
/*
enum
{
	ePSError_None					= 0,
	ePSError_Base_DSI				= -1000,			// DaeStatus.h
	ePSError_Base_DirectIO			= -6000,			// DirectIODefs.h
	ePSError_Base_DirectMIDI		= -6500,			// DirectIODefs.h
	
	ePSError_Base_DAE_Plugins		= -7000,			// FicErrors.h
	ePSError_Base_DAE_Disk			= -8000,			// FicErrors.h
	ePSError_Base_DAE_General		= -9000,			// FicErrors.h
	ePSError_Base_DAE_DCM			= -11000,			// FicErrors.h
	ePSError_General_PLEASESTOPUSINGTHIS = -12000,
	ePSError_Generic_PLEASESTOPUSINGTHIS = -12001,
	ePSError_OutOfMemory			= -12002,
	ePSError_OutOfHardwareMemory	= -12003,
	ePSError_FixedListTooSmall		= -12004,
	ePSError_FileNotFound			= -12005,
	ePSError_Timeout				= -12006,
	ePSError_FileReadError			= -12007,
	ePSError_InvalidArgs			= -12008,
	
	ePSError_DEXBase_Interrupts		= -12100,
	ePSError_DEXBase_PCI			= -12200,
	ePSError_DEXBase_Task			= -12300,
	ePSError_DEXBase_Console		= -12400,
	ePSError_Base_PalmerEngine		= -12500,
	ePSError_Base_IP				= -12600,
	ePSError_Base_DEXLoader			= -12700,
	ePSError_Base_DEXDebugger		= -12800,
	ePSError_Base_DEXDLLLoader		= -12900,
	ePSError_Base_Thread			= -13000,
	ePSError_Base_Hardware			= -13100,
	ePSError_Base_TMS				= -13400,			// TMSErrors.h
	ePSError_Base_Harpo				= -13500,			// Dhm_HarpoInterface.h
	ePSError_Base_FlashProgram		= -13600,			// Hampton_HostFPGAProgramming.h
	ePSError_Base_Balance			= -13700,			// Dhm_Balance.h
	ePSError_Base_CTIDSP			= -13800,			// Dhm_Core_TIDSP.h
	ePSError_Base_ONFPGASerial		= -13900,			// Dhm_COnFPGASerialController.h
	ePSError_Base_AAXHost			= -14000,			// AAXH.h
	ePSError_Base_TISys				= -14100,			// TISysError.h
	ePSError_Base_DIDL				= -14200,			// DIDL.h
	ePSError_Base_TIDSPMgr			= -14300,			// TIDspMgrAllocationReturnCodes.h
	ePSError_Base_Berlin			= -14400,			// Dhm_Berlin.h
	ePSError_Base_Isoch				= -14500,			// Dhm_IsochEngine.h
	ePSError_SuppHW_NotSupported	= -14600,			// Dhm_SuppHW.h
	
	// Add new ranges here...
	
	ePSError_Base_AAXPlugIns		= -20000,			// AAX_Errors.h
	
	ePSError_Base_DynamicErrors		= -30000,			// Dynamically Generated error tokens
	
	
	
	ePSError_Base_GenericErrorTranslations = -21000,	//	these errors used to be ePSError_Generic_PLEASESTOPUSINGTHIS - splitting into unique error codes
	//	putting this out in space in case anyone's using other numbers on another branch
	ePSError_CEthDCMDeviceInterface_CreatePort_UncaughtException													= -21001,
	ePSError_CEthDCMDeviceInterface_DestroyPort_UncaughtException													= -21002,
	ePSError_CEEPro1000Imp_InitializeAndAllocateBuffers_NullE1000State												= -21003,
	ePSError_CIODeviceOverviewsManager_OvwDataThreadNull															= -21004,
	ePSError_CIODeviceOverviewsManager_ThreadAlreadyRunning															= -21005,
	ePSError_CPalmerEngineKernelImp_CreateIsochronousStream_PalmerEngineIsCurrentlyShuttingDown						= -21006,
	ePSError_CPalmerEngineKernelImp_SetStreamEnabledState_PalmerEngineIsCurrentlyShuttingDown						= -21007,
	ePSError_CPalmerEngineImplementation_StartOperating_DidNotFindPartnerInTime										= -21008,
	ePSError_CPalmerEngineImplementation_TransmitAsyncMessage_PalmerEngineIsCurrentlyShuttingDown					= -21009,
	ePSError_CPalmerEngineImplementation_TransmitAsyncMessageAndWaitForReply_PalmerEngineIsCurrentlyShuttingDown	= -21010,
	ePSError_CPalmerEngineImplementation_TransmitAsyncMessageAndWaitForReply_PalmerEngineIsShuttingDownAfterReply	= -21011,
	ePSError_CPalmerEngineImplementation_TransmitGeneralAsyncPacket_PalmerEngineIsShuttingDown						= -21012,
	ePSError_CEthernetDeviceSimpleImp_InitializeAndAllocateBuffers_FailedToGetBufferInfo							= -21013,
	ePSError_CPEInterface_Imp_GetFeatureSetList_FailedWinGetResourceOfModuleByName									= -21014,
	ePSError_CPEInterface_Imp_GetFourPartVersion_FailedWinGetVersionOfModuleByName									= -21015,
	ePSError_CHamptonHostDEXLifeLine_Common_TransmitMessageAndGetReply_TransmitAndWaitForReplyFailed				= -21016,
	ePSError_CHamptonHostDEXLifeLine_Common_TransmitMessageAndGetReply_ConnectionClosedOrNotEstablished				= -21017,
	ePSError_CHamptonHostDEXLifeLine_Common_TransmitMessageAndGetReply_GotUnexpectedReply							= -21018,
	ePSError_PerformLoadNotSupportedOnMac																			= -21019,
	ePSError_PerformLoad_FailedGetUnusedUDPPort																		= -21020,
	ePSError_PerformLoad_FailedCreateLocalUDPEndPoint																= -21021,
	ePSError_PerformLoad_FailedCreateRemoteUDPEndPoint																= -21022,
	ePSError_PerformLoad_FailedToGetPacketFromEndpoint																= -21023,
	ePSError_PerformLoad_FirstPacketContainsUnexpectedData															= -21024,
	ePSError_PerformLoad_SecondPacketContainsUnexpectedData															= -21025,
	ePSError_PerformLoad_FailedToGetCorrectPacketFromEndpoint														= -21026,
	ePSError_HamptonDEXLoader_LoadOverUDP_UpdateImageBootInterfaceHeaderFailed										= -21027,
	ePSError_HamptonDEXLoader_ResetOverUDP_FailedGetUnusedUDPPort													= -21028,
	ePSError_HamptonDEXLoader_ResetOverUDP_FailedCreateLocalUDPEndPoint												= -21029,
	ePSError_CTask_Imp_SetSchedulingParameters_FailedThreadSpecificDataInit											= -21030,
	ePSError_CTask_Imp_SetSchedulingParameters_FailedToSetFirstThreadPriority										= -21031,
	ePSError_CTask_Imp_SetSchedulingParameters_FailedToSetSecondThreadPriority										= -21032,
	ePSError_CTask_Imp_SetSchedulingParameters_FailedToVerifyNewPolicy												= -21033,
	ePSError_CTask_Imp_SetSchedulingParameters_FailedToSetTimeshareToFalse											= -21034,
	ePSError_CTask_Imp_SetSchedulingParameters_FailedToGetThreadPolicy												= -21035,
	ePSError_CTask_Imp_SetSchedulingParameters_FailedToSetThirdThreadPriority										= -21036,
	ePSError_CTask_Imp_SetSchedulingParameters_FailedToGetThreadPolicyAgain											= -21037,
	ePSError_CModule_Hardware_Imp_GetHardwareMemoryAvailable_WinError												= -21038,
	ePSError_CModule_Hardware_Imp_SetHardwareMemoryRequired_WinError												= -21039,
	ePSError_Win_CModule_Hardware_Imp_MapAndGetDALDevices_MapIOCTLFailed											= -21040,
	ePSError_CModule_Hardware_Imp_ThreadMethod_CreateDALHandleFailed												= -21041,
	ePSError_CSyncPrim_Semaphore_Imp_CSyncPrim_Semaphore_Imp_CreateSemaphoreFailed									= -21042,
	ePSError_CSyncPrim_Event_Imp_CSyncPrim_Event_Imp_CreateEventFailed												= -21043,
	ePSError_CTask_Imp_SetSchedulingParameters_gSetInfoThreadProcNotSet												= -21044,
	ePSError_CTask_Imp_SetSchedulingParameters_SetThreadPriorityFailed												= -21045,
	ePSError_CTask_Imp_SetProcessorAffinityMask_SetThreadAffinityMaskFailed											= -21046,
	ePSError_PSThreadTable_VerifyTableEntryExists_NotFound															= -21047,
	ePSError_PSM_SimpleThread_ThreadMethod_RunThrewException														= -21048,
	ePSError_Hampton_DEXImage_MakeROM_BadFilename																	= -21049,
	ePSError_MakeDllIntoHex_BadFilename																				= -21050,
	ePSError_MakeDllIntoHex_BadPayloadObject																		= -21051,
	ePSError_MakeDllIntoHex_FailedCreatePEInterface																	= -21052,
	ePSError_MakeDllIntoHex_FailedResolvedAllSymbols																= -21053,
	ePSError_MakeDllIntoHexWithStdCLib_BadFilename																	= -21054,
	ePSError_MakeDllIntoHexWithStdCLib_NULLDEXImages																= -21055,
	ePSError_CDEXWin32Kernel_ExceptionsModule_Initialize_FailedToCreateTLSContext									= -21056,
	ePSError_CDEXIP_ARP_Imp_GetMACForGivenIP_IPAddressMaskBad														= -21057,
	ePSError_CDEXIP_ARP_Imp_GetMACForGivenIP_IPAddressInvalid														= -21058,
	ePSError_DEXIntegrityCheck_VerifySection_FailureCheckingSectionCookies											= -21059,
	ePSError_DEXIntegrityCheck_VerifySection_FailureCheckingSectionBufferCookie										= -21060,
	ePSError_DEXIntegrityCheck_VerifyTextSection_FailedChecksum														= -21061,
	ePSError_Mac_CModule_Hardware_Imp_MapAndGetDALDevices_MapIOCTLFailed											= -21062,
	ePSError_CModule_Hardware_Imp_ThreadMethod_mach_port_allocate_failed											= -21063,
	ePSError_DEXTool_main_ExceptionThrown																			= -21064,
	ePSError_Hampton_DEXImage_MakeHexIntoBin_HEXFileNameVersion_StandardExceptionThrown								= -21065,
	ePSError_Hampton_DEXImage_MakeHexIntoBin_HEXFileNameVersion_UnknownExceptionThrown								= -21066,
	ePSError_Hampton_DEXImage_MakeHexIntoBin_HEXDataVersion_StandardExceptionThrown									= -21067,
	ePSError_Hampton_DEXImage_MakeHexIntoBin_HEXDataVersion_UnknownExceptionThrown									= -21068
};
*/

// TISysError.h
/*
///
/// TISysError contains shared error codes used by TIShell and TIDspMgr.
/// Be sure to add default text below when adding new codes
///
enum
{
	eTISysErrorSuccess									= 0,						///< success code
	eTISysErrorBase										= ePSError_Base_TISys,		///< -14100 see PlatformSupport_Error.h
	eTISysErrorNotImpl									= eTISysErrorBase - 1,		///< not implemented
	eTISysErrorMemory									= eTISysErrorBase - 2,		///< out of memory
	eTISysErrorParam									= eTISysErrorBase - 3,		///< invalid parameter
	eTISysErrorNull										= eTISysErrorBase - 4,		///< NULL value
	eTISysErrorCommunication							= eTISysErrorBase - 5,		///< Communication problem with Shell
	eTISysErrorIllegalAccess							= eTISysErrorBase - 6,
	eTISysErrorDirectAccessOfFifoBlocksUnsupported		= eTISysErrorBase - 7,
	eTISysErrorPortIdOutOfBounds						= eTISysErrorBase - 8,
	eTISysErrorPortTypeDoesNotSupportDirectAccess		= eTISysErrorBase - 9,
	eTISysErrorFIFOFull									= eTISysErrorBase - 10,		///< FIFO doesn't have capacity
	eTISysErrorRPCTimeOutOnDSP							= eTISysErrorBase - 11,
	eTISysErrorShellMgrChip_SegsDontMatchAddrs			= eTISysErrorBase - 12,
	eTISysErrorOnChipRPCNotRegistered					= eTISysErrorBase - 13,
	eTISysErrorUnexpectedBufferLength					= eTISysErrorBase - 14,
	eTISysErrorUnexpectedEntryPointName					= eTISysErrorBase - 15,
	eTISysErrorPortIDTooLargeForContextBlock			= eTISysErrorBase - 16,
	eTISysErrorMixerDelayNotSupportedForPlugIns			= eTISysErrorBase - 17,
	eTISysErrorShellFailedToStartUp						= eTISysErrorBase - 18,
	eTISysErrorUnexpectedCondition						= eTISysErrorBase - 19,
	eTISysErrorShellNotRunningWhenExpected				= eTISysErrorBase - 20,
	eTISysErrorFailedToCreateNewPIInstance				= eTISysErrorBase - 21,
	eTISysErrorUnknownPIInstance						= eTISysErrorBase - 22,
	eTISysErrorTooManyInstancesForSingleBufferProcessing = eTISysErrorBase - 23,
	eTISysErrorNoDSPs									= eTISysErrorBase - 24,
	eTISysBadDSPID										= eTISysErrorBase - 25,
	eTISysBadPIContextWriteBlockSize					= eTISysErrorBase - 26,
	eTISysInstanceInitFailed							= eTISysErrorBase - 28,
	eTISysSameModuleLoadedTwiceOnSameChip				= eTISysErrorBase - 29,
	eTISysCouldNotOpenPlugInModule						= eTISysErrorBase - 30,
	eTISysPlugInModuleMissingDependcies					= eTISysErrorBase - 31,
	eTISysPlugInModuleLoadableSegmentCountMismatch		= eTISysErrorBase - 32,
	eTISysPlugInModuleLoadFailure						= eTISysErrorBase - 33,
	eTISysOutOfOnChipDebuggingSpace						= eTISysErrorBase - 34,
	eTISysMissingAlgEntryPoint							= eTISysErrorBase - 35,
	eTISysInvalidRunningStatus							= eTISysErrorBase - 36,
	eTISysExceptionRunningInstantiation					= eTISysErrorBase - 37,
	eTISysTIShellBinaryNotFound							= eTISysErrorBase - 38,
	eTISysTimeoutWaitingForTIShell						= eTISysErrorBase - 39,
	eTISysSwapScriptTimeout								= eTISysErrorBase - 40,
	eTISysTIDSPModuleNotFound							= eTISysErrorBase - 41,
	eTISysTIDSPReadError								= eTISysErrorBase - 42,
	
};

*/

/// @cond ignore
#endif // AAX_ERRORS_H
/// @endcond
