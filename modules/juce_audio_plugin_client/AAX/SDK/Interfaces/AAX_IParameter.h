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
 *	\file   AAX_IParameter.h
 *
 *	\brief The base interface for all normalizable plug-in parameters
 *
 */ 
/*================================================================================================*/


#ifndef AAX_IPARAMETER_H
#define AAX_IPARAMETER_H

#include "AAX.h"	//for types

//Forward Declarations
class AAX_CString;
class AAX_IAutomationDelegate;
class AAX_ITaperDelegateBase;
class AAX_IDisplayDelegateBase;
class AAX_IString;

/** @brief An abstract interface representing a parameter value of arbitrary type
 
	@details
	@sdkinternal
 
	@sa \ref AAX_IParameter
 */
class AAX_IParameterValue
{
public:
	/** \brief Virtual destructor
	 *
	 *	\note This destructor MUST be virtual to prevent memory leaks.
	 */
	virtual ~AAX_IParameterValue()			{  }
	
	/** \brief Clones the parameter object
	 *
	 *	\note Does NOT set the automation delegate on the clone; ownership of the automation
	 *	delegate and parameter registration/unregistration stays with the original parameter
	 */
	virtual AAX_IParameterValue*		Clone() const = 0;
	
	/** \brief Returns the parameter's unique identifier
	 *
	 *	This unique ID is used by the \ref AAXLibraryFeatures_ParameterManager and by outside
	 *	applications to uniquely identify and target control messages.  This value may not be
	 *	changed after the parameter has been constructed.
	 */
	virtual AAX_CParamID				Identifier() const = 0;
	
	/** @name Typed accessors
	 *
	 */
	//@{
	/** \brief Retrieves the parameter's value as a bool
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to bool was successful
	 *	\retval false	The conversion to bool was unsuccessful
	 */
	virtual bool		GetValueAsBool(bool* value) const = 0;
	
	/** \brief Retrieves the parameter's value as an int32_t
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to int32_t was successful
	 *	\retval false	The conversion to int32_t was unsuccessful
	 */
	virtual bool		GetValueAsInt32(int32_t* value) const = 0;
	
	/** \brief Retrieves the parameter's value as a float
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to float was successful
	 *	\retval false	The conversion to float was unsuccessful
	 */
	virtual bool		GetValueAsFloat(float* value) const = 0;
	
	/** \brief Retrieves the parameter's value as a double
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to double was successful
	 *	\retval false	The conversion to double was unsuccessful
	 */
	virtual bool		GetValueAsDouble(double* value) const = 0;
	
	/** \brief Retrieves the parameter's value as a string
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to string was successful
	 *	\retval false	The conversion to string was unsuccessful
	 */
	virtual bool		GetValueAsString(AAX_IString* value) const = 0;
	//@} Typed accessors
};

/** @brief The base interface for all normalizable plug-in parameters
	
	@details
	@sdkinternal
	
	This class is an outside interface for an arbitrarily typed parameter.  The subclasses of this
	generic interface hold the parameter's state and conversion functionality.
	
	\note This class is \em not part of the %AAX ABI and must not be passed between the plug-in and
	the host.  Version checking is recommended when passing references to this interface between
	plug-in modules (e.g. between the data model and the GUI)

	\ingroup AAXLibraryFeatures_ParameterManager
	
*/
class AAX_IParameter
{
public:	
	/** \brief Virtual destructor
	 *
	 *	\note This destructor MUST be virtual to prevent memory leaks.
	 */
	virtual ~AAX_IParameter()			{  }
	
	/** \brief Clone the parameter's value to a new \ref AAX_IParameterValue object
	 *
	 *	The returned object is independent from the \ref AAX_IParameter. For example,
	 *	changing the state of the returned object will not result in a change to the
	 *	original \ref AAX_IParameter.
	 */
	virtual AAX_IParameterValue*	CloneValue() const = 0;
	
