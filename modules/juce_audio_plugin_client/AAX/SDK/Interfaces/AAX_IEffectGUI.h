/*================================================================================================*/
/*
 *
 *	Copyright 2014-2017, 2019-2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IEffectGUI.h
 *
 *	\brief The interface for a %AAX Plug-in's user interface
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IEFFECTGUI_H
#define AAX_IEFFECTGUI_H

#include "AAX_IACFEffectGUI.h"
#include "AAX.h"
#include "CACFUnknown.h"


/** @brief	The interface for a %AAX Plug-in's user interface.
	
	@details
	@pluginimp

	This is the interface for an instance of a plug-in's GUI that gets exposed to 
	the host application.  You need to inherit from this interface and override all 
	of the virtual functions to create a plug-in GUI. 

	To create the GUI for an %AAX plug-in it is required that you inherit from 
	this interface and override all of the virtual functions from 
	\ref AAX_IACFEffectGUI.  In nearly all cases you will be able to take advantage of 
	the implementations in the %AAX library's \ref AAX_CEffectGUI class and only override the 
	few specific methods that you want to explicitly customize.
	
	\note This class always inherits from the latest version of the interface and thus requires any 
	subclass to implement all the methods in the latest version of the interface.
	
	\ingroup CommonInterface_GUI
*/
class AAX_IEffectGUI :	public AAX_IACFEffectGUI,
						public CACFUnknown
{	
public:
	ACF_DECLARE_STANDARD_UNKNOWN()
	
	ACFMETHOD(InternalQueryInterface)(const acfIID & riid, void **ppvObjOut) override;
	
	// CACFUnknown does not support operator=()
	AAX_DELETE(AAX_IEffectGUI& operator= (const AAX_IEffectGUI&));
};

#endif //AAX_IEFFECTGUI_H
