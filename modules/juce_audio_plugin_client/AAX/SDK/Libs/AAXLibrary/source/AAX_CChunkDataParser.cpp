/*================================================================================================*/
/*
 *	Copyright 2013-2015, 2023-2024 Avid Technology, Inc.
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
 *	\file   AAX_CChunkDataParser.cpp
 *	
 *	\author Steven Massey
 *
 */ 
/*================================================================================================*/
#include "AAX_CChunkDataParser.h"

#include <string.h>

#include <string>
#include <vector>
#include <stdexcept>

#include "AAX_EndianSwap.h"
#include "AAX_Assert.h"


AAX_CChunkDataParser::AAX_CChunkDataParser()
:	mLastFoundIndex(-1),
	mChunkData(NULL),
	mChunkVersion(-1)
{
}

AAX_CChunkDataParser::~AAX_CChunkDataParser()
{
}

int32_t AAX_CChunkDataParser::GetChunkData(AAX_SPlugInChunk *chunk)
{
	uint16_t chunkDataElement16;
	uint32_t chunkDataElement32;
	int64_t chunkDataElement64;
	AAX_CString chunkDataElementStr;

	int32_t signedChunkDataSize = this->GetChunkDataSize();
	size_t chunkDataSize = 0;
	if (signedChunkDataSize < AAX_ChunkDataParserDefs::HEADER_SIZE)
	{
		return AAX_ChunkDataParserDefs::BUILD_DATA_FAILED;
	}
	else
	{
		chunkDataSize = static_cast<size_t>(signedChunkDataSize);
	}
	
	try
	{
		char *chunkData = reinterpret_cast<char *>(chunk->fData);
		uint32_t dataIndex = 0;
		
		//SW - macOS has junk in the chunk
		memset(chunkData, 0, chunkDataSize);
		
		// Set AAX_CChunkDataParser version number:
		memcpy(&chunkData[dataIndex], &AAX_ChunkDataParserDefs::VERSION_ID_1, 4);
		dataIndex += 4;
		
		// Go thru chunk elements, and place them into the chunk - byte swap if needed
		for (size_t i = 0; i < mDataValues.size(); i++) {
			
			// Calculate the size of each element in the chunk
			const uint32_t elemIdentifierSize = AAX_ChunkDataParserDefs::STRING_IDENTIFIER_SIZE;
			const uint32_t elemNameSize = 1+mDataValues[i].mDataName.Length();
			uint32_t elemDataSize(0);
			if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::DOUBLE_TYPE) {
				chunkDataElement64 = mDataValues[i].mIntValue;
				AAX_BigEndianNativeSwapInPlace(&chunkDataElement64);
				elemDataSize = AAX_ChunkDataParserDefs::DOUBLE_TYPE_SIZE;
			} else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::SHORT_TYPE) {
				chunkDataElement16 = uint16_t(mDataValues[i].mIntValue);
				AAX_BigEndianNativeSwapInPlace(&chunkDataElement16);
				elemDataSize = AAX_ChunkDataParserDefs::SHORT_TYPE_SIZE;
			} else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::STRING_TYPE) {
				chunkDataElementStr = mDataValues[i].mStringValue;
				elemDataSize = 1 + chunkDataElementStr.Length();
			} else {
				chunkDataElement32 = uint32_t(mDataValues[i].mIntValue);
				AAX_BigEndianNativeSwapInPlace(&chunkDataElement32);
				elemDataSize = AAX_ChunkDataParserDefs::DEFAULT32BIT_TYPE_SIZE;
			}
			
			// Verify that this item will fit into the buffer
			if ((dataIndex + (elemIdentifierSize + elemNameSize + elemDataSize)) > chunkDataSize)
			{
				throw std::out_of_range("aborting: chunk data will overflow buffer");
			}
			
			// Put data element's name into chunk
			if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::DOUBLE_TYPE) {
				strncpy(&chunkData[dataIndex], AAX_ChunkDataParserDefs::DOUBLE_STRING_IDENTIFIER, elemIdentifierSize);
			} else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::LONG_TYPE) {
				strncpy(&chunkData[dataIndex], AAX_ChunkDataParserDefs::LONG_STRING_IDENTIFIER, elemIdentifierSize);
			} else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::FLOAT_TYPE) {
				strncpy(&chunkData[dataIndex], AAX_ChunkDataParserDefs::FLOAT_STRING_IDENTIFIER, elemIdentifierSize);
			} else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::SHORT_TYPE) {
				strncpy(&chunkData[dataIndex], AAX_ChunkDataParserDefs::SHORT_STRING_IDENTIFIER, elemIdentifierSize);
			} else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::STRING_TYPE) {
				strncpy(&chunkData[dataIndex], AAX_ChunkDataParserDefs::STRING_STRING_IDENTIFIER, elemIdentifierSize);
			}
			dataIndex += elemIdentifierSize; // Increment past "f_", "d_", etc. id string
			
			strncpy(&chunkData[dataIndex], mDataValues[i].mDataName.Get(), elemNameSize);
			dataIndex += elemNameSize;
			WordAlign(dataIndex);
			
			// Put data element into chunk
			if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::DOUBLE_TYPE) {
				memcpy(&chunkData[dataIndex], &chunkDataElement64, elemDataSize);
				dataIndex += AAX_ChunkDataParserDefs::DOUBLE_TYPE_INCR;
			} else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::SHORT_TYPE) {
				memcpy(&chunkData[dataIndex], &chunkDataElement16, elemDataSize);
				dataIndex += AAX_ChunkDataParserDefs::SHORT_TYPE_INCR; // keep life word aligned
			} else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::STRING_TYPE) {
				memcpy(&chunkData[dataIndex], chunkDataElementStr.Get(), chunkDataElementStr.Length() + 1);
				dataIndex += elemDataSize;
				WordAlign(dataIndex);
			} else {
				memcpy(&chunkData[dataIndex], &chunkDataElement32, elemDataSize);
				dataIndex += AAX_ChunkDataParserDefs::DEFAULT32BIT_TYPE_INCR;
			}
		}
	}

	
	catch(const std::exception& e)
	{
		AAX_TRACE_RELEASE(kAAX_Trace_Priority_High, "AAX_CChunkDataParser::GetChunkData ERROR: %s", e.what());
		return AAX_ChunkDataParserDefs::BUILD_DATA_FAILED;
	}
	catch(...)
	{
		return AAX_ChunkDataParserDefs::BUILD_DATA_FAILED;
	}
	
	chunk->fSize = signedChunkDataSize;

	return 0;
}

