//-----------------------------------------------------------------------------
// Project     : VST SDK
// Flags       : clang-format SMTGSequencer
//
// Category    : moduleinfo
// Filename    : public.sdk/source/vst/moduleinfo/moduleinfocreator.cpp
// Created by  : Steinberg, 12/2021
// Description : utility functions to create moduleinfo json files
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

#include "moduleinfocreator.h"
#include "jsoncxx.h"
#include <algorithm>
#include <stdexcept>
#include <string>

//------------------------------------------------------------------------
namespace Steinberg::ModuleInfoLib {
using namespace VST3;
namespace {

//------------------------------------------------------------------------
struct JSON5Writer
{
private:
	std::ostream& stream;
	bool beautify;
	bool lastIsComma {false};
	int32_t intend {0};

	void doBeautify ()
	{
		if (beautify)
		{
			stream << '\n';
			for (int i = 0; i < intend; ++i)
				stream << "  ";
		}
	}

	void writeComma ()
	{
		if (lastIsComma)
			return;
		stream << ",";
		lastIsComma = true;
	}
	void startObject ()
	{
		stream << "{";
		++intend;
		lastIsComma = false;
	}
	void endObject ()
	{
		--intend;
		doBeautify ();
		stream << "}";
		lastIsComma = false;
	}
	void startArray ()
	{
		stream << "[";
		++intend;
		lastIsComma = false;
	}
	void endArray ()
	{
		--intend;
		doBeautify ();
		stream << "]";
		lastIsComma = false;
	}

public:
	JSON5Writer (std::ostream& stream, bool beautify = true) : stream (stream), beautify (beautify)
	{
	}

	void string (std::string_view str)
	{
		stream << "\"" << str << "\"";
		lastIsComma = false;
	}

	void boolean (bool val)
	{
		stream << (val ? "true" : "false");
		lastIsComma = false;
	}

	template <typename ValueT>
	void value (ValueT val)
	{
		stream << val;
		lastIsComma = false;
	}

	template <typename Proc>
	void object (Proc proc)
	{
		startObject ();
		proc ();
		endObject ();
	}

	template <typename Iterator, typename Proc>
	void array (Iterator begin, Iterator end, Proc proc)
	{
		startArray ();
		while (begin != end)
		{
			doBeautify ();
			proc (begin);
			++begin;
			writeComma ();
		}
		endArray ();
	}