	/** @name Identification methods
	 *
	 */
	//@{
	/** \brief Returns the parameter's unique identifier
	 *
	 *	This unique ID is used by the \ref AAXLibraryFeatures_ParameterManager and by outside
	 *	applications to uniquely identify and target control messages.  This value may not be
	 *	changed after the parameter has been constructed.
	 */
	virtual AAX_CParamID		Identifier() const = 0;

	/** \brief Sets the parameter's display name
	 *
	 *	This name is used for display only, it is not used for indexing or identifying the parameter
	 *	This name may be changed after the parameter has been created, but display name changes
	 *	may not be recognized by all %AAX hosts.
	 *
	 *	\param[in] name
	 *		Display name that will be assigned to the parameter
	 */
	virtual void				SetName(const AAX_CString& name) = 0;
	
	/** \brief Returns the parameter's display name
	 *
	 *  \note This method returns a const reference in order to prevent a string copy.  Do not cast
	 *	away the const to change this value.
	 */
	virtual const AAX_CString&	Name() const = 0;
	
	/** \brief Sets the parameter's shortened display name
	 *
	 *	This name is used for display only, it is not used for indexing or identifying the parameter
	 *	These names show up when the host asks for shorter length parameter names for display on Control Surfaces 
	 *  or other string length constrained situations.
	 *
	 *	\param[in] name
	 *		Shortened display names that will be assigned to the parameter
	 */
	virtual void				AddShortenedName(const AAX_CString& name) = 0;
	
	/** \brief Returns the parameter's shortened display name
	 *
	 *  \note This method returns a const reference in order to prevent a string copy.  Do not cast
	 *	away the const to change this value.
	 */
	virtual const AAX_CString&	ShortenedName(int32_t iNumCharacters) const = 0;
	
	/** \brief Clears the internal list of shortened display names.
	 *
	 */
	virtual void				ClearShortenedNames() = 0;	
	//@} Identification methods
	
	
	/** @name Automation methods
	 *
	 */
	//@{
	/** \brief Returns true if the parameter is automatable, false if it is not
	 *
	 *	\note Subclasses that return true in this method must support host-based automation.
	 */
	virtual bool		Automatable() const = 0;
	
	/** \brief Sets the automation delegate (if one is required)
	 *
	 *	\param[in] iAutomationDelegate
	 *		A reference to the parameter manager's automation delegate interface
	 */
	virtual void		SetAutomationDelegate( AAX_IAutomationDelegate * iAutomationDelegate ) = 0;
	
	/** \brief Signals the automation system that a control has been touched
	 *	
	 *	Call this method in response to GUI events that begin editing, such as a mouse down.
	 *	After this method has been called you are free to call SetNormalizedValue() as much
	 *	as you need, e.g. in order to respond to subsequent mouse moved events.  Call Release() to
	 *	free the parameter for updates from other controls.
	 */
	virtual void		Touch() = 0;
	
	/** \brief Signals the automation system that a control has been released
	 *
	 *	Call this method in response to GUI events that complete editing, such as a mouse up.
	 *	Once this method has been called you should not call SetNormalizedValue() again until
	 *	after the next call to Touch().
	 */
	virtual void		Release() = 0;
	//@} Automation methods
		
	/** @name Taper methods
	 *
	 */
	//@{
	/** \brief Sets a parameter value using it's normalized representation
	 *
	 *	For more information regarding normalized values, see
	 *	\ref AAXLibraryFeatures_ParameterManager
	 *
	 *	\param[in] newNormalizedValue
	 *		New value (normalized) to which the parameter will be set
	 */
	virtual void		SetNormalizedValue(double newNormalizedValue) = 0;  
		
	/** \brief Returns the normalized representation of the parameter's current real value
	*
	*/
	virtual double		GetNormalizedValue() const = 0;

	/** \brief Sets the parameter's default value using its normalized representation
	 *	
	 */
	virtual void		SetNormalizedDefaultValue(double normalizedDefault) = 0;

	/**	\brief Returns the normalized representation of the parameter's real default value
	 *
	 */
	virtual double		GetNormalizedDefaultValue() const = 0;
	
	/** \brief Restores the state of this parameter to its default value
	 *
	 */
	virtual void		SetToDefaultValue() = 0;