void AAX_CChunkDataParser::LoadChunk(const AAX_SPlugInChunk *chunk)
{
	Clear();

	mChunkVersion = chunk->fVersion;
	const char *chunkData = chunk->fData; // Do not dereference until after verifying that chunkDataSize > 4
	int32_t chunkDataSize;

	int32_t i = 0;
	chunkDataSize = chunk->fSize;
	
	// Check the version number
	if (chunkDataSize >= 4) {
		if ( *(reinterpret_cast<const int32_t *>(chunkData)) != AAX_ChunkDataParserDefs::VERSION_ID_1) 
			return;
		else i += 4;
	}

	if (chunkDataSize > 4 ) {
		//chunkData = chunk->fData;
		
		uint16_t chunkDataElement16;
		uint32_t chunkDataElement32;
		int64_t chunkDataElement64;
		AAX_CString chunkDataElementStr;
		char name[AAX_ChunkDataParserDefs::MAX_NAME_LENGTH+3];
		
		do {
			if (i < chunkDataSize) {
				DataValue newValue;
				
				size_t length = std::min(AAX_ChunkDataParserDefs::MAX_NAME_LENGTH, static_cast<size_t>(chunkDataSize - i)); // Just trying to always stay in the buffer
				strncpy(name, &chunkData[i], length);
				i += (int32_t(strlen(name)) + 1);
				WordAlign(i);
				// Only comparing first character, leave the 2nd for future
				if (strncmp(name, AAX_ChunkDataParserDefs::DOUBLE_STRING_IDENTIFIER, 1) == 0) {		
					memcpy(&chunkDataElement64, &chunkData[i], 8);
					AAX_BigEndianNativeSwapInPlace(&chunkDataElement64);
					i+=8;
					newValue.mDataType = AAX_ChunkDataParserDefs::DOUBLE_TYPE;
					newValue.mIntValue = chunkDataElement64;
				} else if (strncmp(name, AAX_ChunkDataParserDefs::FLOAT_STRING_IDENTIFIER, 1) == 0) {
					memcpy(&chunkDataElement32, &chunkData[i], 4);
					AAX_BigEndianNativeSwapInPlace(&chunkDataElement32);
					chunkDataElement64 = chunkDataElement32;
					i+=4;
					newValue.mDataType = AAX_ChunkDataParserDefs::FLOAT_TYPE;
					newValue.mIntValue = chunkDataElement64;
				} else if (strncmp(name, AAX_ChunkDataParserDefs::LONG_STRING_IDENTIFIER, 1) == 0) {
					memcpy(&chunkDataElement32, &chunkData[i], 4);
					AAX_BigEndianNativeSwapInPlace(&chunkDataElement32);
					chunkDataElement64 = chunkDataElement32;
					i+=4;
					newValue.mDataType = AAX_ChunkDataParserDefs::LONG_TYPE;
					newValue.mIntValue = chunkDataElement64;
				} else if (strncmp(name, AAX_ChunkDataParserDefs::SHORT_STRING_IDENTIFIER, 1) == 0) {
					memcpy(&chunkDataElement16, &chunkData[i], 2);
					AAX_BigEndianNativeSwapInPlace(&chunkDataElement16);
					chunkDataElement64 = chunkDataElement16;
					i+=4; // keep things word aligned
					newValue.mDataType = AAX_ChunkDataParserDefs::SHORT_TYPE;
					newValue.mIntValue = chunkDataElement64;
				} else if (strncmp(name, AAX_ChunkDataParserDefs::STRING_STRING_IDENTIFIER, 1) == 0) {
					chunkDataElementStr = &chunkData[i];	// just read until we hit null-terminator
					i += chunkDataElementStr.Length() + 1;
					newValue.mDataType = AAX_ChunkDataParserDefs::STRING_TYPE;
					newValue.mStringValue = chunkDataElementStr;
				}

				AAX_CString dataName(&name[2]);		// chop of first two characters
				newValue.mDataName = dataName;
				
				mDataValues.push_back(newValue);
			}
		} while (i < chunkDataSize);
	}
}


