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
 *	\file  AAX_IACFEffectGUI.h
 *
 *	\brief The GUI interface that gets exposed to the host application
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFEFFECTGUI_H
#define AAX_IACFEFFECTGUI_H

#include "AAX.h"			
#include "AAX_GUITypes.h"
#include "AAX_IString.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"


/** @brief	The interface for a %AAX Plug-in's GUI (graphical user interface).
	
	@details
	This is the interface for an instance of a plug-in's GUI that gets
	exposed to the host application.  The %AAX host interacts with your
	plug-in's GUI via this interface.  See \ref CommonInterface_GUI.
	
	The plug-in's implementation of this interface is responsible for
	managing the plug-in's window and graphics objects and for defining the
	interactions between GUI views and the plug-in's data model.
	
	At \ref Initialize() "initialization", the host provides this interface
	with a reference to AAX_IController.  The GUI may use this controller to
	retrieve a pointer to the plug-in's AAX_IEffectParameters interface,
	allowing the GUI to request changes to the plug-in's data model in
	response to view events.  In addition, the controller provides a means of
	querying information from the host such as stem format or sample rate
	
	When managing a plug-in's GUI it is important to remember that this is
	just one of many possible sets of views for the plug-in's parameters.
	Other views and editors, such as automation lanes or control surfaces,
	also have the ability to synchronously interact with the plug-in's
	abstract data model interface. Therefore, the GUI should not take
	asymmetric control over the data model, act as a
	secondary data model, or otherwise assume exclusive ownership of the
	plug-in's state.  In general, the data model's abstraction to a pure
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
	
	\legacy In the legacy plug-in SDK, these methods were found in CProcess and
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

	/** @name %AAX host and plug-in event notification
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
	 *	- \note some notifications are sent only to the plug-in GUI while other notifications are sent only to the
	 *	  plug-in data model. If you are not seeing an expected notification, try checking the other plug-in objects'
	 *	  \c NotificationReceived() methods.
	 *	- \note the host may dispatch notifications synchronously or asynchronously, and calls to this method may
	 *	  occur concurrently on multiple threads.
	 *
	 *	A plug-in may also dispatch custom notifications using \ref AAX_IController::SendNotification(). Custom
	 *	notifications will be posted back to the plug-in's other objects which support a \c NotificationReceived()
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
	//@}end %AAX host and plug-in event notification
	
	/** @name View accessors
	 */
	//@{
	/*!
	 *  \brief Provides a handle to the main plug-in window
	 *
	 *  \param[in] iViewContainer 
	 *		An \ref AAX_IViewContainer providing a native handle to the plug-in's window
	 */
	virtual AAX_Result		SetViewContainer ( IACFUnknown * iViewContainer ) = 0;
	/*!
	 *  \brief Retrieves the size of the plug-in window
	 *
	 *	\sa \ref AAX_IViewContainer::SetViewSize()
	 *
	 *  \param[out] oViewSize 
	 *		The size of the plug-in window as a point (width, height)
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
	 *	This wakeup runs continuously and cannot be armed/disarmed by the plug-in.
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
	 *	\brief Called by host application to retrieve a custom plug-in string
	 *
	 *	If no string is provided then the host's default will be used.
	 *
	 *	\param[in] iSelector
	 *		The requested strong. One of \ref AAX_EPlugInStrings
	 *	\param[out] oString
	 *		The plug-in's custom value for the requested string
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
	 *	name in the legacy plug-in SDK.
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

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif //AAX_IACFEFFECTGUI_H
