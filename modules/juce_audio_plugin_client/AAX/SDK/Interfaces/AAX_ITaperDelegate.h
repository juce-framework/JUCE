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
 *	\file   AAX_ITaperDelegate.h
 *	
 *	\brief	Defines the taper conversion behavior for a parameter
 *
 */ 
/*================================================================================================*/

 
#ifndef AAX_ITAPERDELEGATE_H
#define AAX_ITAPERDELEGATE_H



/** \brief Defines the taper conversion behavior for a parameter.
	
	\details
	\sdkinternal

	This interface represents a delegate class to be used in conjunction with \ref AAX_IParameter.
	\ref AAX_IParameter delegates all conversion operations between normalized and real parameter
	values to classes that meet this interface.  You can think of \ref AAX_ITaperDelegate subclasses
	as simple taper conversion routines that enable a specific taper or range conversion
	function on an arbitrary parameter.
	
	To demonstrate the use of this interface, we will examine a simple call routine into a
	parameter:

	\par
		1. The host application calls into the plug-in's \ref AAX_CParameterManager with a Parameter
		ID and a new normalized parameter value.  This new value could be coming from an automation
		lane, a control surface, or any other parameter control; from the plug-in's perspective,
		these are all identical.
	
	\par
		2. The AAX_CParameterManager finds the specified \ref AAX_CParameter and calls
		\ref AAX_IParameter::SetNormalizedValue() on that parameter
	
	\par
		3. \ref AAX_IParameter::SetNormalizedValue() results in a call into the parameter's concrete
		taper delegate to convert the normalized value to a real value.
		
	Using this pattern, the parameter manager is able to use real parameter values without
	actually knowing how to perform the conversion between normalized and real values.

	The inverse of the above example can also happen, e.g. when a control is updated
	from within the data model.  In this case, the parameter can call into its concrete
	taper delegate in order to normalize the updated value, which can then be passed on to any
	observers that require normalized values, such as the host app.
	
	For more information about the parameter manager, see the \ref AAXLibraryFeatures_ParameterManager
	documentation page.

	\ingroup AAXLibraryFeatures_ParameterManager_TaperDelegates

*/
class AAX_ITaperDelegateBase
{
public:
	/** \brief Virtual destructor
	 *
	 *	\note This destructor MUST be virtual to prevent memory leaks.
	 */
	virtual ~AAX_ITaperDelegateBase()				{ }
};

/** Taper delegate interface template
 
	\copydoc AAXLibraryFeatures_ParameterManager_TaperDelegates
	\ingroup AAXLibraryFeatures_ParameterManager_TaperDelegates
 */
template <typename T>
class AAX_ITaperDelegate : public AAX_ITaperDelegateBase
{
public:
	/** \brief Constructs and returns a copy of the taper delegate
	 *
	 *	In general, this method's implementation can use a simple copy constructor:
	 
		\code
			template <typename T>
			AAX_CSubclassTaperDelegate<T>*	AAX_CSubclassTaperDelegate<T>::Clone() const
			{
				return new AAX_CSubclassTaperDelegate(*this);
			}
		\endcode

	 */
	virtual AAX_ITaperDelegate*	Clone() const = 0;
	
	/** \brief Returns the taper's maximum real value
	 *
	 */
	virtual T		GetMaximumValue()  const = 0;

	/** \brief Returns the taper's minimum real value
	 *
	 */
	virtual T		GetMinimumValue() const = 0;
	
	/** \brief Applies a contraint to the value and returns the constrained value
	 *
	 *	This method is useful if the taper requires a contraint beyond simple minimum and maximum
	 *	real value limits.
	 *
	 *	\note This is the function that should actually enforces the constraints in
	 *	NormalizeToReal() and RealToNormalized().
	 *
	 *	\param[in] value
	 *		The unconstrained value
	 */
	virtual T		ConstrainRealValue(T value)	const = 0;
	
	/** \brief Converts a normalized value to a real value
	 *
	 *	This is where the actual taper algorithm is implemented.
	 *
	 *	This function should perform the exact inverse of RealToNormalized(), to
	 *	within the roundoff precision of the individual taper implementation.
	 *
	 *	\param[in] normalizedValue
	 *		The normalized value that will be converted
	 */
	virtual T		NormalizedToReal(double normalizedValue) const = 0;
	
	/** \brief Normalizes a real parameter value
	 *
	 *	This is where the actual taper algorithm is implemented.
	 *
	 *	This function should perform the exact inverse of NormalizedToReal(), to
	 *	within the roundoff precision of the individual taper implementation.
	 *
	 *	\param[in] realValue
	 *		The real parameter value that will be normalized
	 */
	virtual double	RealToNormalized(T realValue) const = 0;
};



#endif //AAX_ITAPERDELEGATE_H
