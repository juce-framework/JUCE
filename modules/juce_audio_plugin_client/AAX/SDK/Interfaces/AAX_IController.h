/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IController.h
 *
 *	\brief Interface for the %AAX host's view of a single instance of an effect
 *
 */ 
/*================================================================================================*/


#ifndef _AAX_ICONTROLLER_H_
#define _AAX_ICONTROLLER_H_

#include "AAX_Properties.h"
#include "AAX_IString.h"
#include "AAX.h"
#include <memory>

// Forward declarations
class AAX_IPageTable;


/**
 *	\brief 	Interface for the %AAX host's view of a single instance of an effect.
 *			Used by both clients of the %AAX host and by effect components.
 *
 *	\details
 *	\hostimp
 */
class AAX_IController
{
public:	
	
		virtual ~AAX_IController(void) {}
	
		/** @name Host information getters
		 *
		 *	Call these methods to retrieve environment and run-time information from the %AAX host.
		 */
		//@{
		virtual AAX_Result	GetEffectID (
			AAX_IString *	outEffectID) const = 0;
		/*!
		 *	\brief CALL: Returns the current literal sample rate
		 *
		 *	\param[out] outSampleRate
		 *		The current sample rate
		 */
		virtual // AAX_VController
		AAX_Result
		GetSampleRate (
			AAX_CSampleRate *outSampleRate ) const = 0;
		/*!
		 *	\brief CALL: Returns the plug-in's input stem format
		 *
		 *	\param[out] outStemFormat
		 *		The current input stem format
		 */
		virtual // AAX_VController
		AAX_Result
		GetInputStemFormat (
			AAX_EStemFormat *outStemFormat ) const = 0;
		/*!
		 *	\brief CALL: Returns the plug-in's output stem format
		 *
		 *	\param[out] outStemFormat
		 *		The current output stem format
		 */
		virtual // AAX_VController
		AAX_Result
		GetOutputStemFormat (
			AAX_EStemFormat *outStemFormat) const = 0;
		/*!
		 *	\brief CALL: Returns the most recent signal (algorithmic) latency that has been
		 *	published by the plug-in
		 *	
		 *	This method provides the most recently published signal latency. The host may not
		 *	have updated its delay compensation to match this signal latency yet, so plug-ins
		 *	that dynamically change their latency using
		 *	\ref AAX_IController::SetSignalLatency() "SetSignalLatency()" should always wait for
		 *	an \ref AAX_eNotificationEvent_SignalLatencyChanged notification before updating its
		 *	algorithm to incur this latency.
		 *
		 *	\sa \ref AAX_IController::SetSignalLatency() "SetSignalLatency()"
		 *
		 *	\param[out] outSamples
		 *		The number of samples of signal delay published by the plug-in
		 */
		virtual 
		AAX_Result	
		GetSignalLatency(
			int32_t* outSamples) const = 0;
		/*!
		 *	\brief CALL: returns the plug-in's current real-time DSP cycle count
		 *
		 *	This method provides the number of cycles that the %AAX host expects the DSP plug-in
		 *	to consume.  The host uses this value when allocating DSP resources for the plug-in.
		 *
		 *	\note A plug-in should never apply a DSP algorithm with more demanding resource
		 *	requirements than what is currently accounted for by the host.  To set a higher cycle
		 *	count value, a plug-in must call \ref AAX_IController::SetCycleCount(), then poll
		 *	\ref AAX_IController::GetCycleCount() until the new value has been applied.  Once the
		 *	host has recognized the new cycle count value, the plug-in may apply the more
		 *	demanding algorithm.
		 *
		 *	\param[in] inWhichCycleCount
		 *		Selector for the requested cycle count metric.  One of:
		 *			\li \ref AAX_eProperty_TI_SharedCycleCount
		 *			\li \ref AAX_eProperty_TI_InstanceCycleCount
		 *			\li \ref AAX_eProperty_TI_MaxInstancesPerChip
		 *	\param[in] outNumCycles
		 *		The current value of the selected cycle count metric
		 *
		 *	\todo PLACEHOLDER - NOT CURRENTLY IMPLEMENTED IN HOST
		 */		
		virtual
		AAX_Result
		GetCycleCount(
			AAX_EProperty inWhichCycleCount,
			AAX_CPropertyValue* outNumCycles) const = 0;
		/*!
		 *	\brief CALL: Returns the current Time Of Day (TOD) of the system
		 *
		 *	This method provides a plug-in the TOD (in samples) of the current system. TOD is the
		 *	number of samples that the playhead has traversed since the beginning of playback.
		 *
		 *	\note The TOD value is the immediate value of the audio engine playhead. This value is
		 *	incremented within the audio engine's real-time rendering context; it is not
		 *	synchronized with non-real-time calls to plug-in interface methods.
		 *
		 *	\param[out] outTODLocation
		 *		The current Time Of Day as set by the host
		 */
		virtual
		AAX_Result
		GetTODLocation (
			AAX_CTimeOfDay* outTODLocation ) const = 0;
		//@} Host information accessors
		
