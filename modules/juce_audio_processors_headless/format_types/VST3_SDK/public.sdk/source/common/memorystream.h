//-----------------------------------------------------------------------------
// Project     : SDK Core
//
// Category    : Common Classes
// Filename    : public.sdk/source/common/memorystream.h
// Created by  : Steinberg, 03/2008
// Description : IBStream Implementation for memory blocks
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/ibstream.h"

namespace Steinberg {

//------------------------------------------------------------------------
/** Memory based Stream for IBStream implementation (using malloc).
\ingroup sdkBase
*/
class MemoryStream : public IBStream
{
public:
	//------------------------------------------------------------------------
	MemoryStream ();
	MemoryStream (void* memory, TSize memorySize); 	///< reuse a given memory without getting ownership
	virtual ~MemoryStream ();

	//---IBStream---------------------------------------
	tresult PLUGIN_API read  (void* buffer, int32 numBytes, int32* numBytesRead) SMTG_OVERRIDE;
	tresult PLUGIN_API write (void* buffer, int32 numBytes, int32* numBytesWritten) SMTG_OVERRIDE;
	tresult PLUGIN_API seek  (int64 pos, int32 mode, int64* result) SMTG_OVERRIDE;
	tresult PLUGIN_API tell  (int64* pos) SMTG_OVERRIDE;

	TSize getSize () const;		///< returns the current memory size
	void setSize (TSize size);	///< set the memory size, a realloc will occur if memory already used
	char* getData () const;		///< returns the memory pointer
	char* detachData ();	///< returns the memory pointer and give up ownership
	bool truncate ();		///< realloc to the current use memory size if needed
	bool truncateToCursor ();	///< truncate memory at current cursor position

	//------------------------------------------------------------------------
	DECLARE_FUNKNOWN_METHODS
protected:
	char* memory;				// memory block
	TSize memorySize;			// size of the memory block
	TSize size;					// size of the stream
	int64 cursor;				// stream pointer
	bool ownMemory;				// stream has allocated memory itself
	bool allocationError;       // stream invalid
};

} // namespace
