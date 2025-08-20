//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/utility/optional.h
// Created by  : Steinberg, 08/2016
// Description : optional helper
//
//-----------------------------------------------------------------------------
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

#pragma once

#include <cassert>
#include <memory>
#include <utility>

//------------------------------------------------------------------------
namespace VST3 {

//------------------------------------------------------------------------
template <typename T>
struct Optional
{
	Optional () noexcept : valid (false) {}
	explicit Optional (const T& v) noexcept : _value (v), valid (true) {}
	Optional (T&& v) noexcept : _value (std::move (v)), valid (true) {}

	Optional (Optional&& other) noexcept { *this = std::move (other); }
	Optional& operator= (Optional&& other) noexcept
	{
		valid = other.valid;
		_value = std::move (other._value);
		return *this;
	}

	explicit operator bool () const noexcept
	{
		setValidationChecked ();
		return valid;
	}

	const T& operator* () const noexcept
	{
		checkValid ();
		return _value;
	}

	const T* operator-> () const noexcept
	{
		checkValid ();
		return &_value;
	}

	T& operator* () noexcept
	{
		checkValid ();
		return _value;
	}

	T* operator-> () noexcept
	{
		checkValid ();
		return &_value;
	}

	T&& value () noexcept
	{
		checkValid ();
		return std::move (_value);
	}

	const T& value () const noexcept
	{
		checkValid ();
		return _value;
	}

	void swap (T& other) noexcept
	{
		checkValid ();
		auto tmp = std::move (other);
		other = std::move (_value);
		_value = std::move (tmp);
	}

private:
	T _value {};
	bool valid;

#if !defined(NDEBUG)
	mutable bool validationChecked {false};
#endif

	void setValidationChecked () const
	{
#if !defined(NDEBUG)
		validationChecked = true;
#endif
	}
	void checkValid () const
	{
#if !defined(NDEBUG)
		assert (validationChecked);
#endif
	}
};

//------------------------------------------------------------------------
}