		/** @name Host information setters
		 *
		 *	Call these methods to set dynamic plug-in run-time information on the %AAX host.
		 */
		//@{
		/*!
		 *	\brief CALL: Submits a request to change the delay compensation value that the host uses
		 *	to account for the plug-in's signal (algorithmic) latency
		 *
		 *	This method is used to request a change in the number of samples that the %AAX host
		 *	expects the plug-in to delay a signal.
		 *
		 *	The host is not guaranteed to immediately apply the new latency value. A plug-in should
		 *	avoid incurring an actual algorithmic latency that is different than the latency accounted
		 *	for by the host.
		 *
		 *	To set a new latency value, a plug-in must call \ref AAX_IController::SetSignalLatency(),
		 *	then wait for an \ref AAX_eNotificationEvent_SignalLatencyChanged notification. Once this
		 *	notification has been received, \ref AAX_IController::GetSignalLatency() will reflect the
		 *	updated latency value and the plug-in should immediately apply any relevant algorithmic
		 *	changes that alter its latency to this new value.
		 *
		 *	\warning Parameters which affect the latency of a plug-in should not be made available for
		 *	control through automation. This will result in audible glitches when delay compensation
		 *	is adjusted while playing back automation for these parameters.
		 *
		 *	\param[in] inNumSamples
		 *		The number of samples of signal delay that the plug-in requests to incur
		 */
		virtual 
		AAX_Result	
		SetSignalLatency(
			int32_t inNumSamples) = 0;
		/*!
		 *	\brief CALL: Indicates a change in the plug-in's real-time DSP cycle count
		 *
		 *	This method is used to request a change in the number of cycles that the %AAX host
		 *	expects the DSP plug-in to consume.
		 *
		 *	\note A plug-in should never apply a DSP algorithm with more demanding resource
		 *	requirements than what is currently accounted for by the host.  To set a higher cycle
		 *	count value, a plug-in must call \ref AAX_IController::SetCycleCount(), then poll
		 *	\ref AAX_IController::GetCycleCount() until the new value has been applied.  Once the
		 *	host has recognized the new cycle count value, the plug-in may apply the more
		 *	demanding algorithm.
		 *
		 *	\param[in] inWhichCycleCounts
		 *		Array of selectors indicating the specific cycle count metrics that should be set.
		 *		Each selector must be one of:
		 *			\li \ref AAX_eProperty_TI_SharedCycleCount
		 *			\li \ref AAX_eProperty_TI_InstanceCycleCount
		 *			\li \ref AAX_eProperty_TI_MaxInstancesPerChip
		 *	\param[in] iValues
		 *		An array of values requested, one for each of the selected cycle count metrics.
		 *	\param[in] numValues
		 *		The size of \p iValues
		 *
		 *	\todo PLACEHOLDER - NOT CURRENTLY IMPLEMENTED IN HOST
		 */		
		virtual
		AAX_Result
		SetCycleCount(
			AAX_EProperty* inWhichCycleCounts,
			AAX_CPropertyValue* iValues,
			int32_t numValues) = 0;
		//@} Host information setters

