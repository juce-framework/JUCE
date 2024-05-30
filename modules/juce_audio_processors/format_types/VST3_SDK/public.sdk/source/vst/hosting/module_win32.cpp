//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/hosting/module_win32.cpp
// Created by  : Steinberg, 08/2016
// Description : hosting module classes (win32 implementation)
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

#include "../utility/optional.h"
#include "../utility/stringconvert.h"
#include "module.h"

#include <shlobj.h>
#include <windows.h>

#include <algorithm>
#include <iostream>

#if SMTG_CPP17

#if __has_include(<filesystem>)
#define USE_FILESYSTEM 1
#elif __has_include(<experimental/filesystem>)
#define USE_FILESYSTEM 0
#endif

#else // !SMTG_CPP17

#define USE_FILESYSTEM 0

#endif // SMTG_CPP17

#if USE_FILESYSTEM == 1

#include <filesystem>
namespace filesystem = std::filesystem;

#else // USE_FILESYSTEM == 0

// The <experimental/filesystem> header is deprecated. It is superseded by the C++17 <filesystem>
// header. You can define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING to silence the
// warning, otherwise the build will fail in VS2019 16.3.0
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;

#endif // USE_FILESYSTEM

#pragma comment(lib, "Shell32")

//------------------------------------------------------------------------
extern "C" {
using InitModuleFunc = bool (PLUGIN_API*) ();
using ExitModuleFunc = bool (PLUGIN_API*) ();
}

//------------------------------------------------------------------------
namespace VST3 {
namespace Hosting {

constexpr unsigned long kIPPathNameMax = 1024;

//------------------------------------------------------------------------
namespace {

#define USE_OLE !USE_FILESYSTEM

// for testing only
#if 0 // DEVELOPMENT
#define LOG_ENABLE 1
#else
#define LOG_ENABLE 0
#endif

#if SMTG_PLATFORM_64

#if SMTG_OS_WINDOWS_ARM

#if SMTG_CPU_ARM_64EC
constexpr auto architectureString = "arm64ec-win";
constexpr auto architectureX64String = "x86_64-win";
#else // !SMTG_CPU_ARM_64EC
constexpr auto architectureString = "arm64-win";
#endif // SMTG_CPU_ARM_64EC

constexpr auto architectureArm64XString = "arm64x-win";

#else // !SMTG_OS_WINDOWS_ARM
constexpr auto architectureString = "x86_64-win";
#endif // SMTG_OS_WINDOWS_ARM

#else // !SMTG_PLATFORM_64

#if SMTG_OS_WINDOWS_ARM
constexpr auto architectureString = "arm-win";
#else // !SMTG_OS_WINDOWS_ARM
constexpr auto architectureString = "x86-win";
#endif // SMTG_OS_WINDOWS_ARM

#endif // SMTG_PLATFORM_64

#if USE_OLE
//------------------------------------------------------------------------
struct Ole
{
	static Ole& instance ()
	{
		static Ole gInstance;
		return gInstance;
	}

private:
	Ole () { OleInitialize (nullptr); }
	~Ole () { OleUninitialize (); }
};
#endif // USE_OLE

//------------------------------------------------------------------------
class Win32Module : public Module
{
public:
	template <typename T>
	T getFunctionPointer (const char* name)
	{
		return reinterpret_cast<T> (GetProcAddress (mModule, name));
	}

	~Win32Module () override
	{
		factory = PluginFactory (nullptr);

		if (mModule)
		{
			// ExitDll is optional
			if (auto dllExit = getFunctionPointer<ExitModuleFunc> ("ExitDll"))
				dllExit ();

			FreeLibrary ((HMODULE)mModule);
		}
	}

	//--- -----------------------------------------------------------------------
	HINSTANCE loadAsPackage (const std::string& inPath, const char* archString = architectureString)
	{
		filesystem::path p (inPath);
		auto filename = p.filename ();
		p /= "Contents";
		p /= archString;
		p /= filename;
		auto wideStr = StringConvert::convert (p.string ());
		HINSTANCE instance = LoadLibraryW (reinterpret_cast<LPCWSTR> (wideStr.data ()));
#if SMTG_CPU_ARM_64EC
		if (instance == nullptr)
			instance = loadAsPackage (inPath, architectureArm64XString);
		if (instance == nullptr)
			instance = loadAsPackage (inPath, architectureX64String);
#endif
		return instance;
	}

	//--- -----------------------------------------------------------------------
	HINSTANCE loadAsDll (const std::string& inPath, std::string& errorDescription)
	{
		auto wideStr = StringConvert::convert (inPath);
		HINSTANCE instance = LoadLibraryW (reinterpret_cast<LPCWSTR> (wideStr.data ()));
		if (instance == nullptr)
		{
			auto lastError = GetLastError ();
			LPVOID lpMessageBuffer {nullptr};
			if (FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			                    nullptr, lastError, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
			                    (LPSTR)&lpMessageBuffer, 0, nullptr) > 0)
			{
				errorDescription = "LoadLibray failed: " + std::string ((char*)lpMessageBuffer);
				LocalFree (lpMessageBuffer);
			}
			else
			{
				errorDescription =
				    "LoadLibrary failed with error number : " + std::to_string (lastError);
			}
		}
		else
		{
			hasBundleStructure = false;
		}
		return instance;
	}

	//--- -----------------------------------------------------------------------
	bool load (const std::string& inPath, std::string& errorDescription) override
	{
		// filesystem::u8path is deprecated in C++20
#if SMTG_CPP20
		const filesystem::path tmp (inPath);
#else
		const filesystem::path tmp = filesystem::u8path (inPath);
#endif
		if (filesystem::is_directory (tmp))
		{
			// try as package (bundle)
			mModule = loadAsPackage (inPath);
		}
		else
		{
			// try old definition without package
			mModule = loadAsDll (inPath, errorDescription);
		}
		if (mModule == nullptr)
			return false;

		auto factoryProc = getFunctionPointer<GetFactoryProc> ("GetPluginFactory");
		if (!factoryProc)
		{
			errorDescription = "The dll does not export the required 'GetPluginFactory' function";
			return false;
		}
		// InitDll is optional
		auto dllEntry = getFunctionPointer<InitModuleFunc> ("InitDll");
		if (dllEntry && !dllEntry ())
		{
			errorDescription = "Calling 'InitDll' failed";
			return false;
		}
		auto f = Steinberg::FUnknownPtr<Steinberg::IPluginFactory> (owned (factoryProc ()));
		if (!f)
		{
			errorDescription = "Calling 'GetPluginFactory' returned nullptr";
			return false;
		}
		factory = PluginFactory (f);
		return true;
	}

	HINSTANCE mModule {nullptr};
};

//------------------------------------------------------------------------
bool openVST3Package (const filesystem::path& p, const char* archString,
                      filesystem::path* result = nullptr)
{
	auto path = p;
	path /= "Contents";
	path /= archString;
	path /= p.filename ();
	auto hFile = CreateFileW (reinterpret_cast<LPCWSTR> (path.c_str ()), GENERIC_READ,
	                          FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle (hFile);
		if (result)
			*result = path;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------
bool checkVST3Package (const filesystem::path& p, filesystem::path* result = nullptr,
                       const char* archString = architectureString)
{
	if (openVST3Package (p, archString, result))
		return true;

#if SMTG_CPU_ARM_64EC
	if (openVST3Package (p, architectureArm64XString, result))
		return true;
	if (openVST3Package (p, architectureX64String, result))
		return true;
#endif
	return false;
}

//------------------------------------------------------------------------
bool isFolderSymbolicLink (const filesystem::path& p)
{
#if USE_FILESYSTEM
	if (/*filesystem::exists (p) &&*/ filesystem::is_symlink (p))
		return true;
#else
	std::wstring wString = p.generic_wstring ();
	auto attrib = GetFileAttributesW (reinterpret_cast<LPCWSTR> (wString.c_str ()));
	if (attrib & FILE_ATTRIBUTE_REPARSE_POINT)
	{
		auto hFile = CreateFileW (reinterpret_cast<LPCWSTR> (wString.c_str ()), GENERIC_READ,
		                          FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return true;
		CloseHandle (hFile);
	}
#endif
	return false;
}

//------------------------------------------------------------------------
Optional<std::string> getKnownFolder (REFKNOWNFOLDERID folderID)
{
	PWSTR wideStr {};
	if (FAILED (SHGetKnownFolderPath (folderID, 0, nullptr, &wideStr)))
		return {};
	return StringConvert::convert (Steinberg::wscast (wideStr));
}

//------------------------------------------------------------------------
VST3::Optional<filesystem::path> resolveShellLink (const filesystem::path& p)
{
#if USE_FILESYSTEM
	return {filesystem::read_symlink (p).lexically_normal ()};
#elif USE_OLE
	Ole::instance ();

	IShellLink* shellLink = nullptr;
	if (!SUCCEEDED (CoCreateInstance (CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
	                                  IID_IShellLink, reinterpret_cast<LPVOID*> (&shellLink))))
		return {};

	IPersistFile* persistFile = nullptr;
	if (!SUCCEEDED (
	        shellLink->QueryInterface (IID_IPersistFile, reinterpret_cast<void**> (&persistFile))))
		return {};

	if (!SUCCEEDED (persistFile->Load (p.wstring ().data (), STGM_READ)))
		return {};

	if (!SUCCEEDED (shellLink->Resolve (nullptr, MAKELONG (SLR_NO_UI, 500))))
		return {};

	WCHAR resolvedPath[kIPPathNameMax];
	if (!SUCCEEDED (shellLink->GetPath (resolvedPath, kIPPathNameMax, nullptr, SLGP_SHORTPATH)))
		return {};

	std::wstring longPath;
	longPath.resize (kIPPathNameMax);
	auto numChars =
	    GetLongPathNameW (resolvedPath, const_cast<wchar_t*> (longPath.data ()), kIPPathNameMax);
	if (!numChars)
		return {};
	longPath.resize (numChars);

	persistFile->Release ();
	shellLink->Release ();

	return {filesystem::path (longPath)};
#else
	return {};
#endif
}

//------------------------------------------------------------------------
void addToPathList (Module::PathList& pathList, const std::string& toAdd)
{
#if LOG_ENABLE
		std::cout << "=> add: " << toAdd << "\n";
#endif

	pathList.push_back (toAdd);
}

//------------------------------------------------------------------------
void findFilesWithExt (const filesystem::path& path, const std::string& ext,
                       Module::PathList& pathList, bool recursive = true)
{
	for (auto& p : filesystem::directory_iterator (path))
	{
#if USE_FILESYSTEM
		filesystem::path finalPath (p);
		if (isFolderSymbolicLink (p))
		{
			if (auto res = resolveShellLink (p))
			{
				finalPath = *res;
				if (!filesystem::exists (finalPath))
					continue;
			}
			else
				continue;
		}
		const auto& cpExt = finalPath.extension ();
		if (cpExt == ext)
		{
			filesystem::path result;
			if (checkVST3Package (finalPath, &result))
			{
				addToPathList (pathList, result.generic_string ());
				continue;
			}
		}

		if (filesystem::is_directory (finalPath))
		{
			if (recursive)
				findFilesWithExt (finalPath, ext, pathList, recursive);
		}
		else if (cpExt == ext)
			addToPathList (pathList, finalPath.generic_string ());
#else
		const auto& cp = p.path ();
		const auto& cpExt = cp.extension ();
		if (cpExt == ext)
		{
			if ((p.status ().type () == filesystem::file_type::directory) ||
			    isFolderSymbolicLink (p))
			{
				filesystem::path result;
				if (checkVST3Package (p, &result))
				{
					addToPathList (pathList, result.generic_u8string ());
					continue;
				}
				findFilesWithExt (cp, ext, pathList, recursive);
			}
			else
				addToPathList (pathList, cp.generic_u8string ());
		}
		else if (recursive)
		{
			if (p.status ().type () == filesystem::file_type::directory)
			{
				findFilesWithExt (cp, ext, pathList, recursive);
			}
			else if (cpExt == ".lnk")
			{
				if (auto resolvedLink = resolveShellLink (cp))
				{
					if (resolvedLink->extension () == ext)
					{
						if (filesystem::is_directory (*resolvedLink) ||
						    isFolderSymbolicLink (*resolvedLink))
						{
							filesystem::path result;
							if (checkVST3Package (*resolvedLink, &result))
							{
								addToPathList (pathList, result.generic_u8string ());
								continue;
							}
							findFilesWithExt (*resolvedLink, ext, pathList, recursive);
						}
						else
							addToPathList (pathList, resolvedLink->generic_u8string ());
					}
					else if (filesystem::is_directory (*resolvedLink))
					{
						const auto& str = resolvedLink->generic_u8string ();
						if (cp.generic_u8string ().compare (0, str.size (), str.data (),
						                                    str.size ()) != 0)
							findFilesWithExt (*resolvedLink, ext, pathList, recursive);
					}
				}
			}
		}
#endif
	}
}

//------------------------------------------------------------------------
void findModules (const filesystem::path& path, Module::PathList& pathList)
{
	if (filesystem::exists (path))
		findFilesWithExt (path, ".vst3", pathList);
}

//------------------------------------------------------------------------
Optional<filesystem::path> getContentsDirectoryFromModuleExecutablePath (
    const std::string& modulePath)
{
	filesystem::path path (modulePath);

	path = path.parent_path ();
	if (path.filename () != architectureString)
		return {};
	path = path.parent_path ();
	if (path.filename () != "Contents")
		return {};

	return Optional<filesystem::path> {std::move (path)};
}

//------------------------------------------------------------------------
} // anonymous

//------------------------------------------------------------------------
Module::Ptr Module::create (const std::string& path, std::string& errorDescription)
{
	auto _module = std::make_shared<Win32Module> ();
	if (_module->load (path, errorDescription))
	{
		_module->path = path;
		auto it = std::find_if (path.rbegin (), path.rend (),
		                        [] (const std::string::value_type& c) { return c == '/'; });
		if (it != path.rend ())
			_module->name = {it.base (), path.end ()};
		return _module;
	}
	return nullptr;
}

//------------------------------------------------------------------------
Module::PathList Module::getModulePaths ()
{
	// find plug-ins located in common/VST3
	PathList list;
	if (auto knownFolder = getKnownFolder (FOLDERID_UserProgramFilesCommon))
	{
		filesystem::path path (*knownFolder);
		path.append ("VST3");
#if LOG_ENABLE
		std::cout << "Check folder: " << path << "\n";
#endif
		findModules (path, list);
	}

	if (auto knownFolder = getKnownFolder (FOLDERID_ProgramFilesCommon))
	{
		filesystem::path path (*knownFolder);
		path.append ("VST3");
#if LOG_ENABLE
		std::cout << "Check folder: " << path << "\n";
#endif
		findModules (path, list);
	}
	
	// find plug-ins located in VST3 (application folder)
	WCHAR modulePath[kIPPathNameMax];
	GetModuleFileNameW (nullptr, modulePath, kIPPathNameMax);
	auto appPath = StringConvert::convert (Steinberg::wscast (modulePath));
	filesystem::path path (appPath);
	path = path.parent_path ();
	path = path.append ("VST3");
#if LOG_ENABLE
	std::cout << "Check folder: " << path << "\n";
#endif
	findModules (path, list);

	return list;
}

//------------------------------------------------------------------------
Optional<std::string> Module::getModuleInfoPath (const std::string& modulePath)
{
	auto path = getContentsDirectoryFromModuleExecutablePath (modulePath);
	if (!path)
	{
		filesystem::path p;
		if (!checkVST3Package ({modulePath}, &p))
			return {};
		p = p.parent_path ();
		p = p.parent_path ();
		path = Optional<filesystem::path> {p};
	}

	*path /= "Resources";
	*path /= "moduleinfo.json";

	if (filesystem::exists (*path))
	{
		return {path->generic_string ()};
	}
	return {};
}

//------------------------------------------------------------------------
Module::SnapshotList Module::getSnapshots (const std::string& modulePath)
{
	SnapshotList result;
	auto path = getContentsDirectoryFromModuleExecutablePath (modulePath);
	if (!path)
	{
		filesystem::path p;
		if (!checkVST3Package ({modulePath}, &p))
			return result;
		p = p.parent_path ();
		p = p.parent_path ();
		path = Optional<filesystem::path> (p);
	}

	*path /= "Resources";
	*path /= "Snapshots";

	if (filesystem::exists (*path) == false)
		return result;

	PathList pngList;
	findFilesWithExt (*path, ".png", pngList, false);
	for (auto& png : pngList)
	{
		filesystem::path p (png);
		auto filename = p.filename ().generic_string ();
		auto uid = Snapshot::decodeUID (filename);
		if (!uid)
			continue;
		auto scaleFactor = 1.;
		if (auto decodedScaleFactor = Snapshot::decodeScaleFactor (filename))
			scaleFactor = *decodedScaleFactor;

		Module::Snapshot::ImageDesc desc;
		desc.scaleFactor = scaleFactor;
		desc.path = std::move (png);
		bool found = false;
		for (auto& entry : result)
		{
			if (entry.uid != *uid)
				continue;
			found = true;
			entry.images.emplace_back (std::move (desc));
			break;
		}
		if (found)
			continue;
		Module::Snapshot snapshot;
		snapshot.uid = *uid;
		snapshot.images.emplace_back (std::move (desc));
		result.emplace_back (std::move (snapshot));
	}
	return result;
}

//------------------------------------------------------------------------
} // Hosting
} // VST3
