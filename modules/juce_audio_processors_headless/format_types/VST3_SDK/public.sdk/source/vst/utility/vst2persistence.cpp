//------------------------------------------------------------------------
// Flags       : clang-format SMTGSequencer
// Project     : VST3 SDK
// Filename    : public.sdk/source/vst/utility/vst2persistence.cpp
// Created by  : Steinberg, 12/2019
// Description : vst2 persistence helper
//
//------------------------------------------------------------------------
// LICENSE
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "public.sdk/source/vst/utility/vst2persistence.h"
#include "pluginterfaces/base/fplatform.h"
#include <limits>

//------------------------------------------------------------------------
namespace VST3 {
namespace {
namespace IO {

//------------------------------------------------------------------------
enum class Error
{
	NoError,
	Unknown,
	EndOfFile,
	BufferToBig,
	NotAllowed,
	InvalidArgument,
};

//------------------------------------------------------------------------
enum class SeekMode
{
	Set,
	End,
	Current
};

//------------------------------------------------------------------------
struct Result
{
	Error error {Error::Unknown};
	uint64_t bytes {0u};

	Result () noexcept = default;
	Result (Error error, uint64_t bytes = 0) noexcept : error (error), bytes (bytes) {}

	operator bool () const noexcept { return error == Error::NoError; }
};

//------------------------------------------------------------------------
struct ReadBufferDesc
{
	const uint64_t bytes;
	void* ptr;
};

//------------------------------------------------------------------------
struct WriteBufferDesc
{
	const uint64_t bytes;
	const void* ptr;
};

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
class ByteOrderStream
{
public:
//------------------------------------------------------------------------
	ByteOrderStream (Steinberg::IBStream& stream) noexcept : stream (stream) {}
	ByteOrderStream (ByteOrderStream&&) noexcept = delete;
	ByteOrderStream& operator= (ByteOrderStream&&) noexcept = delete;
	ByteOrderStream (const ByteOrderStream&) noexcept = delete;
	ByteOrderStream& operator= (const ByteOrderStream&) noexcept = delete;

	inline Result operator<< (const std::string& input) noexcept;
	inline Result operator>> (std::string& output) noexcept;

	template <typename T>
	inline Result operator<< (const T& input) noexcept;
	template <typename T>
	inline Result operator>> (T& output) const noexcept;

