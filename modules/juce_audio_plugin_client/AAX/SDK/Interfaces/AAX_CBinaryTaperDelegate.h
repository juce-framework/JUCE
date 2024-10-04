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
 *	\file AAX_CBinaryTaperDelegate.h
 *
 *	\brief A binary taper delegate.
 *
 */ 
/*================================================================================================*/


#ifndef	AAX_CBINARYTAPERDELEGATE_H
#define AAX_CBINARYTAPERDELEGATE_H

#include "AAX_ITaperDelegate.h"


/** \brief A binary taper conforming to \ref AAX_ITaperDelegate
	
	\details
	This taper maps positive real values to 1 and negative or zero real values to 0.  This is
	the standard taper used on all bool parameters.
	
	When this taper is constructed with a bool template type, its normalized values are
	automatically typecast to the proper boolean value.

	\ingroup TaperDelegates
	
*/
template <typename T>
class AAX_CBinaryTaperDelegate : public AAX_ITaperDelegate<T>
{
public:

	/** \brief Constructs a Binary Taper 
	 *
	 */
	AAX_CBinaryTaperDelegate( );
	
	//Virtual Overrides
	AAX_ITaperDelegate<T>*	Clone() const AAX_OVERRIDE;
	T						GetMaximumValue() const AAX_OVERRIDE;
	T						GetMinimumValue() const AAX_OVERRIDE;
	T						ConstrainRealValue(T value)	const AAX_OVERRIDE;
	T						NormalizedToReal(double normalizedValue) const AAX_OVERRIDE;
	double					RealToNormalized(T realValue) const AAX_OVERRIDE;
};






template <typename T>
AAX_CBinaryTaperDelegate<T>::AAX_CBinaryTaperDelegate( )	:   
	AAX_ITaperDelegate<T>()
{
}

template <typename T>
AAX_ITaperDelegate<T>*	AAX_CBinaryTaperDelegate<T>::Clone() const
{
	return new AAX_CBinaryTaperDelegate(*this);
}

template <typename T>
T		AAX_CBinaryTaperDelegate<T>::GetMinimumValue() const
{
	return false;
}

template <typename T>
T		AAX_CBinaryTaperDelegate<T>::GetMaximumValue() const
{
	return true;
}

template <typename T>
T		AAX_CBinaryTaperDelegate<T>::ConstrainRealValue(T value)	const
{
	return value;		
}

template <typename T>
T		AAX_CBinaryTaperDelegate<T>::NormalizedToReal(double normalizedValue) const
{
	if (normalizedValue > 0.0f)
		return (T)(1);		//should construct true for bool
	return (T)(0);			//should construct false for bool
}

template <typename T>
double	AAX_CBinaryTaperDelegate<T>::RealToNormalized(T realValue) const
{
	if (realValue > (T)(0))
		return 1.0f;
	return 0.0f;
}




#endif //AAX_CBINARYTAPERDELEGATE_H


