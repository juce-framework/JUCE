/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFViewContainer.h
 *
 *	\brief Interface for the %AAX host's view of a single instance of an 
 *	effect.  Used by both clients of the AAXHost and by effect components.
 *
 */ 
/*================================================================================================*/


#ifndef _AAX_IACFVIEWCONTAINER_H_
#define _AAX_IACFVIEWCONTAINER_H_

#include "AAX_GUITypes.h"
#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"

/** \brief Interface for the %AAX host's view of a single instance of an 
 *	effect.  Used by both clients of the host app and by effect components.
 *	
 *	\details
 *	\sa \ref AAX_IViewContainer
 */
class AAX_IACFViewContainer : public IACFUnknown
{
public:	
	/** @name View and GUI state queries
	 */
	//@{
	virtual int32_t		GetType () = 0;	///< \copydoc AAX_IViewContainer::GetType()
    virtual void *		GetPtr () = 0;	///< \copydoc AAX_IViewContainer::GetPtr()
	virtual AAX_Result	GetModifiers ( uint32_t * outModifiers ) = 0;	///< \copydoc AAX_IViewContainer::GetModifiers()
	//@}end View and GUI state queries

	/** @name View change requests
	 */
	//@{
    virtual AAX_Result	SetViewSize ( AAX_Point & inSize ) = 0;	///< \copydoc AAX_IViewContainer::SetViewSize()
	//@}end View change requests
	
	/** @name Host event handlers
	 */
	//@{	
	virtual AAX_Result	HandleParameterMouseDown	( AAX_CParamID inParamID, uint32_t inModifiers ) = 0;	///< \copydoc AAX_IViewContainer::HandleParameterMouseDown()
	virtual AAX_Result	HandleParameterMouseDrag	( AAX_CParamID inParamID, uint32_t inModifiers ) = 0;	///< \copydoc AAX_IViewContainer::HandleParameterMouseDrag()
	virtual AAX_Result	HandleParameterMouseUp		( AAX_CParamID inParamID, uint32_t inModifiers ) = 0;	///< \copydoc AAX_IViewContainer::HandleParameterMouseUp()
	//@}end Host event handlers
};


/** \brief Supplemental interface for the %AAX host's view of a single instance of an 
 *	effect.  Used by both clients of the host app and by effect components.
 *	
 *	\details
 *	\sa \ref AAX_IViewContainer
 */
class AAX_IACFViewContainer_V2 : public AAX_IACFViewContainer
{
public:	
	/** @name Host event handlers
	 */
	//@{
	virtual AAX_Result	HandleMultipleParametersMouseDown ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) = 0;	  ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseDown()
	virtual AAX_Result	HandleMultipleParametersMouseDrag ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) = 0;	  ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseDrag()
	virtual AAX_Result	HandleMultipleParametersMouseUp   ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) = 0;    ///< \copydoc AAX_IViewContainer::HandleMultipleParametersMouseUp()
	//@}end Host event handlers
};


/** \brief Additional methods to track mouse as it moves over controls
  *
  *    \details
  *    \sa \ref AAX_IViewContainer
  */
class AAX_IACFViewContainer_V3 : public AAX_IACFViewContainer_V2
{
public:
    /** @name Host event handlers
     */
    //@{
	virtual AAX_Result	HandleParameterMouseEnter(AAX_CParamID inParamID, uint32_t inModifiers ) = 0;   ///< \copydoc AAX_IViewContainer::HandleParameterMouseEnter()
	virtual AAX_Result	HandleParameterMouseExit(AAX_CParamID inParamID, uint32_t inModifiers ) = 0;    ///< \copydoc AAX_IViewContainer::HandleParameterMouseExit()
    //@}end Host event handlers
};


#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif
