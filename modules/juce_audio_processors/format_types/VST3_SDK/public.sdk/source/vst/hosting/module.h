//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/hosting/module.h
// Created by  : Steinberg, 08/2016
// Description : hosting module classes
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

#include "../utility/uid.h"
#include "pluginterfaces/base/ipluginbase.h"
#include <utility>
#include <vector>

//------------------------------------------------------------------------
namespace VST3 {
namespace Hosting {

//------------------------------------------------------------------------
class FactoryInfo
{
public:
//------------------------------------------------------------------------
	using PFactoryInfo = Steinberg::PFactoryInfo;

	FactoryInfo () noexcept {}
	~FactoryInfo () noexcept {}
	FactoryInfo (const FactoryInfo&) noexcept = default;
	FactoryInfo (PFactoryInfo&&) noexcept;
	FactoryInfo (FactoryInfo&&) noexcept = default;
	FactoryInfo& operator= (const FactoryInfo&) noexcept = default;
	FactoryInfo& operator= (FactoryInfo&&) noexcept;
	FactoryInfo& operator= (PFactoryInfo&&) noexcept;

	std::string vendor () const noexcept;
	std::string url () const noexcept;
	std::string email () const noexcept;
	Steinberg::int32 flags () const noexcept;
	bool classesDiscardable () const noexcept;
	bool licenseCheck () const noexcept;
	bool componentNonDiscardable () const noexcept;

	PFactoryInfo& get () noexcept { return info; }
//------------------------------------------------------------------------
private:
	PFactoryInfo info {};
};

//------------------------------------------------------------------------
class ClassInfo
{
public:
//------------------------------------------------------------------------
	using SubCategories = std::vector<std::string>;
	using PClassInfo = Steinberg::PClassInfo;
	using PClassInfo2 = Steinberg::PClassInfo2;
	using PClassInfoW = Steinberg::PClassInfoW;

//------------------------------------------------------------------------
	ClassInfo () noexcept {}
	explicit ClassInfo (const PClassInfo& info) noexcept;
	explicit ClassInfo (const PClassInfo2& info) noexcept;
	explicit ClassInfo (const PClassInfoW& info) noexcept;
	ClassInfo (const ClassInfo&) = default;
	ClassInfo& operator= (const ClassInfo&) = default;
	ClassInfo (ClassInfo&&) = default;
	ClassInfo& operator= (ClassInfo&&) = default;

	const UID& ID () const noexcept;
	int32_t cardinality () const noexcept;
	const std::string& category () const noexcept;
	const std::string& name () const noexcept;
	const std::string& vendor () const noexcept;
	const std::string& version () const noexcept;
	const std::string& sdkVersion () const noexcept;
	const SubCategories& subCategories () const noexcept;
	std::string subCategoriesString () const noexcept;
	Steinberg::uint32 classFlags () const noexcept;

	struct Data
	{
		UID classID;
		int32_t cardinality;
		std::string category;
		std::string name;
		std::string vendor;
		std::string version;
		std::string sdkVersion;
		SubCategories subCategories;
		Steinberg::uint32 classFlags = 0;
	};

	Data& get () noexcept { return data; }
//------------------------------------------------------------------------
private:
	void parseSubCategories (const std::string& str) noexcept;
	Data data {};
};

//------------------------------------------------------------------------
class PluginFactory
{
public:
//------------------------------------------------------------------------
	using ClassInfos = std::vector<ClassInfo>;
	using PluginFactoryPtr = Steinberg::IPtr<Steinberg::IPluginFactory>;

//------------------------------------------------------------------------
	explicit PluginFactory (const PluginFactoryPtr& factory) noexcept;

	void setHostContext (Steinberg::FUnknown* context) const noexcept;

	FactoryInfo info () const noexcept;
	uint32_t classCount () const noexcept;
	ClassInfos classInfos () const noexcept;

	template <typename T>
	Steinberg::IPtr<T> createInstance (const UID& classID) const noexcept;

	const PluginFactoryPtr& get () const noexcept { return factory; }
//------------------------------------------------------------------------
private:
	PluginFactoryPtr factory;
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
class Module
{
public:
//------------------------------------------------------------------------
	struct Snapshot
	{
		struct ImageDesc
		{
			double scaleFactor {1.};
			std::string path;
		};
		UID uid;
		std::vector<ImageDesc> images;

		static Optional<double> decodeScaleFactor (const std::string& path);
		static Optional<UID> decodeUID (const std::string& filename);
	};

	using Ptr = std::shared_ptr<Module>;
	using PathList = std::vector<std::string>;
	using SnapshotList = std::vector<Snapshot>;

//------------------------------------------------------------------------
	static Ptr create (const std::string& path, std::string& errorDescription);
	static PathList getModulePaths ();
	static SnapshotList getSnapshots (const std::string& modulePath);
	/** get the path to the module info json file if it exists */
	static Optional<std::string> getModuleInfoPath (const std::string& modulePath);

	const std::string& getName () const noexcept { return name; }
	const std::string& getPath () const noexcept { return path; }
	const PluginFactory& getFactory () const noexcept { return factory; }
	bool isBundle () const noexcept { return hasBundleStructure; }
//------------------------------------------------------------------------
protected:
	virtual ~Module () noexcept = default;
	virtual bool load (const std::string& path, std::string& errorDescription) = 0;

	PluginFactory factory {nullptr};
	std::string name;
	std::string path;
	bool hasBundleStructure {true};
};

//------------------------------------------------------------------------
template <typename T>
inline Steinberg::IPtr<T> PluginFactory::createInstance (const UID& classID) const noexcept
{
	T* obj = nullptr;
	if (factory->createInstance (classID.data (), T::iid, reinterpret_cast<void**> (&obj)) ==
	    Steinberg::kResultTrue)
		return Steinberg::owned (obj);
	return nullptr;
}

//------------------------------------------------------------------------
} // Hosting
} // VST3