	/**	\brief Sets the number of discrete steps for this parameter
	 *
	 *	Stepped parameter values are useful for discrete parameters and for "jumping" events such
	 *	as mouse wheels, page up/down, etc.  The parameter's step size is used to specify the
	 *	coarseness of those changes.
     *
     *  \note numSteps MUST be greater than zero.  All other values may be considered an error
     *  by the host.
	 *
	 *	\param[in] numSteps
	 *		The number of steps that the parameter will use
	 */
	virtual void		SetNumberOfSteps(uint32_t numSteps) = 0;

	/** \brief Returns the number of discrete steps used by the parameter
	 *	
	 *	See \ref SetNumberOfSteps() for more information about parameter steps.
	 */
	virtual uint32_t	GetNumberOfSteps() const = 0;
	
	/** \brief Returns the current step for the current value of the parameter
	 *	
	 *	See \ref SetNumberOfSteps() for more information about parameter steps.
	 */
	virtual uint32_t	GetStepValue() const = 0;
	
	/** \brief Returns the normalized value for a given step
	 *
	 *	See \ref SetNumberOfSteps() for more information about parameter steps.
	 */
	virtual double		GetNormalizedValueFromStep(uint32_t iStep) const = 0;
	
	/** \brief Returns the step value for a normalized value of the parameter
	 *
	 *	See \ref SetNumberOfSteps() for more information about parameter steps.
	 */
	virtual uint32_t	GetStepValueFromNormalizedValue(double normalizedValue) const = 0;
	
	/** \brief Returns the current step for the current value of the parameter
	 *	
	 *	See \ref SetNumberOfSteps() for more information about parameter steps.
	 */
	virtual void		SetStepValue(uint32_t iStep) = 0;
		
	//@} Taper methods
	
	
	/** @name Display methods
	 *
	 *	This functionality is most often used by GUIs, but can also be useful for state
	 *	serialization.
	 */
	//@{
	/** \brief Serializes the parameter value into a string
	 *
	 *	\param[out] valueString 
	 *		A string representing the parameter's real value
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		GetValueString(AAX_CString*	valueString) const = 0;

	/** \brief Serializes the parameter value into a string, size hint included.
	 *
	 *	\param[in] iMaxNumChars
	 *		A size hint for the size of the string being requested.  Useful for control surfaces and other limited area text fields.  (make sure that size of desired string also has room for null termination)
	 *	\param[out] valueString 
	 *		A string representing the parameter's real value
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		GetValueString(int32_t iMaxNumChars, AAX_CString*	valueString) const = 0;
	
	/** \brief Converts a bool to a normalized parameter value
	 *
	 *	\param[in] value
	 *		A value for the parameter
	 *	\param[out] normalizedValue
	 *		The normalized parameter value associated with value
	 *
	 *	\retval true	The value conversion was successful
	 *	\retval false	The value conversion was unsuccessful
	 */
	virtual bool		GetNormalizedValueFromBool(bool value, double *normalizedValue) const = 0;
	
	/** \brief Converts an integer to a normalized parameter value
	 *
	 *	\param[in] value
	 *		A value for the parameter
	 *	\param[out] normalizedValue
	 *		The normalized parameter value associated with value
	 *
	 *	\retval true	The value conversion was successful
	 *	\retval false	The value conversion was unsuccessful
	 */
	virtual bool		GetNormalizedValueFromInt32(int32_t value, double *normalizedValue) const = 0;
	
	/** \brief Converts a float to a normalized parameter value
	 *
	 *	\param[in] value
	 *		A value for the parameter
	 *	\param[out] normalizedValue
	 *		The normalized parameter value associated with value
	 *
	 *	\retval true	The value conversion was successful
	 *	\retval false	The value conversion was unsuccessful
	 */
	virtual bool		GetNormalizedValueFromFloat(float value, double *normalizedValue) const = 0;
	
	/** \brief Converts a double to a normalized parameter value
	 *
	 *	\param[in] value
	 *		A value for the parameter
	 *	\param[out] normalizedValue
	 *		The normalized parameter value associated with value
	 *
	 *	\retval true	The value conversion was successful
	 *	\retval false	The value conversion was unsuccessful
	 */
	virtual bool		GetNormalizedValueFromDouble(double value, double *normalizedValue) const = 0;
	
