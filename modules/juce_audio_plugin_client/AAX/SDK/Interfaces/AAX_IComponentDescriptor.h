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
 *	\file  AAX_IComponentDescriptor.h
 *
 *	\brief Description interface for an %AAX plug-in algorithm
 *
 */ 
/*================================================================================================*/


#ifndef _AAX_ICOMPONENTDESCRIPTOR_H_
#define _AAX_ICOMPONENTDESCRIPTOR_H_

#include "AAX.h"
#include "AAX_IDma.h"
#include "AAX_Callbacks.h"

class AAX_IPropertyMap;


/** \brief	Description interface for an %AAX plug-in component
		
	\details	
	\hostimp
	
	This is an abstract interface containing everything needed to describe a single 
	algorithm of an Effect.  For more information about algorithm processing in
	%AAX plug-ins, see \ref CommonInterface_Algorithm.
		
	\ingroup CommonInterface_Describe
*/
class AAX_IComponentDescriptor
{
public:
	virtual ~AAX_IComponentDescriptor() {}
		
	/*!
	 *  \brief Clears the descriptor
	 *  
	 *  Clears the descriptor and readies it for the next algorithm description
	 *
	 */	
	virtual AAX_Result					Clear () = 0;
	
	/*!
	 *  \brief Subscribes an audio input context field
	 *  
	 *  Defines an audio in port for host-provided information in the algorithm's
	 *	context structure.
     *
     *  - Data type: float**
     *  - Data kind: An array of float arrays, one for each input channel
	 *
	 *  \param[in] inFieldIndex
	 *		Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
	 */	
    virtual AAX_Result                  AddAudioIn ( AAX_CFieldIndex inFieldIndex ) = 0;

	/*!
	*  \brief Subscribes an audio output context field
	*  
	*  Defines an audio out port for host-provided information in the algorithm's
     *	context structure.
     *
     *  - Data type: float**
     *  - Data kind: An array of float arrays, one for each output channel
	*
	*  \param[in] inFieldIndex
	*		Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
	*/	
    virtual AAX_Result                  AddAudioOut ( AAX_CFieldIndex inFieldIndex ) = 0;

	/*!
	*  \brief Subscribes a buffer length context field
	*  
	*  Defines a buffer length port for host-provided information in the algorithm's
	*  context structure.
	*
	*  - Data type: int32_t*
	*  - Data kind: The number of samples in the current audio buffer
	*
	*  \param[in] inFieldIndex
	*		Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
	*/	
    virtual AAX_Result                  AddAudioBufferLength ( AAX_CFieldIndex inFieldIndex ) = 0;

	/*!
	*  \brief Subscribes a sample rate context field
	*  
	*  Defines a sample rate port for host-provided information in the algorithm's
	*  context structure.
	*
	*  - Data type: \ref AAX_CSampleRate *
	*  - Data kind: The current sample rate
	*
	*  \param[in] inFieldIndex
	*		Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
	*/	
    virtual AAX_Result                  AddSampleRate ( AAX_CFieldIndex inFieldIndex ) = 0;

	/*!
	*  \brief Subscribes a clock context field
	*  
	*  Defines a clock port for host-provided information in the algorithm's
	*  context structure.
	*
	*  - Data type: \ref AAX_CTimestamp *
	*  - Data kind: A running counter which increments even when the transport is not
	*               playing. The counter increments exactly once per sample quantum.
	*
	*  \compatibility As of Pro Tools 11.1, this field may be used in both Native
	*  and DSP plug-ins. The DSP clock data is a 16-bit cycling counter. This field
	*  was only available for Native plug-ins in previous Pro Tools versions.
	*
	*  \param[in] inFieldIndex
	*		Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
	*/	
    virtual AAX_Result                  AddClock ( AAX_CFieldIndex inFieldIndex ) = 0;

	/*!
	*  \brief Subscribes a side-chain input context field
	*  
	*  Defines a side-chain input port for host-provided information in the algorithm's
	*  context structure.
	*
	*  - Data type: int32_t*
	*  - Data kind: The index of the plug-in's first side-chain input channel
    *               within the array of input audio buffers
	*
	*  \param[in] inFieldIndex
	*		Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
	*/	
    virtual AAX_Result                  AddSideChainIn ( AAX_CFieldIndex inFieldIndex ) = 0;

