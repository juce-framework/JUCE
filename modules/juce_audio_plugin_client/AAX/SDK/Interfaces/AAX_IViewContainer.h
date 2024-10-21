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
 *	\file  AAX_IViewContainer.h
 *
 *	\brief Interface for the %AAX host's view of a single instance of an effect
 *
 */ 
/*================================================================================================*/


#ifndef _AAX_IVIEWCONTAINER_H_
#define _AAX_IVIEWCONTAINER_H_

#include "AAX_GUITypes.h"
#include "AAX.h"


/**	\brief	Interface for the %AAX host's view of a single instance of an 
 *	effect.  Used both by clients of the %AAX host and by effect components.	
 *	
 *	\details
 *	\hostimp
 *
 *	\ingroup CommonInterface_GUI
 */
class AAX_IViewContainer
{
public:	
	virtual ~AAX_IViewContainer(void) {}
	
	/** @name View and GUI state queries
	 */
	//@{
	/**	\brief Returns the raw view type as one of \ref AAX_EViewContainer_Type
	 */
	virtual int32_t		GetType () = 0;
	/**	\brief Returns a pointer to the raw view
	 */
    virtual void *		GetPtr () = 0;
	/**	\brief Queries the host for the current \ref AAX_EModifiers "modifier keys".
	 
		This method returns a bit mask with bits set for each of the currently active
		modifier keys.  This method does not return the state of the \ref AAX_eModifiers_SecondaryButton.
	 
		\compatibility Although this method allows plug-ins to acquire the current state of the Windows
		key (normally blocked by Pro Tools), plug-ins should not use key combinations that require this key.
		
		\param[out] outModifiers
			Current modifiers as a bitmask of \ref AAX_EModifiers
	 */
	virtual AAX_Result	GetModifiers ( uint32_t * outModifiers ) = 0;
	//@}end View and GUI state queries
    
	/** @name View change requests
	 */
	//@{
	/**	\brief Request a change to the main view size
	 
	 	\note \li For compatibility with the smallest supported displays,
		plug-in GUI dimensions should not exceed 749x617 pixels, or 749x565
		pixels for plug-ins with sidechain support.
	 
		\param[in] inSize
			The new size to which the plug-in view should be set
	 */
    virtual AAX_Result	SetViewSize ( AAX_Point & inSize ) = 0;
	//@}end View change requests
	
	/** @name Host event handlers
	 *
	 *	These methods are used to pass plug-in GUI events to the host for
	 *	handling.  Events should always be passed on in this way when there
	 *	is a possibility of the host overriding the event with its own
	 *	behavior.
	 *
	 *	For example, in Pro Tools a command-control-option-click on any
	 *	automatable plug-in parameter editor should bring up that parameter's
	 *	automation pop-up menu, and a control-right click should display
	 *	the parameter's automation lane in the Pro Tools Edit window.  In
	 *	order for Pro Tools to handle these events, the plug-in must pass
	 *	them on using \ref HandleParameterMouseDown()
	 *
	 *	For each of these methods:
	 *	\li \ref AAX_SUCCESS is returned if the event was successfully handled
	 *	by the host.  In most cases, no further action will be required from
	 *	the plug-in after the host successfully handles an event.
	 *	\li \ref AAX_ERROR_UNIMPLEMENTED is returned if the event was not
	 *	handled by the host.  In this case, the plug-in should perform its own
	 *	event handling.
	 *	
	 */
	//@{	
    /**	\brief Alert the host to a mouse down event
	 
		\param[in] inParamID
			ID of the parameter whose control is being edited
		\param[in] inModifiers
			A bitmask of \ref AAX_EModifiers values
	 */
	virtual AAX_Result	HandleParameterMouseDown ( AAX_CParamID inParamID, uint32_t inModifiers ) = 0;
	/**	\brief Alert the host to a mouse drag event
		
		\warning The host may return \ref AAX_ERROR_UNIMPLEMENTED for this
		event even if the host did handle the corresponding mouse down event.
		A plug-in should ignore any following mouse drag and mouse up events
		that correspond to a host-managed mouse down event. (\ref PTSW-195209)
		
		\param[in] inParamID
			ID of the parameter whose control is being edited
		\param[in] inModifiers
			A bitmask of \ref AAX_EModifiers values
	 */
	virtual AAX_Result	HandleParameterMouseDrag ( AAX_CParamID inParamID, uint32_t inModifiers ) = 0;
	/**	\brief Alert the host to a mouse up event
		
		\warning The host may return \ref AAX_ERROR_UNIMPLEMENTED for this
		event even if the host did handle the corresponding mouse down event.
		A plug-in should ignore any following mouse drag and mouse up events
		that correspond to a host-managed mouse down event. (\ref PTSW-195209)
		
		\param[in] inParamID
			ID of the parameter whose control is being edited
		\param[in] inModifiers
			A bitmask of \ref AAX_EModifiers values
	 */
	virtual AAX_Result	HandleParameterMouseUp ( AAX_CParamID inParamID, uint32_t inModifiers ) = 0;