	/** \brief Converts a given string to a normalized parameter value
	 *
	 *	\param[in] valueString
	 *		A string representing a possible real value for the parameter
	 *	\param[out] normalizedValue
	 *		The normalized parameter value associated with valueString
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		GetNormalizedValueFromString(const AAX_CString&	valueString, double *normalizedValue) const = 0;
	
	/** \brief Converts a normalized parameter value to a bool representing the corresponding real
	 *	value
	 *
	 *	\param[in] normalizedValue
	 *		The normalized value to convert
	 *	\param[out] value
	 *		The converted value. Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to bool was successful
	 *	\retval false	The conversion to bool was unsuccessful
	 */
	virtual bool		GetBoolFromNormalizedValue(double normalizedValue, bool* value) const = 0;
	
	/** \brief Converts a normalized parameter value to an integer representing the corresponding real
	 *	value
	 *
	 *	\param[in] normalizedValue
	 *		The normalized value to convert
	 *	\param[out] value
	 *		The converted value. Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to int32_t was successful
	 *	\retval false	The conversion to int32_t was unsuccessful
	 */
	virtual bool		GetInt32FromNormalizedValue(double normalizedValue, int32_t* value) const = 0;
	
	/** \brief Converts a normalized parameter value to a float representing the corresponding real
	 *	value
	 *
	 *	\param[in] normalizedValue
	 *		The normalized value to convert
	 *	\param[out] value
	 *		The converted value. Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to float was successful
	 *	\retval false	The conversion to float was unsuccessful
	 */
	virtual bool		GetFloatFromNormalizedValue(double normalizedValue, float* value) const = 0;
	
	/** \brief Converts a normalized parameter value to a double representing the corresponding real
	 *	value
	 *
	 *	\param[in] normalizedValue
	 *		The normalized value to convert
	 *	\param[out] value
	 *		The converted value. Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to double was successful
	 *	\retval false	The conversion to double was unsuccessful
	 */
	virtual bool		GetDoubleFromNormalizedValue(double normalizedValue, double* value) const = 0;

	/** \brief Converts a normalized parameter value to a string representing the corresponding real
	 *	value
	 *
	 *	\param[in] normalizedValue
	 *		A normalized parameter value
	 *	\param[out] valueString
	 *		A string representing the parameter value associated with normalizedValue
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		GetStringFromNormalizedValue(double normalizedValue, AAX_CString&	valueString) const = 0;

	/** \brief Converts a normalized parameter value to a string representing the corresponding real, size hint included.
	 *	value
	 *
	 *	\param[in] normalizedValue
	 *		A normalized parameter value
	 *	\param[in] iMaxNumChars
	 *		A size hint for the size of the string being requested.  Useful for control surfaces and other limited area text fields.  (make sure that size of desired string also has room for null termination)
	 *	\param[out] valueString
	 *		A string representing the parameter value associated with normalizedValue
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		GetStringFromNormalizedValue(double normalizedValue, int32_t iMaxNumChars, AAX_CString&	valueString) const = 0;
	
	/** \brief Converts a string to a real parameter value and sets the parameter to this value
	 *
	 *	\param[in] newValueString
	 *		A string representing the parameter's new real value
	 *
	 *	\retval true	The string conversion was successful
	 *	\retval false	The string conversion was unsuccessful
	 */
	virtual bool		SetValueFromString(const AAX_CString&	newValueString) = 0;
	//@} Display methods
	
	/** @name Typed accessors
	 *
	 */
	//@{
	/** \brief Retrieves the parameter's value as a bool
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to bool was successful
	 *	\retval false	The conversion to bool was unsuccessful
	 */
	virtual bool		GetValueAsBool(bool* value) const = 0;
	
	/** \brief Retrieves the parameter's value as an int32_t
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to int32_t was successful
	 *	\retval false	The conversion to int32_t was unsuccessful
	 */
	virtual bool		GetValueAsInt32(int32_t* value) const = 0;