		/** @name Posting methods
		 *
		 *	Call these methods to post new plug-in information to the host's data management system.
		 */
		//@{
		/*!
		 *	\brief CALL: Posts a data packet to the host for routing between plug-in components
		 *
		 *	The posted packet is identified with a \ref AAX_CFieldIndex packet index value, which is
		 *	equivalent to the target data port's identifier.  The packet's
		 *	payload must have the expected size for the given packet index / data port, as defined
		 *	when the port is created in \ref CommonInterface_Describe "Describe".
		 *	See AAX_IComponentDescriptor::AddDataInPort().
		 *
		 *	\warning Any data structures that will be passed between platforms (for example, sent to
		 *	a TI DSP in an %AAX DSP plug-in) must be properly data-aligned for compatibility across
		 *	both platforms. See \ref AAX_ALIGN_FILE_ALG for more information about guaranteeing
		 *	cross-platform compatibility of data structures used for algorithm processing.
		 *
		 *	\note All calls to this method should be made within the scope of
		 *	\ref AAX_IEffectParameters::GenerateCoefficients(). Calls from outside this method may
		 *	result in packets not being delivered. See \ref PT-206161
		 *
		 *	\param[in] inFieldIndex
		 *		The packet's destination port
		 *	\param[in] inPayloadP
		 *		A pointer to the packet's payload data
		 *	\param[in] inPayloadSize
		 *		The size, in bytes, of the payload data
		 */
		virtual // AAX_VController
		AAX_Result
		PostPacket (
			AAX_CFieldIndex	inFieldIndex,
			const void *	inPayloadP, 
			uint32_t		inPayloadSize) = 0;
		//@} Posting methods

		/** @name Notification methods
		 *
		 *	Call these methods to send events among plug-in components
		 */
		//@{
		/*!
		 *	\brief CALL: Dispatch a notification
		 *
		 *   The notification is handled by the host and may be delivered back to other plug-in components such as
		 *   the GUI or data model (via \ref AAX_IEffectGUI::NotificationReceived() or
		 *   \ref AAX_IEffectParameters::NotificationReceived(), respectively) depending on the notification type.
		 *
		 *   The host may choose to dispatch the posted notification either synchronously or asynchronously.
		 *
		 *   See the \ref AAX_ENotificationEvent documentation for more information.
         *
         *   This method is supported by %AAX V2 Hosts only.  Check the return code
         *   on the return of this function.  If the error is \ref AAX_ERROR_UNIMPLEMENTED, your plug-in is being
         *   loaded into a host that doesn't support this feature.
		 *
		 *	\param[in] inNotificationType
		 *		Type of notification to send
		 *	\param[in] inNotificationData
		 *		Block of notification data
		 *	\param[in] inNotificationDataSize
		 *		Size of \p inNotificationData, in bytes
		 */
		virtual	AAX_Result	SendNotification (/* AAX_ENotificationEvent */ AAX_CTypeID inNotificationType, const void* inNotificationData, uint32_t inNotificationDataSize) = 0;
		/*!
		 *	\brief CALL: Sends an event to the GUI (no payload)
		 *
		 *   This version of the notification method is a convenience for notifications which do not take any
		 *   payload data. Internally, it simply calls
		 *   \ref AAX_IController::SendNotification(AAX_CTypeID, const void*, uint32_t) with a null payload.
		 *
		 *	\param[in] inNotificationType
		 *		Type of notification to send
		 */
		virtual	AAX_Result	SendNotification (/* AAX_ENotificationEvent */ AAX_CTypeID inNotificationType) = 0;
		//@} Notification methods

