/*================================================================================================*/
/*
 *
 *	Copyright 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_IACFSessionDocumentClient.h
 */ 
/*================================================================================================*/

#pragma once
#ifndef AAX_IACFSessionDocumentClient_H
#define AAX_IACFSessionDocumentClient_H

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif

#include "AAX_UIDs.h"
#include "AAX.h"
#include "acfunknown.h"

/**
 * \brief Interface representing a client of the session document interface
 * 
 * For example, a plug-in implementation that makes calls on the session
 * document interface provided by the host.
 */
class AAX_IACFSessionDocumentClient : public IACFUnknown
{
public:
	/** @name Initialization and uninitialization
	 */
	//@{
	virtual AAX_Result Initialize (IACFUnknown * iUnknown) = 0;
	virtual AAX_Result Uninitialize (void) = 0;
	//@}end Initialization and uninitialization

	/** @name Session document access
	 */
	//@{
	/**
	 * \brief Sets or removes a session document
	 *
	 * \param[in] iSessionDocument 
	 * Interface supporting at least \ref AAX_IACFSessionDocument, or
	 * \c nullptr to indicate that any session document that is currently
	 * held should be released.
	 */
	virtual AAX_Result SetSessionDocument(IACFUnknown * iSessionDocument) = 0;
	//@}end Session document access
	
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
	 *	- \note Different notifications are sent to different objects within a plug-in. If you are not seeing an expected
	 *	  notification, try checking the other plug-in objects' \c NotificationReceived() methods.
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
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_IACFSessionDocumentClient_H