	/** \brief Alert the host to a mouse enter event to the parameter's control
        
		\param[in] inParamID
			ID of the parameter whose control is being entered
		\param[in] inModifiers
			A bitmask of \ref AAX_EModifiers values

		\brief Returns AAX_SUCCESS if event was processed successfully, otherwise an AAX_ERROR code
	 */
	virtual AAX_Result	HandleParameterMouseEnter ( AAX_CParamID inParamID, uint32_t inModifiers ) = 0;

	/** \brief Alert the host to a mouse exit event from the parameter's control
        
		\param[in] inParamID
			ID of the parameter whose control is being exited
		\param[in] inModifiers
			A bitmask of \ref AAX_EModifiers values

		\brief Returns AAX_SUCCESS if event was processed successfully, otherwise an AAX_ERROR code
	 */
	virtual AAX_Result	HandleParameterMouseExit( AAX_CParamID inParamID, uint32_t inModifiers ) = 0;
    
	/**	\brief Alert the host to a mouse down event
	 
		\param[in] inParamIDs
			IDs of the parameters that belong to the same GUI element whose controls are being edited
		\param[in] inNumOfParams
			Number of parameter IDS
		\param[in] inModifiers
			A bitmask of \ref AAX_EModifiers values
	 */
	virtual AAX_Result	HandleMultipleParametersMouseDown ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) = 0;
	/**	\brief Alert the host to a mouse drag event
	 
		\warning The host may return \ref AAX_ERROR_UNIMPLEMENTED for this
		event even if the host did handle the corresponding mouse down event.
		A plug-in should ignore any following mouse drag and mouse up events
		that correspond to a host-managed mouse down event. (\ref PTSW-195209)
	 
		\param[in] inParamIDs
			IDs of the parameters that belong to the same GUI element whose controls are being edited
		\param[in] inNumOfParams
			Number of parameter IDS
		\param[in] inModifiers
			A bitmask of \ref AAX_EModifiers values
	 */
	virtual AAX_Result	HandleMultipleParametersMouseDrag ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) = 0;
	/**	\brief Alert the host to a mouse up event
	 
		\warning The host may return \ref AAX_ERROR_UNIMPLEMENTED for this
		event even if the host did handle the corresponding mouse down event.
		A plug-in should ignore any following mouse drag and mouse up events
		that correspond to a host-managed mouse down event. (\ref PTSW-195209)
	 
		\param[in] inParamIDs
			IDs of the parameters that belong to the same GUI element whose controls are being edited
		\param[in] inNumOfParams
			Number of parameter IDS
		\param[in] inModifiers
			A bitmask of \ref AAX_EModifiers values
	 */
	virtual AAX_Result	HandleMultipleParametersMouseUp   ( const AAX_CParamID* inParamIDs, uint32_t inNumOfParams, uint32_t inModifiers ) = 0;
	//@}end Host event handlers
};

#endif

