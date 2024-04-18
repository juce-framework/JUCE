/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IACFEffectDirectData.h
 *
 *	\brief The direct data access interface that gets exposed to the host application
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IACFEFFECTDIRECTDATA_H
#define AAX_IACFEFFECTDIRECTDATA_H

#include "AAX.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "acfunknown.h"


/** @brief	Optional interface for direct access to a plug-in's alg memory.
	
	@details
	Direct data access allows a plug-in to directly manipulate the data in its algorithm's
	private data blocks.  The callback methods in this interface provide a safe context
	from which this kind of access may be attempted.

	\ingroup AuxInterface_DirectData
*/
class AAX_IACFEffectDirectData : public IACFUnknown
{	
public:
	
	/** @name Initialization and uninitialization
	 */
	//@{
	/*!
	 *  \brief Main initialization
	 *  
	 *	Called when the interface is created
	 *
	 *  \param[in] iController 
	 *		A versioned reference that resolves to an AAX_IController interface
	 */
	virtual AAX_Result		Initialize ( IACFUnknown * iController ) = 0;
	/*!
	 *  \brief Main uninitialization
	 *
	 *	Called when the interface is destroyed.
	 *
	 */
	virtual AAX_Result		Uninitialize () = 0;
	//@}end Initialization and uninitialization

	
	/** @name Safe data update callbacks
	 *
	 *	These callbacks provide a safe context from which to directly access the
	 *	algorithm's private data blocks.  Each callback is called regularly with
	 *	roughly the schedule of its corresponding AAX_IEffectParameters counterpart.
	 *
	 *	\note Do not attempt to directly access the algorithm's data from outside
	 *	these callbacks.  Instead, use the packet system to route data to the
	 *	algorithm using the %AAX host's buffered data transfer facilities.
	 *	
	 */
	//@{
	/*!  
	 *	\brief Periodic wakeup callback for idle-time operations
	 *
	 *	Direct alg data updates must be triggered from this method.
	 *
	 *	This method is called from the host using a non-main thread.  In general, it should
	 *	be driven at approximately one call per 30 ms.  However, the wakeup is not guaranteed to
	 *	be called at any regular interval - for example, it could be held off by a high real-time
	 *	processing load - and there is no host contract regarding maximum latency between wakeup
	 *	calls.
	 *
	 *	This wakeup thread runs continuously and cannot be armed/disarmed or by the plug-in.
	 *
	 *	\param[in] iDataAccessInterface
	 *		Reference to the direct access interface.
	 *	
	 *			\note It is not safe to save this address or call
	 *			the methods in this interface from other threads.
	 */
	virtual AAX_Result		TimerWakeup (
		IACFUnknown *		iDataAccessInterface ) = 0;
	//@} end
};


class AAX_IACFEffectDirectData_V2 : public AAX_IACFEffectDirectData{
public:
    /** @name %AAX host and plug-in event notification
     */
    //@{
    /*!
     *    \brief Notification Hook
     *
     *    Called from the host to deliver notifications to this object.
     *
     *    Look at the \ref AAX_ENotificationEvent enumeration to see a description of events you can listen for and the
     *    data they come with.
     *
     *    - \note some notifications are sent only to the plug-in GUI while other notifications are sent only to the
     *      plug-in data model. If you are not seeing an expected notification, try checking the other plug-in objects'
     *      \c NotificationReceived() methods.
     *    - \note the host may dispatch notifications synchronously or asynchronously, and calls to this method may
     *      occur concurrently on multiple threads.
     *
     *    A plug-in may also dispatch custom notifications using \ref AAX_IController::SendNotification(). Custom
     *    notifications will be posted back to the plug-in's other objects which support a \c NotificationReceived()
     *    method (e.g. the GUI).
     *
     *    \param[in] inNotificationType
     *        Type of notification being received. Notifications form the host are one of \ref AAX_ENotificationEvent
     *    \param[in] inNotificationData
     *        Block of incoming notification data
     *    \param[in] inNotificationDataSize
     *        Size of \p inNotificationData, in bytes
     */
    virtual    AAX_Result            NotificationReceived( /* AAX_ENotificationEvent */ AAX_CTypeID inNotificationType, const void * inNotificationData, uint32_t    inNotificationDataSize) = 0;
    //@}end %AAX host and plug-in event notification

};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif //AAX_IACFEFFECTDIRECTDATA_H
