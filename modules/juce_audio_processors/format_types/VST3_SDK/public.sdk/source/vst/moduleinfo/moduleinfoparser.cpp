//-----------------------------------------------------------------------------
// Project     : VST SDK
// Flags       : clang-format SMTGSequencer
//
// Category    : moduleinfo
// Filename    : public.sdk/source/vst/moduleinfo/moduleinfoparser.cpp
// Created by  : Steinberg, 01/2022
// Description : utility functions to parse moduleinfo json files
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
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "moduleinfoparser.h"
#include "jsoncxx.h"
#include "pluginterfaces/base/ipluginbase.h"
#include <limits>
#include <stdexcept>

//------------------------------------------------------------------------
namespace Steinberg::ModuleInfoLib {
namespace {

//------------------------------------------------------------------------
void printJsonParseError (json_parse_result_s& parseResult, std::ostream& errorOut)
{
	errorOut << "error  : "
	         << JSON::errorToString (static_cast<json_parse_error_e> (parseResult.error)) << '\n';
	errorOut << "offset : " << parseResult.error_offset << '\n';
	errorOut << "line no: " << parseResult.error_line_no << '\n';
	errorOut << "row no : " << parseResult.error_row_no << '\n';
}

//------------------------------------------------------------------------
struct parse_error : std::exception
{
	parse_error (const std::string& str, const JSON::Value& value)
	: str (str), location (value.getSourceLocation ())
	{
		addLocation (location);
	}
	parse_error (const std::string& str, const JSON::String& value)
	: str (str), location (value.getSourceLocation ())
	{
		addLocation (location);
	}
	const char* what () const noexcept override { return str.data (); }

private:
	void addLocation (const JSON::SourceLocation& loc)
	{
		str += '\n';
		str += "offset:";
		str += std::to_string (loc.offset);
		str += '\n';
		str += "line:";
		str += std::to_string (loc.line);
		str += '\n';
		str += "row:";
		str += std::to_string (loc.row);
		str += '\n';
	}

	std::string str;
	JSON::SourceLocation location;
};

//------------------------------------------------------------------------
struct ModuleInfoJsonParser
{
	ModuleInfoJsonParser () = default;

	std::string_view getText (const JSON::Value& value) const
	{
		if (auto str = value.asString ())
			return str->text ();
		throw parse_error ("Expect a String here", value);
	}

	template <typename T>
	T getInteger (const JSON::Value& value) const
	{
		if (auto number = value.asNumber ())
		{
			if (auto result = number->getInteger ())
			{
				if (result > static_cast<int64_t> (std::numeric_limits<T>::max ()) ||
				    result < static_cast<int64_t> (std::numeric_limits<T>::min ()))
					throw parse_error ("Value is out of range here", value);
				return static_cast<T> (*result);
			}
			throw parse_error ("Expect an Integer here", value);
		}
		throw parse_error ("Expect a Number here", value);
	}

	double getDouble (const JSON::Value& value) const
	{
		if (auto number = value.asNumber ())
		{
			if (auto result = number->getDouble ())
				return *result;
			throw parse_error ("Expect a Double here", value);
		}
		throw parse_error ("Expect a Number here", value);
	}

