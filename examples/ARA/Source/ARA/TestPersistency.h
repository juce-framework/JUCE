//------------------------------------------------------------------------------
//! \file        TestPersistency.h
//! \description archiver/unarchiver implementation for the ARA sample plug-in
//!              actual plug-ins will have a persistency implementation that is independent of ARA
//! \project     ARA SDK, examples
//------------------------------------------------------------------------------
// Copyright (c) 2012-2018, Celemony Software GmbH, All Rights Reserved.
// 
// License & Disclaimer:
// 
// This Software Development Kit (SDK) may not be distributed in parts or 
// its entirety without prior written agreement of Celemony Software GmbH. 
//
// This SDK must not be used to modify, adapt, reproduce or transfer any 
// software and/or technology used in any Celemony and/or Third-party 
// application and/or software module (hereinafter referred as "the Software"); 
// to modify, translate, reverse engineer, decompile, disassemble or create 
// derivative works based on the Software (except to the extent applicable laws 
// specifically prohibit such restriction) or to lease, assign, distribute or 
// otherwise transfer rights to the Software.
// Neither the name of the Celemony Software GmbH nor the names of its 
// contributors may be used to endorse or promote products derived from this 
// SDK without specific prior written permission.
//
// This SDK is provided by Celemony Software GmbH "as is" and any express or 
// implied warranties, including, but not limited to, the implied warranties of 
// non-infringement, merchantability and fitness for a particular purpose are
// disclaimed.
// In no event shall Celemony Software GmbH be liable for any direct, indirect, 
// incidental, special, exemplary, or consequential damages (including, but not 
// limited to, procurement of substitute goods or services; loss of use, data, 
// or profits; or business interruption) however caused and on any theory of 
// liability, whether in contract, strict liability, or tort (including 
// negligence or otherwise) arising in any way out of the use of this software, 
// even if advised of the possibility of such damage.
// Where the liability of Celemony Software GmbH is ruled out or restricted, 
// this will also apply for the personal liability of employees, representatives 
// and vicarious agents.
// The above restriction of liability will not apply if the damages suffered
// are attributable to willful intent or gross negligence or in the case of 
// physical injury.
//------------------------------------------------------------------------------

#pragma once

#include <string>
#include <cstdint>

namespace ARA
{
namespace PlugIn
{

class HostArchiveWriter;
class HostArchiveReader;

}	// namespace PlugIn
}	// namespace ARA

// Archiver/Unarchiver
// actual plug-ins will already feature some persistency implementation which is independent of ARA.
// some adapter can usually be written to hook up this existing code to ARA's archive readers/writers.
// the following code merely drafts such an implementation, it cannot be used in actual products!

enum class TestArchiveState
{
	noError = 0,
	iOError,                // could not read or write bytes
	                        // in ARA, host handles I/O and will display proper error message in this case
	unkownFormatError,      // archive was written by future version of the program
	                        // in ARA, actual plug-ins will display a proper error message in this case
	incompatibleDataError   // archive contains numbers that cannot be represented on the current architecture
	                        // (e.g. 64 bit archive with size_t that exceeds 32 bit architecture)
	                        // in ARA, actual plug-ins will display a proper error message in this case
};

/*******************************************************************************/
// encoder class
class TestArchiver
{
public:
	TestArchiver (ARA::PlugIn::HostArchiveWriter* archiveWriter);

	void writeDouble (double data);
	void writeInt64 (int64_t data);
	void writeSize (size_t data);
	void writeString (std::string data);

	TestArchiveState getState () const { return _state; }
	bool didSucceed () const { return (_state == TestArchiveState::noError); }

private:
	void write8ByteData (uint64_t data);

private:
	ARA::PlugIn::HostArchiveWriter* const _archiveWriter;
	size_t _location;
	TestArchiveState _state;
};

/*******************************************************************************/
// decoder class
class TestUnarchiver
{
public:
	TestUnarchiver (ARA::PlugIn::HostArchiveReader* archiveReader);

	double readDouble ();
	int64_t readInt64 ();
	size_t readSize ();
	std::string readString ();

	TestArchiveState getState () const { return _state; }
	bool didSucceed () const { return (_state == TestArchiveState::noError); }

private:
	uint64_t read8ByteData ();

private:
	ARA::PlugIn::HostArchiveReader* const _archiveReader;
	size_t _location;
	TestArchiveState _state;
};