		/** @name Metering methods
		 *
		 *	Methods to access the plug-in's host-managed metering information.
		 *
		 *	\sa \ref AdditionalFeatures_Meters
		 */
		//@{
		/*!
		 *	\brief CALL: Retrieves the current value of a host-managed plug-in meter
		 *
		 *	\param[in] inMeterID
		 *		ID of the meter that is being queried
		 *	\param[out] outMeterValue
		 *		The queried meter's current value
		 */
		virtual AAX_Result	GetCurrentMeterValue ( AAX_CTypeID inMeterID, float * outMeterValue ) const = 0;
		/*!
		 *	\brief CALL: Retrieves the currently held peak value of a host-managed plug-in meter
		 *
		 *	\param[in] inMeterID
		 *		ID of the meter that is being queried
		 *	\param[out] outMeterPeakValue
		 *		The queried meter's currently held peak value
		 */
		virtual AAX_Result	GetMeterPeakValue ( AAX_CTypeID inMeterID, float * outMeterPeakValue ) const = 0;
		/*!
		 *	\brief CALL: Clears the peak value from a host-managed plug-in meter
		 *
		 *	\param[in] inMeterID
		 *		ID of the meter that is being cleared
		 */
		virtual AAX_Result	ClearMeterPeakValue ( AAX_CTypeID inMeterID ) const = 0;
		/*!
		 *	\brief CALL: Retrieves the number of host-managed meters registered by a plug-in.
		 *
		 *	See AAX_IComponentDescriptor::AddMeters().
		 *
		 *	\param[out] outMeterCount
		 *		The number of registered plug-in meters.
		 */
		virtual AAX_Result	GetMeterCount ( uint32_t * outMeterCount ) const = 0;
		/*!
		*	\brief CALL: Retrieves the clipped flag from a host-managed plug-in meter.
		*
		*	See AAX_IComponentDescriptor::AddMeters().
		*
		*	\param[in] inMeterID
		*		ID of the meter that is being queried.
		*	\param[out] outClipped
		*		The queried meter's clipped flag.
		*/
		virtual AAX_Result	GetMeterClipped ( AAX_CTypeID inMeterID, AAX_CBoolean * outClipped ) const = 0;
		/*!
		*	\brief CALL: Clears the clipped flag from a host-managed plug-in meter.
		*
		*	See AAX_IComponentDescriptor::AddMeters().
		*
		*	\param[in] inMeterID
		*		ID of the meter that is being cleared.
		*/
		virtual AAX_Result	ClearMeterClipped ( AAX_CTypeID inMeterID ) const = 0;
		//@} Metering methods


		/** @name MIDI methods
		 *
		 *	Methods to access the plug-in's host-managed MIDI information.
		 *
		 */
		//@{
		/*!
		 *	\brief CALL: Retrieves MIDI packets for described MIDI nodes.
		 *
		 *	\param[out] outPort
		 *		port ID of the MIDI node that has unhandled packet
		 *	\param[out] outPacket
		 *		The MIDI packet
		 */
		virtual AAX_Result	GetNextMIDIPacket ( AAX_CFieldIndex* outPort, AAX_CMidiPacket* outPacket ) = 0;
		//@} MIDI methods
		
