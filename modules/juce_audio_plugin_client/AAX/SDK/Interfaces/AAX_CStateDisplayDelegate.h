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
 *	\file  AAX_CStateDisplayDelegate.h
 *
 *	\brief A state display delegate.
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CSTATEDISPLAYDELEGATE_H
#define AAX_CSTATEDISPLAYDELEGATE_H

#include "AAX_IDisplayDelegate.h"
#include "AAX_CString.h"

#include <vector>
#if defined(WINDOWS_VERSION) || defined(LINUX_VERSION)
#include <algorithm>
#endif



/**	\brief A generic display format conforming to AAX_IDisplayDelegate
	
	\details
	This display delegate is similar to AAX_CNumberDisplayDelegate, but does not include
	precision or spacing templatizations.

	\ingroup DisplayDelegates

 */
template <typename T>
class AAX_CStateDisplayDelegate : public AAX_IDisplayDelegate<T>
{
public:
	/**	\brief Constructor taking a vector of C strings
	 
		Each state name will be copied into the display delegate; the C strings
		may be disposed after construction.
	 
		\note \c iStateStrings must be NULL-terminated
	 */
	explicit AAX_CStateDisplayDelegate( const char * iStateStrings[], T iMinState = 0 );
	
	/**	\brief Constructor taking a vector of C strings
		
		Each state name will be copied into the display delegate; the C strings
		may be disposed after construction.
		
		State strings will be copied into the display delegate until either a
		NULL pointer is encountered or \c inNumStates strings have been copied
	 */
	explicit AAX_CStateDisplayDelegate( int32_t inNumStates, const char * iStateStrings[], T iMinState = 0 );
	
	/**	\brief Constructor taking a vector of \ref AAX_IString objects.
	 
		Each \ref AAX_IString will be copied into the display delegate and may be
		disposed after construction. The \ref AAX_IString will not be mutated.
	 */
	explicit AAX_CStateDisplayDelegate( const std::vector<AAX_IString*>& iStateStrings, T iMinState = 0 );
	
	AAX_CStateDisplayDelegate(const AAX_CStateDisplayDelegate& other);

	//Virtual Overrides
	AAX_IDisplayDelegate<T>*	Clone() const AAX_OVERRIDE;
	bool						ValueToString(T value, AAX_CString* valueString) const AAX_OVERRIDE;
	bool						ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const AAX_OVERRIDE;
	bool						StringToValue(const AAX_CString& valueString, T* value) const AAX_OVERRIDE;
	
	//AAX_CStateDisplayDelegate
	void						AddShortenedStrings( const char * iStateStrings[], int iLength );
	bool						Compare( const AAX_CString& valueString, const AAX_CString& stateString ) const;

private:
	AAX_CStateDisplayDelegate();		//private contructor to prevent its use externally.

	T							mMinState;
	std::vector<AAX_CString>	mStateStrings;
	
	struct StringTable
	{
		int							mStrLength;
		std::vector<AAX_CString>	mStateStrings;
	};
	static bool StringTableSortFunc(struct StringTable i, struct StringTable j)
	{
		return (i.mStrLength < j.mStrLength);
	}

	std::vector<struct StringTable>	mShortenedStrings;
};

template <typename T>
AAX_CStateDisplayDelegate<T>::AAX_CStateDisplayDelegate( const char * iStateStrings[], T iMinState /* = 0 */ )
{
	mMinState = iMinState;
	for ( int index = 0; iStateStrings[ index ] != 0; ++index )
		mStateStrings.push_back( AAX_CString( iStateStrings[ index ] ) );
}

template <typename T>
AAX_CStateDisplayDelegate<T>::AAX_CStateDisplayDelegate( int32_t inNumStates, const char * iStateStrings[], T iMinState /* = 0 */ )
{
	mMinState = iMinState;
	for ( int index = 0; (index < inNumStates) && (iStateStrings[ index ] != 0); ++index )
		mStateStrings.push_back( AAX_CString( iStateStrings[ index ] ) );
}

template <typename T>
AAX_CStateDisplayDelegate<T>::AAX_CStateDisplayDelegate( const std::vector<AAX_IString*>& iStateStrings, T iMinState /* = 0 */ )
{
	mMinState = iMinState;
	for ( std::vector<AAX_IString*>::const_iterator iter = iStateStrings.begin(); iter != iStateStrings.end(); ++iter )
	{
		if (*iter)
		{
			mStateStrings.push_back( *(*iter) );
		}
	}
}