	void parseFactoryInfo (const JSON::Value& value)
	{
		enum ParsedBits
		{
			Vendor = 1 << 0,
			URL = 1 << 1,
			EMail = 1 << 2,
			Flags = 1 << 3,
		};
		uint32_t parsed {0};
		if (auto obj = value.asObject ())
		{
			for (const auto& el : *obj)
			{
				auto elementName = el.name ().text ();
				if (elementName == "Vendor")
				{
					if (parsed & ParsedBits::Vendor)
						throw parse_error ("Only one 'Vendor' key allowed", el.name ());
					parsed |= ParsedBits::Vendor;
					info.factoryInfo.vendor = getText (el.value ());
				}
				else if (elementName == "URL")
				{
					if (parsed & ParsedBits::URL)
						throw parse_error ("Only one 'URL' key allowed", el.name ());
					parsed |= ParsedBits::URL;
					info.factoryInfo.url = getText (el.value ());
				}
				else if (elementName == "E-Mail")
				{
					if (parsed & ParsedBits::EMail)
						throw parse_error ("Only one 'E-Mail' key allowed", el.name ());
					parsed |= ParsedBits::EMail;
					info.factoryInfo.email = getText (el.value ());
				}
				else if (elementName == "Flags")
				{
					if (parsed & ParsedBits::Flags)
						throw parse_error ("Only one 'Flags' key allowed", el.name ());
					auto flags = el.value ().asObject ();
					if (!flags)
						throw parse_error ("Expect 'Flags' to be a JSON Object", el.name ());
					for (const auto& flag : *flags)
					{
						auto flagName = flag.name ().text ();
						auto flagValue = flag.value ().asBoolean ();
						if (!flagValue)
							throw parse_error ("Flag must be a boolean", flag.value ());
						if (flagName == "Classes Discardable")
						{
							if (*flagValue)
								info.factoryInfo.flags |= PFactoryInfo::kClassesDiscardable;
						}
						else if (flagName == "Component Non Discardable")
						{
							if (*flagValue)
								info.factoryInfo.flags |= PFactoryInfo::kComponentNonDiscardable;
						}
						else if (flagName == "Unicode")
						{
							if (*flagValue)
								info.factoryInfo.flags |= PFactoryInfo::kUnicode;
						}
						else
							throw parse_error ("Unknown flag", flag.name ());
					}
					parsed |= ParsedBits::Flags;
				}
			}
		}
		if (!(parsed & ParsedBits::Vendor))
			throw std::logic_error ("Missing 'Vendor' in Factory Info");
		if (!(parsed & ParsedBits::URL))
			throw std::logic_error ("Missing 'URL' in Factory Info");
		if (!(parsed & ParsedBits::EMail))
			throw std::logic_error ("Missing 'EMail' in Factory Info");
		if (!(parsed & ParsedBits::Flags))
			throw std::logic_error ("Missing 'Flags' in Factory Info");
	}

