/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "projucer_LiveCodeBuilderDLL.h"


struct CompileEngineDLL
{
    CompileEngineDLL()
    {
        tryLoadDll();
    }

    ~CompileEngineDLL()
    {
        shutdown();
    }

    void tryLoadDll()
    {
        // never load the dynamic lib multiple times
        if (! isLoaded())
        {
            File f = findDLLFile();

            if (f != File() && dll.open (f.getLinkedTarget().getFullPathName()))
            {
               #define INIT_LIVE_DLL_FN(name, returnType, params)    name = (name##_type) dll.getFunction (#name);
                LIVE_DLL_FUNCTIONS (INIT_LIVE_DLL_FN);
               #undef INIT_LIVE_DLL_FN
            }
        }
    }

    void initialise (CrashCallbackFunction crashFn, QuitCallbackFunction quitFn, bool setupSignals)
    {
        if (isLoaded())
            projucer_initialise (crashFn, quitFn, setPropertyCallback, getPropertyCallback, setupSignals);
    }

    void shutdown()
    {
        if (isLoaded())
            projucer_shutdown();
    }

    bool isLoaded() const
    {
        #define CHECK_LIVE_DLL_FN(name, returnType, params)    if (name == nullptr) return false;
        LIVE_DLL_FUNCTIONS (CHECK_LIVE_DLL_FN);
        #undef CHECK_LIVE_DLL_FN

        return projucer_getVersion() == requiredVersion;
    }

    #define DECLARE_LIVE_DLL_FN(name, returnType, params) \
        typedef returnType (*name##_type) params; \
        name##_type name = nullptr;

    LIVE_DLL_FUNCTIONS (DECLARE_LIVE_DLL_FN);

    #undef DECLARE_LIVE_DLL_FN

    static String getDLLName()
    {
       #if JUCE_MAC
        return "JUCECompileEngine.dylib";
       #elif JUCE_LINUX
        return "JUCECompileEngine.so";
       #elif JUCE_WINDOWS
        return "JUCECompileEngine.dll";
       #else
        #error
        return "JUCECompileEngine.so";
       #endif
    }

    static File getVersionedUserAppSupportFolder()
    {
        File userAppData (File::getSpecialLocation (File::userApplicationDataDirectory));

       #if JUCE_MAC
        userAppData = userAppData.getChildFile ("Application Support");
       #endif

        return userAppData.getChildFile (String ("Projucer-") + ProjectInfo::versionString);
    }

private:
    DynamicLibrary dll;

    enum { requiredVersion = 1 };

    static File findDLLFile()
    {
        File dllFile;

        if (tryFindDLLFileInAppFolder(dllFile))
            return dllFile;

       #if JUCE_MAC
        if (tryFindDLLFileInAppBundle(dllFile))
            return dllFile;
       #endif

        if (tryFindDLLFileInAppConfigFolder(dllFile))
            return dllFile;

        return File();
    }

   #if JUCE_MAC
    static bool tryFindDLLFileInAppBundle(File &outFile)
    {
        File currentAppFile (File::getSpecialLocation (File::currentApplicationFile));
        return tryFindDLLFileInFolder (currentAppFile.getChildFile ("Contents"), outFile);
    }
   #endif

    static bool tryFindDLLFileInAppFolder(File &outFile)
    {
        File currentAppFile (File::getSpecialLocation (File::currentApplicationFile));
        return tryFindDLLFileInFolder (currentAppFile.getParentDirectory(), outFile);
    }

    static bool tryFindDLLFileInAppConfigFolder(File &outFile)
    {
        File userAppDataFolder (getVersionedUserAppSupportFolder());
        return tryFindDLLFileInFolder (userAppDataFolder, outFile);
    }

    static bool tryFindDLLFileInFolder(File folder, File& outFile)
    {
        File file = folder.getChildFile (getDLLName());
        if (isDLLFile (file))
        {
            outFile = file;
            return true;
        }

        return false;
    }

    static bool isDLLFile (const File& f)
    {
        return f.getFileName().equalsIgnoreCase (getDLLName()) && f.exists();
    }

    static void setPropertyCallback (const char* key, const char* value)
    {
        if (String (key).isNotEmpty())
            getGlobalProperties().setValue (key, value);
        else
            jassertfalse;
    }

    static void getPropertyCallback (const char* key, char* value, size_t size)
    {
        jassert (getGlobalProperties().getValue (key).getNumBytesAsUTF8() < size);

        value[0] = 0;
        getGlobalProperties().getValue (key).copyToUTF8 (value, size);
    }
};
