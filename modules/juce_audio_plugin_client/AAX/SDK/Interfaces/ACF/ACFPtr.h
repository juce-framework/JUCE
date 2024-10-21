/***********************************************************************

	This file is part of the Avid AAX SDK.

	The AAX SDK is subject to commercial or open-source licensing.

	By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
	Agreement and Avid Privacy Policy.

	AAX SDK License: https://developer.avid.com/aax
	Privacy Policy: https://www.avid.com/legal/privacy-policy-statement

	Or: You may also use this code under the terms of the GPL v3 (see
	www.gnu.org/licenses).

	THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
	EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
	DISCLAIMED.

	Copyright (c) 2004, 2024 Avid Technology, Inc. All rights reserved.

************************************************************************/




/*!
	 \file ACFPtr.h
	 \brief Smart pointer template. Used for automation of referance counting
*/

#ifndef ACFPtr_h
#define ACFPtr_h

#include "acfbasetypes.h"
#include "acfresult.h"
#include "acfassert.h"

#include "acfunknown.h" // for IACFUnknown and ACFMETHODCALLTYPE

/*! \def ACFPTR_CAN_THROW
	Macro to conditionally include or exclude the use of C++ exceptions.

	If client code does not need exceptions then define ACFPTR_CAN_THROW 0
	for the each executable project that uses this header.
 */
#ifndef ACFPTR_CAN_THROW
#define ACFPTR_CAN_THROW 1 // default is to allow throwing of C++ exceptions.
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif // __clang__

/*!
	Class used to prevent add ref on stuff by ACFPtr<T>::operator ->().
	Added by Stephen Wilson of Digidesign.
 */
template <class T>
class ACFNoAddRefReleaseOnPtr : public T
{
private:
	virtual acfUInt32 ACFMETHODCALLTYPE AddRef()=0;
	virtual acfUInt32 ACFMETHODCALLTYPE Release()=0;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__


/*!
	 Template arguments:

	 T: the kind of object to which this pointer will
	 point. This class must support QueryInterface(), AddRef() and Release() methods.
*/
template <typename T>
class ACFPtr
{
public:
    /// Default constructor 
    ACFPtr ();

    /// Copy constructor
    ACFPtr (const ACFPtr<T> & rhs);

    /// Construct from a reference, the reference will be AddRef'd.
    ACFPtr (T * reference);

    /// Constructor shorthand for reference0->QueryInterface(iid, (void**)&_reference).
    /// NOTE: The return value from QueryInterface is ignored. if the internal reference
    /// is 0 then eiher the error "was" ACF_E_NOINTERFACE or ACF_E_INVALIDARG if 
    /// reference0 was 0 (or ACF_E_POINTER).
    ACFPtr (const acfIID & iid, IACFUnknown * reference0);

    /// Destructor
    ~ACFPtr ();

    /// assignment operator
    ACFPtr<T> & operator= (const ACFPtr<T> & rhs);

    /// assignment operator for a new reference
    ACFPtr<T> & operator= (T * rhs);

	/// Method that is used pass the internal reference as an input argument to a 
	/// function or method. This is just a more explicit version of the coercion
	/// operator T * () const;
	/// \note The reference count for the returned interface pointer has not been increased with AddRef();
	T* inArg() const ;

	/// Method that is used pass the internal reference as an output argument to a 
	/// function or method.
	/// \note If there is an internal reference to an interface then it will be released.
	T** outArg();

	/// Method that is used pass the internal reference as an in/out argument to a 
	/// function or method.
	/// \note If there is an internal reference to an interface then it is not released.
	/// The function or method that is accepting the in/out argument it responsible for
	/// either releasing, reusing or replacing the given interface pointed to by T*. If
	/// the given interface is reused it does not have to be AddRef'd.
	T** inOutArg();

	/// Allows passing this smart ptr as argument to methods which expect
	/// a T** or a void**, in order to fill it in. (e.g. QueryInterface). NOTE: if the internal
	/// reference is non-NULL it will be Released.
	/// to this object.
	/// \deprecated Please use the outArg() method (or inOutArg()) instead. 
   	T** operator& ();
 
    /// Allows passing this smart ptr as argument to methods which expect
    /// a T *.
    operator T * () const;
 
    /// member access operators. NOTE: this method will throw ACFRESULT(ACF_E_POINTER) 
    /// if _reference is 0.
    ACFNoAddRefReleaseOnPtr<T> * operator-> ();
 
    /// const member access operator. NOTE: this method will throw ACFRESULT(ACF_E_POINTER) 
    /// if _reference is 0. (note: AddRef() and Release() are non-const so they cannot
    /// be called from this const operator.)
    const T * operator-> () const;

