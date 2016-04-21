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
        File f = findDLLFile();

        if (f != File() && dll.open (f.getLinkedTarget().getFullPathName()))
        {
            #define INIT_LIVE_DLL_FN(name, returnType, params)    name = (name##_type) dll.getFunction (#name);
            LIVE_DLL_FUNCTIONS (INIT_LIVE_DLL_FN);
            #undef INIT_LIVE_DLL_FN
        }
    }

    ~CompileEngineDLL()
    {
        shutdown();
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

    static bool isDLLFile (const File& f)
    {
        return f.getFileName().equalsIgnoreCase (getDLLName()) && f.exists();
    }

    static File findDLLFile()
    {
        File appFile = File::getSpecialLocation (File::currentApplicationFile);

       #if JUCE_MAC
        // Look in the app bundle..
        for (DirectoryIterator i (appFile, true, "*", File::findFilesAndDirectories); i.next();)
            if (isDLLFile (i.getFile()))
                return i.getFile();

        {
            // Try in Application Support..
            File f = File ("~/Library/Application Support/Projucer").getChildFile (getDLLName());
            if (isDLLFile (f))
                return f;

            f = File ("/Library/Application Support/Projucer").getChildFile (getDLLName());
            if (isDLLFile (f))
                return f;
        }

       #elif JUCE_WINDOWS
        {
            // Look in the application folder
            File f = appFile.getParentDirectory().getChildFile (getDLLName());
            if (isDLLFile (f))
                return f;
        }
       #elif JUCE_LINUX
        // TODO?
       #else
        #error
       #endif

        {
            // Look for a DLL in extras/Projucer/Builds
            File f = appFile.getParentDirectory();

            for (int i = 5; --i >= 0;)
            {
                if (f.getFileName().equalsIgnoreCase ("Builds")
                     && f.getParentDirectory().getFileName().equalsIgnoreCase ("Projucer"))
                {
                    f = f.getSiblingFile (getDLLName());
                    if (isDLLFile (f))
                        return f;

                    break;
                }

                f = f.getParentDirectory();
            }
        }

        // See if there's one in the same folder as the app...
        File f = appFile.getSiblingFile (getDLLName());
        if (isDLLFile (f))
            return f;

        // Look in some common folders as a last resort..
        f = File::getSpecialLocation (File::userHomeDirectory).getChildFile (getDLLName());
        if (isDLLFile (f))
            return f;

        f = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile (getDLLName());
        if (isDLLFile (f))
            return f;

        return File();
    }

private:
    DynamicLibrary dll;

    enum { requiredVersion = 1 };

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
