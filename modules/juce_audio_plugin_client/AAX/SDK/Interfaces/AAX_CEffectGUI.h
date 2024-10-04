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
 *	\file AAX_CEffectGUI.h
 *
 *	\brief A default implementation of the AAX_IEffectGUI interface.
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CEFFECTGUI_H
#define AAX_CEFFECTGUI_H

#include "AAX_IEffectGUI.h"
#include "AAX_IACFEffectParameters.h"

#include <string>
#include <vector>
#include <map>
#include <memory>


class AAX_IEffectParameters;
class AAX_IController;
class AAX_IViewContainer;
class AAX_ITransport;



/** @brief	Default implementation of the AAX_IEffectGUI interface.

	@details
	This class provides a default implementation of the AAX_IEffectGUI interface.
	
	\legacy The default implementations in this class are mostly derived from their
	equivalent implementations in CProcess and CEffectProcess.  For additional
	CProcess-derived implementations, see AAX_CEffectParameters.
	
	\note See AAX_IACFEffectGUI for further information.
	
	\ingroup CommonInterface_GUI
*/
class AAX_CEffectGUI :	public AAX_IEffectGUI
{
public: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectGUI

	AAX_CEffectGUI(void);
	~AAX_CEffectGUI(void) AAX_OVERRIDE;

public: ///////////////////////////////////////////////////////////////////////////// AAX_IEffectGUI

	/** @name Initialization and uninitialization
	 */
	//@{
	AAX_Result	Initialize (IACFUnknown * iController ) AAX_OVERRIDE;
	AAX_Result	Uninitialize (void) AAX_OVERRIDE;
	//@}end Initialization and uninitialization

	/** @name %AAX host and plug-in event notification
	 */
	//@{
	/** \copydoc AAX_IACFEffectGUI::NotificationReceived()
		
		\note The default implementation doesn't do anything at this point, but it is probably still a good idea to
		call into the base class \ref AAX_CEffectGUI::NotificationReceived() function in case we want to implement
		some default behaviors in the future.
	 */
	AAX_Result	NotificationReceived(AAX_CTypeID inNotificationType, const void * inNotificationData, uint32_t inNotificationDataSize) AAX_OVERRIDE;
	//@}end %AAX host and plug-in event notification
	
	/** @name View accessors
	 */
	//@{
	AAX_Result SetViewContainer (IACFUnknown * iViewContainer ) AAX_OVERRIDE;
	AAX_Result GetViewSize (AAX_Point * /* oViewSize */ ) const AAX_OVERRIDE
	{
		return AAX_SUCCESS;
	}
	//@}end View accessors

	/** @name GUI update methods
	 */
	//@{
	AAX_Result Draw (AAX_Rect * /* iDrawRect */ ) AAX_OVERRIDE
	{
		return AAX_SUCCESS;
	}
	AAX_Result TimerWakeup (void) AAX_OVERRIDE
	{
		return AAX_SUCCESS;
	}
	AAX_Result ParameterUpdated(AAX_CParamID paramID) AAX_OVERRIDE;
	//@}end GUI update methods
	
	/** @name Host interface methods
	 *
	 *	Miscellaneous methods to provide host-specific functionality
	 */
	//@{
	AAX_Result		GetCustomLabel ( AAX_EPlugInStrings iSelector, AAX_IString * oString ) const AAX_OVERRIDE;

	AAX_Result SetControlHighlightInfo (AAX_CParamID /* iParameterID */, AAX_CBoolean /* iIsHighlighted */, AAX_EHighlightColor /* iColor */) AAX_OVERRIDE
	{
		return AAX_SUCCESS;
	}
	//@}end Direct host interface methods

protected: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectGUI

	/** @name AAX_CEffectGUI pure virtual interface
	 *
	 *	The implementations of these methods will be specific to the particular GUI framework that
	 *	is being incorporated with AAX_CEffectGUI.  Classes that inherit from AAX_CEffectGUI must
	 *	override these methods with their own framework-specific implementations.
	 *
	 */
	//@{
	/*!
	 *  \brief Creates any required top-level GUI components
	 *
	 *	This method is called by default from AAX_CEffectGUI::Initialize()
	 */
	virtual void CreateViewContents (void) = 0;
	/*!
	 *  \brief Initializes the plug-in window and creates the main GUI view or frame
	 *
	 *	This method is called by default from AAX_CEffectGUI::SetViewContainer() when a valid
	 *	window is present
	 */
	virtual void CreateViewContainer (void) = 0;
	/*!
	 *  \brief Uninitializes the plug-in window and deletes the main GUI view or frame
	 *
	 *	This method is called by default from AAX_CEffectGUI::SetViewContainer() when no valid
	 *	window is present.  It may also be appropriate for inheriting classes to call this
	 *	method from their destructors, depending on their own internal implementation.
	 */
	virtual void DeleteViewContainer (void) = 0;
	//@}end AAX_CEffectGUI pure virtual interface
	
	/** @name Helper methods
	 */
	//@{
	/*!
	 *  \brief Requests an update to the GUI for every parameter view
	 *
	 *	By default, calls AAX_CEffectGUI::ParameterUpdated() on every registered parameter.
	 *
	 *	By default, called from AAX_CEffectGUI::SetViewContainer() after a new view container
	 *	has been created.
	 *
	 *	\todo Rename to \c UpdateAllParameterViews() or another name that does not lead to
	 *	confusion regarding what exactly this method should be doing.
	 */
	virtual void UpdateAllParameters (void);
	//@}end Helper methods

public:  //These accessors are public here as they are often needed by contained views.
    
	/** @name Private member accessors
	 */
	//@{
	/*!
	 *  \brief Retrieves a reference to the plug-in's controller interface
	 *
	 */
    AAX_IController* GetController (void);
	const AAX_IController* GetController (void) const;
    
	/*!
	 *  \brief Retrieves a reference to the plug-in's data model interface
	 *
	 */
    AAX_IEffectParameters* GetEffectParameters (void);
	const AAX_IEffectParameters* GetEffectParameters (void) const;
    
	/*!
	 *  \brief Retrieves a reference to the plug-in's view container interface
	 *
	 */
    AAX_IViewContainer* GetViewContainer (void);
	const AAX_IViewContainer* GetViewContainer (void) const;
    
	/*!
	 *  \brief Retrieves a reference to the plug-in's Transport interface
	 *
	 */
	AAX_ITransport*				Transport();
	const AAX_ITransport*		Transport() const;
	
    /*!
     *  \brief Retrieves the Container and it's type.
     *
     */
	AAX_EViewContainer_Type		GetViewContainerType ();
	void *						GetViewContainerPtr ();	
	//@}end Private member accessors

private:
    //These are private, but they all have protected accessors. 
	AAX_IController *			mController;
	AAX_IEffectParameters *		mEffectParameters;
	AAX_UNIQUE_PTR(AAX_IViewContainer)	mViewContainer;
	AAX_ITransport*				mTransport;
};


#endif
