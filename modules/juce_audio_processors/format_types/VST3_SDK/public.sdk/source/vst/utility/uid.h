//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/utility/uid.h
// Created by  : Steinberg, 08/2016
// Description : UID
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2023, Steinberg Media Technologies GmbH, All Rights Reserved
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

#pragma once

#include "optional.h"
#include "pluginterfaces/base/funknown.h"
#include <string>

//------------------------------------------------------------------------
namespace VST3 {

//------------------------------------------------------------------------
struct UID
{
#if defined(SMTG_OS_WINDOWS) && SMTG_OS_WINDOWS == 1
	static constexpr bool defaultComFormat = true;
#else
	static constexpr bool defaultComFormat = false;
#endif

	using TUID = Steinberg::TUID;

	constexpr UID () noexcept = default;
	UID (uint32_t l1, uint32_t l2, uint32_t l3, uint32_t l4, bool comFormat = defaultComFormat)
	noexcept;
	UID (const TUID& uid) noexcept;
	UID (const UID& uid) noexcept;
	UID& operator= (const UID& uid) noexcept;
	UID& operator= (const TUID& uid) noexcept;

	constexpr const TUID& data () const noexcept;
	constexpr size_t size () const noexcept;

	std::string toString (bool comFormat = defaultComFormat) const noexcept;

	template<typename StringT>
	static Optional<UID> fromString (const StringT& str,
	                                 bool comFormat = defaultComFormat) noexcept;

	static UID fromTUID (const TUID _uid) noexcept;
//------------------------------------------------------------------------
private:
	Steinberg::TUID _data {};

