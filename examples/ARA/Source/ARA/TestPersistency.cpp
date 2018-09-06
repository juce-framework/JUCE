//------------------------------------------------------------------------------
//! \file        TestPersistency.cpp
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

#include "TestPersistency.h"

#include "ARA_Library/PlugIn/ARAPlug.h"

#include <vector>

// host-network byte ordering include
#if defined (_WIN32)
    #include <winsock2.h>
#elif defined (__APPLE__)
    #include <arpa/inet.h>
#endif

enum { archiveVersion = 1 };

using namespace ARA;
using namespace PlugIn;

/*******************************************************************************/

TestArchiver::TestArchiver (HostArchiveWriter* archiveWriter)
: _archiveWriter (archiveWriter),
  _location (0),
  _state (TestArchiveState::noError)
{
    writeInt64 (archiveVersion);
}

void TestArchiver::writeDouble (double data)
{
    write8ByteData (*(uint64_t *)&data);
}

void TestArchiver::writeInt64 (int64_t data)
{
    write8ByteData (*(uint64_t *)&data);
}

void TestArchiver::writeSize (size_t data)
{
    static_assert (sizeof (size_t) <= sizeof (uint64_t), "only implemented for architectures where size_t can be mapped to uint64_t without losing precision");
    write8ByteData ((uint64_t) data);
}

void TestArchiver::writeString (std::string data)
{
    size_t numBytes = data.size ();
    writeSize (numBytes);
    if (didSucceed () && !_archiveWriter->writeBytesToArchive (_location, numBytes, (const ARAByte*) data.c_str ()))
        _state = TestArchiveState::iOError;
    _location += numBytes;
}

void TestArchiver::write8ByteData (uint64_t data)
{
    uint64_t encodedData = htonll (data);
    if (didSucceed () && !_archiveWriter->writeBytesToArchive (_location, sizeof (data), (const ARAByte*) &encodedData))
        _state = TestArchiveState::iOError;
    _location += sizeof (data);
}

/*******************************************************************************/

TestUnarchiver::TestUnarchiver (HostArchiveReader* archiveReader)
: _archiveReader (archiveReader),
  _location (0),
  _state (TestArchiveState::noError)
{
    int64_t version = readInt64 ();
    if (didSucceed () && (version != archiveVersion))
        _state = TestArchiveState::unkownFormatError;
}

double TestUnarchiver::readDouble ()
{
    uint64_t encodedData = read8ByteData ();
    return *(double *)&encodedData;
}

int64_t TestUnarchiver::readInt64 ()
{
    uint64_t encodedData = read8ByteData ();
    return *(int64_t *)&encodedData;
}

size_t TestUnarchiver::readSize ()
{
    uint64_t data = read8ByteData ();

#if __cplusplus >= 201703L
    if constexpr (sizeof (size_t) < sizeof (uint64_t))
#else
    if (sizeof (size_t) < sizeof (uint64_t))
#endif
    {
        if (data > std::numeric_limits<size_t>::max ())
        {
            _state = TestArchiveState::incompatibleDataError;
            data = 0;
        }
    }

    return (size_t) data;
}

std::string TestUnarchiver::readString ()
{
    std::string data;
    size_t numBytes = readSize ();
    if (didSucceed () && numBytes)
    {
        std::vector<char> stringBuffer (numBytes + 1);
        if (_archiveReader->readBytesFromArchive (_location, numBytes, (ARAByte*) stringBuffer.data ()))
            data = stringBuffer.data ();
        else
            _state = TestArchiveState::iOError;
        _location += numBytes;
    }
    return data;
}

uint64_t TestUnarchiver::read8ByteData ()
{
    uint64_t encodedData;
    if (didSucceed () && !_archiveReader->readBytesFromArchive (_location, sizeof (encodedData), (ARAByte*) &encodedData))
        _state = TestArchiveState::iOError;
    if (!didSucceed ())
        encodedData = 0;
    _location += sizeof (encodedData);
    return ntohll (encodedData);
}