        /*!
         *	\brief CALL: Returns the latency between the algorithm normal input samples and the inputs returning from the hyrbid component
         *
         *	This method provides the number of samples that the %AAX host expects the plug-in to delay
         *	a signal.  The host will use this value when accounting for latency across the system.
         *
         *	\note   This value will generally scale up with sample rate, although it's not a simple multiple due to some fixed overhead.
         *          This value will be fixed for any given sample rate regardless of other buffer size settings in the host app.
         *
         *	\param[out] outSamples
         *		The number of samples of hybrid signal delay
		 *
		 *	\ingroup additionalFeatures_Hybrid
         */
        virtual
        AAX_Result GetHybridSignalLatency(int32_t* outSamples) const = 0;
        /*!
         *	\brief CALL: Returns the current automation timestamp if called during the
		 *	\ref AAX_IACFEffectParameters::GenerateCoefficients() "GenerateCoefficients()" call AND the generation
		 *	of coefficients is being triggered by an automation point instead of immediate changes.
         *
         *	\note   This function will return 0 if called from outside of
		 *			\ref AAX_IACFEffectParameters::GenerateCoefficients() "GenerateCoefficients()" or if the
		 *			\ref AAX_IACFEffectParameters::GenerateCoefficients() "GenerateCoefficients()"
		 *			call was initiated due to a non-automated change.  In those cases, you can get your sample offset from the transport
		 *			start using \ref GetTODLocation().
         *
         *	\param[out] outTimestamp
         *		The current coefficient timestamp. Sample count from transport start.
         */
        virtual
        AAX_Result GetCurrentAutomationTimestamp(AAX_CTransportCounter* outTimestamp) const = 0;
        /*!
         *	\brief CALL: Returns name of the host application this plug-in instance is being loaded by.  This string also typically includes version information.
         *
		 *	\compatibility Pro Tools versions from Pro Tools 11.0 to Pro Tools 12.3.1 will return a generic
		 *	version string to this call. This issue is resolved beginning in Pro Tools 12.4.
		 *
         *	\param[out] outHostNameString
         *		The name of the current host application.
         */
        virtual
        AAX_Result GetHostName(AAX_IString* outHostNameString) const = 0;
        /*!
         *	\brief CALL: Returns execution platform type, native or TI
         *
         *	\param[out] outTargetPlatform
         *		The type of the current execution platform as one of \ref AAX_ETargetPlatform.
         */
		virtual
		AAX_Result GetPlugInTargetPlatform(AAX_CTargetPlatform* outTargetPlatform) const = 0;
		/*!
         *	\brief CALL: Returns true for AudioSuite instances
         *
         *	\param[out] outIsAudioSuite
         *		The boolean flag which indicate true for AudioSuite instances.
         */
		virtual 
		AAX_Result GetIsAudioSuite(AAX_CBoolean* outIsAudioSuite) const = 0;

		/**
		 *	\brief Copy the current page table data for a particular plug-in type
		 *
		 *	The host may restrict plug-ins to only copying page table data from certain plug-in types,
		 *	such as plug-ins from the same manufacturer or plug-in types within the same effect.
		 *
		 *	See \ref AAX_Page_Table_Guide for more information about page tables.
		 *	
		 *	\returns A new page table object to which the requested page table data has been copied. Ownership
		 *	of this object passes to the caller.
		 *
		 *	\returns a null pointer if the requested plug-in type is unknown, if
		 *	\p inTableType is unknown or if \p inTablePageSize is not a supported size for the given table
		 *	type.
		 *
		 *	\param[in] inManufacturerID
		 *		\ref AAX_eProperty_ManufacturerID "Manufacturer ID" of the desired plug-in type
		 *	\param[in] inProductID
		 *		\ref AAX_eProperty_ProductID "Product ID" of the desired plug-in type
		 *	\param[in] inPlugInID
		 *		Type ID of the desired plug-in type (\ref AAX_eProperty_PlugInID_Native, \ref AAX_eProperty_PlugInID_TI)
		 *	\param[in] inTableType
		 *		Four-char type identifier for the requested table type (e.g. \c 'PgTL', \c 'Av81', etc.)
		 *	\param[in] inTablePageSize
		 *		Page size for the requested table. Some tables support multiple page sizes.
		 */
		virtual
		AAX_IPageTable*
		CreateTableCopyForEffect(
			AAX_CPropertyValue inManufacturerID,
			AAX_CPropertyValue inProductID,
			AAX_CPropertyValue inPlugInID,
			uint32_t inTableType,
			int32_t inTablePageSize) const = 0;
	
