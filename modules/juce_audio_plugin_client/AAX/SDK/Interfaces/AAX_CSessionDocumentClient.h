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
 *	\file AAX_CSessionDocumentClient.h
 */ 
/*================================================================================================*/

#pragma once
#ifndef AAX_CSessionDocumentClient_H
#define AAX_CSessionDocumentClient_H

#include "AAX_ISessionDocumentClient.h"
#include <memory>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

class AAX_IController;
class AAX_IEffectParameters;
class AAX_ISessionDocument;
class AAX_VSessionDocument;


/** @brief	Default implementation of the \ref AAX_ISessionDocumentClient interface.
*/
class AAX_CSessionDocumentClient :	public AAX_ISessionDocumentClient
{
public: ///////////////////////////////////////////////////////////////////////////// AAX_CSessionDocumentClient

	AAX_CSessionDocumentClient(void);
	~AAX_CSessionDocumentClient(void) AAX_OVERRIDE;

public: ///////////////////////////////////////////////////////////////////////////// AAX_ISessionDocumentClient

	/** @name Initialization and uninitialization
	 */
	//@{
	/**
	 * \copydoc AAX_IACFSessionDocumentClient::Initialize()
	 */
	AAX_Result Initialize (IACFUnknown * iUnknown) AAX_OVERRIDE;
	/**
	 * \copydoc AAX_IACFSessionDocumentClient::Uninitialize()
	 */
	AAX_Result Uninitialize (void) AAX_OVERRIDE;
	//@}end Initialization and uninitialization

	/** @name Session document access
	 */
	//@{
	/**
	 * \copydoc AAX_IACFSessionDocumentClient::SetSessionDocument()
	 */
	AAX_Result SetSessionDocument(IACFUnknown * iSessionDocument) AAX_OVERRIDE;
	//@}end Session document access
	
	/** @name %AAX host and plug-in event notification
	 */
	//@{
	/**
	 * \copydoc AAX_IACFSessionDocumentClient::NotificationReceived()
	 */
	AAX_Result NotificationReceived(/* AAX_ENotificationEvent */ AAX_CTypeID /*inNotificationType*/, const void * /*inNotificationData*/, uint32_t /*inNotificationDataSize*/) AAX_OVERRIDE { return AAX_SUCCESS; }
	//@}end %AAX host and plug-in event notification

protected: ///////////////////////////////////////////////////////////////////////////// AAX_CSessionDocumentClient

	/** @name Session document change notifications
	 */
	//@{
	/**
	 * \brief The session document interface is about to be added, replaced,
	 * or removed.
	 * 
	 * \details
	 * Custom implementations should stop using the current session document
	 * interface, which is about to become invalid.
	*/
	virtual AAX_Result SessionDocumentWillChange() { return AAX_SUCCESS; }
	/**
	 * \brief The session document interface has been added, replaced, or
	 * removed.
	 * 
	 * \details
	 * Custom implementations should update local references to the
	 * session document interface.
	*/
	virtual AAX_Result SessionDocumentChanged() { return AAX_SUCCESS; }
	//@}end Session document change notifications

	/** @name Private member accessors
	 */
	//@{
	/*!
	 *  \brief Retrieves a reference to the plug-in's controller interface
	 *
	 */
    AAX_IController* GetController (void);
	const AAX_IController* GetController (void) const; ///< \copydoc AAX_CSessionDocumentClient::GetController()
    
	/*!
	 *  \brief Retrieves a reference to the plug-in's data model interface
	 *
	 */
    AAX_IEffectParameters* GetEffectParameters (void);
	const AAX_IEffectParameters* GetEffectParameters (void) const; ///< \copydoc AAX_CSessionDocumentClient::GetEffectParameters()
    
	/*!
	 *  \brief Retrieves a reference to the session document interface
	 *
	 */
    std::shared_ptr<AAX_ISessionDocument> GetSessionDocument (void);
	std::shared_ptr<const AAX_ISessionDocument> GetSessionDocument (void) const; ///< \copydoc AAX_CSessionDocumentClient::GetSessionDocument()
	//@}end Private member accessors

private:
	void ClearInternalState();

    //These are private, but they all have protected accessors. 
	AAX_UNIQUE_PTR(AAX_IController) mController;
	AAX_IEffectParameters * mEffectParameters;
	std::shared_ptr<AAX_VSessionDocument> mSessionDocument;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // AAX_CSessionDocumentClient