    /// Allows caller to determine whether or not the internal reference pointer has been 
    /// assigned.
    bool isNull() const;

    /// Allows calling operator ! just like a regular pointer. Returns true if the 
    /// internal reference pointer isNull.
    bool operator! () const;

    /// Direct assignment of reference without calling AddRef(). This could
    /// be called if an interface has already been AddRef'd.
    void attach(T * reference);

    /// Return the internal reference without calling Release().
    T * detach(void);

private:
    /// Internal method to acquire another reference to the interface stored in _reference
    /// (call AddRef())
    void acquire(void);

    /// Release the current reference.
    void clear(void);

private:
    /// Current referenced interface
    T * _reference;
};




//
// Examples and recommended practices
//

#if 0
#endif






//
// Implementations
//

template <typename T>
inline ACFPtr<T>::ACFPtr ()
    : _reference (0)
{
}


template <typename T>
inline ACFPtr<T>::ACFPtr(const ACFPtr<T> & rhs)
    : _reference (rhs._reference)
{
    acquire();
}


template <typename T>
inline ACFPtr<T>::ACFPtr(T * reference)
    : _reference (reference)
{
    acquire();
}

template <typename T>
inline ACFPtr<T>::ACFPtr (const acfIID & iid, IACFUnknown * reference0)
    : _reference (0)
{
	if (!reference0) {
#if ACFPTR_CAN_THROW
		throw ACFRESULT( ACF_E_INVALIDARG );
#else
		ACFASSERT(reference0);
#endif
	} else {
		ACFRESULT result = reference0->QueryInterface (iid, reinterpret_cast <void**> (&_reference));
		if (ACFFAILED(result))
#if ACFPTR_CAN_THROW
			throw ACFRESULT(result);
#else
			ACFASSERT(ACFSUCCEEDED(result));
#endif
	}
}

template <typename T>
inline ACFPtr<T>::~ACFPtr ()
{
    clear();
}


template <typename T>
inline ACFPtr<T> & ACFPtr<T>::operator= (const ACFPtr<T> & rhs)
{
    if (&rhs != this)
    {
        clear();
        _reference = rhs._reference;
        acquire();
    }
    return *this;
}


template <typename T>
inline ACFPtr<T> & ACFPtr<T>::operator= (T * rhs)
{
    if (rhs != _reference)
    {
        if (rhs)
            rhs->AddRef();
        clear();
        _reference = rhs;
    }
    return *this;
}

template <typename T>
inline T* ACFPtr<T>::inArg() const
{
	return _reference;
}

template <typename T>
inline T**  ACFPtr<T>::outArg()
{
	clear();
	return &_reference;
}

template <typename T>
inline T**  ACFPtr<T>::inOutArg()
{
	return &_reference;
}


template <typename T>
inline T** ACFPtr<T>::operator & ()
{
	clear();
    return &_reference;
}


template <typename T>
inline ACFPtr<T>::operator T * () const
{
    return _reference;
}


template <typename T>
inline ACFNoAddRefReleaseOnPtr<T>* ACFPtr<T>::operator-> ()
{
#if ACFPTR_CAN_THROW
	if (NULL == _reference)
		throw ACFRESULT(ACF_E_POINTER);
#else
    ACFASSERT(_reference); // reference pointer has not been initialized!
#endif
    return (ACFNoAddRefReleaseOnPtr<T>*)_reference;
}


template <typename T>
inline const T* ACFPtr<T>::operator-> () const
{
#if ACFPTR_CAN_THROW
	if (NULL == _reference)
		throw ACFRESULT(ACF_E_POINTER);
#else
    ACFASSERT(_reference); // reference pointer has not been initialized!
#endif
    return _reference;
}


template <typename T>
inline bool ACFPtr<T>::isNull () const
{
    return (NULL == _reference);
}


template <typename T>
inline bool ACFPtr<T>::operator! () const
{
    return isNull();
}


template <typename T>
inline void ACFPtr<T>::attach (T * reference)
{
    clear();
    _reference = reference;
}


template <typename T>
inline T * ACFPtr<T>::detach (void)
{
#if ACFPTR_CAN_THROW
	if (NULL == _reference)
		throw ACFRESULT(ACF_E_POINTER);
#else
	ACFASSERT(_reference); // reference pointer has not been initialized!
#endif
    T * reference = _reference;
    _reference = 0;
    return reference;
}


template <typename T>
inline void ACFPtr<T>::acquire(void)
{
    if (_reference)
        _reference->AddRef();
}


template <typename T>
inline void ACFPtr<T>::clear (void)
{
    if (_reference)
    {
        _reference->Release();
        _reference = 0;
    }

}


#endif // ! ACFPtr_h
