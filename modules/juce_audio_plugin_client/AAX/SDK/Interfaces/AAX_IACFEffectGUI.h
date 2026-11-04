/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2023-2026 Avid Technology, Inc.
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
 *	\file  AAX_IACFEffectGUI.h
 *
 *	\brief The GUI interface that gets exposed to the host application
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFEFFECTGUI_H
#define AAX_IACFEFFECTGUI_H

#include "AAX_GUITypes.h"
#include "AAX_IString.h"
#include "AAX_Enums.h"
#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"


/** @brief	The interface for a %AAX plugin's GUI (graphical user interface).
	
	@details
	This is the interface for an instance of a plugin's GUI that gets
	exposed to the host application.  The %AAX host interacts with your
	plugin's GUI via this interface.  See \ref CommonInterface_GUI.
	
	The plugin's implementation of this interface is responsible for
	managing the plugin's window and graphics objects and for defining the
	interactions between GUI views and the plugin's data model.
	
	At \ref Initialize() "initialization", the host provides this interface
	with a reference to AAX_IController.  The GUI may use this controller to
	retrieve a pointer to the plugin's AAX_IEffectParameters interface,
	allowing the GUI to request changes to the plugin's data model in
	response to view events.  In addition, the controller provides a means of
	querying information from the host such as stem format or sample rate
	
	When managing a plugin's GUI it is important to remember that this is
	just one of many possible sets of views for the plugin's parameters.
	Other views and editors, such as automation lanes or control surfaces,
	also have the ability to synchronously interact with the plugin's
	abstract data model interface. Therefore, the GUI should not take
	asymmetric control over the data model, act as a
	secondary data model, or otherwise assume exclusive ownership of the
	plugin's state.  In general, the data model's abstraction to a pure
	virtual interface will protect against such aberrations, but this remains
	an important consideration when managing sophisiticated GUI interactions.
	
	You will most likely inherit your implementation of this interface from
	AAX_CEffectGUI, a default implementation that provides basic GUI
	functionality and which you can override and customize as needed.
	
	The SDK includes several examples of how the GUI interface may be
	extended and implemented in order to provide support for third-party
	frameworks.  These examples can be found in the /Extensions/GUI directory
	in the SDK.
	
	\note Your implementation of this interface must inherit from AAX_IEffectGUI. 
	
	\legacy In the legacy plugin SDK, these methods were found in CProcess and
	CEffectProcess.  For additional CProcess methods, see AAX_IEffectParameters.
	
	\ingroup CommonInterface_GUI
*/
class AAX_IACFEffectGUI : public IACFUnknown
{	
public:
	
	/** @name Initialization and uninitialization
	 */
	//@{
	/*!
	 *  \brief Main GUI initialization
	 *  
	 *	Called when the GUI is created
	 *
	 *  \param[in] iController 
	 *		A versioned reference that resolves to an AAX_IController interface
	 */
	virtual AAX_Result		Initialize ( IACFUnknown * iController ) = 0;
	/*!
	 *  \brief Main GUI uninitialization
	 *
	 *	Called when the GUI is destroyed.  Frees the GUI.
	 *
	 */
	virtual AAX_Result		Uninitialize () = 0;
	//@}end Initialization and uninitialization

	/** @name %AAX host and plugin event notification
	 */
	//@{
	/*!
	 *	\brief Notification Hook
	 *
	 *	Called from the host to deliver notifications to this object.
	 *
	 *	Look at the \ref AAX_ENotificationEvent enumeration to see a description of events you can listen for and the
	 *	data they come with.
	 *
	 *	- \note some notifications are sent only to the plugin GUI while other notifications are sent only to the
	 *	  plugin data model. If you are not seeing an expected notification, try checking the other plugin objects'
	 *	  \c NotificationReceived() methods.
	 *	- \note the host may dispatch notifications synchronously or asynchronously, and calls to this method may
	 *	  occur concurrently on multiple threads.
	 *
	 *	A plugin may also dispatch custom notifications using \ref AAX_IController::SendNotification(). Custom
	 *	notifications will be posted back to the plugin's other objects which support a \c NotificationReceived()
	 *	method (e.g. the data model).
	 *
	 *	\param[in] inNotificationType
	 *		Type of notification being received. Notifications form the host are one of \ref AAX_ENotificationEvent
	 *	\param[in] inNotificationData
	 *		Block of incoming notification data
	 *	\param[in] inNotificationDataSize
	 *		Size of \p inNotificationData, in bytes
	 */
	virtual	AAX_Result			NotificationReceived(/* AAX_ENotificationEvent */ AAX_CTypeID inNotificationType, const void * inNotificationData, uint32_t inNotificationDataSize) = 0;
	//@}end %AAX host and plugin event notification
	
	/** @name View accessors
	 */
	//@{
	/*!
	 *  \brief Provides a handle to the main plugin window
	 *
	 *  \param[in] iViewContainer 
	 *		An \ref AAX_IViewContainer providing a native handle to the plugin's window
	 */
	virtual AAX_Result		SetViewContainer ( IACFUnknown * iViewContainer ) = 0;
	/*!
	 *  \brief Retrieves the size of the plugin window
	 *
	 *	If this method is inapplicable, return \ref AAX_SUCCESS and do not
	 *	modify \p oViewSize.
	 *
	 *	\sa \ref AAX_IViewContainer::SetViewSize()
	 *
	 *  \param[out] oViewSize 
	 *		The size of the plugin window as a point (width, height)
	 */
	virtual AAX_Result		GetViewSize ( AAX_Point * oViewSize )  const = 0;
	//@}end View accessors
	