	struct GUID
	{
		uint32_t Data1;
		uint16_t Data2;
		uint16_t Data3;
		uint8_t Data4[8];
	};
};

//------------------------------------------------------------------------
inline bool operator== (const UID& uid1, const UID& uid2)
{
	const uint64_t* p1 = reinterpret_cast<const uint64_t*> (uid1.data ());
	const uint64_t* p2 = reinterpret_cast<const uint64_t*> (uid2.data ());
	return p1[0] == p2[0] && p1[1] == p2[1];
}

//------------------------------------------------------------------------
inline bool operator!= (const UID& uid1, const UID& uid2)
{
	return !(uid1 == uid2);
}

//------------------------------------------------------------------------
inline bool operator< (const UID& uid1, const UID& uid2)
{
	const uint64_t* p1 = reinterpret_cast<const uint64_t*> (uid1.data ());
	const uint64_t* p2 = reinterpret_cast<const uint64_t*> (uid2.data ());
	return (p1[0] < p2[0]) && (p1[1] < p2[1]);
}

//------------------------------------------------------------------------
inline UID::UID (uint32_t l1, uint32_t l2, uint32_t l3, uint32_t l4, bool comFormat) noexcept
{
	if (comFormat)
	{
		_data[0] = static_cast<int8_t> ((l1 & 0x000000FF));
		_data[1] = static_cast<int8_t> ((l1 & 0x0000FF00) >> 8);
		_data[2] = static_cast<int8_t> ((l1 & 0x00FF0000) >> 16);
		_data[3] = static_cast<int8_t> ((l1 & 0xFF000000) >> 24);
		_data[4] = static_cast<int8_t> ((l2 & 0x00FF0000) >> 16);
		_data[5] = static_cast<int8_t> ((l2 & 0xFF000000) >> 24);
		_data[6] = static_cast<int8_t> ((l2 & 0x000000FF));
		_data[7] = static_cast<int8_t> ((l2 & 0x0000FF00) >> 8);
		_data[8] = static_cast<int8_t> ((l3 & 0xFF000000) >> 24);
		_data[9] = static_cast<int8_t> ((l3 & 0x00FF0000) >> 16);
		_data[10] = static_cast<int8_t> ((l3 & 0x0000FF00) >> 8);
		_data[11] = static_cast<int8_t> ((l3 & 0x000000FF));
		_data[12] = static_cast<int8_t> ((l4 & 0xFF000000) >> 24);
		_data[13] = static_cast<int8_t> ((l4 & 0x00FF0000) >> 16);
		_data[14] = static_cast<int8_t> ((l4 & 0x0000FF00) >> 8);
		_data[15] = static_cast<int8_t> ((l4 & 0x000000FF));
	}
	else
	{
		_data[0] = static_cast<int8_t> ((l1 & 0xFF000000) >> 24);
		_data[1] = static_cast<int8_t> ((l1 & 0x00FF0000) >> 16);
		_data[2] = static_cast<int8_t> ((l1 & 0x0000FF00) >> 8);
		_data[3] = static_cast<int8_t> ((l1 & 0x000000FF));
		_data[4] = static_cast<int8_t> ((l2 & 0xFF000000) >> 24);
		_data[5] = static_cast<int8_t> ((l2 & 0x00FF0000) >> 16);
		_data[6] = static_cast<int8_t> ((l2 & 0x0000FF00) >> 8);
		_data[7] = static_cast<int8_t> ((l2 & 0x000000FF));
		_data[8] = static_cast<int8_t> ((l3 & 0xFF000000) >> 24);
		_data[9] = static_cast<int8_t> ((l3 & 0x00FF0000) >> 16);
		_data[10] = static_cast<int8_t> ((l3 & 0x0000FF00) >> 8);
		_data[11] = static_cast<int8_t> ((l3 & 0x000000FF));
		_data[12] = static_cast<int8_t> ((l4 & 0xFF000000) >> 24);
		_data[13] = static_cast<int8_t> ((l4 & 0x00FF0000) >> 16);
		_data[14] = static_cast<int8_t> ((l4 & 0x0000FF00) >> 8);
		_data[15] = static_cast<int8_t> ((l4 & 0x000000FF));
	}
}

//------------------------------------------------------------------------
inline UID::UID (const TUID& uid) noexcept
{
	*this = uid;
}

//------------------------------------------------------------------------
inline UID::UID (const UID& uid) noexcept
{
	*this = uid;
}

//------------------------------------------------------------------------
inline UID& UID::operator= (const UID& uid) noexcept
{
	*this = uid.data ();
	return *this;
}

//------------------------------------------------------------------------
inline UID& UID::operator= (const TUID& uid) noexcept
{
	uint64_t* p1 = reinterpret_cast<uint64_t*> (_data);
	const uint64_t* p2 = reinterpret_cast<const uint64_t*> (uid);
	p1[0] = p2[0];
	p1[1] = p2[1];
	return *this;
}

//------------------------------------------------------------------------
inline constexpr auto UID::data () const noexcept -> const TUID&
{
	return _data;
}

//------------------------------------------------------------------------
inline constexpr size_t UID::size () const noexcept
{
	return sizeof (TUID);
}

//------------------------------------------------------------------------
inline std::string UID::toString (bool comFormat) const noexcept
{
	std::string result;
	result.reserve (32);
	if (comFormat)
	{
		const auto& g = reinterpret_cast<const GUID*> (_data);

		char tmp[21] {};
		snprintf (tmp, 21, "%08X%04X%04X", g->Data1, g->Data2, g->Data3);
		result = tmp;

		for (uint32_t i = 0; i < 8; ++i)
		{
			char s[3] {};
			snprintf (s, 3, "%02X", g->Data4[i]);
			result += s;
		}
	}
	else
	{
		for (uint32_t i = 0; i < 16; ++i)
		{
			char s[3] {};
			snprintf (s, 3, "%02X", static_cast<uint8_t> (_data[i]));
			result += s;
		}
	}
	return result;
}

//------------------------------------------------------------------------
template<typename StringT>
inline Optional<UID> UID::fromString (const StringT& str, bool comFormat) noexcept
{
	if (str.length () != 32)
		return {};
	// TODO: this is a copy from FUID. there are no input validation checks !!!
	if (comFormat)
	{
		TUID uid {};
		GUID g;
		char s[33];

		strcpy (s, str.data ());
		s[8] = 0;
		sscanf (s, "%x", &g.Data1);
		strcpy (s, str.data () + 8);
		s[4] = 0;
		sscanf (s, "%hx", &g.Data2);
		strcpy (s, str.data () + 12);
		s[4] = 0;
		sscanf (s, "%hx", &g.Data3);

		memcpy (uid, &g, 8);

		for (uint32_t i = 8; i < 16; ++i)
		{
			char s2[3] {};
			s2[0] = str[i * 2];
			s2[1] = str[i * 2 + 1];

			int32_t d = 0;
			sscanf (s2, "%2x", &d);
			uid[i] = static_cast<char> (d);
		}
		return {uid};
	}
	else
	{
		TUID uid {};
		for (uint32_t i = 0; i < 16; ++i)
		{
			char s[3] {};
			s[0] = str[i * 2];
			s[1] = str[i * 2 + 1];

			int32_t d = 0;
			sscanf (s, "%2x", &d);
			uid[i] = static_cast<char> (d);
		}
		return {uid};
	}
}

//------------------------------------------------------------------------
inline UID UID::fromTUID (const TUID _uid) noexcept
{
	UID result;

	uint64_t* p1 = reinterpret_cast<uint64_t*> (result._data);
	const uint64_t* p2 = reinterpret_cast<const uint64_t*> (_uid);
	p1[0] = p2[0];
	p1[1] = p2[1];

	return result;
}

//------------------------------------------------------------------------
} // VST3