	/** \brief Retrieves the parameter's value as a float
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to float was successful
	 *	\retval false	The conversion to float was unsuccessful
	 */
	virtual bool		GetValueAsFloat(float* value) const = 0;

	/** \brief Retrieves the parameter's value as a double
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to double was successful
	 *	\retval false	The conversion to double was unsuccessful
	 */
	virtual bool		GetValueAsDouble(double* value) const = 0;
	
	/** \brief Retrieves the parameter's value as a string
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion to string was successful
	 *	\retval false	The conversion to string was unsuccessful
	 */
	virtual bool		GetValueAsString(AAX_IString* value) const = 0;

	/** \brief Sets the parameter's value as a bool
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion from bool was successful
	 *	\retval false	The conversion from bool was unsuccessful
	 */
	virtual bool		SetValueWithBool(bool value) = 0;

	/** \brief Sets the parameter's value as an int32_t
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion from int32_t was successful
	 *	\retval false	The conversion from int32_t was unsuccessful
	 */
	virtual bool		SetValueWithInt32(int32_t value) = 0;
		
	/** \brief Sets the parameter's value as a float
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion from float was successful
	 *	\retval false	The conversion from float was unsuccessful
	 */
	virtual bool		SetValueWithFloat(float value) = 0;
	
	/** \brief Sets the parameter's value as a double
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion from double was successful
	 *	\retval false	The conversion from double was unsuccessful
	 */
	virtual bool		SetValueWithDouble(double value) = 0;

	/** \brief Sets the parameter's value as a string
	 *
	 *	\param[out] value
	 *		The parameter's real value.  Set only if conversion is successful.
	 *
	 *	\retval true	The conversion from string was successful
	 *	\retval false	The conversion from string was unsuccessful
	 */
	virtual bool		SetValueWithString(const AAX_IString& value) = 0;
	//@} Typed accessors


	/** \brief Sets the type of this parameter
	 *
	 *	See \ref GetType for use cases
	 *
	 *	\param[in] iControlType
	 *		The parameter's new type as an AAX_EParameterType
	 */
	virtual void		SetType( AAX_EParameterType iControlType ) = 0;
	
	/**	\brief Returns the type of this parameter as an AAX_EParameterType
	 *
	 *	\todo Document use cases for control type
	 */
	virtual AAX_EParameterType	GetType() const = 0;

	
	/** \brief Sets the orientation of this parameter
	 *
	 *	\param[in] iOrientation
	 *		The parameter's new orientation
	 */
	virtual void		SetOrientation( AAX_EParameterOrientation iOrientation ) = 0;
	
	/**	\brief Returns the orientation of this parameter
	 *
	 */
	virtual AAX_EParameterOrientation	GetOrientation() const = 0;
	
	/*!
	 *  \brief Sets the parameter's taper delegate
	 *
	 *	\param[in] inTaperDelegate
	 *		A reference to the parameter's new taper delegate
	 *	\param[in] inPreserveValue
	 *		\todo Document this parameter
	 */
	virtual void SetTaperDelegate ( AAX_ITaperDelegateBase & inTaperDelegate, bool inPreserveValue ) = 0;

	/*!
	 *  \brief Sets the parameter's display delegate
	 *
	 *	\param[in] inDisplayDelegate
	 *		A reference to the parameter's new display delegate
	 */
	virtual void SetDisplayDelegate ( AAX_IDisplayDelegateBase & inDisplayDelegate ) = 0;

public:
	/** @name Host interface methods
	 *
	 */
	//@{
	/*!
	 *  \brief Sets the parameter's state given a normalized value
	 *
	 *	This is the second half of the parameter setting operation that is initiated with a call to
	 *	SetValue().  Parameters should not be set directly using this method; instead, use
	 *	SetValue().
	 *
	 *	\param[in] newNormalizedValue
	 *		Normalized value that will be used to set the parameter's new state
	 */	
	virtual void		UpdateNormalizedValue(double newNormalizedValue) = 0;
	//@} Host interface methods
		
};

#endif //AAX_IPARAMETER_H




