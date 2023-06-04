//-----------------------------------------------------------------------------
// Project     : VST SDK
// Flags       : clang-format SMTGSequencer
//
// Category    : moduleinfotool
// Filename    : public.sdk/samples/vst-utilities/moduleinfotool/main.cpp
// Created by  : Steinberg, 12/2021
// Description : main entry point
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

#include "base/source/fcommandline.h"
#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/base/iplugincompatibility.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/common/readfile.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/moduleinfo/moduleinfocreator.h"
#include "public.sdk/source/vst/moduleinfo/moduleinfoparser.h"
#include "public.sdk/source/vst/utility/stringconvert.h"
#include <cstdio>
#include <fstream>
#include <iostream>

//------------------------------------------------------------------------
namespace Steinberg {
namespace ModuleInfoTool {
namespace {

//------------------------------------------------------------------------
constexpr auto BUILD_INFO = "moduleinfotool 1.0.0 [Built " __DATE__ "]";

//------------------------------------------------------------------------
//-- Options
constexpr auto optHelp = "help";
constexpr auto optCreate = "create";
constexpr auto optValidate = "validate";
constexpr auto optModuleVersion = "version";
constexpr auto optModulePath = "path";
constexpr auto optInfoPath = "infopath";
constexpr auto optModuleCompatPath = "compat";
constexpr auto optOutputPath = "output";

//------------------------------------------------------------------------
void printUsage (std::ostream& s)
{
	s << "Usage:\n";
	s << "  moduleinfotool -create -version VERSION -path MODULE_PATH [-compat PATH -output PATH]\n";
	s << "  moduleinfotool -validate -path MODULE_PATH [-infopath PATH]\n";
}

//------------------------------------------------------------------------
std::optional<ModuleInfo::CompatibilityList> openAndParseCompatJSON (const std::string& path)
{
	auto data = readFile (path);
	if (data.empty ())
	{
		std::cout << "Can not read '" << path << "'\n";
		printUsage (std::cout);
		return {};
	}
	std::stringstream error;
	auto result = ModuleInfoLib::parseCompatibilityJson (data, &error);
	if (!result)
	{
		std::cout << "Can not parse '" << path << "'\n";
		std::cout << error.str ();
		printUsage (std::cout);
		return {};
	}
	return result;
}

//------------------------------------------------------------------------
std::optional<ModuleInfo::CompatibilityList> loadCompatibilityFromModule (const VST3::Hosting::Module& module)
{
	const auto& factory = module.getFactory();
	const auto& infos = factory.classInfos();

	const auto iter = std::find_if (infos.begin(), infos.end(), [&] (const auto& info)
	{
		return info.category() == kPluginCompatibilityClass;
	});

	if (iter == infos.end())
		return {};

	const auto compatibility = factory.createInstance<Steinberg::IPluginCompatibility> (iter->ID());

	if (compatibility == nullptr)
		return {};

	Steinberg::MemoryStream stream;

	if (compatibility->getCompatibilityJSON (&stream) != kResultOk)
		return {};

	const std::string_view streamView (stream.getData(), stream.getSize());

	return ModuleInfoLib::parseCompatibilityJson (streamView, nullptr);
}

//------------------------------------------------------------------------
int createJSON (const std::optional<ModuleInfo::CompatibilityList>& compat,
                const std::string& modulePath, const std::string& moduleVersion,
                std::ostream& outStream)
{
	std::string errorStr;
	auto module = VST3::Hosting::Module::create (modulePath, errorStr);
	if (!module)
	{
		std::cout << errorStr;
		return 1;
	}
	auto moduleInfo = ModuleInfoLib::createModuleInfo (*module, false);
	if (compat)
		moduleInfo.compatibility = *compat;
	else if (auto loaded = loadCompatibilityFromModule (*module))
		moduleInfo.compatibility = *loaded;

	moduleInfo.version = moduleVersion;

	std::stringstream output;
	ModuleInfoLib::outputJson (moduleInfo, output);
	auto str = output.str ();
	outStream << str;
	return 0;
}

//------------------------------------------------------------------------
struct validate_error : std::exception
{
	validate_error (const std::string& str) : str (str) {}
	const char* what () const noexcept override { return str.data (); }

private:
	std::string str;
};

//------------------------------------------------------------------------
void validate (const ModuleInfo& moduleInfo, const VST3::Hosting::Module& module)
{
	auto factory = module.getFactory ();
	auto factoryInfo = factory.info ();
	auto classInfoList = factory.classInfos ();
	auto snapshotList = module.getSnapshots (module.getPath ());

	if (factoryInfo.vendor () != moduleInfo.factoryInfo.vendor)
		throw validate_error ("factoryInfo.vendor different: " + moduleInfo.factoryInfo.vendor);
	if (factoryInfo.url () != moduleInfo.factoryInfo.url)
		throw validate_error ("factoryInfo.url different: " + moduleInfo.factoryInfo.url);
	if (factoryInfo.email () != moduleInfo.factoryInfo.email)
		throw validate_error ("factoryInfo.email different: " + moduleInfo.factoryInfo.email);
	if (factoryInfo.flags () != moduleInfo.factoryInfo.flags)
		throw validate_error ("factoryInfo.flags different: " +
		                      std::to_string (moduleInfo.factoryInfo.flags));

	for (const auto& ci : moduleInfo.classes)
	{
		auto cid = VST3::UID::fromString (ci.cid);
		if (!cid)
			throw validate_error ("could not parse class UID: " + ci.cid);
		auto it = std::find_if (classInfoList.begin (), classInfoList.end (),
		                        [&] (const auto& el) { return el.ID () == *cid; });
		if (it == classInfoList.end ())
			throw validate_error ("cannot find CID in class list: " + ci.cid);
		if (it->name () != ci.name)
			throw validate_error ("class name different: " + ci.name);
		if (it->category () != ci.category)
			throw validate_error ("class category different: " + ci.category);
		if (it->vendor () != ci.vendor)
			throw validate_error ("class vendor different: " + ci.vendor);
		if (it->version () != ci.version)
			throw validate_error ("class version different: " + ci.version);
		if (it->sdkVersion () != ci.sdkVersion)
			throw validate_error ("class sdkVersion different: " + ci.sdkVersion);
		if (it->subCategories () != ci.subCategories)
			throw validate_error ("class subCategories different: " /* + ci.subCategories*/);
		if (it->cardinality () != ci.cardinality)
			throw validate_error ("class cardinality different: " +
			                      std::to_string (ci.cardinality));
		if (it->classFlags () != ci.flags)
			throw validate_error ("class flags different: " + std::to_string (ci.flags));
		classInfoList.erase (it);

		auto snapshotListIt = std::find_if (snapshotList.begin (), snapshotList.end (),
		                                    [&] (const auto& el) { return el.uid == *cid; });
		if (snapshotListIt == snapshotList.end () && !ci.snapshots.empty ())
			throw validate_error ("cannot find snapshots for: " + ci.cid);
		for (const auto& snapshot : ci.snapshots)
		{
			auto snapshotIt = std::find_if (
			    snapshotListIt->images.begin (), snapshotListIt->images.end (),
			    [&] (const auto& el) { return el.scaleFactor == snapshot.scaleFactor; });
			if (snapshotIt == snapshotListIt->images.end ())
				throw validate_error ("cannot find snapshots for scale factor: " +
				                      std::to_string (snapshot.scaleFactor));
			std::string_view path (snapshotIt->path);
			if (path.find (module.getPath ()) == 0)
				path.remove_prefix (module.getPath ().size () + 1);
			if (path != snapshot.path)
				throw validate_error ("cannot find snapshots with path: " + snapshot.path);
			snapshotListIt->images.erase (snapshotIt);
		}
		if (snapshotListIt != snapshotList.end () && !snapshotListIt->images.empty ())
		{
			std::string errorStr = "Missing Snapshots in moduleinfo:\n";
			for (const auto& s : snapshotListIt->images)
			{
				errorStr += s.path + '\n';
			}
			throw validate_error (errorStr);
		}
		if (snapshotListIt != snapshotList.end ())
			snapshotList.erase (snapshotListIt);
	}
	if (!classInfoList.empty ())
		throw validate_error ("Missing classes in moduleinfo");
	if (!snapshotList.empty ())
		throw validate_error ("Missing snapshots in moduleinfo");
}

//------------------------------------------------------------------------
int validate (const std::string& modulePath, std::string infoJsonPath)
{
	if (infoJsonPath.empty ())
	{
		auto path = VST3::Hosting::Module::getModuleInfoPath (modulePath);
		if (!path)
		{
			std::cerr << "Module does not contain a moduleinfo.json: '" << modulePath << "'"
			          << '\n';
			return 1;
		}
		infoJsonPath = *path;
	}

	auto data = readFile (infoJsonPath);
	if (data.empty ())
	{
		std::cerr << "Empty or non existing file: '" << infoJsonPath << "'" << '\n';
		printUsage (std::cout);
		return 1;
	}
	auto moduleInfo = ModuleInfoLib::parseJson (data, &std::cerr);
	if (moduleInfo)
	{
		std::string errorStr;
		auto module = VST3::Hosting::Module::create (modulePath, errorStr);
		if (!module)
		{
			std::cerr << errorStr;
			printUsage (std::cout);
			return 1;
		}
		try
		{
			validate (*moduleInfo, *module);
		}
		catch (const std::exception& exc)
		{
			std::cerr << "Error:\n" << exc.what () << '\n';
			printUsage (std::cout);
			return 1;
		}
		return 0;
	}
	printUsage (std::cout);
	return 1;
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
int run (int argc, char* argv[])
{
	// parse command line
	CommandLine::Descriptions desc;
	CommandLine::VariablesMap valueMap;
	CommandLine::FilesVector files;

	using Description = CommandLine::Description;
	desc.addOptions (
	    BUILD_INFO,
	    {
	        {optCreate, "Create moduleinfo", Description::kBool},
	        {optValidate, "Validate moduleinfo", Description::kBool},
	        {optModuleVersion, "Module version", Description::kString},
	        {optModulePath, "Path to module", Description::kString},
	        {optInfoPath, "Path to moduleinfo.json", Description::kString},
	        {optModuleCompatPath, "Path to compatibility.json", Description::kString},
	        {optOutputPath, "Write json to file instead of stdout", Description::kString},
	        {optHelp, "Print help", Description::kBool},
	    });
	CommandLine::parse (argc, argv, desc, valueMap, &files);

	bool isCreate = valueMap.count (optCreate) != 0 && valueMap.count (optModuleVersion) != 0 &&
	                valueMap.count (optModulePath) != 0;
	bool isValidate = valueMap.count (optValidate) && valueMap.count (optModulePath) != 0;

	if (valueMap.hasError () || valueMap.count (optHelp) || !(isCreate || isValidate))
	{
		std::cout << '\n' << desc << '\n';
		printUsage (std::cout);
		return 1;
	}

	int result = 1;

	const auto& modulePath = valueMap[optModulePath];
	if (isCreate)
	{
		auto* outputStream = &std::cout;
		std::optional<ModuleInfo::CompatibilityList> compat;
		if (valueMap.count (optModuleCompatPath) != 0)
		{
			const auto& compatPath = valueMap[optModuleCompatPath];
			compat = openAndParseCompatJSON (compatPath);
			if (!compat)
				return 1;
		}
		bool writeToFile = false;
		if (valueMap.count (optOutputPath) != 0)
		{
			writeToFile = true;
#if SMTG_OS_WINDOWS
			auto tmp = VST3::StringConvert::convert (valueMap[optOutputPath]);
			auto outputFile = reinterpret_cast<const wchar_t*> (tmp.data ());
#else
			auto outputFile = valueMap[optOutputPath];
#endif
			auto ostream = new std::ofstream (outputFile);
			
			if (ostream->is_open ())
				outputStream = ostream;
			else
			{
				std::cout << "Cannot create output file: " << valueMap[optOutputPath] << '\n';
				return result;
			}
		}
		const auto& moduleVersion = valueMap[optModuleVersion];
		result = createJSON (compat, modulePath, moduleVersion, *outputStream);
		if (writeToFile)
			delete outputStream;
	}
	else if (isValidate)
	{
		std::string moduleInfoJsonPath;
		if (valueMap.count (optInfoPath) != 0)
			moduleInfoJsonPath = valueMap[optInfoPath];
		result = validate (modulePath, moduleInfoJsonPath);
	}
	return result;
}

//------------------------------------------------------------------------
} // ModuleInfoTool
} // Steinberg

//------------------------------------------------------------------------
#if SMTG_OS_WINDOWS
//------------------------------------------------------------------------
#include <Windows.h>
#include <vector>

//------------------------------------------------------------------------
using Utf8String = std::string;

//------------------------------------------------------------------------
using Utf8Args = std::vector<Utf8String>;
Utf8Args toUtf8Args (int argc, wchar_t* wargv[])
{
	Utf8Args utf8Args;
	for (int i = 0; i < argc; i++)
	{
		auto str = reinterpret_cast<const Steinberg::Vst::TChar*>(wargv[i]);
		utf8Args.push_back (VST3::StringConvert::convert (str));
	}

	return utf8Args;
}

//------------------------------------------------------------------------
using Utf8ArgPtrs = std::vector<char*>;
Utf8ArgPtrs toUtf8ArgPtrs (Utf8Args& utf8Args)
{
	Utf8ArgPtrs utf8ArgPtrs;
	for (auto& el : utf8Args)
	{
		utf8ArgPtrs.push_back (el.data ());
	}

	return utf8ArgPtrs;
}

//------------------------------------------------------------------------
int wmain (int argc, wchar_t* wargv[])
{
	Utf8Args utf8Args = toUtf8Args (argc, wargv);
	Utf8ArgPtrs utf8ArgPtrs = toUtf8ArgPtrs (utf8Args);

	char** argv = &(utf8ArgPtrs.at (0));
	return Steinberg::ModuleInfoTool::run (argc, argv);
}
#else
int main (int argc, char* argv[])
{
	return Steinberg::ModuleInfoTool::run (argc, argv);
}
#endif