int32_t AAX_CChunkDataParser::GetChunkDataSize()
{
	int32_t chunkDataSize = AAX_ChunkDataParserDefs::HEADER_SIZE;
	size_t numElements = mDataValues.size();
	for (size_t i = 0; i < numElements; i++) {
		uint32_t nameLength = mDataValues[i].mDataName.Length() + 3; // 2+1=3; two char id + null termination = 3
		WordAlign(nameLength);
		chunkDataSize += nameLength;
		
		if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::DOUBLE_TYPE) chunkDataSize += 8;
		else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::FLOAT_TYPE) chunkDataSize += 4;
		else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::LONG_TYPE) chunkDataSize += 4;
		else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::SHORT_TYPE) chunkDataSize += 4; // we keep things word aligned
		else if (mDataValues[i].mDataType == AAX_ChunkDataParserDefs::STRING_TYPE)
		{
			nameLength = (mDataValues[i].mStringValue.Length() + 1);
			WordAlign(nameLength);
			chunkDataSize += nameLength;
		}
	}

	return chunkDataSize;
}

void AAX_CChunkDataParser::WordAlign(uint32_t &index)
{
	uint32_t align = index%4;
	if (align != 0) {
		index -= align;
		index += 4;
	}
}

void AAX_CChunkDataParser::WordAlign(int32_t &index)
{
	int32_t align = index%4;
	if (align != 0) {
		index -= align;
		index += 4;
	}
}

bool AAX_CChunkDataParser::ReplaceDouble(const char *name, double value) //SW added for fela
{
	int32_t i;
	int64_t data;

	AAX_CString doubleName(name);

	i = FindName(doubleName);

	if (i == AAX_ChunkDataParserDefs::NAME_NOT_FOUND || 0 > i) return false;
	if (mDataValues[static_cast<size_t>(i)].mDataType != AAX_ChunkDataParserDefs::DOUBLE_TYPE) return false;

	memcpy(&data, &value, 8);
	mDataValues[static_cast<size_t>(i)].mIntValue = data;
	return true;
}



void AAX_CChunkDataParser::AddFloat(const char *name, float value)
{
	uint32_t data = 0;
	memcpy(&data, &value, 4);
	
	DataValue dataValue;
	dataValue.mDataName = name;
	dataValue.mIntValue = static_cast<int64_t>(data);
	dataValue.mDataType = AAX_ChunkDataParserDefs::FLOAT_TYPE;
	mDataValues.push_back(dataValue);
}

void AAX_CChunkDataParser::AddDouble(const char *name, double value)
{
	int64_t data = 0;
	memcpy(&data, &value, 8);
	
	DataValue dataValue;
	dataValue.mDataName = name;
	dataValue.mIntValue = data;
	dataValue.mDataType = AAX_ChunkDataParserDefs::DOUBLE_TYPE;
	mDataValues.push_back(dataValue);
}

void AAX_CChunkDataParser::AddString(const char *name, AAX_CString value)
{
	DataValue dataValue;
	dataValue.mDataName = name;
	dataValue.mStringValue = value;
	dataValue.mDataType = AAX_ChunkDataParserDefs::STRING_TYPE;
	mDataValues.push_back(dataValue);
}

void AAX_CChunkDataParser::AddInt32(const char *name, int32_t value)
{
	DataValue dataValue;
	dataValue.mDataName = name;
	dataValue.mIntValue = static_cast<int64_t>(value);
	dataValue.mDataType = AAX_ChunkDataParserDefs::LONG_TYPE;
	mDataValues.push_back(dataValue);
}