	template <typename Proc>
	void keyValue (std::string_view key, Proc proc)
	{
		doBeautify ();
		string (key);
		stream << ": ";
		proc ();
		writeComma ();
	}
};

//------------------------------------------------------------------------
void writeSnapshots (const ModuleInfo::SnapshotList& snapshots, JSON5Writer& w)
{
	w.keyValue ("Snapshots", [&] () {
		w.array (snapshots.begin (), snapshots.end (), [&] (const auto& el) {
			w.object ([&] () {
				w.keyValue ("Scale Factor", [&] () { w.value (el->scaleFactor); });
				w.keyValue ("Path", [&] () { w.string (el->path); });
			});
		});
	});
}

//------------------------------------------------------------------------
void writeClassInfo (const ModuleInfo::ClassInfo& cls, JSON5Writer& w)
{
	w.keyValue ("CID", [&] () { w.string (cls.cid); });
	w.keyValue ("Category", [&] () { w.string (cls.category); });
	w.keyValue ("Name", [&] () { w.string (cls.name); });
	w.keyValue ("Vendor", [&] () { w.string (cls.vendor); });
	w.keyValue ("Version", [&] () { w.string (cls.version); });
	w.keyValue ("SDKVersion", [&] () { w.string (cls.sdkVersion); });
	const auto& sc = cls.subCategories;
	if (!sc.empty ())
	{
		w.keyValue ("Sub Categories", [&] () {
			w.array (sc.begin (), sc.end (), [&] (const auto& cat) { w.string (*cat); });
		});
	}
	w.keyValue ("Class Flags", [&] () { w.value (cls.flags); });
	w.keyValue ("Cardinality", [&] () { w.value (cls.cardinality); });
	writeSnapshots (cls.snapshots, w);
}

//------------------------------------------------------------------------
void writePluginCompatibility (const ModuleInfo::CompatibilityList& compat, JSON5Writer& w)
{
	if (compat.empty ())
		return;
	w.keyValue ("Compatibility", [&] () {
		w.array (compat.begin (), compat.end (), [&] (auto& el) {
			w.object ([&] () {
				w.keyValue ("New", [&] () { w.string (el->newCID); });
				w.keyValue ("Old", [&] () {
					w.array (el->oldCID.begin (), el->oldCID.end (),
					         [&] (auto& oldEl) { w.string (*oldEl); });
				});
			});
		});
	});
}

//------------------------------------------------------------------------
void writeFactoryInfo (const ModuleInfo::FactoryInfo& fi, JSON5Writer& w)
{
	w.keyValue ("Factory Info", [&] () {
		w.object ([&] () {
			w.keyValue ("Vendor", [&] () { w.string (fi.vendor); });
			w.keyValue ("URL", [&] () { w.string (fi.url); });
			w.keyValue ("E-Mail", [&] () { w.string (fi.email); });
			w.keyValue ("Flags", [&] () {
				w.object ([&] () {
					w.keyValue ("Unicode",
					            [&] () { w.boolean (fi.flags & PFactoryInfo::kUnicode); });
					w.keyValue ("Classes Discardable", [&] () {
						w.boolean (fi.flags & PFactoryInfo::kClassesDiscardable);
					});
					w.keyValue ("Component Non Discardable", [&] () {
						w.boolean (fi.flags & PFactoryInfo::kComponentNonDiscardable);
					});
				});
			});
		});
	});
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
ModuleInfo createModuleInfo (const VST3::Hosting::Module& module, bool includeDiscardableClasses)
{
	ModuleInfo info;

	const auto& factory = module.getFactory ();
	auto factoryInfo = factory.info ();

	info.name = module.getName ();
	auto pos = info.name.find_last_of ('.');
	if (pos != std::string::npos)
		info.name.erase (pos);

	info.factoryInfo.vendor = factoryInfo.vendor ();
	info.factoryInfo.url = factoryInfo.url ();
	info.factoryInfo.email = factoryInfo.email ();
	info.factoryInfo.flags = factoryInfo.flags ();

	if (factoryInfo.classesDiscardable () == false ||
	    (factoryInfo.classesDiscardable () && includeDiscardableClasses))
	{
		auto snapshots = VST3::Hosting::Module::getSnapshots (module.getPath ());
		for (const auto& ci : factory.classInfos ())
		{
			ModuleInfo::ClassInfo classInfo;
			classInfo.cid = ci.ID ().toString ();
			classInfo.category = ci.category ();
			classInfo.name = ci.name ();
			classInfo.vendor = ci.vendor ();
			classInfo.version = ci.version ();
			classInfo.sdkVersion = ci.sdkVersion ();
			classInfo.subCategories = ci.subCategories ();
			classInfo.cardinality = ci.cardinality ();
			classInfo.flags = ci.classFlags ();
			auto snapshotIt = std::find_if (snapshots.begin (), snapshots.end (),
			                                [&] (const auto& el) { return el.uid == ci.ID (); });
			if (snapshotIt != snapshots.end ())
			{
				for (auto& s : snapshotIt->images)
				{
					std::string_view path (s.path);
					if (path.find (module.getPath ()) == 0)
						path.remove_prefix (module.getPath ().size () + 1);
					classInfo.snapshots.emplace_back (
					    ModuleInfo::Snapshot {s.scaleFactor, {path.data (), path.size ()}});
				}
				snapshots.erase (snapshotIt);
			}
			info.classes.emplace_back (std::move (classInfo));
		}
	}
	return info;
}

//------------------------------------------------------------------------
void outputJson (const ModuleInfo& info, std::ostream& output)
{
	JSON5Writer w (output);
	w.object ([&] () {
		w.keyValue ("Name", [&] () { w.string (info.name); });
		w.keyValue ("Version", [&] () { w.string (info.version); });
		writeFactoryInfo (info.factoryInfo, w);
		writePluginCompatibility (info.compatibility, w);
		w.keyValue ("Classes", [&] () {
			w.array (info.classes.begin (), info.classes.end (),
			         [&] (const auto& cls) { w.object ([&] () { writeClassInfo (*cls, w); }); });
		});
	});
}

//------------------------------------------------------------------------
} // Steinberg::ModuleInfoLib