	void parseClasses (const JSON::Value& value)
	{
		enum ParsedBits
		{
			CID = 1 << 0,
			Category = 1 << 1,
			Name = 1 << 2,
			Vendor = 1 << 3,
			Version = 1 << 4,
			SDKVersion = 1 << 5,
			SubCategories = 1 << 6,
			ClassFlags = 1 << 7,
			Snapshots = 1 << 8,
			Cardinality = 1 << 9,
		};

		auto array = value.asArray ();
		if (!array)
			throw parse_error ("Expect Classes Array", value);
		for (const auto& classInfoEl : *array)
		{
			auto classInfo = classInfoEl.value ().asObject ();
			if (!classInfo)
				throw parse_error ("Expect Class Object", classInfoEl.value ());

			ModuleInfo::ClassInfo ci {};

			uint32_t parsed {0};

			for (const auto& el : *classInfo)
			{
				auto elementName = el.name ().text ();
				if (elementName == "CID")
				{
					if (parsed & ParsedBits::CID)
						throw parse_error ("Only one 'CID' key allowed", el.name ());
					ci.cid = getText (el.value ());
					parsed |= ParsedBits::CID;
				}
				else if (elementName == "Category")
				{
					if (parsed & ParsedBits::Category)
						throw parse_error ("Only one 'Category' key allowed", el.name ());
					ci.category = getText (el.value ());
					parsed |= ParsedBits::Category;
				}
				else if (elementName == "Name")
				{
					if (parsed & ParsedBits::Name)
						throw parse_error ("Only one 'Name' key allowed", el.name ());
					ci.name = getText (el.value ());
					parsed |= ParsedBits::Name;
				}
				else if (elementName == "Vendor")
				{
					if (parsed & ParsedBits::Vendor)
						throw parse_error ("Only one 'Vendor' key allowed", el.name ());
					ci.vendor = getText (el.value ());
					parsed |= ParsedBits::Vendor;
				}
				else if (elementName == "Version")
				{
					if (parsed & ParsedBits::Version)
						throw parse_error ("Only one 'Version' key allowed", el.name ());
					ci.version = getText (el.value ());
					parsed |= ParsedBits::Version;
				}
				else if (elementName == "SDKVersion")
				{
					if (parsed & ParsedBits::SDKVersion)
						throw parse_error ("Only one 'SDKVersion' key allowed", el.name ());
					ci.sdkVersion = getText (el.value ());
					parsed |= ParsedBits::SDKVersion;
				}
				else if (elementName == "Sub Categories")
				{
					if (parsed & ParsedBits::SubCategories)
						throw parse_error ("Only one 'Sub Categories' key allowed", el.name ());
					auto subCatArr = el.value ().asArray ();
					if (!subCatArr)
						throw parse_error ("Expect Array here", el.value ());
					for (const auto& catEl : *subCatArr)
					{
						auto cat = getText (catEl.value ());
						ci.subCategories.emplace_back (cat);
					}
					parsed |= ParsedBits::SubCategories;
				}
				else if (elementName == "Class Flags")
				{
					if (parsed & ParsedBits::ClassFlags)
						throw parse_error ("Only one 'Class Flags' key allowed", el.name ());
					ci.flags = getInteger<uint32_t> (el.value ());
					parsed |= ParsedBits::ClassFlags;
				}
				else if (elementName == "Cardinality")
				{
					if (parsed & ParsedBits::Cardinality)
						throw parse_error ("Only one 'Cardinality' key allowed", el.name ());
					ci.cardinality = getInteger<int32_t> (el.value ());
					parsed |= ParsedBits::Cardinality;
				}
				else if (elementName == "Snapshots")
				{
					if (parsed & ParsedBits::Snapshots)
						throw parse_error ("Only one 'Snapshots' key allowed", el.name ());
					auto snapArr = el.value ().asArray ();
					if (!snapArr)
						throw parse_error ("Expect Array here", el.value ());
					for (const auto& snapEl : *snapArr)
					{
						auto snap = snapEl.value ().asObject ();
						if (!snap)
							throw parse_error ("Expect Object here", snapEl.value ());
						ModuleInfo::Snapshot snapshot;
						for (const auto& spEl : *snap)
						{
							auto spElName = spEl.name ().text ();
							if (spElName == "Path")
								snapshot.path = getText (spEl.value ());
							else if (spElName == "Scale Factor")
								snapshot.scaleFactor = getDouble (spEl.value ());
							else
								throw parse_error ("Unexpected key", spEl.name ());
						}
						if (snapshot.scaleFactor == 0. || snapshot.path.empty ())
							throw parse_error ("Missing Snapshot keys", snapEl.value ());
						ci.snapshots.emplace_back (std::move (snapshot));
					}
					parsed |= ParsedBits::Snapshots;
				}
				else
					throw parse_error ("Unexpected key", el.name ());
			}
			if (!(parsed & ParsedBits::CID))
				throw parse_error ("'CID' key missing", classInfoEl.value ());
			if (!(parsed & ParsedBits::Category))
				throw parse_error ("'Category' key missing", classInfoEl.value ());
			if (!(parsed & ParsedBits::Name))
				throw parse_error ("'Name' key missing", classInfoEl.value ());
			if (!(parsed & ParsedBits::Vendor))
				throw parse_error ("'Vendor' key missing", classInfoEl.value ());
			if (!(parsed & ParsedBits::Version))
				throw parse_error ("'Version' key missing", classInfoEl.value ());
			if (!(parsed & ParsedBits::SDKVersion))
				throw parse_error ("'SDK Version' key missing", classInfoEl.value ());
			if (!(parsed & ParsedBits::ClassFlags))
				throw parse_error ("'Class Flags' key missing", classInfoEl.value ());
			if (!(parsed & ParsedBits::Cardinality))
				throw parse_error ("'Cardinality' key missing", classInfoEl.value ());
			info.classes.emplace_back (std::move (ci));
		}
	}

	void parseCompatibility (const JSON::Value& value)
	{
		auto arr = value.asArray ();
		if (!arr)
			throw parse_error ("Expect Array here", value);
		for (const auto& el : *arr)
		{
			auto obj = el.value ().asObject ();
			if (!obj)
				throw parse_error ("Expect Object here", el.value ());

			ModuleInfo::Compatibility compat;
			for (const auto& objEl : *obj)
			{
				auto elementName = objEl.name ().text ();
				if (elementName == "New")
					compat.newCID = getText (objEl.value ());
				else if (elementName == "Old")
				{
					auto oldElArr = objEl.value ().asArray ();
					if (!oldElArr)
						throw parse_error ("Expect Array here", objEl.value ());
					for (const auto& old : *oldElArr)
					{
						compat.oldCID.emplace_back (getText (old.value ()));
					}
				}
			}
			if (compat.newCID.empty ())
				throw parse_error ("Expect New CID here", el.value ());
			if (compat.oldCID.empty ())
				throw parse_error ("Expect Old CID here", el.value ());
			info.compatibility.emplace_back (std::move (compat));
		}
	}

