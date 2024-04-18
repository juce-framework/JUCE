/*================================================================================================*/
/*
 *
 *	Copyright 2013-2015, 2017, 2021, 2023-2024 Avid Technology, Inc.
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
 *	\file  AAX_CString.h
 *
 *	\brief A generic %AAX string class with similar functionality to std::string
 *
 */ 
/*================================================================================================*/

#pragma once

#ifndef AAX_CSTRING_H
#define AAX_CSTRING_H


#include "AAX_IString.h"
#include "AAX.h"

#include <string>
#include <map>


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#endif
///////////////////////////////////////////////////////////////

/**
 *	\brief A generic %AAX string class with similar functionality to <tt>std::string</tt>
 */
class AAX_CString : public AAX_IString
{
public:
	static const uint32_t	kInvalidIndex = static_cast<uint32_t>(-1);
	static const uint32_t	kMaxStringLength = static_cast<uint32_t>(-2);

	// AAX_IString Virtual Overrides
	uint32_t		Length() const AAX_OVERRIDE;
	uint32_t		MaxLength() const AAX_OVERRIDE;
	const char *	Get () const AAX_OVERRIDE;
	void			Set ( const char * iString ) AAX_OVERRIDE;
	AAX_IString &	operator=(const AAX_IString & iOther) AAX_OVERRIDE;
	AAX_IString &	operator=(const char * iString) AAX_OVERRIDE;
	
	/** Constructs an empty string. */
	AAX_CString();
	
	/** Implicit conversion constructor: Constructs a string with a const char* pointer to copy. */
	AAX_CString(const char* str);
	
	/** Copy constructor: Constructs a string from a std::string. Beware of STL variations across various binaries. */
	explicit AAX_CString(const std::string& str);
	
	/** Copy constructor: Constructs a string with another concrete AAX_CString. */
	AAX_CString(const AAX_CString& other);

	/** Copy constructor: Constructs a string from another string that meets the AAX_IString interface. */
	AAX_CString(const AAX_IString& other);

	/** Default move constructor */
	AAX_DEFAULT_MOVE_CTOR(AAX_CString);


	/** Direct access to a std::string. */
	std::string&		StdString();
	
	/** Direct access to a const std::string. */
	const std::string&	StdString() const;

	/** Assignment operator from another AAX_CString */
	AAX_CString&	operator=(const AAX_CString& other);
	
	/** Assignment operator from a std::string. Beware of STL variations across various binaries. */
	AAX_CString &	operator=(const std::string& other);

	/** Move operator */
	AAX_CString &	operator=(AAX_CString&& other);
			
	/** output stream operator for concrete AAX_CString */
	friend std::ostream& operator<< (std::ostream& os, const AAX_CString& str);

	/** input stream operator for concrete AAX_CString */
	friend std::istream& operator>> (std::istream& os, AAX_CString& str);

	
	// String Formatting Functions
	void			Clear();
	bool			Empty() const;
	AAX_CString&	Erase(uint32_t pos, uint32_t n);
	AAX_CString&	Append(const AAX_CString& str);
	AAX_CString&	Append(const char* str);
	AAX_CString&	AppendNumber(double number, int32_t precision);
	AAX_CString&	AppendNumber(int32_t number);
	AAX_CString&	AppendHex(int32_t number, int32_t width);
	AAX_CString&	Insert(uint32_t	pos, const AAX_CString&	str);
	AAX_CString&	Insert(uint32_t	pos, const char* str);
	AAX_CString&	InsertNumber(uint32_t pos, double number, int32_t precision);
	AAX_CString&	InsertNumber(uint32_t pos, int32_t number);
	AAX_CString&	InsertHex(uint32_t pos, int32_t number, int32_t width);
	AAX_CString&	Replace(uint32_t pos, uint32_t n, const AAX_CString& str);
	AAX_CString&	Replace(uint32_t pos, uint32_t n, const char* str);
	uint32_t		FindFirst(const AAX_CString& findStr) const;
	uint32_t		FindFirst(const char* findStr) const;
	uint32_t		FindFirst(char findChar) const;
	uint32_t		FindLast(const AAX_CString& findStr) const;
	uint32_t		FindLast(const char* findStr) const;
	uint32_t		FindLast(char findChar) const;	
	const char*		CString()	const;
	bool			ToDouble(double* oValue)	const;
	bool			ToInteger(int32_t* oValue)  const;
	void			SubString(uint32_t pos, uint32_t n, AAX_IString* outputStr) const;
	bool			Equals(const AAX_CString& other) const { return operator==(other); }
	bool			Equals(const char* other) const { return operator==(other); }
	bool			Equals(const std::string& other) const { return operator==(other); } //beware of STL variations between binaries.
	
	// Operator Overrides
	bool			operator==(const AAX_CString& other) const;
	bool			operator==(const char* otherStr) const;
	bool			operator==(const std::string& otherStr) const;      //beware of STL variations between binaries.
	bool			operator!=(const AAX_CString& other) const;
	bool			operator!=(const char* otherStr) const;
	bool			operator!=(const std::string& otherStr) const;      //beware of STL variations between binaries.
	bool			operator<(const AAX_CString& other) const;
	bool			operator>(const AAX_CString& other) const;
	const char&		operator[](uint32_t index) const;
	char&			operator[](uint32_t index);
	AAX_CString&	operator+=(const AAX_CString& str);
	AAX_CString&	operator+=(const std::string& str);
	AAX_CString&	operator+=(const char* str);

protected:
	std::string		mString;
};

// Non-member operators
inline AAX_CString operator+(AAX_CString lhs, const AAX_CString& rhs)
{
	lhs += rhs;
	return lhs;
}
inline AAX_CString operator+(AAX_CString lhs, const char* rhs)
{
	lhs += rhs;
	return lhs;
}
inline AAX_CString operator+(const char* lhs, const AAX_CString& rhs)
{
	return AAX_CString(lhs) + rhs;
}


///////////////////////////////////////////////////////////////
#if 0
#pragma mark -
#endif
///////////////////////////////////////////////////////////////

/**	\brief Helper class to store a collection of name abbreviations
 */
class AAX_CStringAbbreviations
{
public:
	explicit AAX_CStringAbbreviations(const AAX_CString& inPrimary)
	: mPrimary(inPrimary)
	, mAbbreviations()
	{
	}
	
	void SetPrimary(const AAX_CString& inPrimary) { mPrimary = inPrimary; }
	const AAX_CString& Primary() const { return mPrimary; }
	
	void Add(const AAX_CString& inAbbreviation)
	{
		uint32_t stringLength = inAbbreviation.Length();
		mAbbreviations[stringLength] = inAbbreviation;	//Does a string copy into the map.
	}
	
	const AAX_CString& Get(int32_t inNumCharacters) const
	{
		//More characters than the primary string or no specific shortened names.
		if ((inNumCharacters >= int32_t(mPrimary.Length())) || (mAbbreviations.empty()) || (0 > inNumCharacters))
			return mPrimary;
		
		std::map<uint32_t, AAX_CString>::const_iterator iter = mAbbreviations.upper_bound(static_cast<uint32_t>(inNumCharacters));
		
		//If the iterator is already pointing to shortest string, return that.
		if (iter == mAbbreviations.begin())
			return iter->second;
		
		//lower_bound() will return the iterator that is larger than the desired value, so decrement the iterator.
		--iter;
		return iter->second;
	}
	
	void Clear() { mAbbreviations.clear(); }
	
private:
	AAX_CString									mPrimary;
	std::map<uint32_t, AAX_CString>				mAbbreviations;
};

#endif //AAX_CSTRING_H
