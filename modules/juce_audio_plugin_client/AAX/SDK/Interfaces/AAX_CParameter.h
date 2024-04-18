/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_CParameter.h
 *
 *	\brief Generic implementation of an AAX_IParameter
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CPARAMETER_H
#define AAX_CPARAMETER_H

#include "AAX_Assert.h"
#include "AAX_IParameter.h"
#include "AAX_ITaperDelegate.h"
#include "AAX_IDisplayDelegate.h"
#include "AAX_IAutomationDelegate.h"
#include "AAX_CString.h"					//concrete class required for name.

#include <cstring>
#include <list>
#include <map>


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#endif
///////////////////////////////////////////////////////////////

////// AAX_CParameterValue Class Template Declaration ///////

/**	\brief Concrete implementation of \ref AAX_IParameterValue
 
 Used by \ref AAX_CParameter
 
 */
template <typename T>
class AAX_CParameterValue : public AAX_IParameterValue
{
public:
	enum Defaults {
        eParameterDefaultMaxIdentifierSize = kAAX_ParameterIdentifierMaxSize,
        eParameterDefaultMaxIdentifierLength = eParameterDefaultMaxIdentifierSize - 1 // NULL terminated
    };
	
public:
	AAX_DEFAULT_DTOR_OVERRIDE(AAX_CParameterValue);
	
	AAX_DEFAULT_MOVE_CTOR(AAX_CParameterValue);
	AAX_DEFAULT_MOVE_OPER(AAX_CParameterValue);
	
	AAX_DELETE(AAX_CParameterValue& operator=(const AAX_CParameterValue&));
	
	/**	\brief Constructs an \ref AAX_CParameterValue object
	 *
	 *	\param[in] identifier
	 *		Unique ID for the parameter value, these can only be 31 characters long at most.  (the fixed length is a requirement for some optimizations in the host)
     *	
	 *	\note The initial state of the parameter value is undefined
	 */
	explicit AAX_CParameterValue(AAX_CParamID identifier);
	
	/**	\brief Constructs an \ref AAX_CParameterValue object with a defined initial state
	 *
	 *	\param[in] identifier
	 *		Unique ID for the parameter value, these can only be 31 characters long at most.  (the fixed length is a requirement for some optimizations in the host)
     *	\param[in] value
	 *		Initial state of the parameter value
	 */
	explicit AAX_CParameterValue(AAX_CParamID identifier, const T& value);
	
	/**	\brief Copy constructor for \ref AAX_CParameterValue
	 */
	explicit AAX_CParameterValue(const AAX_CParameterValue<T>& other);
	
public: // AAX_CParameterValue<T> implementation
	/** \brief Direct access to the template instance's value
	 */
	const T& Get() const { return mValue; }
	/** \brief Direct access to the template instance's value
	 */
	void Set(const T& inValue) { mValue = inValue; }
	
public: // AAX_IParameterValue implementation
	
	AAX_IParameterValue*	Clone() const AAX_OVERRIDE { return new AAX_CParameterValue<T>(*this); }
	AAX_CParamID			Identifier() const AAX_OVERRIDE { return mIdentifier; }
	