	/*!
	 *  \brief Adds a custom data port to the algorithm context
	 *  
	 *  Defines a read-only data port for plug-in information in the algorithm's
	 *	context structure. The plug-in can send information to this port
	 *	using \ref AAX_IController::PostPacket().
	 *
	 *	The host guarantees that all packets will be delivered to this
	 *	port in the order in which they were posted, up to the point of
	 *	a packet buffer overflow, though some packets may be dropped depending
	 *	on the \p inPortType and host implementation.
	 *
	 *	\note When a plug-in is operating in offline (AudioSuite) mode, all
	 *	data ports operate as \ref AAX_eDataInPortType_Unbuffered ports
	 *
	 *  \param[in] inFieldIndex
	 *		Unique identifier for the port, generated using \ref AAX_FIELD_INDEX
	 *	\param[in] inPacketSize
	 *		Size of the data packets that will be sent to this port
	 *	\param[in] inPortType
	 *		The requested packet delivery behavior for this port
	 */	
	virtual AAX_Result					AddDataInPort ( AAX_CFieldIndex inFieldIndex, uint32_t inPacketSize, AAX_EDataInPortType inPortType = AAX_eDataInPortType_Buffered ) = 0;

	/** @brief	Adds an auxiliary output stem for a plug-in.
	 *
     *  Use this method to add  additional output channels to the algorithm context.
	 *
	 *	The aux output stem audio buffers will be added to the end of the audio outputs array
	 *	in the order in which they are described. When writing audio data to a specific aux
	 *	output, find the proper starting channel by accumulating all of the channels of the
	 *	main output stem format and any previously-described aux output stems.
	 *
	 *	The plug-in is responsible for providing a meaningful name for each aux outputs. At
	 *	the very least, individual outputs should be labeled "Output xx", where "xx" is the
	 *	aux output number as it is defined in the plug-in. The output name should also include
	 *	the words "mono" and "stereo" to support when users are looking for an output with a
	 *	specific stem format.
	 *
	 *	\compatibility There is a hard limit to the number of outputs that Pro Tools supports for a
	 *	single plug-in instance.  This limit is currently set at 256 channels, which includes all of
	 *	the plug-in's output channels in addition to the sum total of all of its aux output stem
	 *	channels.
	 *
	 *	\compatibility Pro Tools supports only mono and stereo auxiliary output stem formats
	 *
	 *	\warning This method will return an error code on hosts which do not support auxiliary
	 *	output stems. This indicates that the host will not provide audio buffers for auxiliary
	 *	output stems during processing. A plug-in must not attempt to write data into auxiliary
	 *	output stem buffers which have not been provided by the host!
     *
	 *  \param[in] inFieldIndex
	 *		DEPRECATED: This parameter is no longer needed by the host, but is included in the interface for binary compatibility
	 *	\param[in] inStemFormat
     *      The stem format of the new aux output
	 *	\param[in] inNameUTF8
     *      The name of the aux output. This name is static and cannot be changed after the descriptor is submitted to the host
	 *		
	 */
	virtual AAX_Result					AddAuxOutputStem ( AAX_CFieldIndex inFieldIndex, int32_t inStemFormat, const char inNameUTF8[]) = 0;
	/*!
	 *  \brief Adds a private data port to the algorithm context
	 *  
	 *  Defines a read/write data port for private state data.
	 *	Data written to this port will be maintained by the host
	 *	between calls to the algorithm context.
     *
	 *	\sa alg_pd_registration
	 *
	 *  \param[in] inFieldIndex
	 *		Unique identifier for the port, generated using \ref AAX_FIELD_INDEX
	 *	\param[in] inDataSize
	 *		Size of the data packets that will be sent to this port
	 *	\param[in]	inOptions
	 *		Options that define the private data port's behavior
	 */
	virtual AAX_Result					AddPrivateData ( AAX_CFieldIndex inFieldIndex, int32_t inDataSize, /* AAX_EPrivateDataOptions */ uint32_t inOptions = AAX_ePrivateDataOptions_DefaultOptions ) = 0;
    
