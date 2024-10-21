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
 *	\file   AAX_IEffectDescriptor.h
 *
 *	\brief	Description interface for an effect's (plug-in type's) components
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IEFFECTDESCRIPTOR_H
#define AAX_IEFFECTDESCRIPTOR_H

#include "AAX.h"
#include "AAX_Callbacks.h"

class AAX_IComponentDescriptor;
class AAX_IPropertyMap;

/**
	\brief	Description interface for an effect's (plug-in type's) components
	
	\details
	\hostimp
 
	Each Effect represents a different "type" of plug-in. The host will present different
	Effects to the user as separate products, even if they are derived from the same
	\ref AAX_ICollection description.
	
	\sa \ref AAX_ICollection::AddEffect()
	
 	\ingroup CommonInterface_Describe
 
 */
class AAX_IEffectDescriptor
{
public:
	virtual ~AAX_IEffectDescriptor() {}
	/** @brief	Create an instance of a component descriptor.
			
	*/
	virtual AAX_IComponentDescriptor *			NewComponentDescriptor () = 0;
	/** @brief	Add a component to an instance of a component descriptor.
			
			Unlike with \ref AAX_ICollection::AddEffect(), the \ref AAX_IEffectDescriptor does
			not take ownership of the \ref AAX_IComponentDescriptor that is passed to it in this
			method. The host copies out the contents of this descriptor, and thus the plug-in may
			re-use the same descriptor object when creating additional similar components.
	 
			@param[in]	inComponentDescriptor	
			
	*/
	virtual AAX_Result							AddComponent ( AAX_IComponentDescriptor* inComponentDescriptor ) = 0;
	/** @brief	Add a name to the Effect.
			
			May be called multiple times to add abbreviated Effect names.
			
			\note Every Effect must include at least one name variant with 31
			or fewer characters, plus a null terminating character
			
			@param[in]	inPlugInName
						The name assigned to the plug-in.
	*/
	virtual AAX_Result							AddName ( const char *inPlugInName ) = 0;
	/** @brief	Add a category to your plug-in. See \ref AAX_EPlugInCategory.
			
			@param[in]	inCategory
						One of the categories for the plug-in.
				
	*/
	virtual AAX_Result							AddCategory ( uint32_t inCategory ) = 0;
	/** @brief	Add a category to your plug-in. See \ref AAX_EPlugInCategory.
	 
			@param[in]	inCategory
						One of the categories for the plug-in.
			@param[in]	inParamID
						The parameter ID of the parameter used to bypass the category seciont of the plug-in.
			 
	 */
	virtual AAX_Result							AddCategoryBypassParameter ( uint32_t inCategory, AAX_CParamID inParamID ) = 0;
	/** @brief	Add a process pointer.
			
			@param[in]	inProcPtr
						A process pointer.
			@param[in]	inProcID
						A process ID.			
			
	*/
	virtual AAX_Result							AddProcPtr ( void * inProcPtr, AAX_CProcPtrID inProcID ) = 0;
	/** @brief	Create a new property map
			
						
	*/
	virtual AAX_IPropertyMap *					NewPropertyMap () = 0;
	/** @brief	Set the properties of a new property map.
	
			@param[in]	inProperties
						Description
		
			
	*/
	virtual AAX_Result							SetProperties ( AAX_IPropertyMap * inProperties ) = 0;
	/** @brief	Set resource file info.
			
			@param[in]	inResourceType
						See AAX_EResourceType.
			@param[in]	inInfo
						Definition varies on the resource type.
			
	*/
	virtual AAX_Result							AddResourceInfo ( AAX_EResourceType inResourceType, const char * inInfo ) = 0;
	/** @brief	Add name and property map to meter with given ID.

			@param[in]	inMeterID
						The ID of the meter being described.
			@param[in]	inMeterName
						The name of the meter.
			@param[in]	inProperties
						The property map containing meter related data such as meter type, orientation, etc.

	*/
	virtual AAX_Result							AddMeterDescription( AAX_CTypeID inMeterID, const char * inMeterName, AAX_IPropertyMap * inProperties ) = 0;
	/** @brief	Add a control MIDI node to the plug-in data model.
			
			- This MIDI node may receive note data as well as control data.
			- To send MIDI data to the plug-in's algorithm, use
			  \ref AAX_IComponentDescriptor::AddMIDINode().
			
			\sa \ref AAX_IACFEffectParameters_V2::UpdateControlMIDINodes()
			
			@param[in]	inNodeID
						The ID for the new control MIDI node.
			@param[in]	inNodeType
						The type of the node.
			@param[in]	inNodeName
						The name of the node.
			@param[in]	inChannelMask
						The bit mask for required nodes channels (up to 16) or required global events for global node.
	*/
	virtual AAX_Result							AddControlMIDINode ( AAX_CTypeID inNodeID, AAX_EMIDINodeType inNodeType, const char inNodeName[], uint32_t inChannelMask ) = 0;

};

#endif // AAX_IEFFECTDESCRIPTOR_H