void AAX_CChunkDataParser::AddInt16(const char *name, int16_t value)
{
	DataValue dataValue;
	dataValue.mDataName = name;
	dataValue.mIntValue = static_cast<int64_t>(value);
	dataValue.mDataType = AAX_ChunkDataParserDefs::SHORT_TYPE;
	mDataValues.push_back(dataValue);
}

bool AAX_CChunkDataParser::FindDouble(const char *name, double *value)
{
	int32_t i;
	int64_t data;

	AAX_CString doubleName(name);

	i = FindName(doubleName);
		
	if (i == AAX_ChunkDataParserDefs::NAME_NOT_FOUND || 0 > i) return false;
	if (mDataValues.size() > 0 && mDataValues[static_cast<size_t>(i)].mDataType != AAX_ChunkDataParserDefs::DOUBLE_TYPE) return false;

	data = mDataValues[static_cast<size_t>(i)].mIntValue;
	memcpy(value, &data, 8);
	return true;
}


bool AAX_CChunkDataParser::FindFloat (const char *name, float *value)
{
	int32_t i;
	int32_t data;

	AAX_CString floatName(name);

	i = FindName(floatName);
		
	if (i == AAX_ChunkDataParserDefs::NAME_NOT_FOUND || 0 > i) return false;
	if (mDataValues.size() > 0 && mDataValues[static_cast<size_t>(i)].mDataType != AAX_ChunkDataParserDefs::FLOAT_TYPE) return false;

	data = static_cast<int32_t>(mDataValues[static_cast<size_t>(i)].mIntValue);
	memcpy(value, &data, 4);
	return true;
}


bool AAX_CChunkDataParser::FindString (const char *name, AAX_CString *value)
{
	int32_t i;
	AAX_CString stringName(name);
	
	i = FindName(stringName);
	
	if (i == AAX_ChunkDataParserDefs::NAME_NOT_FOUND || 0 > i) return false;
	if (mDataValues.size() > 0 && mDataValues[static_cast<size_t>(i)].mDataType != AAX_ChunkDataParserDefs::STRING_TYPE) return false;
	
	value->Set(mDataValues[static_cast<size_t>(i)].mStringValue.Get());

	return true;
}


bool AAX_CChunkDataParser::FindInt32(const char *name, int32_t *value)
{
	int32_t i;

	AAX_CString longName(name);

	i = FindName(longName);
		
	if (i == AAX_ChunkDataParserDefs::NAME_NOT_FOUND || 0 > i) return false;
	if (mDataValues.size() > 0 && mDataValues[static_cast<size_t>(i)].mDataType != AAX_ChunkDataParserDefs::LONG_TYPE) return false;

	*value = static_cast<int32_t>(mDataValues[static_cast<size_t>(i)].mIntValue);
	//memcpy(value, &data, 8);
	return true;
}


bool AAX_CChunkDataParser::FindInt16(const char *name, int16_t *value)
{
	int32_t i;

	AAX_CString longName(name);

	i = FindName(longName);
		
	if (i == AAX_ChunkDataParserDefs::NAME_NOT_FOUND || 0 > i) return false;
	if (mDataValues.size() > 0 && mDataValues[static_cast<size_t>(i)].mDataType != AAX_ChunkDataParserDefs::SHORT_TYPE) return false;

	*value = static_cast<int16_t>(mDataValues[static_cast<size_t>(i)].mIntValue);
	//memcpy(value, &data, 8);
	return true;
}

int32_t AAX_CChunkDataParser::FindName(const AAX_CString &Name)
// I keep track of the last found index to speed up searching since chunk values 
// will tend to be extracted in the same order every time.
{	
	size_t numDatum = mDataValues.size();
	if (mLastFoundIndex >= (static_cast<int32_t>(numDatum)-1))
		mLastFoundIndex = -1;
	
	if (-1 > mLastFoundIndex)
	{
		return AAX_ChunkDataParserDefs::NAME_NOT_FOUND;
	}
	else
	{
		uint32_t i = static_cast<uint32_t>(mLastFoundIndex + 1);

		while(i < numDatum ) {
			if (mDataValues[i].mDataName == Name) {
				mLastFoundIndex = static_cast<int32_t>(i);
				return mLastFoundIndex;
			}
			i++;
		}

		if (0 <= mLastFoundIndex)
		{
			uint32_t j = 0;
			while(j <= static_cast<uint32_t>(mLastFoundIndex)) {
				if (mDataValues[j].mDataName == Name) {
					mLastFoundIndex = static_cast<int32_t>(j);
					return mLastFoundIndex;
				}
				j++;
			}
		}
	}

	return AAX_ChunkDataParserDefs::NAME_NOT_FOUND;
}

bool AAX_CChunkDataParser::IsEmpty()
{
	return mDataValues.empty();
}

void AAX_CChunkDataParser::Clear()
{
	mLastFoundIndex = -1;
	mChunkVersion = -1;
	mDataValues.clear();
}