    /*! 
    *   \brief Adds a block of data to a context that is not saved between callbacks and is scaled by the system buffer size.
    *
    *   This can be very useful if you use block processing and need to store intermediate results.  Just specify your base element
    *   size and the system will scale the overall block size by the buffer size.  For example, to create a buffer of floats that is
    *   the length of the block, specify 4 bytes as the elementsize.
    *
    *   This data block does not retain state across callback and can also be reused across instances on memory contrained systems.
    *
    *   \param[in] inFieldIndex
    *       Unique identifier for the port, generated using \ref AAX_FIELD_INDEX
    *   \param[in] inDataElementSize
    *       The size of a single piece of data in the block.  This number will be multipied by the processing block size to determine total block size.
    */
    virtual AAX_Result					AddTemporaryData( AAX_CFieldIndex inFieldIndex, uint32_t inDataElementSize) = 0;

	/**
		\brief Adds a DMA field to the plug-in's context
	 
		DMA (direct memory access) provides efficient reads from and writes to external memory on the
		DSP.  DMA behavior is emulated in host-based plug-ins for cross-platform portability.
		
		\note The order in which DMA instances are added defines their priority and therefore order of
		execution of DMA operations. In most plug-ins, Scatter fields should be placed first in order
		to achieve the lowest possible access latency.
		
		For more information, see \ref additionalFeatures_DMA .
		
		\todo Update the DMA system management such that operation priority can be set arbitrarily
	 
		\param[in]	inFieldIndex
			Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
		\param[in]	inDmaMode
			AAX_IDma::EMode that will apply to this field
				
	*/
	virtual AAX_Result					AddDmaInstance ( AAX_CFieldIndex inFieldIndex, AAX_IDma::EMode inDmaMode ) = 0;
	
	/**	@brief Adds a meter field to the plug-in's context
	
		Meter fields include an array of meter tap values, with one tap per meter per context.  Only
		one meter field should be added per Component.  Individual meter behaviors can be described
		at the Effect level.
	 
		For more information, see \ref AdditionalFeatures_Meters .
		
		\param[in]	inFieldIndex
			Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
		\param[in]	inMeterIDs
			Array of 32-bit IDs, one for each meter.  Meter IDs must be unique within the Effect.
		\param[in]	inMeterCount
			The number of meters included in this field
	 */
	virtual AAX_Result					AddMeters ( AAX_CFieldIndex inFieldIndex, const AAX_CTypeID* inMeterIDs, const uint32_t inMeterCount ) = 0;

	/** @brief	Adds a MIDI node field to the plug-in's context
	 
	 - Data type: \ref AAX_IMIDINode *
	 
	 The resulting MIDI node data will be available both in the algorithm context and in the plug-in's
	 \ref AAX_IEffectParameters "data model" via
	 \ref AAX_IACFEffectParameters_V2::UpdateMIDINodes() "UpdateMIDINodes()".
	 
	 To add a MIDI node that is only accessible to the plug-in's data model, use
	 \ref AAX_IEffectDescriptor::AddControlMIDINode()
	 
	 \compatibility Due to current restrictions MIDI data won't be delivered to DSP algorithms, only to %AAX Native.

		\param[in]	inFieldIndex
			The ID of the port.  MIDI node ports should formatted as a pointer to an \ref AAX_IMIDINode.
		\param[in]	inNodeType
			The type of MIDI node, as \ref AAX_EMIDINodeType
		\param[in]	inNodeName
			The name of the MIDI node as it should appear in the host's UI
		\param[in]	channelMask
			The channel mask for the MIDI node. This parameter specifies used MIDI channels. For Global MIDI nodes, use a mask of \ref AAX_EMidiGlobalNodeSelectors
	*/
	virtual AAX_Result					AddMIDINode ( AAX_CFieldIndex inFieldIndex, AAX_EMIDINodeType inNodeType, const char inNodeName[], uint32_t channelMask ) = 0;

	/*!
	*  \brief Subscribes a context field to host-provided services or information
	*  
	*  \note Currently for internal use only. 
	*
	*  \param[in] inFieldIndex
	*		Unique identifier for the field, generated using \ref AAX_FIELD_INDEX
	*  \param[in] inFieldType
	*		Type of field that is being added
	*/	
	virtual AAX_Result					AddReservedField ( AAX_CFieldIndex inFieldIndex, uint32_t inFieldType ) = 0;

	/** @brief	Creates a new, empty property map.
		
		The component descriptor owns the reference to the resulting property map, and
		the underlying property map is destroyed when the component descriptor is
		released.
				
	*/
	virtual AAX_IPropertyMap *			NewPropertyMap () const = 0; // CONST?
	