	/** @name GUI update methods
	 */
	//@{
    /*! \brief DEPRECATED, Not called from host any longer.  
     *  Your chosen graphics framework should be directly handling draw events from the OS.
     */
	virtual AAX_Result		Draw ( AAX_Rect * iDrawRect ) = 0;
	/*!  
	 *	\brief Periodic wakeup callback for idle-time operations
	 *
	 *	GUI animation events such as meter updates should be triggered from this method.
	 *
	 *	This method is called from the host's main thread.  In general, it should
	 *	be driven at approximately one call per 30 ms.  However, the wakeup is not guaranteed to
	 *	be called at any regular interval - for example, it could be held off by a high real-time
	 *	processing load - and there is no host contract regarding maximum latency between wakeup
	 *	calls.
	 *
	 *	This wakeup runs continuously and cannot be armed/disarmed by the plugin.
	 *
	 */
	virtual AAX_Result		TimerWakeup () = 0;
	/*!  
	 *	\brief Notifies the GUI that a parameter value has changed
	 *
	 *	This method is called by the host whenever a parameter value has been modified
	 *	
	 *	This method may be called on a non-main thread
	 *
	 *	\internal
	 *	\todo Create a "batch" version of this method, or convert this API to accept multiple
	 *	updates in a single call a la \ref AAX_IACFEffectParameters::GenerateCoefficients().
	 *	\endinternal
	 *
	 */
	virtual AAX_Result		ParameterUpdated( AAX_CParamID inParamID) = 0;
	//@}end GUI update methods
	
	
	/** @name Host interface methods
	 *
	 *	Miscellaneous methods to provide host-specific functionality
	 */
	//@{
	/*!
	 *	\brief Called by host application to retrieve a custom plugin string
	 *
	 *	If no string is provided then the host's default will be used.
	 *
	 *	\param[in] iSelector
	 *		The requested strong. One of \ref AAX_EPlugInStrings
	 *	\param[out] oString
	 *		The plugin's custom value for the requested string
	 *
	 */
	virtual AAX_Result		GetCustomLabel ( AAX_EPlugInStrings iSelector, AAX_IString * oString ) const = 0;
	/*!
	 *  \brief Called by host application. Indicates that a control widget should be
	 *	updated with a highlight color
	 *
	 *	\todo Document this method
	 *
	 *	\legacy This method was re-named from \c SetControlHighliteInfo(), its
	 *	name in the legacy plugin SDK.
	 *
	 *	\param[in] iParameterID
	 *		ID of parameter whose widget(s) must be highlighted
	 *	\param[in] iIsHighlighted
	 *		True if turning highlight on, false if turning it off
	 *	\param[in] iColor
	 *		Desired highlight color.  One of \ref AAX_EHighlightColor
	 *
	 */	
	virtual AAX_Result		SetControlHighlightInfo ( AAX_CParamID iParameterID, AAX_CBoolean iIsHighlighted, AAX_EHighlightColor iColor ) = 0;
	//@}end Host interface methods
	
};


class AAX_IACFEffectGUI_V2 : public AAX_IACFEffectGUI
{
public:
	/** @name View accessors
	 */
	//@{
	/*!
	 *  \brief Retrieves the requested minimum size of the plugin window
	 *
	 *	The host is not required to respect this minimum size, and may
	 *	provide a smaller window.
	 *
	 *	If this method is inapplicable, return \ref AAX_SUCCESS and do not
	 *	modify \p oMinimumViewSize.
	 *
	 *  \param[out] oMinimumViewSize
	 *		The minimum size of the plugin window as a point (width, height)
	 */
	virtual AAX_Result		GetMinimumViewSize ( AAX_Point * oMinimumViewSize ) const = 0;
	//@}end View accessors
};

class AAX_IACFEffectGUI_V3 : public AAX_IACFEffectGUI_V2
{
public:
	/** @name GUI update methods
	 */
	//@{
	/*!
	 *  \brief Updates the scaling factor for the plugin view and window
	 *
	 *	The host will call this to let the plugin know that the view container's
	 *	scaling factor has changed. This can occur when the plugin is moved to a different
	 *	screen with a different scaling factor (Windows only). The plugin should call
	 * \ref AAX_IViewContainer::SetViewSize() to resize the view container to the new size.
	 *
	 *	- \return AAX_SUCCESS if the plugin supports this function, e.g. if it adjusts the
	 *	  size of the image that it renders to the view container upon a change to the view
	 *	  container's scaling factor
	 *	- \return AAX_ERROR_UNIMPLEMENTED if the plugin does not support this function
	 *
	 *  \param[in] inViewContainer
	 *		The view container to update
	 *
	 *  \param[in] inScalingFactor
	 *		The new scaling factor
	 */
	virtual AAX_Result		UpdateScalingFactor ( IACFUnknown * inViewContainer, float inScalingFactor ) = 0;
	//@}end View accessors
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif //AAX_IACFEFFECTGUI_H
