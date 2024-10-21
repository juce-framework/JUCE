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
 *	\file AAX_CEffectDirectData.h
 *
 *	\brief A default implementation of the AAX_IEffectDirectData interface.
 *
 */ 
/*================================================================================================*/

#pragma once
#ifndef AAX_CEFFECTDIRECTDATA_H
#define AAX_CEFFECTDIRECTDATA_H

#include "AAX_IEffectDirectData.h"



class AAX_IPrivateDataAccess;
class AAX_IEffectParameters;
class AAX_IController;



/** @brief	Default implementation of the AAX_IEffectDirectData interface.
	
	@details
	This class provides a default implementation of the AAX_IEffectDirectData interface.
	
	\ingroup AuxInterface_DirectData
*/
class AAX_CEffectDirectData : public AAX_IEffectDirectData
{
public: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectDirectData

	AAX_CEffectDirectData(
		void);
	
	virtual
	~AAX_CEffectDirectData(
		void);

public: ///////////////////////////////////////////////////////////////////////////// AAX_IEffectDirectData

	/** @name Initialization and uninitialization
	 */
	//@{
	/*!	\brief Non-virtual implementation of AAX_IEfectDirectData::Initialize()
	 *
	 *	This implementation initializes all private AAX_CEffectDirectData
	 *	members and calls Initialize_PrivateDataAccess().  For custom
	 *	initialization, inherited classes should override 
	 *	Initialize_PrivateDataAccess().
	 *
	 *	\param[in] iController
	 *		Unknown pointer that resolves to an AAX_IController.
	 *
	 */
	AAX_Result Initialize (IACFUnknown * iController ) AAX_OVERRIDE AAX_FINAL;
	AAX_Result Uninitialize (void) AAX_OVERRIDE;
	//@}end Initialization and uninitialization
	
	/** @name Data update callbacks
	 *
	 */
	//@{
	/*!	\brief Non-virtual implementation of AAX_IEfectDirectData::TimerWakeup()
	 *
	 *	This implementation interprets the IACFUnknown and forwards
	 *	the resulting AAX_IPrivateDataAccess to \ref TimerWakeup_PrivateDataAccess()
	 *
	 *	\param[in] iDataAccessInterface
	 *		Unknown pointer that resolves to an AAX_IPrivateDataAccess.  This
	 *		interface is only valid for the duration of this method's execution
	 *		and is discarded when the method returns.
	 *
	 */
	AAX_Result TimerWakeup (IACFUnknown * iDataAccessInterface ) AAX_OVERRIDE;
	//@}end Data update callbacks
    
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
     *    - \note some notifications are sent only to the plug-in GUI or plug-in data model while other notifications are sent only to the
     *      EffectDirectData. If you are not seeing an expected notification, try checking the other plug-in objects'
     *      \c NotificationReceived() methods.
     *    - \note the host may dispatch notifications synchronously or asynchronously, and calls to this method may
     *      occur concurrently on multiple threads.
     *
     *    \param[in] inNotificationType
     *        Type of notification being received. Notifications form the host are one of \ref AAX_ENotificationEvent
     *    \param[in] inNotificationData
     *        Block of incoming notification data
     *    \param[in] inNotificationDataSize
     *        Size of \p inNotificationData, in bytes
     */
    AAX_Result NotificationReceived( AAX_CTypeID inNotificationType,
                                    const void * inNotificationData,
                                    uint32_t inNotificationDataSize) AAX_OVERRIDE;
    //@} end %AAX host and plug-in event notification

	
public: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectDirectData

	/** @name Private member accessors
	 */
	//@{
	/*!
	 *  \brief Returns a pointer to the plug-in's controller interface
	 *
	 *	\todo Change to GetController to match other AAX_CEffect modules
	 */
	AAX_IController* Controller (void);
	/*!
	 *  \brief Returns a pointer to the plug-in's data model interface
	 *
	 *	\todo Change to GetController to match other AAX_CEffect modules
	 */
	AAX_IEffectParameters* EffectParameters (void);
	//@}end Private member accessors

protected: ///////////////////////////////////////////////////////////////////////////// AAX_CEffectDirectData
	
	/** @name AAX_CEffectDirectData virtual interface
	 */
	//@{
	/*!
	 *	\brief Initialization routine for classes that inherit from AAX_CEffectDirectData.
	 *	This method is called by the default Initialize() implementation after all
	 *	internal members have been initialized, and provides a safe location in which to
	 *	perform any additional initialization tasks.
	 *
	 */
	virtual AAX_Result Initialize_PrivateDataAccess();
	/*!
	 *	\brief Callback provided with an AAX_IPrivateDataAccess.  Override this method
	 *	to access the algorithm's private data using the AAX_IPrivateDataAccess interface.
	 *
	 *	\param[in] iPrivateDataAccess
	 *		Pointer to an AAX_IPrivateDataAccess interface.  This interface is only
	 *		valid for the duration of this method.
	 */
	virtual AAX_Result TimerWakeup_PrivateDataAccess(AAX_IPrivateDataAccess* iPrivateDataAccess);
	//@}end AAX_CEffectDirectData virtual interface
	
private:
	AAX_IController*							mController;
	AAX_IEffectParameters*						mEffectParameters;
};


#endif // AAX_CEFFECTDIRECTDATA_H