	/** @brief	Creates a new property map using an existing property map
	 
	 The component descriptor owns the reference to the resulting property map, and
	 the underlying property map is destroyed when the component descriptor is
	 released.
	 
		\param[in] inPropertyMap
			The property values in this map will be copied into the new map
	 
	 */
	virtual AAX_IPropertyMap *			DuplicatePropertyMap (AAX_IPropertyMap* inPropertyMap) const = 0;
	/** \brief	Registers an algorithm processing entrypoint (process procedure) for the
	 *	native architecture
	 *
	 *	\param[in]	inProcessProc
	 *		Symbol for this processing callback
	 *	\param[in]	inProperties
	 *		A property map for this processing callback. The property map's values are copied
	 *		by the host and associated with the new ProcessProc. The property map contents are
	 *		unchanged and the map may be re-used when registering additional ProcessProcs.
	 *	\param[in]	inInstanceInitProc
	 *		Initialization routine that will be called when a new instance of the Effect
	 *		is created.  See \ref alg_initialization.
	 *	\param[in]	inBackgroundProc
	 *		Background routine that will be called in an idle context within the same
	 *		address space as the associated process procedure.  See
	 *		\ref additionalFeatures_BackgroundProc
	 *	\param[out]	outProcID
	 *		\todo document this parameter
	 */
	virtual AAX_Result					AddProcessProc_Native (	
		AAX_CProcessProc inProcessProc,
		AAX_IPropertyMap * inProperties = NULL,
		AAX_CInstanceInitProc inInstanceInitProc = NULL,
		AAX_CBackgroundProc inBackgroundProc = NULL,
		AAX_CSelector * outProcID = NULL) = 0;
	/** \brief	Registers an algorithm processing entrypoint (process procedure) for the
	 *	native architecture
	 *
	 *	\param[in]	inDLLFileNameUTF8
	 *		UTF-8 encoded filename for the ELF DLL containing the algorithm code fragment
	 *	\param[in]	inProcessProcSymbol
	 *		Symbol for this processing callback
	 *	\param[in]	inProperties
	 *		A property map for this processing callback. The property map's values are copied
	 *		by the host and associated with the new ProcessProc. The property map contents are
	 *		unchanged and the map may be re-used when registering additional ProcessProcs.
	 *	\param[in]	inInstanceInitProcSymbol
	 *		Initialization routine that will be called when a new instance of the Effect
	 *		is created.  Must be included in the same DLL as the main algorithm
	 *		entrypoint.  See \ref alg_initialization.
	 *	\param[in]	inBackgroundProcSymbol
	 *		Background routine that will be called in an idle context within the same
	 *		address space as the associated process procedure.  Must be included in the
	 *		same DLL as the main algorithm entrypoint.  See
	 *		\ref additionalFeatures_BackgroundProc
	 *	\param[out]	outProcID
	 *		\todo document this parameter
	 */
	virtual AAX_Result					AddProcessProc_TI ( 
		const char inDLLFileNameUTF8 [], 
		const char inProcessProcSymbol [], 
		AAX_IPropertyMap * inProperties,
		const char inInstanceInitProcSymbol [] = NULL,
		const char inBackgroundProcSymbol [] = NULL,
		AAX_CSelector * outProcID = NULL) = 0;
	
