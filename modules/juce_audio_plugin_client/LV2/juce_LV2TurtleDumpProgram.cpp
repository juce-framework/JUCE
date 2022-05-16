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

#ifdef _WIN32
 #include <windows.h>
 HMODULE dlopen (const char* filename, int) { return LoadLibrary (filename); }
 FARPROC dlsym (HMODULE handle, const char* name) { return GetProcAddress (handle, name); }
 enum { RTLD_LAZY = 0 };
#else
 #include <dlfcn.h>
#endif

#include <cstdint>

// Replicating some of the LV2 header here so that we don't have to set up any
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
    if (argc != 2)
        return 1;

    const auto* libraryPath = argv[1];

    struct RecallFeature
    {
        int (*doRecall) (const char*);
    };

    if (auto* handle = dlopen (libraryPath, RTLD_LAZY))
        if (auto* getDescriptor = reinterpret_cast<const LV2_Descriptor* (*) (uint32_t)> (dlsym (handle, "lv2_descriptor")))
            if (auto* descriptor = getDescriptor (0))
                if (auto* extensionData = descriptor->extension_data)
                    if (auto* recallFeature = reinterpret_cast<const RecallFeature*> (extensionData ("https://lv2-extensions.juce.com/turtle_recall")))
                        if (auto* doRecall = recallFeature->doRecall)
                            return doRecall (libraryPath);

    return 1;
}
