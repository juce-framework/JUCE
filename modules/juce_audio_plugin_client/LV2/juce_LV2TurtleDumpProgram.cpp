/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#ifdef _WIN32
 #undef UNICODE
 #undef _UNICODE

 #define UNICODE 1
 #define _UNICODE 1

 #include <windows.h>
 #include <tchar.h>
 HMODULE dlopen (const TCHAR* filename, int) { return LoadLibrary (filename); }
 FARPROC dlsym (HMODULE handle, const char* name) { return GetProcAddress (handle, name); }
 static void printError()
 {
     constexpr DWORD numElements = 256;
     TCHAR messageBuffer[numElements]{};

     FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    GetLastError(),
                    MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                    messageBuffer,
                    numElements - 1,
                    nullptr);

     _tprintf (_T ("%s"), messageBuffer);
 }

 enum { RTLD_LAZY = 0 };

 class ArgList
 {
 public:
     ArgList (int, const char**) {}
     ArgList (const ArgList&) = delete;
     ArgList (ArgList&&) = delete;
     ArgList& operator= (const ArgList&) = delete;
     ArgList& operator= (ArgList&&) = delete;
     ~ArgList() { LocalFree (argv); }

     LPWSTR get (int i) const { return argv[i]; }

     int size() const { return argc; }

 private:
     int argc = 0;
     LPWSTR* argv = CommandLineToArgvW (GetCommandLineW(), &argc);
 };

 static std::vector<char> toUTF8 (const TCHAR* str)
 {
     const auto numBytes = WideCharToMultiByte (CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
     std::vector<char> result (numBytes);
     WideCharToMultiByte (CP_UTF8, 0, str, -1, result.data(), static_cast<int> (result.size()), nullptr, nullptr);
     return result;
 }

#else
 #include <dlfcn.h>
 static void printError() { printf ("%s\n", dlerror()); }
 class ArgList
 {
 public:
     ArgList (int argcIn, const char** argvIn) : argc (argcIn), argv (argvIn) {}
     ArgList (const ArgList&) = delete;
     ArgList (ArgList&&) = delete;
     ArgList& operator= (const ArgList&) = delete;
     ArgList& operator= (ArgList&&) = delete;
     ~ArgList() = default;

     const char* get (int i) const { return argv[i]; }

     int size() const { return argc; }

 private:
     int argc = 0;
     const char** argv = nullptr;
 };

 static std::vector<char> toUTF8 (const char* str) { return std::vector<char> (str, str + std::strlen (str) + 1); }
#endif

// Replicating part of the LV2 header here so that we don't have to set up any
// custom include paths for this file.
// Normally this would be a bad idea, but the LV2 API has to keep these definitions
// in order to remain backwards-compatible.

extern "C"
{
    typedef struct LV2_Descriptor
    {
        const void* a;
        const void* b;
        const void* c;
        const void* d;
        const void* e;
        const void* f;
        const void* g;
        const void* (*extension_data)(const char* uri);
    } LV2_Descriptor;
}

int main (int argc, const char** argv)
{
    const ArgList argList { argc, argv };

    if (argList.size() != 2)
        return 1;

    const auto* libraryPath = argList.get (1);

    struct RecallFeature
    {
        int (*doRecall) (const char*);
    };

    if (auto* handle = dlopen (libraryPath, RTLD_LAZY))
    {
        if (auto* getDescriptor = reinterpret_cast<const LV2_Descriptor* (*) (uint32_t)> (dlsym (handle, "lv2_descriptor")))
        {
            if (auto* descriptor = getDescriptor (0))
            {
                if (auto* extensionData = descriptor->extension_data)
                {
                    if (auto* recallFeature = reinterpret_cast<const RecallFeature*> (extensionData ("https://lv2-extensions.juce.com/turtle_recall")))
                    {
                        if (auto* doRecall = recallFeature->doRecall)
                        {
                            const auto converted = toUTF8 (libraryPath);
                            return doRecall (converted.data());
                        }
                    }
                }
            }
        }
    }
    else
    {
        printError();
    }

    return 1;
}