		/**
		 *	\brief Copy the current page table data for a particular plug-in effect and page table layout
		 *
		 *	The host may restrict plug-ins to only copying page table data from certain effects,
		 *	such as effects registered within the current AAX plug-in bundle.
		 *
		 *	See \ref AAX_Page_Table_Guide for more information about page tables.
		 *
		 *	\returns A new page table object to which the requested page table data has been copied. Ownership
		 *	of this object passes to the caller.
		 *
		 *	\returns a null pointer if the requested effect ID is unknown or if \p inLayoutName is not a
		 *	valid layout name for the page tables registered for the effect.
		 *
		 *	\param[in] inEffectID
		 *		Effect ID for the desired effect. See \ref AAX_ICollection::AddEffect()
		 *	\param[in] inLayoutName
		 *		Page table layout name ("name" attribute of the \c PTLayout XML tag)
		 *	\param[in] inTableType
		 *		Four-char type identifier for the requested table type (e.g. \c 'PgTL', \c 'Av81', etc.)
		 *	\param[in] inTablePageSize
		 *		Page size for the requested table. Some tables support multiple page sizes.
		 */
		virtual
		AAX_IPageTable*
		CreateTableCopyForLayout(
			const char * inEffectID,
			const char * inLayoutName,
			uint32_t inTableType,
			int32_t inTablePageSize) const = 0;
	
		/** \copybrief AAX_IController::CreateTableCopyForEffect()
		 *
		 *	\returns A new page table object to which the requested page table data has been copied. Ownership
		 *	of this object passes to the caller.
		 *
		 *	\returns a null pointer if the requested plug-in type is unkown, if
		 *	\p inTableType is unknown or if \p inTablePageSize is not a supported size for the given table type.
		 *	
		 *	\param[in] inPageTableFilePath
		 *		Path to XML page table file.
		 *	\param[in] inFilePathEncoding
		 *		File path text encoding.
		 *	\param[in] inManufacturerID
		 *		\ref AAX_eProperty_ManufacturerID "Manufacturer ID" of the desired plug-in type
		 *	\param[in] inProductID
		 *		\ref AAX_eProperty_ProductID "Product ID" of the desired plug-in type
		 *	\param[in] inPlugInID
		 *		Type ID of the desired plug-in type (\ref AAX_eProperty_PlugInID_Native, \ref AAX_eProperty_PlugInID_TI)
		 *	\param[in] inTableType
		 *		Four-char type identifier for the requested table type (e.g. \c 'PgTL', \c 'Av81', etc.)
		 *	\param[in] inTablePageSize
		 *		Page size for the requested table. Some tables support multiple page sizes.
		*/
		virtual
		AAX_IPageTable*
		CreateTableCopyForEffectFromFile(
			const char* inPageTableFilePath,
			AAX_ETextEncoding inFilePathEncoding,
			AAX_CPropertyValue inManufacturerID,
			AAX_CPropertyValue inProductID,
			AAX_CPropertyValue inPlugInID,
			uint32_t inTableType,
			int32_t inTablePageSize) const = 0;

		/** \copybrief AAX_IController::CreateTableCopyForLayout()
		 *
		 *	\returns A new page table object to which the requested page table data has been copied. Ownership
		 *	of this object passes to the caller.
		 *
		 *	\returns a null pointer if \p inLayoutName is not a valid layout name for the page tables file.
		 *
		 *	\param[in] inPageTableFilePath
		 *		Path to XML page table file.
		 *	\param[in] inFilePathEncoding
		 *		File path text encoding.
		 *	\param[in] inLayoutName
		 *		Page table layout name ("name" attribute of the \c PTLayout XML tag)
		 *	\param[in] inTableType
		 *		Four-char type identifier for the requested table type (e.g. \c 'PgTL', \c 'Av81', etc.)
		 *	\param[in] inTablePageSize
		 *		Page size for the requested table. Some tables support multiple page sizes.
		*/
		virtual
		AAX_IPageTable*
		CreateTableCopyForLayoutFromFile(
			const char* inPageTableFilePath,
			AAX_ETextEncoding inFilePathEncoding,
			const char* inLayoutName,
			uint32_t inTableType,
			int32_t inTablePageSize) const = 0;
};

#endif // #ifndef _AAX_IPLUGIN_H_
