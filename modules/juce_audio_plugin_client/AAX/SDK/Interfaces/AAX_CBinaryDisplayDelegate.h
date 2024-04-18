/*================================================================================================*/
/*
 *
 *	Copyright 2013-2017, 2019, 2023-2024 Avid Technology, Inc.
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
 *	\file AAX_CBinaryDisplayDelegate.h
 *
 *	\brief A binary display delegate.
 *
 */ 
/*================================================================================================*/


#ifndef AAX_CBINARYDISPLAYDELEGATE_H
#define AAX_CBINARYDISPLAYDELEGATE_H

#include "AAX_IDisplayDelegate.h"
#include "AAX_CString.h"


#include <vector>
#ifdef WINDOWS_VERSION
#include <algorithm>
#endif

#include <algorithm>


/**	\brief A binary display format conforming to AAX_IDisplayDelegate
	
	\details
	This display delegate converts a parameter value to one of two provided strings (e.g.
	"True" and "False".)

	\ingroup DisplayDelegates

 */
template <typename T>
class AAX_CBinaryDisplayDelegate : public AAX_IDisplayDelegate<T>
{
public:
	/** \brief Constructor
	 *
	 *	\details
	 *	\param[in] falseString
	 *		The string that will be associated with false parameter values
	 *	\param[in] trueString
	 *		The string that will be associated with true parameter values
	 */
	AAX_CBinaryDisplayDelegate(const char* falseString, const char* trueString);
	AAX_CBinaryDisplayDelegate(const AAX_CBinaryDisplayDelegate& other);

	//Virtual Overrides
	AAX_IDisplayDelegate<T>*	Clone() const AAX_OVERRIDE;
	bool						ValueToString(T value, AAX_CString* valueString) const AAX_OVERRIDE;
	bool						ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const AAX_OVERRIDE;
	bool						StringToValue(const AAX_CString& valueString, T* value) const AAX_OVERRIDE;
	
	// AAX_CBinaryDisplayDelegate
	virtual void						AddShortenedStrings(const char* falseString, const char* trueString, int iStrLength);

private:
	AAX_CBinaryDisplayDelegate();		//private contructor to prevent its use externally.

	const AAX_CString		mFalseString;
	const AAX_CString		mTrueString;
	uint32_t                 mMaxStrLength;
	
	struct StringTable
	{
		int							mStrLength;
		AAX_CString			mFalseString;
		AAX_CString			mTrueString;
	};
	static bool StringTableSortFunc(struct StringTable i, struct StringTable j)
	{
		return (i.mStrLength < j.mStrLength);
	}
	
	std::vector<struct StringTable>	mShortenedStrings;
};


template <typename T>
AAX_CBinaryDisplayDelegate<T>::AAX_CBinaryDisplayDelegate(const char* falseString, const char* trueString) :
	mFalseString(falseString),
	mTrueString(trueString),
	mMaxStrLength(0)
{
	mMaxStrLength = (std::max)(mMaxStrLength, mFalseString.Length());
	mMaxStrLength = (std::max)(mMaxStrLength, mTrueString.Length());
}

template <typename T>
AAX_CBinaryDisplayDelegate<T>::AAX_CBinaryDisplayDelegate(const AAX_CBinaryDisplayDelegate& other) :
	mFalseString(other.mFalseString),
	mTrueString(other.mTrueString),
	mMaxStrLength(other.mMaxStrLength)
{
	if ( other.mShortenedStrings.size() > 0 )
	{
		for ( size_t i = 0; i < other.mShortenedStrings.size(); i++ )
			mShortenedStrings.push_back( other.mShortenedStrings.at(i) );
	}
}

template <typename T>
void AAX_CBinaryDisplayDelegate<T>::AddShortenedStrings(const char* falseString, const char* trueString, int iStrLength)
{
	struct StringTable shortendTable;
	shortendTable.mStrLength = iStrLength;
	shortendTable.mFalseString = AAX_CString(falseString);
	shortendTable.mTrueString = AAX_CString(trueString);
	mShortenedStrings.push_back(shortendTable);
	
	// keep structure sorted by str lengths
	std::sort(mShortenedStrings.begin(), mShortenedStrings.end(), AAX_CBinaryDisplayDelegate::StringTableSortFunc );
}


template <typename T>
AAX_IDisplayDelegate<T>*		AAX_CBinaryDisplayDelegate<T>::Clone() const
{
	return new AAX_CBinaryDisplayDelegate(*this);
}

template <typename T>
bool	AAX_CBinaryDisplayDelegate<T>::ValueToString(T value, AAX_CString* valueString) const
{
	if (value)
		*valueString = mTrueString;
	else
		*valueString = mFalseString;
	return true;
}

template <typename T>
bool	AAX_CBinaryDisplayDelegate<T>::ValueToString(T value, int32_t maxNumChars, AAX_CString* valueString) const
{
	// if we don't ahve any shortened strings, just return the full length version
	if ( mShortenedStrings.size() == 0 )
		return this->ValueToString(value, valueString);
	
	// first see if requested length is longer than normal strings
	const uint32_t maxNumCharsUnsigned = (0 <= maxNumChars) ? static_cast<uint32_t>(maxNumChars) : 0;
	if ( maxNumCharsUnsigned >= mMaxStrLength )
	{
		if (value)
			*valueString = mTrueString;
		else
			*valueString = mFalseString;
		return true;
	}
	
	// iterate through shortened strings from longest to shortest
	// taking the first set that is short enough
	for ( int i = static_cast<int>(mShortenedStrings.size())-1; i >= 0; i-- )
	{
		struct StringTable shortStrings = mShortenedStrings.at(static_cast<unsigned int>(i));
		if ( shortStrings.mStrLength <= maxNumChars )
		{
			if (value)
				*valueString = shortStrings.mTrueString;
			else
				*valueString = shortStrings.mFalseString;
			return true;
		}
	}

	// if we can't find one short enough, just use the shortest version we can find
	struct StringTable shortestStrings = mShortenedStrings.at(0);
	if (value)
		*valueString = shortestStrings.mTrueString;
	else
		*valueString = shortestStrings.mFalseString;
	
	return true;
}

template <typename T>
bool	AAX_CBinaryDisplayDelegate<T>::StringToValue(const AAX_CString& valueString, T* value) const
{
	if (valueString == mTrueString)
	{
		*value = (T)(true);
		return true;
	}
	if (valueString == mFalseString)
	{
		*value = (T)(false);
		return true;
	}
	*value = (T)(false);
	return false;
}
	



#endif //AAX_CBINARYDISPLAYDELEGATE_H
