/*================================================================================================*/
/*
 *
 *	Copyright 2014-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_IDisplayDelegateDecorator.h
 *
 *	\brief The base class for all concrete display delegate decorators
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IDISPLAYDELEGATEDECORATOR_H
#define AAX_IDISPLAYDELEGATEDECORATOR_H

#include "AAX_IDisplayDelegate.h"


/** \brief The base class for all concrete display delegate decorators
	
	@details
	\ingroup AAXLibraryFeatures_ParameterManager_DisplayDelegates_Decorators
	 
	\copydetails AAXLibraryFeatures_ParameterManager_DisplayDelegates_Decorators
	
	\note This class is \em not part of the %AAX ABI and must not be passed between the plug-in
	and the host.

 */
template <typename T>
class AAX_IDisplayDelegateDecorator : public AAX_IDisplayDelegate<T>
{
public:
	/** \brief Constructor
	 *
	 *	This class implements the decorator pattern, which is a sort of wrapper.  The object that
	 *	is being wrapped is passed into this constructor.  This object is passed by reference
	 *	because it must be copied to prevent any potential memory ambigities.
	 *
	 *	This constructor sets the local mWrappedDisplayDelegate member to a clone of the provided 
	 *	AAX_IDisplayDelegate. 
	 *
	 *	\param[in] displayDelegate
	 *		The decorated display delegate.
	 */
	AAX_IDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate);
	
	/** \brief Copy constructor
	 *	
	 *	This class implements the decorator pattern, which is a sort of wrapper.  The object that
	 *	is being wrapped is passed into this constructor.  This object is passed by reference
	 *	because it must be copied to prevent any potential memory ambigities.
	 *
	 *	This constructor sets the local mWrappedDisplayDelegate member to a clone of the provided 
	 *	AAX_IDisplayDelegateDecorator, allowing multiply-decorated display delegates.
	 *
	 *	\param[in] other
	 *		The display delegate decorator that will be set as the wrapped delegate of this object
	 */
	AAX_IDisplayDelegateDecorator(const AAX_IDisplayDelegateDecorator& other);
	
	/** \brief Virtual destructor
	 *	
	 *	\note This destructor must be overriden here in order to delete the wrapped display delegate
	 *	object upon decorator destruction.
	 */
	~AAX_IDisplayDelegateDecorator() AAX_OVERRIDE;
	
	/** \brief Constructs and returns a copy of the display delegate decorator
	 *
	 *	In general, this method's implementation can use a simple copy constructor:
	 
		\code
			template <typename T>
			AAX_CSubclassDisplayDelegate<T>*	AAX_CSubclassDisplayDelegate<T>::Clone() const
			{
				return new AAX_CSubclassDisplayDelegate(*this);
			}
		\endcode
		
	 *
	 *	\note This is an idiomatic method in the decorator pattern, so watch for potential problems
	 *	if this method is ever changed or removed.
	 *
	 */
	AAX_IDisplayDelegateDecorator<T>*	Clone() const AAX_OVERRIDE;
	
	/** \brief Converts a string to a real parameter value
	 *
	 *	Override of the AAX_IDisplayDelegate implementation to call into the wrapped object.  Display
	 *	delegate decorators should call into this implementation to pass ValueToString() calls on
	 *	to the wrapped object after applying their own value-to-string decoration.
	 *
	 *	\param[in] valueString
	 *		The string that will be converted
	 *	\param[out] value
	 *		The real parameter value corresponding to valueString
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	bool		ValueToString(T value, AAX_CString* valueString) const AAX_OVERRIDE;
	
	/** \brief Converts a string to a real parameter value with a size constraint.
	 *
	 *	Override of the AAX_IDisplayDelegate implementation to call into the wrapped object.  Display
	 *	delegate decorators should call into this implementation to pass ValueToString() calls on
	 *	to the wrapped object after applying their own value-to-string decoration.
	 *
	 *	\param[in] valueString
	 *		The string that will be converted
	 *	\param[in] maxNumChars
	 *		Size hint for the desired maximum number of characters in the string (not including null termination)
	 *	\param[out] value
	 *		The real parameter value corresponding to valueString
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	bool		ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const AAX_OVERRIDE;

	/** \brief Converts a string to a real parameter value
	 *
	 *	Override of the DisplayDecorator implementation to call into the wrapped object.  Display
	 *	delegate decorators should call into this implementation to pass StringToValue() calls on
	 *	to the wrapped object after applying their own string-to-value decoding.
	 *	
	 *	\param[in] valueString
	 *		The string that will be converted
	 *	\param[out] value
	 *		The real parameter value corresponding to valueString
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	bool		StringToValue(const AAX_CString& valueString, T* value) const AAX_OVERRIDE;

private:
	const AAX_IDisplayDelegate<T>*		mWrappedDisplayDelegate;
	
	/** Default constructor not allowed. */
	AAX_IDisplayDelegateDecorator()		{ }	
};

template <typename T>
AAX_IDisplayDelegateDecorator<T>::AAX_IDisplayDelegateDecorator(const AAX_IDisplayDelegate<T>& displayDelegate)  :
	AAX_IDisplayDelegate<T>(),
	mWrappedDisplayDelegate(displayDelegate.Clone())
{

}

template <typename T>
AAX_IDisplayDelegateDecorator<T>::AAX_IDisplayDelegateDecorator(const AAX_IDisplayDelegateDecorator& other) :
	mWrappedDisplayDelegate(other.mWrappedDisplayDelegate->Clone())
{
	
}

template <typename T>
AAX_IDisplayDelegateDecorator<T>::~AAX_IDisplayDelegateDecorator()
{
	delete mWrappedDisplayDelegate;
}

template <typename T>
AAX_IDisplayDelegateDecorator<T>*	AAX_IDisplayDelegateDecorator<T>::Clone() const
{
	return new AAX_IDisplayDelegateDecorator(*this);
}

template <typename T>
bool		AAX_IDisplayDelegateDecorator<T>::ValueToString(T value, AAX_CString* valueString) const 
{
	return mWrappedDisplayDelegate->ValueToString(value, valueString);
}

template <typename T>
bool		AAX_IDisplayDelegateDecorator<T>::ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const 
{
	return mWrappedDisplayDelegate->ValueToString(value, maxNumChars, valueString);
}

template <typename T>
bool		AAX_IDisplayDelegateDecorator<T>::StringToValue(const AAX_CString& valueString, T* value) const
{
	return mWrappedDisplayDelegate->StringToValue(valueString, value);
}




#endif //AAX_IDISPLAYDELEGATEDECORATOR_H