	/** @name Typed accessors
	 *
	 */
	//@{
	bool					GetValueAsBool(bool* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameterValue::GetValueAsBool()
	bool					GetValueAsInt32(int32_t* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameterValue::GetValueAsInt32()
	bool					GetValueAsFloat(float* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameterValue::GetValueAsFloat()
	bool					GetValueAsDouble(double* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameterValue::GetValueAsDouble()
	bool					GetValueAsString(AAX_IString* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameterValue::GetValueAsString()
	//@} Typed accessors
	
private:
	void InitIdentifier(const char* inIdentifier);
	
private:
	char mIdentifier[eParameterDefaultMaxIdentifierSize];
	T mValue;
};


////// AAX_CParameterValue Template Definition ///////

template <typename T>
AAX_CParameterValue<T>::AAX_CParameterValue(AAX_CParamID identifier)
: mValue()
{
	InitIdentifier(identifier);
}

template <typename T>
AAX_CParameterValue<T>::AAX_CParameterValue(AAX_CParamID identifier, const T& value)
: mValue(value)
{
	InitIdentifier(identifier);
}

template <typename T>
AAX_CParameterValue<T>::AAX_CParameterValue(const AAX_CParameterValue<T>& other)
: mValue(other.mValue)
{
	InitIdentifier(other.mIdentifier);
}

template<typename T>
bool		AAX_CParameterValue<T>::GetValueAsBool(bool* /*value*/) const
{
	return false;
}
template <>
bool		AAX_CParameterValue<bool>::GetValueAsBool(bool* value) const;


template<typename T>
bool		AAX_CParameterValue<T>::GetValueAsInt32(int32_t* /*value*/) const
{
	return false;
}
template<>
bool		AAX_CParameterValue<int32_t>::GetValueAsInt32(int32_t* value) const;

template<typename T>
bool		AAX_CParameterValue<T>::GetValueAsFloat(float* /*value*/) const
{
	return false;
}
template<>
bool		AAX_CParameterValue<float>::GetValueAsFloat(float* value) const;

template<typename T>
bool		AAX_CParameterValue<T>::GetValueAsDouble(double* /*value*/) const
{
	return false;
}
template<>
bool		AAX_CParameterValue<double>::GetValueAsDouble(double* value) const;

template<typename T>
bool		AAX_CParameterValue<T>::GetValueAsString(AAX_IString* /*value*/) const
{
	return false;
}
template<>
bool		AAX_CParameterValue<AAX_CString>::GetValueAsString(AAX_IString* value) const;

template<typename T>
void AAX_CParameterValue<T>::InitIdentifier(const char *inIdentifier)
{

	const size_t len = strlen(inIdentifier);
	AAX_ASSERT(len < eParameterDefaultMaxIdentifierSize);
    if (len < eParameterDefaultMaxIdentifierSize)
    {
        std::strncpy(mIdentifier, inIdentifier, 1+len);
		mIdentifier[len] = 0;
    }
    else
    {
        std::strncpy(mIdentifier, inIdentifier, eParameterDefaultMaxIdentifierLength);
		mIdentifier[eParameterDefaultMaxIdentifierLength] = 0;
    }
}


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#endif
///////////////////////////////////////////////////////////////

////// AAX_CParameter Class Template Declaration ///////

/**	\brief Generic implementation of an \ref AAX_IParameter
	
	\details
	This is a concrete, templatized implementation of \ref AAX_IParameter for parameters with standard
	types such as \c float, \c uint32, \c bool, etc. 

	Many different behaviors can be composited into this class as delegates.  \ref AAX_ITaperDelegate
	and \ref AAX_IDisplayDelegate are two examples of delegates that this class uses in order to apply
	custom behaviors to the \ref AAX_IParameter interface.

	Plug-in developers can subclass these delegates to create adaptable, reusable parameter
	behaviors, which can then be "mixed in" to individual \ref AAX_CParameter objects without the need
	to modify the objects themselves.

	\note Because \ref AAX_CParameter is a C++ template, each \ref AAX_CParameter template parameter that is
	used creates a new subclass that adheres to the \ref AAX_IParameter interface. 

	\ingroup AAXLibraryFeatures_ParameterManager

 */
template <typename T>
class AAX_CParameter : public AAX_IParameter
{
public:
	
	enum Type {
		eParameterTypeUndefined = 0,
		eParameterTypeBool = 1,
		eParameterTypeInt32 = 2,
		eParameterTypeFloat = 3,
		eParameterTypeCustom = 4
	};
    
    enum Defaults {
        eParameterDefaultNumStepsDiscrete = 2,
        eParameterDefaultNumStepsContinuous = 128
    };
	
	/*!
	 *  \brief Constructs an \ref AAX_CParameter object using the specified taper and display delegates.
	 *
	 *	The delegates are passed in by reference to prevent ambiguities of object ownership.  For
	 *	more information about \p identifer and \p name, please consult the base \ref AAX_IParameter
	 *	interface.
	 *
	 *	\param[in] identifier
	 *		Unique ID for the parameter, these can only be 31 characters long at most.  (the fixed length is a requirement for some optimizations in the host)
	 *	\param[in] name
	 *		The parameter's unabbreviated display name
	 *	\param[in] defaultValue
	 *		The parameter's default value
	 *	\param[in] taperDelegate
	 *		A delegate representing the parameter's taper behavior
	 *	\param[in] displayDelegate
	 *		A delegate representing the parameter's display conversion behavior 
	 *	\param[in] automatable
	 *		A flag to set whether the parameter will be visible to the host's automation system
     *
     *	\note Upon construction, the state (value) of the parameter will be the default value, as
	 *	established by the provided \p taperDelegate.
     *
     *	\compatibility As of Pro Tools 10.2, DAE will check for a matching parameter NAME and not an ID when
     *  reading in automation data from a session saved with an %AAX plug-ins RTAS/TDM counter part.
	 *	\compatibility As of Pro Tools 11.1, AAE will first try to match ID. If that fails, AAE will fall
	 *	back to matching by Name.
     *  
     *
	 */	
	AAX_CParameter(AAX_CParamID identifier, const AAX_IString& name, T defaultValue, const AAX_ITaperDelegate<T>& taperDelegate, const AAX_IDisplayDelegate<T>& displayDelegate, bool automatable=false);
	
	/*!
	 *  \brief Constructs an \ref AAX_CParameter object using the specified taper and display delegates.
	 *
	 *	This constructor uses an \ref AAX_IString for the parameter identifier, which can be a more
	 *	flexible solution for some plug-ins.
	 */
	AAX_CParameter(const AAX_IString& identifier, const AAX_IString& name, T defaultValue, const AAX_ITaperDelegate<T>& taperDelegate, const AAX_IDisplayDelegate<T>& displayDelegate, bool automatable=false);
	
	/*!
	 *	\brief Constructs an \ref AAX_CParameter object with no delegates
	 *
	 *	Delegates may be set on this object after construction. Most parameter operations will not work
	 *	until after delegages have been set.
	 *
	 *	- \sa \ref AAX_CParameter::SetTaperDelegate()
	 *	- \sa \ref AAX_CParameter::SetDisplayDelegate()
	 */
	AAX_CParameter(const AAX_IString& identifier, const AAX_IString& name, T defaultValue, bool automatable=false);
	
	/*!
	 *	\brief Constructs an \ref AAX_CParameter object with no delegates or default value
	 *
	 *	Delegates and default value may be set on this object after construction. Most parameter operations
	 *	will not work until after delegages have been set.
	 *
	 *	- \sa \ref AAX_CParameter::SetDefaultValue()
	 *	- \sa \ref AAX_CParameter::SetTaperDelegate()
	 *	- \sa \ref AAX_CParameter::SetDisplayDelegate()
	 */
	AAX_CParameter(const AAX_IString& identifier, const AAX_IString& name, bool automatable=false);
	
	/** Move constructor and move assignment operator are allowed */
	AAX_DEFAULT_MOVE_CTOR(AAX_CParameter);
	AAX_DEFAULT_MOVE_OPER(AAX_CParameter);
	
	/** Default constructor not allowed, except by possible wrappering classes. */
	AAX_DELETE(AAX_CParameter());
	AAX_DELETE(AAX_CParameter(const AAX_CParameter& other));
	AAX_DELETE(AAX_CParameter& operator= (const AAX_CParameter& other));
	
	/*!
	 *  \brief Virtual destructor used to delete all locally allocated pointers.
	 *
	 */
	~AAX_CParameter() AAX_OVERRIDE;
	
	AAX_IParameterValue*	CloneValue() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::CloneValue()
	
	/** @name Identification methods
	 *
	 */
	//@{
	AAX_CParamID		Identifier() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::Identifier()
	void				SetName(const AAX_CString& name) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetName()
	const AAX_CString&	Name() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::Name()
	void				AddShortenedName(const AAX_CString& name) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::AddShortenedName()
	const AAX_CString&	ShortenedName(int32_t iNumCharacters) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::ShortenedName()
	void				ClearShortenedNames() AAX_OVERRIDE; ///< \copydoc AAX_IParameter::ClearShortenedNames()
	//@} Identification methods

	/** @name Taper methods
	 *
	 */
	//@{
	void				SetNormalizedDefaultValue(double normalizedDefault) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetNormalizedDefaultValue()
	double				GetNormalizedDefaultValue() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNormalizedDefaultValue()
	void				SetToDefaultValue() AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetToDefaultValue()
	void				SetNormalizedValue(double newNormalizedValue) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetNormalizedValue()
	double				GetNormalizedValue() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNormalizedValue()
	void				SetNumberOfSteps(uint32_t numSteps) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetNumberOfSteps()
	uint32_t			GetNumberOfSteps() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNumberOfSteps()
	uint32_t			GetStepValue() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetStepValue()
	double				GetNormalizedValueFromStep(uint32_t iStep) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNormalizedValueFromStep()
	uint32_t			GetStepValueFromNormalizedValue(double normalizedValue) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetStepValueFromNormalizedValue()
	void				SetStepValue(uint32_t iStep) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetStepValue()
	void				SetType(AAX_EParameterType iControlType) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetType()
	AAX_EParameterType	GetType() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetType()
	void				SetOrientation( AAX_EParameterOrientation iOrientation ) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetOrientation()
	AAX_EParameterOrientation	GetOrientation() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetOrientation()
	void				SetTaperDelegate(AAX_ITaperDelegateBase& inTaperDelegate,bool inPreserveValue=true) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetTaperDelegate()
	//@} Taper methods

	/** @name Display methods
	 *
	 */
	//@{
	void				SetDisplayDelegate(AAX_IDisplayDelegateBase& inDisplayDelegate) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetDisplayDelegate()
	bool				GetValueString( AAX_CString* valueString) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetValueString(AAX_CString*) const
	bool				GetValueString(int32_t iMaxNumChars, AAX_CString* valueString) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetValueString(int32_t, AAX_CString*) const
	bool				GetNormalizedValueFromBool(bool value, double *normalizedValue) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNormalizedValueFromBool()
	bool				GetNormalizedValueFromInt32(int32_t value, double *normalizedValue) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNormalizedValueFromInt32()
	bool				GetNormalizedValueFromFloat(float value, double *normalizedValue) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNormalizedValueFromFloat()
	bool				GetNormalizedValueFromDouble(double value, double *normalizedValue) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNormalizedValueFromDouble()
	bool				GetNormalizedValueFromString(const AAX_CString&	valueString, double *normalizedValue) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetNormalizedValueFromString()
	bool				GetBoolFromNormalizedValue(double normalizedValue, bool* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetBoolFromNormalizedValue()
	bool				GetInt32FromNormalizedValue(double normalizedValue, int32_t* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetInt32FromNormalizedValue()
	bool				GetFloatFromNormalizedValue(double normalizedValue, float* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetFloatFromNormalizedValue()
	bool				GetDoubleFromNormalizedValue(double normalizedValue, double* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetDoubleFromNormalizedValue()
	bool				GetStringFromNormalizedValue(double normalizedValue, AAX_CString& valueString) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetStringFromNormalizedValue(double, AAX_CString&) const
	bool				GetStringFromNormalizedValue(double normalizedValue, int32_t iMaxNumChars, AAX_CString&	valueString) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetStringFromNormalizedValue(double, int32_t, AAX_CString&) const
	bool				SetValueFromString(const AAX_CString&	newValueString) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetValueFromString()
	//@} Display methods

	/** @name Automation methods
	 *
	 */
	//@{
	void				SetAutomationDelegate ( AAX_IAutomationDelegate * iAutomationDelegate ) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetAutomationDelegate()
	bool				Automatable() const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::Automatable()
	void				Touch() AAX_OVERRIDE; ///< \copydoc AAX_IParameter::Touch()
	void				Release() AAX_OVERRIDE; ///< \copydoc AAX_IParameter::Release()
	//@} Automation methods
	
	/** @name Typed accessors
	 *
	 */
	//@{
	bool				GetValueAsBool(bool* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetValueAsBool()
	bool				GetValueAsInt32(int32_t* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetValueAsInt32()
	bool				GetValueAsFloat(float* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetValueAsFloat()
	bool				GetValueAsDouble(double* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetValueAsDouble()
	bool				GetValueAsString(AAX_IString* value) const AAX_OVERRIDE; ///< \copydoc AAX_IParameter::GetValueAsString()
	bool				SetValueWithBool(bool value) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetValueWithBool()
	bool				SetValueWithInt32(int32_t value) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetValueWithInt32()
	bool				SetValueWithFloat(float value) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetValueWithFloat()
	bool				SetValueWithDouble(double value) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetValueWithDouble()
	bool				SetValueWithString(const AAX_IString& value) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::SetValueWithString()
	//@} Typed accessors
	
	/** @name Host interface methods
	 *
	 */
	//@{
	void				UpdateNormalizedValue(double newNormalizedValue) AAX_OVERRIDE; ///< \copydoc AAX_IParameter::UpdateNormalizedValue()
	//@} Host interface methods
	
	/**	@name Direct methods on AAX_CParameter
	 *
	 *	These methods can be used to access the parameter's state and properties.  These methods
	 *	are specific to the concrete AAX_CParameter class and are not part of the AAX_IParameter
	 *	interface.
	 */
	//@{
	/*!
	 *  \brief Initiates a host request to set the parameter's value
	 *
	 *	This method normalizes the provided value and sends a request for the value change to the
	 *	%AAX host.  The host responds with a call to AAX_IParameter::UpdateNormalizedValue() to
	 *	complete the set operation.
	 *
	 *	\param[in] newValue
	 *		The parameter's new value
	 */	
	void							SetValue(T newValue );
	/*!
	 *  \brief Returns the parameter's value
	 *
	 *	This is the parameter's real, logical value and should not be normalized
	 *
	 */	
	T								GetValue() const;
	/*!
	 *  \brief Set the parameter's default value
	 *
	 *	This is the parameter's real, logical value and should not be normalized
	 *
	 *	\param[in] newDefaultValue
	 *		The parameter's new default value
	 */	
	void							SetDefaultValue(T newDefaultValue);
	/*!
	 *  \brief Returns the parameter's default value
	 *
	 *	This is the parameter's real, logical value and should not be normalized
	 *
	 */	
	T								GetDefaultValue() const;
	/*!
	 *  \brief Returns a reference to the parameter's taper delegate
	 *
	 */	
	const AAX_ITaperDelegate<T>*	TaperDelegate() const;
	/*!
	 *  \brief Returns a reference to the parameter's display delegate
	 *
	 */	
	const AAX_IDisplayDelegate<T>*	DisplayDelegate() const;
	//@} Direct methods on AAX_CParameter
	
protected:
	AAX_CStringAbbreviations					mNames;
	bool										mAutomatable;
	uint32_t									mNumSteps;
	AAX_EParameterType							mControlType;
	AAX_EParameterOrientation					mOrientation;
	AAX_ITaperDelegate<T> *						mTaperDelegate;
	AAX_IDisplayDelegate<T> *					mDisplayDelegate;
	AAX_IAutomationDelegate *					mAutomationDelegate;
	bool										mNeedNotify;
	
	AAX_CParameterValue<T>	mValue;
	T						mDefaultValue;
	
private:
	void InitializeNumberOfSteps();
};


////// AAX_CParameter Template Definition ///////

template <typename T>
AAX_CParameter<T>::AAX_CParameter(AAX_CParamID identifier, const AAX_IString& name, T defaultValue, const AAX_ITaperDelegate<T>& taperDelegate, const AAX_IDisplayDelegate<T>& displayDelegate, bool automatable)
: mNames(name)
, mAutomatable(automatable)
, mNumSteps(0) // Default set below for discrete/continuous
, mControlType( AAX_eParameterType_Continuous )
, mOrientation( AAX_eParameterOrientation_Default )
, mTaperDelegate(taperDelegate.Clone())
, mDisplayDelegate(displayDelegate.Clone())
, mAutomationDelegate(0)
, mNeedNotify(true)
, mValue(identifier)
, mDefaultValue(defaultValue)
{ 
    this->InitializeNumberOfSteps();
	this->SetToDefaultValue();
}

template <typename T>
AAX_CParameter<T>::AAX_CParameter(const AAX_IString& identifier, const AAX_IString& name, T defaultValue, const AAX_ITaperDelegate<T>& taperDelegate, const AAX_IDisplayDelegate<T>& displayDelegate, bool automatable)
: mNames(name)
, mAutomatable(automatable)
, mNumSteps(0) // Default set below for discrete/continuous
, mControlType( AAX_eParameterType_Continuous )
, mOrientation( AAX_eParameterOrientation_Default )
, mTaperDelegate(taperDelegate.Clone())
, mDisplayDelegate(displayDelegate.Clone())
, mAutomationDelegate(0)
, mNeedNotify(true)
, mValue(identifier.Get())
, mDefaultValue(defaultValue)
{
    this->InitializeNumberOfSteps();
	this->SetToDefaultValue();
}

template <typename T>
AAX_CParameter<T>::AAX_CParameter(const AAX_IString& identifier, const AAX_IString& name, T defaultValue, bool automatable)
: mNames(name)
, mAutomatable(automatable)
, mNumSteps(0)
, mControlType( AAX_eParameterType_Continuous )
, mOrientation( AAX_eParameterOrientation_Default )
, mTaperDelegate(NULL)
, mDisplayDelegate(NULL)
, mAutomationDelegate(NULL)
, mNeedNotify(true)
, mValue(identifier.Get())
, mDefaultValue(defaultValue)
{
	this->InitializeNumberOfSteps();
	this->SetToDefaultValue();
}

template <typename T>
AAX_CParameter<T>::AAX_CParameter(const AAX_IString& identifier, const AAX_IString& name, bool automatable)
: mNames(name)
, mAutomatable(automatable)
, mNumSteps(0)
, mControlType( AAX_eParameterType_Continuous )
, mOrientation( AAX_eParameterOrientation_Default )
, mTaperDelegate(NULL)
, mDisplayDelegate(NULL)
, mAutomationDelegate(NULL)
, mNeedNotify(true)
, mValue(identifier.Get())
, mDefaultValue()
{
	this->InitializeNumberOfSteps();
	this->SetToDefaultValue(); // WARNING: uninitialized default value
}

template <typename T>
AAX_CParameter<T>::~AAX_CParameter()
{
	//Make sure to remove any registration with the token system.
	SetAutomationDelegate(0);
	
	delete mTaperDelegate;
	mTaperDelegate = 0;
	delete mDisplayDelegate;
	mDisplayDelegate = 0;
}

template <typename T>
AAX_IParameterValue*	AAX_CParameter<T>::CloneValue() const
{
	return new AAX_CParameterValue<T>(mValue);
}

template <typename T>
AAX_CParamID		AAX_CParameter<T>::Identifier() const					
{	
	return mValue.Identifier();
}

template <typename T>
void		AAX_CParameter<T>::SetName(const AAX_CString& name)	
{   
	mNames.SetPrimary(name);
	if (mAutomationDelegate) {
		mAutomationDelegate->ParameterNameChanged(this->Identifier());
	}
}

template <typename T>
const AAX_CString&	AAX_CParameter<T>::Name() const						
{	
	return mNames.Primary();
}

template <typename T>
void	AAX_CParameter<T>::AddShortenedName(const AAX_CString& name)
{
	mNames.Add(name);
}

template <typename T>
const AAX_CString&	AAX_CParameter<T>::ShortenedName(int32_t iNumCharacters) const
{
	return mNames.Get(iNumCharacters);
}

template <typename T>
void				AAX_CParameter<T>::ClearShortenedNames()
{
	mNames.Clear();
}



template<typename T>
void	AAX_CParameter<T>::SetValue( T newValue )
{
	double	newNormalizedValue = mTaperDelegate->RealToNormalized(newValue);

	// <DMT> Always go through the automation delegate even if the control isn't automatable to prevent fighting with other GUIs.
	// Somewhere back in the automation delegate, or elsewhere in the system, it will determine the differences in behavior surrounding
	// automation.  The only reason that there wouldn't be an automation delegate is if this parameter has yet to be added to a
	// ParameterManager.  Let's put the null value guards in place, just in case, and also for unit tests.
	if ( mAutomationDelegate )
	{
		//TODO: Create RAII utility class for touch/release
		
		//Touch the control
		Touch();
		
		//Send that token.
		mAutomationDelegate->PostSetValueRequest(Identifier(), newNormalizedValue );

		//Release the control
		Release();
	}
	else
	{
		mNeedNotify = true;

		// In the rare case that an automation delegate doesn't exist, lets still set the value.  It's possible that someone is trying to
		// set the new value before adding the parameter to a parametermanager.
		UpdateNormalizedValue(newNormalizedValue);
	}
}

template <typename T>
void	AAX_CParameter<T>::UpdateNormalizedValue(double newNormalizedValue)
{
	T newValue = mTaperDelegate->NormalizedToReal(newNormalizedValue);
	if (mNeedNotify || (mValue.Get() != newValue))
	{
		//Set the new value
		mValue.Set(newValue);
				
		//<DMT> Always notify that the value has changed through the automation delegate to guarantee that all control surfaces and other
		// GUIs get their values updated.
		if (mAutomationDelegate)
			mAutomationDelegate->PostCurrentValue(Identifier(), newNormalizedValue);

		// clear flag
		mNeedNotify = false;
	}	
}

template <typename T>
void	AAX_CParameter<T>::InitializeNumberOfSteps()
{
	if (mNumSteps == 0) // If no explicit number of steps has been set...
    {
        switch (mControlType)
        {
            case AAX_eParameterType_Discrete:
            {
                // Discrete parameters default to binary unless
                // otherwise specified
                this->SetNumberOfSteps (eParameterDefaultNumStepsDiscrete);
                break;
            }
            case AAX_eParameterType_Continuous:
            {
                // Defaulting to 128 steps to match one full rotation of
                // Command|8 and similar surfaces, which query the num
                // steps to determine tick values for rotary encoders
                this->SetNumberOfSteps (eParameterDefaultNumStepsContinuous);
                break;
            }
            default:
            {
                AAX_ASSERT (0); // Invalid type
                break;
            }
        }
    }
}

template<typename T>
T		AAX_CParameter<T>::GetValue()	const
{
	return mValue.Get();
}


template<typename T>
bool		AAX_CParameter<T>::GetValueAsBool(bool* value) const
{
	return mValue.GetValueAsBool(value);
}

template<typename T>
bool		AAX_CParameter<T>::GetValueAsInt32(int32_t* value) const
{
	return mValue.GetValueAsInt32(value);
}

template<typename T>
bool		AAX_CParameter<T>::GetValueAsFloat(float* value) const
{
	return mValue.GetValueAsFloat(value);
}

template<typename T>
bool		AAX_CParameter<T>::GetValueAsDouble(double* value) const
{
	return mValue.GetValueAsDouble(value);
}

template<typename T>
bool		AAX_CParameter<T>::GetValueAsString(AAX_IString* value) const
{
	bool result = false;
	if (value)
	{
		AAX_CString valueString;
		result = this->GetValueString(&valueString);
		if (true == result)
		{
			*value = valueString;
		}
	}
	return result;
}

template<>
bool		AAX_CParameter<AAX_CString>::GetValueAsString(AAX_IString* /*value*/) const;


template<typename T>
bool		AAX_CParameter<T>::SetValueWithBool(bool /*value*/)
{
	return false;
}
template<>
bool		AAX_CParameter<bool>::SetValueWithBool(bool value);

template<typename T>
bool		AAX_CParameter<T>::SetValueWithInt32(int32_t /*value*/)
{
	return false;
}
template<>
bool		AAX_CParameter<int32_t>::SetValueWithInt32(int32_t value);

template<typename T>
bool		AAX_CParameter<T>::SetValueWithFloat(float /*value*/)
{
	return false;
}
template<>
bool		AAX_CParameter<float>::SetValueWithFloat(float value);

template<typename T>
bool		AAX_CParameter<T>::SetValueWithDouble(double /*value*/)
{
	return false;
}
template<>
bool		AAX_CParameter<double>::SetValueWithDouble(double value);

template<typename T>
bool		AAX_CParameter<T>::SetValueWithString(const AAX_IString& value)
{
	const AAX_CString valueString(value);
	return this->SetValueFromString(valueString);
}
template<>
bool		AAX_CParameter<AAX_CString>::SetValueWithString(const AAX_IString& value);

template<typename T>
void	AAX_CParameter<T>::SetNormalizedDefaultValue(double newNormalizedDefault)
{
	T newDefaultValue = mTaperDelegate->NormalizedToReal(newNormalizedDefault);
	SetDefaultValue(newDefaultValue);
}

template<typename T>
double	AAX_CParameter<T>::GetNormalizedDefaultValue() const
{
	double normalizedDefault = mTaperDelegate->RealToNormalized(mDefaultValue);
	return normalizedDefault;
}

template<typename T>
void	AAX_CParameter<T>::SetDefaultValue(T newDefaultValue)
{
	newDefaultValue = mTaperDelegate->ConstrainRealValue(newDefaultValue);
	mDefaultValue = newDefaultValue;
}

template<typename T>
T		AAX_CParameter<T>::GetDefaultValue() const
{
	return mDefaultValue;
}

template<typename T>
void	AAX_CParameter<T>::SetToDefaultValue()
{
	SetValue(mDefaultValue);
}

template<typename T>
void	AAX_CParameter<T>::SetNumberOfSteps(uint32_t numSteps)
{
	AAX_ASSERT(0 < numSteps);
	if (0 < numSteps)
	{
		mNumSteps = numSteps;
	}
}

template<typename T>
uint32_t	AAX_CParameter<T>::GetNumberOfSteps() const
{
	return mNumSteps;
}

template<typename T>
uint32_t	AAX_CParameter<T>::GetStepValue() const
{
	return GetStepValueFromNormalizedValue(this->GetNormalizedValue());
}

template<typename T>
double		AAX_CParameter<T>::GetNormalizedValueFromStep(uint32_t iStep) const
{
	double	numSteps = (double) this->GetNumberOfSteps ();
	if ( numSteps < 2.0 )
		return 0.0;
	
	double	valuePerStep = 1.0 / ( numSteps - 1.0 );
	double	value = valuePerStep * (double) iStep;
	if ( value < 0.0 )
		value = 0.0;
	else if ( value > 1.0 )
		value = 1.0;
	
	return value;
}

template<typename T>
uint32_t	AAX_CParameter<T>::GetStepValueFromNormalizedValue(double normalizedValue) const
{
	double	numSteps = (double) this->GetNumberOfSteps ();
	if ( numSteps < 2.0 )
		return 0;
	
	double	valuePerStep = 1.0 / ( numSteps - 1.0 );
	double	curStep = ( normalizedValue / valuePerStep ) + 0.5;
	if ( curStep < 0.0 )
		curStep = 0.0;
	else if ( curStep > (double) ( numSteps - 1.0 ) )
		curStep = (double) ( numSteps - 1.0 );
	
	return (uint32_t) curStep;
}

template<typename T>
void	AAX_CParameter<T>::SetStepValue(uint32_t iStep)
{
	double	numSteps = (double) this->GetNumberOfSteps ();
	if ( numSteps < 2.0 )
		return;
	
	this->SetNormalizedValue ( GetNormalizedValueFromStep(iStep) );
}

template<typename T>
void	AAX_CParameter<T>::SetType(AAX_EParameterType iControlType)
{
	mControlType = iControlType;
}

template<typename T>
AAX_EParameterType	AAX_CParameter<T>::GetType() const
{
	return mControlType;
}

template<typename T>
void	AAX_CParameter<T>::SetOrientation(AAX_EParameterOrientation iOrientation)
{
	mOrientation = iOrientation;
}

template<typename T>
AAX_EParameterOrientation	AAX_CParameter<T>::GetOrientation() const
{
	return mOrientation;
}

template<typename T>
void	AAX_CParameter<T>::SetNormalizedValue(double normalizedNewValue)
{
	T newValue = mTaperDelegate->NormalizedToReal(normalizedNewValue);
	this->SetValue(newValue);
}

template<typename T>
double	AAX_CParameter<T>::GetNormalizedValue()	const
{
	T val = GetValue();
	return mTaperDelegate->RealToNormalized(val);
}	


template<typename T>
bool	AAX_CParameter<T>::GetValueString(AAX_CString*	valueString) const
{
	return mDisplayDelegate->ValueToString(this->GetValue(), valueString);
}

template<typename T>
bool	AAX_CParameter<T>::GetValueString(int32_t /*iMaxNumChars*/, AAX_CString*	valueString) const
{
	return mDisplayDelegate->ValueToString(this->GetValue(), valueString);
}

template <typename T>
bool	AAX_CParameter<T>::GetNormalizedValueFromBool(bool /*value*/, double * /*normalizedValue*/) const
{
	return false;
}
template <>
bool	AAX_CParameter<bool>::GetNormalizedValueFromBool(bool value, double *normalizedValue) const;

template <typename T>
bool	AAX_CParameter<T>::GetNormalizedValueFromInt32(int32_t /*value*/, double * /*normalizedValue*/) const
{
	return false;
}
template <>
bool	AAX_CParameter<int32_t>::GetNormalizedValueFromInt32(int32_t value, double *normalizedValue) const;

template <typename T>
bool	AAX_CParameter<T>::GetNormalizedValueFromFloat(float /*value*/, double * /*normalizedValue*/) const
{
	return false;
}
template <>
bool	AAX_CParameter<float>::GetNormalizedValueFromFloat(float value, double *normalizedValue) const;

template <typename T>
bool	AAX_CParameter<T>::GetNormalizedValueFromDouble(double /*value*/, double * /*normalizedValue*/) const
{
	return false;
}
template <>
bool	AAX_CParameter<double>::GetNormalizedValueFromDouble(double value, double *normalizedValue) const;

template <typename T>
bool	AAX_CParameter<T>::GetNormalizedValueFromString(const AAX_CString&	valueString, double *normalizedValue) const
{
	//First, convert the string to a value using the wrapped parameter's display delegate.
	T value;
	if (!mDisplayDelegate->StringToValue(valueString, &value))
		return false;
	
	//Then use the wrapped parameter's taper delegate to convert to a normalized representation.
	//If the parameter is out of range, the normalizedValue will be clamped just to be safe.
	*normalizedValue = mTaperDelegate->RealToNormalized(value);	
	return true;	
}

template<typename T>
bool		AAX_CParameter<T>::GetBoolFromNormalizedValue(double /*inNormalizedValue*/, bool* /*value*/) const
{
	return false;
}
template <>
bool		AAX_CParameter<bool>::GetBoolFromNormalizedValue(double inNormalizedValue, bool* value) const;


template<typename T>
bool		AAX_CParameter<T>::GetInt32FromNormalizedValue(double /*inNormalizedValue*/, int32_t* /*value*/) const
{
	return false;
}
template<>
bool		AAX_CParameter<int32_t>::GetInt32FromNormalizedValue(double inNormalizedValue, int32_t* value) const;

template<typename T>
bool		AAX_CParameter<T>::GetFloatFromNormalizedValue(double /*inNormalizedValue*/, float* /*value*/) const
{
	return false;
}
template<>
bool		AAX_CParameter<float>::GetFloatFromNormalizedValue(double inNormalizedValue, float* value) const;

template<typename T>
bool		AAX_CParameter<T>::GetDoubleFromNormalizedValue(double /*inNormalizedValue*/, double* /*value*/) const
{
	return false;
}
template<>
bool		AAX_CParameter<double>::GetDoubleFromNormalizedValue(double inNormalizedValue, double* value) const;

template <typename T>
bool	AAX_CParameter<T>::GetStringFromNormalizedValue(double normalizedValue, AAX_CString&	valueString) const
{
	T value = mTaperDelegate->NormalizedToReal(normalizedValue);
	if (!mDisplayDelegate->ValueToString(value, &valueString))
		return false;	
	
	//If the parameter is out of range, we should probably return false, even though we clamped the normalizedValue already just to be safe.
	if ((value > mTaperDelegate->GetMaximumValue()) || (value < mTaperDelegate->GetMinimumValue()))
		return false;	
	return true;
}

template <typename T>
bool	AAX_CParameter<T>::GetStringFromNormalizedValue(double normalizedValue, int32_t iMaxNumChars, AAX_CString&	valueString) const
{
	T value = mTaperDelegate->NormalizedToReal(normalizedValue);
	if (!mDisplayDelegate->ValueToString(value, iMaxNumChars, &valueString))
		return false;	
	
	//If the parameter is out of range, we should probably return false, even though we clamped the normalizedValue already just to be safe.
	if ((value > mTaperDelegate->GetMaximumValue()) || (value < mTaperDelegate->GetMinimumValue()))
		return false;	
	return true;
}

template<typename T>
bool	AAX_CParameter<T>::SetValueFromString(const AAX_CString&	newValueString)
{	
	T newValue;
	if (!mDisplayDelegate->StringToValue(newValueString, &newValue))
		return false;
	SetValue(newValue);
	return true;
}

template<typename T>
void	AAX_CParameter<T>::SetTaperDelegate(AAX_ITaperDelegateBase& inTaperDelegate,bool inPreserveValue)
{
	double	normalizeValue = this->GetNormalizedValue ();

	AAX_ITaperDelegate<T>* oldDelegate = mTaperDelegate;
	mTaperDelegate = ((AAX_ITaperDelegate<T> &) inTaperDelegate).Clone();
	delete oldDelegate;

	mNeedNotify = true;
	if ( inPreserveValue )
		this->SetValue ( mValue.Get() );
	else this->UpdateNormalizedValue ( normalizeValue );
}

template<typename T>
void	AAX_CParameter<T>::SetDisplayDelegate(AAX_IDisplayDelegateBase& inDisplayDelegate)
{
	AAX_IDisplayDelegate<T>* oldDelegate = mDisplayDelegate;
	mDisplayDelegate = ((AAX_IDisplayDelegate<T> &)inDisplayDelegate).Clone();
	delete oldDelegate;
	
	if (mAutomationDelegate != 0)
		mAutomationDelegate->PostCurrentValue(this->Identifier(), this->GetNormalizedValue());		//<DMT> Make sure GUIs are all notified of the change.
}

template<typename T>
const AAX_ITaperDelegate<T>*	AAX_CParameter<T>::TaperDelegate() const
{
	return mTaperDelegate;
}

template<typename T>
const AAX_IDisplayDelegate<T>*	AAX_CParameter<T>::DisplayDelegate() const
{
	return mDisplayDelegate;
}

template<typename T>
bool	AAX_CParameter<T>::Automatable() const			
{	
	return mAutomatable;
}

template<typename T>
void	AAX_CParameter<T>::SetAutomationDelegate ( AAX_IAutomationDelegate * iAutomationDelegate )
{
	//Remove the old automation delegate
	if ( mAutomationDelegate )
	{
		mAutomationDelegate->UnregisterParameter ( this->Identifier() );
	}
	
	//Add the new automation delegate, wrapped by the versioning layer.
	mAutomationDelegate = iAutomationDelegate;
	if ( mAutomationDelegate )
		mAutomationDelegate->RegisterParameter ( this->Identifier() );
}

template<typename T>
void	AAX_CParameter<T>::Touch()						
{ 	
	//<DT>  Always send the touch command, even if the control isn't automatable.
	if (mAutomationDelegate)
		mAutomationDelegate->PostTouchRequest( this->Identifier() );
}				

template<typename T>
void	AAX_CParameter<T>::Release()					
{ 	
	//<DT>  Always send the release command, even if the control isn't automatable.
	if (mAutomationDelegate)
		mAutomationDelegate->PostReleaseRequest( this->Identifier() );
}


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#pragma mark AAX_CStatelessParameter
#endif
///////////////////////////////////////////////////////////////

/**
 *	\brief A stateless parameter implementation
 *
 *	This can be useful for mapping event triggers to control surface buttons
 *	or to GUI switches.
 */
class AAX_CStatelessParameter : public AAX_IParameter
{
public:
	AAX_CStatelessParameter(AAX_CParamID identifier, const AAX_IString& name, const AAX_IString& inValueString)
	: mNames(name)
	, mID(identifier)
	, mAutomationDelegate(NULL)
	, mValueString(inValueString)
	{
	}
	
	AAX_CStatelessParameter(const AAX_IString& identifier, const AAX_IString& name, const AAX_IString& inValueString)
	: mNames(name)
	, mID(identifier)
	, mAutomationDelegate(NULL)
	, mValueString(inValueString)
	{
	}
	
	AAX_DEFAULT_DTOR_OVERRIDE(AAX_CStatelessParameter);
	
	AAX_IParameterValue*	CloneValue() const AAX_OVERRIDE { return NULL; }
	
	/** @name Identification methods
	 *
	 */
	//@{
	AAX_CParamID		Identifier() const AAX_OVERRIDE { return mID.CString(); }
	void				SetName(const AAX_CString& name) AAX_OVERRIDE
	{
		mNames.SetPrimary(name);
		if (mAutomationDelegate) {
			mAutomationDelegate->ParameterNameChanged(this->Identifier());
		}
	}
	const AAX_CString&	Name() const AAX_OVERRIDE { return mNames.Primary(); }
	void				AddShortenedName(const AAX_CString& name) AAX_OVERRIDE { mNames.Add(name); }
	const AAX_CString&	ShortenedName(int32_t iNumCharacters) const AAX_OVERRIDE { return mNames.Get(iNumCharacters); }
	void				ClearShortenedNames() AAX_OVERRIDE { mNames.Clear(); }
	//@} Identification methods
	
	/** @name Automation methods
	 *
	 */
	//@{
	bool		Automatable() const AAX_OVERRIDE { return false; }
	void		SetAutomationDelegate( AAX_IAutomationDelegate * iAutomationDelegate ) AAX_OVERRIDE
	{
		//Remove the old automation delegate
		if ( mAutomationDelegate )
		{
			mAutomationDelegate->UnregisterParameter ( this->Identifier() );
		}
		
		//Add the new automation delegate, wrapped by the versioning layer.
		mAutomationDelegate = iAutomationDelegate;
		if ( mAutomationDelegate )
			mAutomationDelegate->RegisterParameter ( this->Identifier() );
	}
	void		Touch() AAX_OVERRIDE { if (mAutomationDelegate) mAutomationDelegate->PostTouchRequest( this->Identifier() ); }
	void		Release() AAX_OVERRIDE { if (mAutomationDelegate) mAutomationDelegate->PostReleaseRequest( this->Identifier() ); }
	//@} Automation methods
	
	/** @name Taper methods
	 *
	 */
	//@{
	void		SetNormalizedValue(double /*newNormalizedValue*/) AAX_OVERRIDE {}
	double		GetNormalizedValue() const AAX_OVERRIDE { return 0.; }
	void		SetNormalizedDefaultValue(double /*normalizedDefault*/) AAX_OVERRIDE {}
	double		GetNormalizedDefaultValue() const AAX_OVERRIDE { return 0.; }
	void		SetToDefaultValue() AAX_OVERRIDE {}
	void		SetNumberOfSteps(uint32_t /*numSteps*/) AAX_OVERRIDE {}
	uint32_t	GetNumberOfSteps() const AAX_OVERRIDE { return 1; }
	uint32_t	GetStepValue() const AAX_OVERRIDE { return 0; }
	double		GetNormalizedValueFromStep(uint32_t /*iStep*/) const AAX_OVERRIDE { return 0.; }
	uint32_t	GetStepValueFromNormalizedValue(double /*normalizedValue*/) const AAX_OVERRIDE { return 0; }
	void		SetStepValue(uint32_t /*iStep*/) AAX_OVERRIDE {}
	//@} Taper methods
	
	/** @name Display methods
	 *
	 *	This functionality is most often used by GUIs, but can also be useful for state
	 *	serialization.
	 */
	//@{
	bool		GetValueString(AAX_CString*	valueString) const AAX_OVERRIDE { if (valueString) *valueString = mValueString; return true; }
	bool		GetValueString(int32_t /*iMaxNumChars*/, AAX_CString* valueString) const AAX_OVERRIDE { return this->GetValueString(valueString); }
	bool		GetNormalizedValueFromBool(bool /*value*/, double* normalizedValue) const AAX_OVERRIDE { if (normalizedValue) { *normalizedValue = 0.; } return true; }
	bool		GetNormalizedValueFromInt32(int32_t /*value*/, double* normalizedValue) const AAX_OVERRIDE { if (normalizedValue) { *normalizedValue = 0.; } return true; }
	bool		GetNormalizedValueFromFloat(float /*value*/, double* normalizedValue) const AAX_OVERRIDE { if (normalizedValue) { *normalizedValue = 0.; } return true; }
	bool		GetNormalizedValueFromDouble(double /*value*/, double* normalizedValue) const AAX_OVERRIDE { if (normalizedValue) { *normalizedValue = 0.; } return true; }
	bool		GetNormalizedValueFromString(const AAX_CString&	/*valueString*/, double* normalizedValue) const AAX_OVERRIDE { if (normalizedValue) { *normalizedValue = 0.; } return true; }
	bool		GetBoolFromNormalizedValue(double /*normalizedValue*/, bool* value) const AAX_OVERRIDE { if (value) { *value = false; } return true; }
	bool		GetInt32FromNormalizedValue(double /*normalizedValue*/, int32_t* /*value*/) const AAX_OVERRIDE { return false; }
	bool		GetFloatFromNormalizedValue(double /*normalizedValue*/, float* /*value*/) const AAX_OVERRIDE { return false; }
	bool		GetDoubleFromNormalizedValue(double /*normalizedValue*/, double* /*value*/) const AAX_OVERRIDE { return false; }
	bool		GetStringFromNormalizedValue(double /*normalizedValue*/, AAX_CString& valueString) const AAX_OVERRIDE { valueString = mValueString; return true; }
	bool		GetStringFromNormalizedValue(double normalizedValue, int32_t /*iMaxNumChars*/, AAX_CString&	valueString) const AAX_OVERRIDE { return this->GetStringFromNormalizedValue(normalizedValue, valueString); }
	bool		SetValueFromString(const AAX_CString&	newValueString) AAX_OVERRIDE { mValueString = newValueString; return true; }
	//@} Display methods
	
	/** @name Typed accessors
	 *
	 */
	//@{
	bool		GetValueAsBool(bool* value) const AAX_OVERRIDE { if (value) { *value = false; } return true; }
	bool		GetValueAsInt32(int32_t* /*value*/) const AAX_OVERRIDE { return false; }
	bool		GetValueAsFloat(float* /*value*/) const AAX_OVERRIDE { return false; }
	bool		GetValueAsDouble(double* /*value*/) const AAX_OVERRIDE { return false; }
	bool		GetValueAsString(AAX_IString* /*value*/) const AAX_OVERRIDE { return false; }
	bool		SetValueWithBool(bool /*value*/) AAX_OVERRIDE { return true; }
	bool		SetValueWithInt32(int32_t /*value*/) AAX_OVERRIDE { return false; }
	bool		SetValueWithFloat(float /*value*/) AAX_OVERRIDE { return false; }
	bool		SetValueWithDouble(double /*value*/) AAX_OVERRIDE { return false; }
	bool		SetValueWithString(const AAX_IString& value) AAX_OVERRIDE { mValueString = value; return true; }
	//@} Typed accessors
	
	void		SetType( AAX_EParameterType /*iControlType*/ ) AAX_OVERRIDE {};
	AAX_EParameterType	GetType() const AAX_OVERRIDE { return AAX_eParameterType_Discrete; }
	
	void		SetOrientation( AAX_EParameterOrientation /*iOrientation*/ ) AAX_OVERRIDE {}
	AAX_EParameterOrientation	GetOrientation() const AAX_OVERRIDE { return AAX_eParameterOrientation_Default; }
	
	void SetTaperDelegate ( AAX_ITaperDelegateBase & /*inTaperDelegate*/, bool /*inPreserveValue*/ ) AAX_OVERRIDE {};
	void SetDisplayDelegate ( AAX_IDisplayDelegateBase & /*inDisplayDelegate*/ ) AAX_OVERRIDE {};
	
	/** @name Host interface methods
	 *
	 */
	//@{
	void		UpdateNormalizedValue(double /*newNormalizedValue*/) AAX_OVERRIDE {};
	//@} Host interface methods
	
protected:
	AAX_CStringAbbreviations mNames;
	AAX_CString mID;
	AAX_IAutomationDelegate * mAutomationDelegate;
	AAX_CString mValueString;
};




#endif //AAX_CParameter_H