	void parse (const JSON::Document& doc)
	{
		auto docObj = doc.asObject ();
		if (!docObj)
			throw parse_error ("Unexpected", doc);

		enum ParsedBits
		{
			Name = 1 << 0,
			Version = 1 << 1,
			FactoryInfo = 1 << 2,
			Compatibility = 1 << 3,
			Classes = 1 << 4,
		};

		uint32_t parsed {0};
		for (const auto& el : *docObj)
		{
			auto elementName = el.name ().text ();
			if (elementName == "Name")
			{
				if (parsed & ParsedBits::Name)
					throw parse_error ("Only one 'Name' key allowed", el.name ());
				parsed |= ParsedBits::Name;
				info.name = getText (el.value ());
			}
			else if (elementName == "Version")
			{
				if (parsed & ParsedBits::Version)
					throw parse_error ("Only one 'Version' key allowed", el.name ());
				parsed |= ParsedBits::Version;
				info.version = getText (el.value ());
			}
			else if (elementName == "Factory Info")
			{
				if (parsed & ParsedBits::FactoryInfo)
					throw parse_error ("Only one 'Factory Info' key allowed", el.name ());
				parseFactoryInfo (el.value ());
				parsed |= ParsedBits::FactoryInfo;
			}
			else if (elementName == "Compatibility")
			{
				if (parsed & ParsedBits::Compatibility)
					throw parse_error ("Only one 'Compatibility' key allowed", el.name ());
				parseCompatibility (el.value ());
				parsed |= ParsedBits::Compatibility;
			}
			else if (elementName == "Classes")
			{
				if (parsed & ParsedBits::Classes)
					throw parse_error ("Only one 'Classes' key allowed", el.name ());
				parseClasses (el.value ());
				parsed |= ParsedBits::Classes;
			}
			else
			{
				throw parse_error ("Unexpected JSON Token", el.name ());
			}
		}
		if (!(parsed & ParsedBits::Name))
			throw std::logic_error ("'Name' key missing");
		if (!(parsed & ParsedBits::Version))
			throw std::logic_error ("'Version' key missing");
		if (!(parsed & ParsedBits::FactoryInfo))
			throw std::logic_error ("'Factory Info' key missing");
		if (!(parsed & ParsedBits::Classes))
			throw std::logic_error ("'Classes' key missing");
	}

	ModuleInfo&& takeInfo () { return std::move (info); }

private:
	ModuleInfo info;
};

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
std::optional<ModuleInfo> parseJson (std::string_view jsonData, std::ostream* optErrorOutput)
{
	auto docVar = JSON::Document::parse (jsonData);
	if (auto res = std::get_if<json_parse_result_s> (&docVar))
	{
		if (optErrorOutput)
			printJsonParseError (*res, *optErrorOutput);
		return {};
	}
	auto doc = std::get_if<JSON::Document> (&docVar);
	assert (doc);
	try
	{
		ModuleInfoJsonParser parser;
		parser.parse (*doc);
		return parser.takeInfo ();
	}
	catch (std::exception& error)
	{
		if (optErrorOutput)
			*optErrorOutput << error.what () << '\n';
		return {};
	}
	// unreachable
}

//------------------------------------------------------------------------
std::optional<ModuleInfo::CompatibilityList> parseCompatibilityJson (std::string_view jsonData,
                                                                     std::ostream* optErrorOutput)
{
	auto docVar = JSON::Document::parse (jsonData);
	if (auto res = std::get_if<json_parse_result_s> (&docVar))
	{
		if (optErrorOutput)
			printJsonParseError (*res, *optErrorOutput);
		return {};
	}
	auto doc = std::get_if<JSON::Document> (&docVar);
	assert (doc);
	try
	{
		ModuleInfoJsonParser parser;
		parser.parseCompatibility (*doc);
		return parser.takeInfo ().compatibility;
	}
	catch (std::exception& error)
	{
		if (optErrorOutput)
			*optErrorOutput << error.what () << '\n';
		return {};
	}
	// unreachable
}

//------------------------------------------------------------------------
} // Steinberg::ModuelInfoLib
