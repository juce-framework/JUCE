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
 *	\file   AAX_IPropertyMap.h
 *	
 *	\brief	Generic plug-in description property map
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IPROPERTYMAP_H
#define AAX_IPROPERTYMAP_H

#include "AAX_Properties.h"
#include "AAX.h"

class IACFUnknown;

/**
 	\brief Generic plug-in description property map
 	
 	\details
 	\hostimp
 	
	Property Maps are used to associate specific sets of properties with plug-in description
	interfaces.  For example, an audio processing component might register mono and stereo callbacks,
	or Native and TI callbacks, assigning each \c ProcessProc the applicable property mapping.
	This allows the host to determine the correct callback to use depending on the environment in
	which the plug-in is instantiated.
		
	%AAX does not require that every value in %AAX IPropertyMap be assigned by the developer.
	Unassigned properties do not have defined default values; if a
	specific value is not assigned to one of an element's properties then the element must 
	support any value for that property.  For example, if an audio processing component does
	not define its callback's audio buffer length property, the host will assume that the callback
	will support any buffer length.
 
	- To create a new property map: \ref AAX_IComponentDescriptor::NewPropertyMap()
	- To copy an existing property map: \ref AAX_IComponentDescriptor::DuplicatePropertyMap()
 
 	\ingroup CommonInterface_Describe
 
 */
class AAX_IPropertyMap 
{
public:
	virtual ~AAX_IPropertyMap() {}
	
	
	//
	// AAX_IACFPropertyMap methods
	//
	
	/** @brief	Get a property value from a property map
			
			Returns true if the selected property is supported, false if it is not
			
			@param[in]	inProperty
						The property ID
			@param[out]	outValue
						The property value
			
			
	*/
	virtual AAX_CBoolean		GetProperty ( AAX_EProperty inProperty, AAX_CPropertyValue * outValue ) const = 0;
	/** @brief	Get a property value from a property map with a pointer-sized value
	 
			Returns true if the selected property is supported, false if it is not
	 
			@param[in]	inProperty
						The property ID
			@param[out]	outValue
						The property value
	 
	 
	 */
	virtual AAX_CBoolean		GetPointerProperty ( AAX_EProperty inProperty, const void** outValue ) const = 0;
	/** @brief	Add a property to a property map
			
			\note This method may return an error if adding the property was unsuccessful. If there is a
			failure when adding a required property then registration of the relevant description element
			must be abandoned and the plug-in's description logic should proceed to the next element.
	 
			@param[in]	inProperty
						The property ID.
			@param[in]	inValue
						
			
	*/
	virtual AAX_Result			AddProperty ( AAX_EProperty inProperty, AAX_CPropertyValue inValue ) = 0;
	/** @brief	Add a property to a property map with a pointer-sized value
	 
			Use this method to add properties which require a pointer-sized value. Do not use this
			method to add a property unless a pointer-sized value is explicitly specified in the
			property documentation.
	 
			\note This method may return an error if adding the property was unsuccessful. If there is a
			failure when adding a required property then registration of the relevant description element
			must be abandoned and the plug-in's description logic should proceed to the next element.
	 
			@param[in]	inProperty
						The property ID.
			@param[in]	inValue
	 
	 */
	virtual AAX_Result			AddPointerProperty ( AAX_EProperty inProperty, const void* inValue ) = 0;
	virtual AAX_Result			AddPointerProperty ( AAX_EProperty inProperty, const char* inValue ) = 0; ///< @copydoc AddPointerProperty(AAX_EProperty, const void*)
	/** @brief	Remove a property from a property map
			
			@param[in]	inProperty
						The property ID.
	*/
	virtual AAX_Result			RemoveProperty ( AAX_EProperty inProperty ) = 0;
    
	/** @brief	Add an array of plug-in IDs to a property map
	 
			 @param[in]	inProperty
						The property ID.
			 @param[in]	inPluginIDs
						An array of \ref AAX_SPlugInIdentifierTriad
			 @param[in]	inNumPluginIDs
						The length of \c iPluginIDs
	 
	 */
    virtual AAX_Result          AddPropertyWithIDArray ( AAX_EProperty inProperty, const AAX_SPlugInIdentifierTriad* inPluginIDs, uint32_t inNumPluginIDs) = 0;
    
	/** @brief	Get an array of plug-in IDs from a property map
	 
			 @param[in]		inProperty
							The property ID.
			 @param[out]	outPluginIDs
							A pointer that will be set to reference an array of \ref AAX_SPlugInIdentifierTriad
			 @param[in]		outNumPluginIDs
							The length of \c oPluginIDs
	 
	 */
    virtual AAX_CBoolean        GetPropertyWithIDArray ( AAX_EProperty inProperty, const AAX_SPlugInIdentifierTriad** outPluginIDs, uint32_t* outNumPluginIDs) const = 0;
	
	
	//
	// AAX_IPropertyMap methods
	//
	
	/** Returns the most up-to-date underlying interface
	 */
	virtual IACFUnknown*		GetIUnknown() = 0;
};

#endif // AAX_IPROPERTYMAP_H
