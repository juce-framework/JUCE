/*================================================================================================*/
/*
 *
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
 *	\file AAX_CChunkDataParser.h
 *
 *	\brief Parser utility for plugin chunks
 *
 */ 
/*================================================================================================*/


#pragma once

#ifndef AAX_CHUNKDATAPARSER_H
#define AAX_CHUNKDATAPARSER_H

#include "AAX.h"
#include "AAX_CString.h"
#include <vector>

//forward declarations
struct AAX_SPlugInChunk;

/*!
 *	\brief Constants used by ChunkDataParser class
 */
namespace AAX_ChunkDataParserDefs {
	const int32_t FLOAT_TYPE = 1;
	const char FLOAT_STRING_IDENTIFIER[] = "f_";

	const int32_t LONG_TYPE = 2;
	const char LONG_STRING_IDENTIFIER[] = "l_";
	
	const int32_t DOUBLE_TYPE = 3;
	const char DOUBLE_STRING_IDENTIFIER[] = "d_";
	const size_t DOUBLE_TYPE_SIZE = 8;
	const size_t DOUBLE_TYPE_INCR = 8;

	const int32_t SHORT_TYPE = 4;
	const char SHORT_STRING_IDENTIFIER[] = "s_";
	const size_t SHORT_TYPE_SIZE = 2;
	const size_t SHORT_TYPE_INCR = 4; // keep life word aligned

	const int32_t STRING_TYPE = 5;
	const char STRING_STRING_IDENTIFIER[] = "r_";
	const size_t MAX_STRINGDATA_LENGTH = 255;
	
	const size_t DEFAULT32BIT_TYPE_SIZE = 4;
	const size_t DEFAULT32BIT_TYPE_INCR = 4;
	
	const size_t STRING_IDENTIFIER_SIZE = 2;
	
	const int32_t NAME_NOT_FOUND = -1;
	const size_t MAX_NAME_LENGTH = 255;
	const int32_t BUILD_DATA_FAILED = -333;
	const int32_t HEADER_SIZE = 4;
	const int32_t VERSION_ID_1 = 0x01010101;
}

/*!	\brief Parser utility for plugin chunks
 *  
 *	\details
 *	\todo Update this documentation for AAX
 *
 *	This class acts as generic repository for data that is stuffed into or extracted 
 *	from a SFicPlugInChunk.  It has an abstracted Add/Find interface to add or retrieve
 *	data values, each uniquely referenced by a c-string.  In conjuction with the Effect
 *	Layer and the "ControlManager" aspects of the CProcess class, this provides a more 
 *	transparent & resilent system for performing save-and-restore on settings that won't break 
 *	so easily from endian issues or from the hard-coded structs that have typically been 
 *	used to build chunk data.
 *	
 *	\par Format of the Chunk Data
 *	The first 4 bytes of the data are the version number (repeated 4 times to be 
 *	immune to byte swapping). Data follows next.\n 
 *	\n
 *	Example: "f_bypa %$#@d_gain #$!#@$%$s_omsi #$"
 *	\code
 *		type	name	value	
 *		----------------------------
 *		float	bypa	%$#@	
 *		double	gain	#$!#@$%$
 *		int16_t	omsi	#$
 *	\endcode
 *	\n
 *	\li The first character denotes the data type:
 *	\code
 *			'f' = float
 *			'd' = double
 *			'l' = int32
 *			's' = int16
 *	\endcode
 *	\n
 *	\li  "_" is an empty placekeeper that could be used to addition future information. Currently, it's
 *	ignored when a chunk is parsed.
 *	
 *	\li  The string name identifier follows next, and can up to 255 characters int32_t.  The Effect Layer
 *	builds chunks it always converts the AAX_FourCharCode of the control to a string.  So, this will always be
 *	4 characters int32_t.  The string is null terminated to indicate the start of the data value.
 *	
 *	\li  The data value follows next, but is possible shifted to aligned word aligned.  The size of 
 *	is determined, of course, by the data type.  
 */
class AAX_CChunkDataParser
{
	public:
		AAX_CChunkDataParser();
		virtual ~AAX_CChunkDataParser();

		/*!	\brief CALL: Adds some data of type float with \a name and \a value to the current chunk
		 *  
		 *	\details
		 *  \sa \ref AddDouble(), \ref AddInt32(), and \ref AddInt16() are the same but with different data types. 
		 */
		void	AddFloat(const char *name, float value);
		void	AddDouble(const char *name, double value);	//!< CALL: See AddFloat()
		void	AddInt32(const char *name, int32_t value);	//!< CALL: See AddFloat()
		void	AddInt16(const char *name, int16_t value);	//!< CALL: See AddFloat()
		void	AddString(const char *name, AAX_CString value);

		/*!	\brief CALL: Finds some data of type float with \a name and \a value in the current chunk
		 *  
		 *	\details
		 *  \sa \ref FindDouble(), \ref FindInt32(), and \ref FindInt16() are the same but with different data types. 
		 */
		bool	FindFloat(const char *name, float *value);
		bool	FindDouble(const char *name, double *value);	//!< CALL: See FindFloat()
		bool	FindInt32(const char *name, int32_t *value);		//!< CALL: See FindFloat()
		bool	FindInt16(const char *name, int16_t *value);		//!< CALL: See FindFloat()
		bool	FindString(const char *name, AAX_CString *value);

		bool	ReplaceDouble(const char *name, double value); //SW added for fela
		int32_t	GetChunkData(AAX_SPlugInChunk *chunk);	//!< CALL: Fills passed in \a chunk with data from current chunk; returns 0 if successful
		int32_t	GetChunkDataSize();		//!< CALL: Returns size of current chunk
		int32_t	GetChunkVersion() {return mChunkVersion;}		//!< CALL: Lists fVersion in chunk header for convenience.  
		bool	IsEmpty();				//!< CALL: Returns true if no data is in the chunk
		void	Clear();				//!< Resets chunk
		//@{
		/*!	\name Internal Methods
		 *  An Effect Layer plugin can ignore these methods. They are handled by or used internally by the Effect Layer.
		 */
		void	LoadChunk(const AAX_SPlugInChunk *chunk);	//!< Sets current chunk to data in \a chunk parameter

	protected:	
		void	WordAlign(uint32_t &index);			//!< sets \a index to 4-byte boundary
		void	WordAlign(int32_t &index);			//!< sets \a index to 4-byte boundary
		int32_t	FindName(const AAX_CString &Name);	//!< used by public Find methods
		//@}	END Internal Methods group
		/*!	\brief	The last index found in the chunk
		 *	
		 *	\details
		 *	Since control values in chunks should tend to stay in order and in sync with
		 *	the way they're checked with controls within the plug-in, we'll keep track of
		 *	the value index to speed up searching.
		 */
		int32_t	mLastFoundIndex;

		char	*mChunkData;
		
		int32_t mChunkVersion;						//!< Equal to fVersion from the chunk header.  Equal to -1 if no chunk is loaded.
public:
		struct DataValue
		{
			int32_t		mDataType;
			AAX_CString	mDataName;		//!< name of the stored data
			int64_t		mIntValue;		//!< used if this DataValue is not a string
			AAX_CString	mStringValue;	//!< used if this DataValue is a string
			
			DataValue():
				mDataType(0),
				mDataName(AAX_CString()),
				mIntValue(0),
				mStringValue(AAX_CString())
			{};
		};
	
		std::vector<DataValue>	mDataValues;
};

#endif  //AAX_CHUNKDATAPARSER_H
