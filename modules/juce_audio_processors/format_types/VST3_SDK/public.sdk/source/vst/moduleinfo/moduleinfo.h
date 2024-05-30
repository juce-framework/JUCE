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
