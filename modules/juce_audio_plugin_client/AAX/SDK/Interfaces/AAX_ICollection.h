/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_ICollection.h
 *
 *	\brief Interface to represent a plug-in binary's static description
 *
 */ 
/*================================================================================================*/


#ifndef AAX_ICOLLECTION_H
#define AAX_ICOLLECTION_H

#include "AAX.h"

class AAX_IEffectDescriptor;
class AAX_IPropertyMap;
class AAX_IDescriptionHost;
class IACFDefinition;

/** @brief Interface to represent a plug-in binary's static description
	
	@details	
	\hostimp
	
	The \ref AAX_ICollection interface provides a creation function for new plug-in descriptors, 
	which in turn provides access to the various interfaces necessary for describing a plug-in. 
	When a plug-in description is complete, it is added to the collection via the \ref AddEffect
	method.	The \ref AAX_ICollection interface also provides some additional description methods that
	are used to describe the overall plug-in package. These methods can be used to describe the
	plug-in	package's name, the name of the plug-in's manufacturer, and the plug-in package
	version.
	
	\legacy The information in AAX_ICollection is roughly analogous to the information
	provided by CProcessGroup in the legacy plug-in library
		
	\ingroup CommonInterface_Describe
*/
class AAX_ICollection
{
public:	
	virtual ~AAX_ICollection() {}
	
public: // AAX_IACFCollection
	
	/** @brief	Create a new Effect descriptor.
			
	*/
	virtual	AAX_IEffectDescriptor *	NewDescriptor () = 0;
	
	/** @brief	Add an Effect description to the collection.
	 
			Each Effect that a plug-in registers with \ref AAX_ICollection::AddEffect() is considered
			a completely different user-facing product. For example, in Avid's Dynamics III plug-in the
			Expander, Compressor, and DeEsser are each registered as separate Effects. All stem format
			variations within each Effect are registered within that Effect's \ref AAX_IEffectDescriptor
			using \ref AAX_IEffectDescriptor::AddComponent() "AddComponent()".
	 
			The \ref AAX_eProperty_ProductID value for all ProcessProcs within a single Effect must be
			identical.
	 
			This method passes ownership of an \ref AAX_IEffectDescriptor object to the
			\ref AAX_ICollection. The \ref AAX_IEffectDescriptor must not be deleted by the %AAX plug-in,
			nor should it be edited in any way after it is passed to the \ref AAX_ICollection.
			
			@param[in]	inEffectID
						The effect ID.
			@param[in]	inEffectDescriptor
						The Effect descriptor.
			
	*/
	virtual	AAX_Result				AddEffect ( const char * inEffectID, AAX_IEffectDescriptor* inEffectDescriptor ) = 0;
	
	/** @brief	Set the plug-in manufacturer name.
			
			@param[in]	inPackageName
						The name of the manufacturer.
			
	*/
	virtual	AAX_Result				SetManufacturerName( const char* inPackageName ) = 0;
	/** @brief	Set the plug-in package name.
			
			May be called multiple times to add abbreviated package names.
			
			\note Every plug-in must include at least one name variant with 16
			or fewer characters, plus a null terminating character. Used for Presets folder.
			
			@param[in]	inPackageName
						The name of the package.
			
	*/
	virtual	AAX_Result				AddPackageName( const char *inPackageName ) = 0;
	/** @brief	Set the plug-in package version number.
			
			@param[in]	inVersion
						The package version numner.
			
	*/
	virtual	AAX_Result				SetPackageVersion( uint32_t inVersion ) = 0;
    	/** @brief	Create a new property map
			
						
	*/
	virtual AAX_IPropertyMap *					NewPropertyMap () = 0;
	/** @brief	Set the properties of the collection
	
			@param[in]	inProperties
						Collection properties
		
			
	*/
	virtual AAX_Result							SetProperties ( AAX_IPropertyMap * inProperties ) = 0;
	
	/** @brief	Get the current version of the host

			See \ref AAXATTR_Client_Version for information about the version data format

			@param[in]	outVersion
						Host version
			
	*/
	virtual AAX_Result GetHostVersion(uint32_t* outVersion) const = 0;

public: // AAX_ICollection
	
	/** Get a pointer to an \ref AAX_IDescriptionHost, if supported by the host
	 
	 This interface is served by the \ref AAX_ICollection in order to avoid requiring a new
	 method prototype for the \ref GetEffectDescriptions() method called from the %AAX Library.
	 
	 @sa \ref AAX_UIDs.h for available feature UIDs, e.g. \ref AAXATTR_ClientFeature_AuxOutputStem
	 */
	virtual AAX_IDescriptionHost* DescriptionHost() = 0;
	virtual const AAX_IDescriptionHost* DescriptionHost() const = 0; ///< \copydoc AAX_ICollection::DescriptionHost()
	
	/** Get a pointer to an \ref IACFDefinition, if supported by the host
	 
	 This interface is served by the \ref AAX_ICollection in order to avoid requiring a new
	 method prototype for the \ref GetEffectDescriptions() method called from the %AAX Library.
	 
	 @sa \ref AAX_UIDs.h for available host attribute UIDs, e.g. \ref AAXATTR_Client_Level
	 
	 The implementation of \ref AAX_ICollection owns the referenced object. No AddRef occurs.
	 
	 \ref IACFDefinition::DefineAttribute() is not supported on this object
	 */
	virtual IACFDefinition* HostDefinition() const = 0;
};

#endif
