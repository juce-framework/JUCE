//-----------------------------------------------------------------------------
// Project     : VST SDK
// Flags       : clang-format SMTGSequencer
//
// Category    :
// Filename    : public.sdk/source/vst/moduleinfo/jsoncxx.h
// Created by  : Steinberg, 12/2021
// Description :
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
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#pragma once

#include "json.h"
#include <cassert>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#if defined(_MSC_VER) || __has_include(<charconv>)
#include <charconv>
#define SMTG_HAS_CHARCONV
#endif

//------------------------------------------------------------------------
namespace JSON {
namespace Detail {

//------------------------------------------------------------------------
template <typename JsonT>
struct Base
{
	explicit Base (JsonT* o) : object_ (o) {}
	explicit Base (const Base& o) : object_ (o.object_) {}

	Base& operator= (const Base& o) = default;

	operator JsonT* () const { return object_; }
	JsonT* jsonValue () const { return object_; }

protected:
	Base () : object_ (nullptr) {}

	JsonT* object_;
};

//------------------------------------------------------------------------
template <typename JsonElement>
struct Iterator
{
	explicit Iterator (JsonElement el) : el (el) {}

	bool operator== (const Iterator& other) const { return other.el == el; }
	bool operator!= (const Iterator& other) const { return other.el != el; }

	const JsonElement& operator* () const { return el; }
	const JsonElement& operator-> () const { return el; }

	Iterator& operator++ ()
	{
		if (el)
			el = el.next ();
		return *this;
	}

	Iterator operator++ (int)
	{
		auto it = Iterator (el);
		operator++ ();
		return it;
	}

private:
	JsonElement el;
};

//------------------------------------------------------------------------
} // Detail

struct Object;
struct Array;
struct String;
struct Number;
struct Boolean;

//------------------------------------------------------------------------
enum class Type
{
	Object,
	Array,
	String,
	Number,
	True,
	False,
	Null,
};

//------------------------------------------------------------------------
struct SourceLocation
{
	size_t offset;
	size_t line;
	size_t row;
};

//------------------------------------------------------------------------
struct Value : Detail::Base<json_value_s>
{
	using Detail::Base<json_value_s>::Base;
	using VariantT = std::variant<Object, Array, String, Number, Boolean, std::nullptr_t>;

	std::optional<Object> asObject () const;
	std::optional<Array> asArray () const;
	std::optional<String> asString () const;
	std::optional<Number> asNumber () const;
	std::optional<Boolean> asBoolean () const;
	std::optional<std::nullptr_t> asNull () const;

	VariantT asVariant () const;
	Type type () const;

	SourceLocation getSourceLocation () const;
};

//------------------------------------------------------------------------
struct Boolean
{
	Boolean (size_t type) : value (type == json_type_true) {}

	operator bool () const { return value; }

private:
	bool value;
};

//------------------------------------------------------------------------
struct String : Detail::Base<json_string_s>
{
	using Detail::Base<json_string_s>::Base;

	std::string_view text () const { return {jsonValue ()->string, jsonValue ()->string_size}; }

	SourceLocation getSourceLocation () const;
};

//------------------------------------------------------------------------
struct Number : Detail::Base<json_number_s>
{
	using Detail::Base<json_number_s>::Base;

	std::string_view text () const { return {jsonValue ()->number, jsonValue ()->number_size}; }

	std::optional<int64_t> getInteger () const;
	std::optional<double> getDouble () const;
};

//------------------------------------------------------------------------
struct ObjectElement : Detail::Base<json_object_element_s>
{
	using Detail::Base<json_object_element_s>::Base;

	String name () const { return String (jsonValue ()->name); }
	Value value () const { return Value (jsonValue ()->value); }

	ObjectElement next () const { return ObjectElement (jsonValue ()->next); }
};

//------------------------------------------------------------------------
struct Object : Detail::Base<json_object_s>
{
	using Detail::Base<json_object_s>::Base;
	using Iterator = Detail::Iterator<ObjectElement>;

	size_t size () const { return jsonValue ()->length; }

	Iterator begin () const { return Iterator (ObjectElement (jsonValue ()->start)); }
	Iterator end () const { return Iterator (ObjectElement (nullptr)); }
};

//------------------------------------------------------------------------
struct ArrayElement : Detail::Base<json_array_element_s>
{
	using Detail::Base<json_array_element_s>::Base;

	Value value () const { return Value (jsonValue ()->value); }

	ArrayElement next () const { return ArrayElement (jsonValue ()->next); }
};

//------------------------------------------------------------------------
struct Array : Detail::Base<json_array_s>
{
	using Detail::Base<json_array_s>::Base;
	using Iterator = Detail::Iterator<ArrayElement>;

	size_t size () const { return jsonValue ()->length; }

	Iterator begin () const { return Iterator (ArrayElement (jsonValue ()->start)); }
	Iterator end () const { return Iterator (ArrayElement (nullptr)); }
};

//------------------------------------------------------------------------
struct Document : Value
{
	static std::variant<Document, json_parse_result_s> parse (std::string_view data)
	{
		auto allocate = [] (void*, size_t allocSize) { return std::malloc (allocSize); };
		json_parse_result_s parse_result {};
		auto value = json_parse_ex (data.data (), data.size (),
		                            json_parse_flags_allow_json5 |
		                                json_parse_flags_allow_location_information,
		                            allocate, nullptr, &parse_result);
		if (value)
			return Document (value);
		return parse_result;
	}
	~Document () noexcept
	{
		if (object_)
			std::free (object_);
	}