template <typename T>
AAX_CStateDisplayDelegate<T>::AAX_CStateDisplayDelegate( const AAX_CStateDisplayDelegate & iOther )
{
	mMinState = iOther.mMinState;

	std::vector<AAX_CString>::const_iterator iter = iOther.mStateStrings.begin();
	for ( ; iter != iOther.mStateStrings.end(); ++iter )
		mStateStrings.push_back( AAX_CString( *iter ) );

	if ( iOther.mShortenedStrings.size() > 0 )
	{
		for ( int i = 0; i < (int)iOther.mShortenedStrings.size(); i++ )
			mShortenedStrings.push_back( iOther.mShortenedStrings.at(i) );
	}
}

template <typename T>
void AAX_CStateDisplayDelegate<T>::AddShortenedStrings( const char * iStateStrings[], int iStrLength )
{
	struct StringTable shortendTable;
	shortendTable.mStrLength = iStrLength;
	for ( int index = 0; iStateStrings[ index ] != 0; ++index )
		shortendTable.mStateStrings.push_back( AAX_CString( iStateStrings[ index ] ) );
	mShortenedStrings.push_back(shortendTable);
	
	// keep structure sorted by str lengths
	std::sort(mShortenedStrings.begin(), mShortenedStrings.end(), AAX_CStateDisplayDelegate::StringTableSortFunc );
}

template <typename T>
AAX_IDisplayDelegate<T>*		AAX_CStateDisplayDelegate<T>::Clone() const
{
	return new AAX_CStateDisplayDelegate(*this);
}

template <typename T>
bool	AAX_CStateDisplayDelegate<T>::ValueToString(T value, AAX_CString* valueString) const
{
	T index = value - mMinState;
	if ( index >= (T) 0 && index < (T) mStateStrings.size() )
	{
		*valueString = mStateStrings[ index ];
		return true;
	}

	return false;
}

template <typename T>
bool	AAX_CStateDisplayDelegate<T>::ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const
{
	// if we don't ahve any shortened strings, just return the full length version
	if ( mShortenedStrings.size() == 0 )
		return this->ValueToString(value, valueString);
	
	// iterate through shortened strings from longest to shortest
	// taking the first set that is short enough
	T index = value - mMinState;

	if ( index < (T) 0 || index >= (T) mStateStrings.size() )
		return true;

	// first see if the normal string is short enough
	if ( mStateStrings[ index ].Length() < uint32_t(maxNumChars) )
	{
		*valueString = mStateStrings[ index ];
		return true;
	}
		
	for ( int i = (int)mShortenedStrings.size()-1; i >= 0; i-- )
	{
		struct StringTable shortStrings = mShortenedStrings.at(i);
		if ( shortStrings.mStrLength <= maxNumChars )
		{
			if ( index >= (T) 0 && index < (T) shortStrings.mStateStrings.size() )
			{
				*valueString = shortStrings.mStateStrings[ index ];
				return true;
			}
		}
	}

	// if we can't find one short enough, just use the shortest version we can find
	struct StringTable shortestStrings = mShortenedStrings.at(0);
	if ( index >= (T) 0 && index < (T) shortestStrings.mStateStrings.size() )
	{
		*valueString = shortestStrings.mStateStrings[ index ];
		return true;
	}
	
	return false;
}

template <typename T>
bool	AAX_CStateDisplayDelegate<T>::StringToValue(const AAX_CString& valueString, T* value) const
{
	std::vector<AAX_CString>::const_iterator iter = mStateStrings.begin();
	for ( T index = 0; iter != mStateStrings.end(); ++index, ++iter )
	{
		if (Compare(valueString,*iter))
		{
			*value = index + mMinState;
			return true;
		}
	}
	
	*value = mMinState;
	return false;
}

template <typename T>
bool	AAX_CStateDisplayDelegate<T>::Compare( const AAX_CString& valueString, const AAX_CString& stateString ) const
{
	return valueString==stateString;
}

	



#endif //AAX_CSTATEDISPLAYDELEGATE_H
