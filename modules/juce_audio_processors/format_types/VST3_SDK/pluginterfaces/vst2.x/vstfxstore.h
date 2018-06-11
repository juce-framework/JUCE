//------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 2.4
//
// Category    : VST 2.x Interfaces
// Filename    : pluginterfaces/vst2.x/vstfxstore.h
// Created by  : Steinberg, 01/2004
// Description : Definition of Program (fxp) and Bank (fxb) structures
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// This Software Development Kit may not be distributed in parts or its entirety
// without prior written agreement by Steinberg Media Technologies GmbH.
// This SDK must not be used to re-engineer or manipulate any technology used
// in any Steinberg or Third-party application or software module,
// unless permitted by law.
// Neither the name of the Steinberg Media Technologies nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SDK IS PROVIDED BY STEINBERG MEDIA TECHNOLOGIES GMBH "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL STEINBERG MEDIA TECHNOLOGIES GMBH BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------------------------

#ifndef __vstfxstore__
#define __vstfxstore__

#ifndef __aeffect__
#include "aeffect.h"
#endif

//-------------------------------------------------------------------------------------------------------
/** Root chunk identifier for Programs (fxp) and Banks (fxb). */
#define cMagic				'CcnK'

/** Regular Program (fxp) identifier. */
#define fMagic				'FxCk'

/** Regular Bank (fxb) identifier. */
#define bankMagic			'FxBk'

/** Program (fxp) identifier for opaque chunk data. */
#define chunkPresetMagic	'FPCh'

/** Bank (fxb) identifier for opaque chunk data. */
#define chunkBankMagic		'FBCh'

/*
	Note: The C data structures below are for illustration only. You can not read/write them directly.
	The byte order on disk of fxp and fxb files is Big Endian. You have to swap integer
	and floating-point values on Little Endian platforms (Windows, MacIntel)!
*/

//-------------------------------------------------------------------------------------------------------
/** Program (fxp) structure. */
//-------------------------------------------------------------------------------------------------------
typedef struct fxProgram
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 chunkMagic;		///< 'CcnK'
	VstInt32 byteSize;			///< size of this chunk, excl. magic + byteSize

	VstInt32 fxMagic;			///< 'FxCk' (regular) or 'FPCh' (opaque chunk)
	VstInt32 version;			///< format version (currently 1)
	VstInt32 fxID;				///< fx unique ID
	VstInt32 fxVersion;			///< fx version

	VstInt32 numParams;			///< number of parameters
	char prgName[28];			///< program name (null-terminated ASCII string)

	union
	{
		float params[1];		///< variable sized array with parameter values
		struct
		{
			VstInt32 size;		///< size of program data
			char chunk[1];		///< variable sized array with opaque program data
		} data;					///< program chunk data
	} content;					///< program content depending on fxMagic
//-------------------------------------------------------------------------------------------------------
} fxProgram;

//-------------------------------------------------------------------------------------------------------
/** Bank (fxb) structure. */
//-------------------------------------------------------------------------------------------------------
struct fxBank
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 chunkMagic;		///< 'CcnK'
	VstInt32 byteSize;			///< size of this chunk, excl. magic + byteSize

	VstInt32 fxMagic;			///< 'FxBk' (regular) or 'FBCh' (opaque chunk)
	VstInt32 version;			///< format version (1 or 2)
	VstInt32 fxID;				///< fx unique ID
	VstInt32 fxVersion;			///< fx version

	VstInt32 numPrograms;		///< number of programs

#if VST_2_4_EXTENSIONS
	VstInt32 currentProgram;	///< version 2: current program number
	char future[124];			///< reserved, should be zero
#else
	char future[128];			///< reserved, should be zero
#endif

	union
	{
		fxProgram programs[1];	///< variable number of programs
		struct
		{
			VstInt32 size;		///< size of bank data
			char chunk[1];		///< variable sized array with opaque bank data
		} data;					///< bank chunk data
	} content;					///< bank content depending on fxMagic
//-------------------------------------------------------------------------------------------------------
};

#endif // __vstfxstore__