	/** \brief	Registers one or more algorithm processing entrypoints (process procedures)
	 *
	 *	Any non-overlapping set of processing entrypoints may be specified. Typically this can
	 *	be used to specify both Native and TI entrypoints using the same call.
	 *
	 *	The %AAX Library implementation of this method includes backwards compatibility logic
	 *	to complete the ProcessProc registration on hosts which do not support this method.
	 *	Therefore plug-in code may use this single registration routine instead of separate
	 *	calls to \ref AddProcessProc_Native(), \ref AddProcessProc_TI(), etc. regardless of the
	 *	host version.
	 *
	 *	The following properties replace the input arguments to the platform-specific
	 *	registration methods:
	 *
	 *	\ref AddProcessProc_Native() (\ref AAX_eProperty_PlugInID_Native, \ref AAX_eProperty_PlugInID_AudioSuite)
	 *	- \ref AAX_CProcessProc \c iProcessProc: \ref AAX_eProperty_NativeProcessProc (required)
	 *	- \ref AAX_CInstanceInitProc \c iInstanceInitProc: \ref AAX_eProperty_NativeInstanceInitProc (optional)
	 *	- \ref AAX_CBackgroundProc \c iBackgroundProc: \ref AAX_eProperty_NativeBackgroundProc (optional)
	 *
	 *	\ref AddProcessProc_TI() (\ref AAX_eProperty_PlugInID_TI)
	 *	- <tt>const char inDLLFileNameUTF8[]</tt>: \ref AAX_eProperty_TIDLLFileName (required)
	 *	- <tt>const char iProcessProcSymbol[]</tt>: \ref AAX_eProperty_TIProcessProc (required)
	 *	- <tt>const char iInstanceInitProcSymbol[]</tt>: \ref AAX_eProperty_TIInstanceInitProc (optional)
	 *	- <tt>const char iBackgroundProcSymbol[]</tt>: \ref AAX_eProperty_TIBackgroundProc (optional)
	 *
	 *	If any platform-specific plug-in ID property is present in \p iProperties then
	 *	\ref AddProcessProc() will check for the required properties for that platform.
	 *
	 *	\note \ref AAX_eProperty_AudioBufferLength will be ignored for the Native and AudioSuite ProcessProcs
	 *	since it should only be used for %AAX DSP.
	 *	
	 *	\param[in] inProperties
	 *		A property map for this processing callback. The property map's values are copied
	 *		by the host and associated with the new ProcessProc. The property map contents are
	 *		unchanged and the map may be re-used when registering additional ProcessProcs.
	 *	
	 *	\param[out]	outProcIDs
	 *		\todo document this parameter
	 *		Returned array will be NULL-terminated
	 *	
	 *	\param[in] inProcIDsSize
	 *		The size of the array provided to \p oProcIDs. If \p oProcIDs is non-NULL but \p iProcIDsSize is not
	 *		large enough for all of the registered ProcessProcs (plus one for NULL termination) then this method
	 *		will fail with \ref AAX_ERROR_ARGUMENT_BUFFER_OVERFLOW
	 */
	virtual AAX_Result					AddProcessProc (
		AAX_IPropertyMap* inProperties,
		AAX_CSelector* outProcIDs = NULL,
		int32_t inProcIDsSize = 0) = 0;
	
	/** \brief	Registers an algorithm processing entrypoint (process procedure) for the
	 *	native architecture
	 *
	 *	This template provides an \ref AAX_CALLBACK based interface to the
	 *	\ref AddProcessProc_Native method.
	 *
	 *	\sa \ref AAX_IComponentDescriptor::AddProcessProc_Native(AAX_CProcessProc,AAX_IPropertyMap*,AAX_CInstanceInitProc,AAX_CBackgroundProc,AAX_CSelector*)
	 *
	 *	\param[in]	inProperties
	 *		A property map for this processing callback. The property map's values are copied
	 *		by the host and associated with the new ProcessProc. The property map contents are
	 *		unchanged and the map may be re-used when registering additional ProcessProcs.
	 */
	template <typename aContextType>
	AAX_Result							AddProcessProc_Native (
		void (AAX_CALLBACK *inProcessProc) ( aContextType * const inInstancesBegin [], const void * inInstancesEnd),
		AAX_IPropertyMap * inProperties = NULL,
		int32_t (AAX_CALLBACK *inInstanceInitProc) ( const aContextType * inInstanceContextPtr, AAX_EComponentInstanceInitAction inAction ) = NULL, 
		int32_t (AAX_CALLBACK *inBackgroundProc) ( void ) = NULL );
};

template <typename aContextType>
inline AAX_Result
AAX_IComponentDescriptor::AddProcessProc_Native ( 
	void (AAX_CALLBACK *inProcessProc) ( aContextType * const	inInstancesBegin [], const void * inInstancesEnd),
	AAX_IPropertyMap * inProperties,
	int32_t (AAX_CALLBACK *inInstanceInitProc) ( const aContextType * inInstanceContextPtr, AAX_EComponentInstanceInitAction inAction ),
	int32_t (AAX_CALLBACK *inBackgroundProc) ( void ) )
{
	return this->AddProcessProc_Native( 
		reinterpret_cast <AAX_CProcessProc>( inProcessProc ),
		inProperties,
		reinterpret_cast<AAX_CInstanceInitProc>( inInstanceInitProc ),
		reinterpret_cast<AAX_CBackgroundProc>( inBackgroundProc ) );
}

#endif // #ifndef _AAX_ICOMPONENTDESCRIPTOR_H_
