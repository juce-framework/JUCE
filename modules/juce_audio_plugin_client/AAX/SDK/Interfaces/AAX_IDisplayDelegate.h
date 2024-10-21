/*================================================================================================*/
/*
 *
 *	Copyright 2014-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IDisplayDelegate.h
 *
 *	\brief Defines the display behavior for a parameter
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IDISPLAYDELETGATE_H
#define AAX_IDISPLAYDELETGATE_H

#include "AAX.h"


//Forward declarations
class AAX_CString;

/** \brief Defines the display behavior for a parameter.
	
	\details
	This interface represents a delegate class to be used in conjunction with AAX_IParameter.
	AAX_IParameter delegates all conversion operations between strings and real parameter
	values to classes that meet this interface.  You can think of AAX_ITaperDelegate subclasses
	as simple string serialization routines that enable a specific string conversions for an
	arbitrary parameter.

	For more information about how parameter delegates operate, see the AAX_ITaperDelegate and
	\ref AAXLibraryFeatures_ParameterManager documentation.
	
	\note This class is \em not part of the %AAX ABI and must not be passed between the plug-in
	and the host.

	\ingroup AAXLibraryFeatures_ParameterManager_DisplayDelegates

*/
class AAX_IDisplayDelegateBase
{
public:
	/** \brief Virtual destructor
	 *
	 *	\note This destructor MUST be virtual to prevent memory leaks.
	 */
	virtual ~AAX_IDisplayDelegateBase()		{ }
};

/** Display delegate interface template
 
 \copydoc AAXLibraryFeatures_ParameterManager_DisplayDelegates
 \ingroup AAXLibraryFeatures_ParameterManager_DisplayDelegates
 */
template <typename T>
class AAX_IDisplayDelegate : public AAX_IDisplayDelegateBase
{
public:

	/** \brief Constructs and returns a copy of the display delegate
	 *
	 *	In general, this method's implementation can use a simple copy constructor:
	 
		\code
			template <typename T>
			AAX_CSubclassDisplayDelegate<T>*	AAX_CSubclassDisplayDelegate<T>::Clone() const
			{
				return new AAX_CSubclassDisplayDelegate(*this);
			}
		\endcode

	 */
	virtual AAX_IDisplayDelegate*	Clone() const = 0;
	
	/** \brief Converts a real parameter value to a string representation
	 *	
	 *	\param[in] value
	 *		The real parameter value that will be converted
	 *	\param[out] valueString
	 *		A string corresponding to value
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		ValueToString(T value, AAX_CString* valueString) const = 0;

	/** \brief Converts a real parameter value to a string representation using a size hint, useful for control surfaces and other character limited displays.
	 *	
	 *	\param[in] value
	 *		The real parameter value that will be converted
	 *	\param[in] maxNumChars
	 *		Size hint for the desired maximum number of characters in the string (not including null termination)
	 *	\param[out] valueString
	 *		A string corresponding to value
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const = 0;
	
	/** \brief Converts a string to a real parameter value
	 *	
	 *	\param[in] valueString
	 *		The string that will be converted
	 *	\param[out] value
	 *		The real parameter value corresponding to valueString
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		StringToValue(const AAX_CString& valueString, T* value) const = 0;
};



#endif //AAX_IDISPLAYDELETGATE_H
