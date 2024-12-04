/*================================================================================================*/
/*
 *
 *	Copyright 2014-2015, 2018, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_CParameterManager.h
 *
 *	\brief A container object for plug-in parameters
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CPARAMETERMANAGER_H
#define AAX_CPARAMETERMANAGER_H

#include "AAX_CParameter.h"
#include "AAX.h"

#include <vector>
#include <map>




class AAX_IAutomationDelegate;

/**	\brief A container object for plug-in parameters
	
	\details
	This implementation uses a STL vector to store a plug-in's set of parameters.  This class
	contains a real implementation of the \ref AAXLibraryFeatures_ParameterManager (as opposed to a proxy.)
	
	For more information, see \ref AAXLibraryFeatures_ParameterManager.

	\todo Should the Parameter Manager return error codes?

	\ingroup AAXLibraryFeatures_ParameterManager
 */
class AAX_CParameterManager
{
public:
	AAX_CParameterManager();
	~AAX_CParameterManager();

	/*!
	 *  \brief Initialize the parameter manager
	 *  
	 *  Called when plug-in instance is first instantiated.  This method will initialize the
	 *	plug-in's automation delegate, among other set-up tasks.
	 *
	 *	\param[in] iAutomationDelegateUnknown
	 *		A reference to the plug-in's AAX_IAutomationDelegate interface
	 */	
	void			Initialize(AAX_IAutomationDelegate* iAutomationDelegateUnknown);
	
	/*!
	 *  \brief Returns the number of parameters in this instance of the parameter manager
	 *
	 */	
	int32_t		NumParameters()	const;	
	
	/*!
	 *  \brief Removes a parameter from the manager
	 *
	 *	\todo Should this method return success/failure code?
	 *
	 *	\param[in] identifier
	 *		ID of the parameter that will be removed
	 */	
	void			RemoveParameterByID(AAX_CParamID identifier);
	
	/*!
	 *  \brief Removes all parameters from the manager
	 *
	 *	\todo Should this method return success/failure code?
	 */	
	void			RemoveAllParameters();

	/*!
	 *  \brief Given a parameter ID, retrieves a reference to the requested parameter
	 *
	 *	\param[in] identifier
	 *		ID of the parameter that will be retrieved
	 */	
	AAX_IParameter*			GetParameterByID(AAX_CParamID  identifier);
	
	/*!
	 *  \brief Given a parameter ID, retrieves a const reference to the requested parameter
	 *
	 *	\param[in] identifier
	 *		ID of the parameter that will be retrieved
	 */	
	const AAX_IParameter*	GetParameterByID(AAX_CParamID  identifier) const;
	
	/*!
	 *  \brief Given a parameter name, retrieves a reference to the requested parameter
	 *
	 *	\note Parameter names may be ambiguous
	 *
	 *	\param[in] name
	 *		Name of the parameter that will be retrieved
	 */
	AAX_IParameter*			GetParameterByName(const char*  name);
	
	/*!
	 *  \brief Given a parameter name, retrieves a const reference to the requested parameter
	 *
	 *	\note Parameter names may be ambiguous
	 *
	 *	\param[in] name
	 *		ID of the parameter that will be retrieved
	 */
	const AAX_IParameter*	GetParameterByName(const char*  name) const;
	
	/*!
	 *  \brief Given a parameter index, retrieves a reference to the requested parameter
	 *
	 *	Parameter indices are incremented in the order that parameters are added to the manager.
	 *	See AddParameter().
	 *
	 *	\param[in] index
	 *		Index of the parameter that will be retrieved
	 */	
	AAX_IParameter*			GetParameter(int32_t index);

	/*!
	 *  \brief Given a parameter index, retrieves a const reference to the requested parameter
	 *
	 *	Parameter indices are incremented in the order that parameters are added to the manager.
	 *	See AddParameter().
	 *
	 *	\param[in] index
	 *		Index of the parameter that will be retrieved
	 */	
	const AAX_IParameter*	GetParameter(int32_t index) const;
	
	/** Given a parameter ID, retrieves the index for the specified parameter
	 *
	 *	\param[in] identifier
	 *		ID of the parameter that will be retrieved
	 */	
	int32_t					GetParameterIndex(AAX_CParamID identifier) const;

	/** Adds a parameter to the manager
	 *
	 *	\todo Should this method return success/failure code?
	 *
	 *	\param[in] param
	 *		Reference to the parameter that will be added
	 */	
	void					AddParameter(AAX_IParameter*	param);
	
	/** Removes a parameter to the manager
	 *
	 *	\todo Should this method return success/failure code?
	 *
	 *	\param[in] param
	 *		Reference to the parameter that will be removed
	 */	
	void					RemoveParameter(AAX_IParameter*	param);
	
protected:
    
	AAX_IAutomationDelegate* 		mAutomationDelegate;        //This object is not ref-counted here.  Do not delete it.  It is ref counted by this object's parent.
	std::vector<AAX_IParameter*>	mParameters;
    std::map<std::string, AAX_IParameter*> mParametersMap;
};




#endif // AAX_CPARAMETERMANAGER_H
