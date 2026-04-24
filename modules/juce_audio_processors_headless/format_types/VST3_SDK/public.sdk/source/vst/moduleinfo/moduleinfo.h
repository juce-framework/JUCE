//-----------------------------------------------------------------------------
// Project     : VST SDK
// Flags       : clang-format SMTGSequencer
//
// Category    : moduleinfo
// Filename    : public.sdk/source/vst/moduleinfo/moduleinfo.h
// Created by  : Steinberg, 12/2021
// Description :
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>

//------------------------------------------------------------------------
namespace Steinberg {

//------------------------------------------------------------------------
struct ModuleInfo
{
//------------------------------------------------------------------------
	struct FactoryInfo
	{
		std::string vendor;
		std::string url;
		std::string email;
		int32_t flags {0};
	};

//------------------------------------------------------------------------
	struct Snapshot
	{
		double scaleFactor {1.};
		std::string path;
	};
	using SnapshotList = std::vector<Snapshot>;

//------------------------------------------------------------------------
	struct ClassInfo
	{
		std::string cid;
		std::string category;
		std::string name;
		std::string vendor;
		std::string version;
		std::string sdkVersion;
		std::vector<std::string> subCategories;
		SnapshotList snapshots;
		int32_t cardinality {0x7FFFFFFF};
		uint32_t flags {0};
	};

//------------------------------------------------------------------------
	struct Compatibility
	{
		std::string newCID;
		std::vector<std::string> oldCID;
	};

	using ClassList = std::vector<ClassInfo>;
	using CompatibilityList = std::vector<Compatibility>;

	std::string name;
	std::string version;
	FactoryInfo factoryInfo;
	ClassList classes;
	CompatibilityList compatibility;
};

//------------------------------------------------------------------------
} // Steinberg