	inline Result read (const ReadBufferDesc& buffer) const noexcept;
	inline Result write (const WriteBufferDesc& buffer) noexcept;
	inline Result seek (SeekMode mode, int64_t bytes) const noexcept;
	inline Result tell () const noexcept;

//------------------------------------------------------------------------
private:
	template <size_t size>
	inline Result swapAndWrite (const uint8_t* buffer) noexcept;
	inline void swap (uint8_t* buffer, uint64_t size) const noexcept;
	Steinberg::IBStream& stream;
};

//------------------------------------------------------------------------
using LittleEndianStream = ByteOrderStream<kLittleEndian>;
using BigEndianStream = ByteOrderStream<kBigEndian>;
using NativeEndianStream = ByteOrderStream<BYTEORDER>;

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::read (const ReadBufferDesc& buffer) const noexcept
{
	if (buffer.bytes > static_cast<uint64_t> (std::numeric_limits<int32_t>::max ()))
		return Result (Error::BufferToBig);
	Steinberg::int32 readBytes = 0;
	auto tres = stream.read (buffer.ptr, static_cast<Steinberg::int32> (buffer.bytes), &readBytes);
	if (tres != Steinberg::kResultTrue)
		return Result (Error::Unknown);
	assert (readBytes >= 0);
	return Result {Error::NoError, static_cast<uint64_t> (readBytes)};
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::write (const WriteBufferDesc& buffer) noexcept
{
	if (buffer.bytes > static_cast<uint64_t> (std::numeric_limits<int32_t>::max ()))
		return Result (Error::BufferToBig);
	Steinberg::int32 writtenBytes = 0;
	auto tres = stream.write (const_cast<void*> (buffer.ptr),
	                          static_cast<Steinberg::int32> (buffer.bytes), &writtenBytes);
	if (tres != Steinberg::kResultTrue)
		return Result (Error::Unknown);
	assert (writtenBytes >= 0);
	return Result {Error::NoError, static_cast<uint64_t> (writtenBytes)};
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::seek (SeekMode mode, int64_t bytes) const noexcept
{
	Steinberg::int32 seekMode = 0;
	switch (mode)
	{
		case SeekMode::Set: seekMode = Steinberg::IBStream::kIBSeekSet; break;
		case SeekMode::Current: seekMode = Steinberg::IBStream::kIBSeekCur; break;
		case SeekMode::End: seekMode = Steinberg::IBStream::kIBSeekEnd; break;
	}
	Steinberg::int64 seekRes = 0;
	auto tres = stream.seek (static_cast<Steinberg::int64> (bytes), seekMode, &seekRes);
	if (tres != Steinberg::kResultTrue || seekRes < 0)
		return Result {Error::Unknown};
	return Result (Error::NoError, static_cast<uint64_t> (seekRes));
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::tell () const noexcept
{
	Steinberg::int64 tellRes = 0;
	auto tres = stream.tell (&tellRes);
	if (tres != Steinberg::kResultTrue || tellRes < 0)
		return Result {Error::Unknown};
	return Result {Error::NoError, static_cast<uint64_t> (tellRes)};
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::operator<< (const std::string& input) noexcept
{
	auto res = *this << static_cast<uint64_t> (input.length ());
	if (!res)
		return res;
	res = stream.write (const_cast<void*> (static_cast<const void*> (input.data ())),
	                    static_cast<Steinberg::int32> (input.length ()));
	res.bytes += sizeof (uint64_t);
	return res;
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
inline Result ByteOrderStream<StreamByteOrder>::operator>> (std::string& output) noexcept
{
	uint64_t length;
	auto res = *this >> length;
	if (!res)
		return res;
	output.resize (length);
	if (length > 0)
	{
		res = stream.read (&output.front (), static_cast<Steinberg::int32> (length));
		res.bytes += sizeof (uint64_t);
	}
	return res;
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
template <typename T>
inline Result ByteOrderStream<StreamByteOrder>::operator<< (const T& input) noexcept
{
	static_assert (std::is_standard_layout<T>::value, "Only standard layout types allowed");
	// with C++17: if constexpr (StreamByteOrder == BYTEORDER)
	if (constexpr bool tmp = (StreamByteOrder == BYTEORDER))
		return write (WriteBufferDesc {sizeof (T), static_cast<const void*> (&input)});

	return swapAndWrite<sizeof (T)> (reinterpret_cast<const uint8_t*> (&input));
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
template <typename T>
inline Result ByteOrderStream<StreamByteOrder>::operator>> (T& output) const noexcept
{
	static_assert (std::is_standard_layout<T>::value, "Only standard layout types allowed");
	auto res = read (ReadBufferDesc {sizeof (T), &output});
	// with C++17: if constexpr (StreamByteOrder == BYTEORDER)
	if (constexpr bool tmp = (StreamByteOrder == BYTEORDER))
		return res;

	swap (reinterpret_cast<uint8_t*> (&output), res.bytes);
	return res;
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
template <size_t _size>
inline Result ByteOrderStream<StreamByteOrder>::swapAndWrite (const uint8_t* buffer) noexcept
{
	// with C++17: if constexpr (_size > 1)
	if (constexpr bool tmp2 = (_size > 1))
	{
		int8_t tmp[_size];

		constexpr auto halfSize = _size / 2;
		auto size = _size;
		auto low = buffer;
		auto high = buffer + size - 1;

		while (size > halfSize)
		{
			tmp[size - 2] = buffer[(_size - size) + 1];
			tmp[(_size - size) + 1] = buffer[size - 2];
			tmp[_size - size] = *high;
			tmp[size - 1] = *low;
			low += 2;
			high -= 2;
			size -= 2;
		}
		return write (WriteBufferDesc {_size, tmp});
	}
	return write (WriteBufferDesc {1, buffer});
}

//------------------------------------------------------------------------
template <uint32_t StreamByteOrder>
inline void ByteOrderStream<StreamByteOrder>::swap (uint8_t* buffer, uint64_t size) const noexcept
{
	if (size < 2)
		return;
	auto low = buffer;
	auto high = buffer + size - 1;
	while (size >= 2)
	{
		auto tmp = *low;
		*low = *high;
		*high = tmp;
		low += 2;
		high -= 2;
		size -= 2;
	}
}

//------------------------------------------------------------------------
} // IO

//------------------------------------------------------------------------
constexpr int32_t cMagic = 'CcnK';
constexpr int32_t bankMagic = 'FxBk';
constexpr int32_t privateChunkID = 'VstW';
constexpr int32_t chunkBankMagic = 'FBCh';
constexpr int32_t programMagic = 'FxCk';
constexpr int32_t chunkProgramMagic = 'FPCh';

//------------------------------------------------------------------------
Optional<VST3::Vst2xProgram> loadProgram (const IO::BigEndianStream& state,
                                          const Optional<int32_t>& vst2xUniqueID)
{
	Vst2xProgram program;
	int32_t id;
	if (!(state >> id))
		return {};
	if (id != cMagic)
		return {};
	int32_t bankSize;
	if (!(state >> bankSize))
		return {};
	int32_t fxMagic;
	if (!(state >> fxMagic))
		return {};
	if (!(fxMagic == programMagic || fxMagic == chunkProgramMagic))
		return {};
	int32_t formatVersion;
	if (!(state >> formatVersion))
		return {};
	int32_t fxId;
	if (!(state >> fxId))
		return {};
	if (vst2xUniqueID && fxId != *vst2xUniqueID)
		return {};
	int32_t fxVersion;
	if (!(state >> fxVersion))
		return {};
	int32_t numParams;
	if (!(state >> numParams))
		return {};
	if (numParams < 0)
		return {};
	char name[29];
	if (!state.read ({28, name}))
		return {};
	name[28] = 0;
	program.name = name;
	program.fxUniqueID = fxId;
	program.fxVersion = fxVersion;
	if (fxMagic == chunkProgramMagic)
	{
		uint32_t chunkSize;
		if (!(state >> chunkSize))
			return {};
		program.chunk.resize (chunkSize);
		if (!state.read ({chunkSize, program.chunk.data ()}))
			return {};
	}
	else
	{
		program.values.resize (numParams);
		float paramValue;
		for (int32_t i = 0; i < numParams; ++i)
		{
			if (!(state >> paramValue))
				return {};
			program.values[i] = paramValue;
		}
	}
	return {std::move (program)};
}

//------------------------------------------------------------------------
bool loadPrograms (Steinberg::IBStream& stream, Vst2xState::Programs& programs,
                   const Optional<int32_t>& vst2xUniqueID)
{
	IO::BigEndianStream state (stream);

	for (auto& program : programs)
	{
		if (auto prg = loadProgram (state, vst2xUniqueID))
			std::swap (program, *prg);
		else
			return false;
	}
	return true;
}

//------------------------------------------------------------------------
template <typename SizeT, typename StreamT, typename Proc>
IO::Error streamSizeWriter (StreamT& stream, Proc proc)
{
	auto startPos = stream.tell ();
	if (startPos.error != IO::Error::NoError)
		return startPos.error;
	auto res = stream << static_cast<SizeT> (0); // placeholder
	if (!res)
		return res.error;
	auto procRes = proc ();
	if (procRes != IO::Error::NoError)
		return procRes;
	auto endPos = stream.tell ();
	if (endPos.error != IO::Error::NoError)
		return endPos.error;
	auto size = (endPos.bytes - startPos.bytes) - 4;
	auto typeSize = static_cast<SizeT> (size);
	if (size != static_cast<uint64_t> (typeSize))
		return IO::Error::Unknown;
	res = stream.seek (IO::SeekMode::Set, startPos.bytes);
	if (!res)
		return res.error;
	res = (stream << typeSize);
	if (!res)
		return res.error;
	res = stream.seek (IO::SeekMode::Set, endPos.bytes);
	return res.error;
}

//------------------------------------------------------------------------
template <typename StreamT>
IO::Error writePrograms (StreamT& stream, const Vst2xState::Programs& programs)
{
	for (const auto& program : programs)
	{
		auto res = stream << cMagic;
		if (!res)
			return res.error;
		res = streamSizeWriter<int32_t> (stream, [&] () {
			bool writeChunk = !program.chunk.empty ();
			if (!(res = stream << (writeChunk ? chunkProgramMagic : programMagic)))
				return res.error;
			int32_t version = 1;
			if (!(res = stream << version))
				return res.error;
			if (!(res = stream << program.fxUniqueID))
				return res.error;
			int32_t fxVersion = program.fxVersion;
			if (!(res = stream << fxVersion))
				return res.error;
			uint32_t numParams = static_cast<uint32_t> (program.values.size ());
			if (!(res = stream << numParams))
				return res.error;
			auto programName = program.name;
			programName.resize (28);
			for (auto c : programName)
			{
				if (!(res = stream << c))
					return res.error;
			}
			if (writeChunk)
			{
				if (!(res = stream << static_cast<int32_t> (program.chunk.size ())))
					return res.error;
				if (!(res = stream.write ({program.chunk.size (), program.chunk.data ()})))
					return res.error;
			}
			else
			{
				for (auto value : program.values)
				{
					if (!(res = stream << value))
						return res.error;
				}
			}
			return IO::Error::NoError;
		});
		if (res.error != IO::Error::NoError)
			return res.error;
	}
	return IO::Error::NoError;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Optional<Vst2xState> tryVst2StateLoad (Steinberg::IBStream& stream,
                                       Optional<int32_t> vst2xUniqueID) noexcept
{
	Vst2xState result;

	IO::BigEndianStream state (stream);
	int32_t version;
	int32_t size;
	int32_t id;
	if (!(state >> id))
		return {};
	if (id == privateChunkID)
	{
		if (!(state >> size))
			return {};
		if (!(state >> version))
			return {};
		int32_t bypass;
		if (!(state >> bypass))
			return {};
		result.isBypassed = bypass ? true : false;
		if (!(state >> id))
			return {};
	}
	if (id != cMagic)
		return {};
	int32_t bankSize;
	if (!(state >> bankSize))
		return {};
	int32_t fxMagic;
	if (!(state >> fxMagic))
		return {};
	if (!(fxMagic == bankMagic || fxMagic == chunkBankMagic))
		return {};
	int32_t bankVersion;
	if (!(state >> bankVersion))
		return {};
	int32_t fxId;
	if (!(state >> fxId))
		return {};
	if (vst2xUniqueID && fxId != *vst2xUniqueID)
		return {};
	result.fxUniqueID = fxId;
	int32_t fxVersion;
	if (!(state >> fxVersion))
		return {};
	result.fxVersion = fxVersion;

	int32_t numPrograms;
	if (!(state >> numPrograms))
		return {};
	if (numPrograms < 1 && fxMagic == bankMagic)
		return {};

	int32_t currentProgram = 0;
	if (bankVersion >= 1)
	{
		if (!(state >> currentProgram))
			return {};
		state.seek (IO::SeekMode::Current, 124); // future
	}
	result.currentProgram = currentProgram;
	if (fxMagic == bankMagic)
	{
		result.programs.resize (numPrograms);
		if (!loadPrograms (stream, result.programs, vst2xUniqueID))
			return {};
		assert (static_cast<int32_t> (result.programs.size ()) > currentProgram);
	}
	else
	{
		uint32_t chunkSize;
		if (!(state >> chunkSize))
			return {};
		if (chunkSize == 0)
			return {};
		result.chunk.resize (chunkSize);
		if (!state.read ({chunkSize, result.chunk.data ()}))
			return {};
	}
	return {std::move (result)};
}

//------------------------------------------------------------------------
bool writeVst2State (const Vst2xState& state, Steinberg::IBStream& _stream,
                     bool writeBypassState) noexcept
{
	IO::BigEndianStream stream (_stream);
	if (writeBypassState)
	{
		if (!(stream << privateChunkID))
			return false;
		if (streamSizeWriter<uint32_t> (stream, [&] () {
			    uint32_t version = 1;
			    auto res = (stream << version);
			    if (!res)
				    return res.error;
			    int32_t bypass = state.isBypassed ? 1 : 0;
			    return (stream << bypass).error;
		    }) != IO::Error::NoError)
		{
			return false;
		}
	}
	if (!(stream << cMagic))
	{
		return false;
	}
	if (streamSizeWriter<int32_t> (stream, [&] () {
		    bool writeChunk = !state.chunk.empty ();
		    IO::Result res;
		    if (!(res = (stream << (writeChunk ? chunkBankMagic : bankMagic))))
			    return res.error;
		    int32_t bankVersion = 2;
		    if (!(res = (stream << bankVersion)))
			    return res.error;
		    if (!(res = (stream << state.fxUniqueID)))
			    return res.error;
		    if (!(res = (stream << state.fxVersion)))
			    return res.error;
		    int32_t numPrograms = writeChunk ? 1 : static_cast<int32_t> (state.programs.size ());
		    if (!(res = (stream << numPrograms)))
			    return res.error;
		    if (bankVersion > 1)
		    {
			    if (!(res = (stream << state.currentProgram)))
				    return res.error;
			    // write 124 zero bytes
			    uint8_t byte = 0;
			    for (uint32_t i = 0; i < 124; ++i)
				    if (!(res = (stream << byte)))
					    return res.error;
		    }
		    if (writeChunk)
		    {
			    auto chunkSize = static_cast<uint32_t> (state.chunk.size ());
			    if (!(res = (stream << chunkSize)))
				    return res.error;
			    stream.write ({state.chunk.size (), state.chunk.data ()});
		    }
		    else
		    {
			    writePrograms (stream, state.programs);
		    }
		    return IO::Error::NoError;
	    }) != IO::Error::NoError)
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------------------
Optional<Vst2xProgram> tryVst2ProgramLoad (Steinberg::IBStream& stream,
                                           Optional<int32_t> vst2xUniqueID) noexcept
{
	IO::BigEndianStream state (stream);
	return loadProgram (state, vst2xUniqueID);
}

//------------------------------------------------------------------------
} // VST3