	Document (Document&& doc) noexcept { *this = std::move (doc); }
	Document& operator= (Document&& doc) noexcept
	{
		std::swap (object_, doc.object_);
		return *this;
	}

private:
	using Value::Value;
};

//------------------------------------------------------------------------
inline std::optional<Object> Value::asObject () const
{
	if (type () != Type::Object)
		return {};
	return Object (json_value_as_object (jsonValue ()));
}

//------------------------------------------------------------------------
inline std::optional<Array> Value::asArray () const
{
	if (type () != Type::Array)
		return {};
	return Array (json_value_as_array (jsonValue ()));
}

//------------------------------------------------------------------------
inline std::optional<String> Value::asString () const
{
	if (type () != Type::String)
		return {};
	return String (json_value_as_string (jsonValue ()));
}

//------------------------------------------------------------------------
inline std::optional<Number> Value::asNumber () const
{
	if (type () != Type::Number)
		return {};
	return Number (json_value_as_number (jsonValue ()));
}

//------------------------------------------------------------------------
inline std::optional<Boolean> Value::asBoolean () const
{
	if (type () == Type::True || type () == Type::False)
		return Boolean (jsonValue ()->type);
	return {};
}

//------------------------------------------------------------------------
inline std::optional<std::nullptr_t> Value::asNull () const
{
	if (type () != Type::Null)
		return {};
	return nullptr;
}

//------------------------------------------------------------------------
inline Type Value::type () const
{
	switch (jsonValue ()->type)
	{
		case json_type_string: return Type::String;
		case json_type_number: return Type::Number;
		case json_type_object: return Type::Object;
		case json_type_array: return Type::Array;
		case json_type_true: return Type::True;
		case json_type_false: return Type::False;
		case json_type_null: return Type::Null;
	}
	assert (false);
	return Type::Null;
}

//------------------------------------------------------------------------
inline Value::VariantT Value::asVariant () const
{
	switch (type ())
	{
		case Type::String: return *asString ();
		case Type::Number: return *asNumber ();
		case Type::Object: return *asObject ();
		case Type::Array: return *asArray ();
		case Type::True: return *asBoolean ();
		case Type::False: return *asBoolean ();
		case Type::Null: return *asNull ();
	}
	assert (false);
	return nullptr;
}

//------------------------------------------------------------------------
inline SourceLocation Value::getSourceLocation () const
{
	auto exValue = reinterpret_cast<json_value_ex_s*> (jsonValue ());
	return {exValue->offset, exValue->line_no, exValue->row_no};
}

//------------------------------------------------------------------------
inline SourceLocation String::getSourceLocation () const
{
	auto exValue = reinterpret_cast<json_string_ex_s*> (jsonValue ());
	return {exValue->offset, exValue->line_no, exValue->row_no};
}

//------------------------------------------------------------------------
inline std::optional<int64_t> Number::getInteger () const
{
#if defined(SMTG_HAS_CHARCONV)
	int64_t result {0};
	auto res = std::from_chars (jsonValue ()->number,
	                            jsonValue ()->number + jsonValue ()->number_size, result);
	if (res.ec == std::errc ())
		return result;
	return {};
#else
	int64_t result {0};
	std::string str (jsonValue ()->number, jsonValue ()->number + jsonValue ()->number_size);
	if (std::sscanf (str.data (), "%lld", &result) != 1)
		return {};
	return result;
#endif
}

//------------------------------------------------------------------------
inline std::optional<double> Number::getDouble () const
{
#if 1 // clang still has no floting point from_chars version
	size_t ctrl {0};
	auto result = std::stod (std::string (jsonValue ()->number, jsonValue ()->number_size), &ctrl);
	if (ctrl > 0)
		return result;
#else
	double result {0.};
	auto res = std::from_chars (jsonValue ()->number,
	                            jsonValue ()->number + jsonValue ()->number_size, result);
	if (res.ec == std::errc ())
		return result;
#endif
	return {};
}

//------------------------------------------------------------------------
inline std::string_view errorToString (json_parse_error_e error)
{
	switch (error)
	{
		case json_parse_error_e::json_parse_error_none: return {};
		case json_parse_error_e::json_parse_error_expected_comma_or_closing_bracket:
			return "json_parse_error_expected_comma_or_closing_bracket";
		case json_parse_error_e::json_parse_error_expected_colon:
			return "json_parse_error_expected_colon";
		case json_parse_error_e::json_parse_error_expected_opening_quote:
			return "json_parse_error_expected_opening_quote";
		case json_parse_error_e::json_parse_error_invalid_string_escape_sequence:
			return "json_parse_error_invalid_string_escape_sequence";
		case json_parse_error_e::json_parse_error_invalid_number_format:
			return "json_parse_error_invalid_number_format";
		case json_parse_error_e::json_parse_error_invalid_value:
			return "json_parse_error_invalid_value";
		case json_parse_error_e::json_parse_error_premature_end_of_buffer:
			return "json_parse_error_premature_end_of_buffer";
		case json_parse_error_e::json_parse_error_invalid_string:
			return "json_parse_error_invalid_string";
		case json_parse_error_e::json_parse_error_allocator_failed:
			return "json_parse_error_allocator_failed";
		case json_parse_error_e::json_parse_error_unexpected_trailing_characters:
			return "json_parse_error_unexpected_trailing_characters";
		case json_parse_error_e::json_parse_error_unknown: return "json_parse_error_unknown";
	}
	return {};
}

//------------------------------------------------------------------------
} // JSON
