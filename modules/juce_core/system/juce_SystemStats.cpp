/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

String SystemStats::getJUCEVersion()
{
    // Some basic tests, to keep an eye on things and make sure these types work ok
    // on all platforms. Let me know if any of these assertions fail on your system!
    static_jassert (sizeof (pointer_sized_int) == sizeof (void*));
    static_jassert (sizeof (int8) == 1);
    static_jassert (sizeof (uint8) == 1);
    static_jassert (sizeof (int16) == 2);
    static_jassert (sizeof (uint16) == 2);
    static_jassert (sizeof (int32) == 4);
    static_jassert (sizeof (uint32) == 4);
    static_jassert (sizeof (int64) == 8);
    static_jassert (sizeof (uint64) == 8);

    return "JUCE v" JUCE_STRINGIFY(JUCE_MAJOR_VERSION)
                "." JUCE_STRINGIFY(JUCE_MINOR_VERSION)
                "." JUCE_STRINGIFY(JUCE_BUILDNUMBER);
}

#if JUCE_ANDROID && ! defined (JUCE_DISABLE_JUCE_VERSION_PRINTING)
 #define JUCE_DISABLE_JUCE_VERSION_PRINTING 1
#endif

#if JUCE_DEBUG && ! JUCE_DISABLE_JUCE_VERSION_PRINTING
 struct JuceVersionPrinter
 {
     JuceVersionPrinter()
     {
         DBG (SystemStats::getJUCEVersion());
     }
 };

 static JuceVersionPrinter juceVersionPrinter;
#endif


//==============================================================================
struct CPUInformation
{
    CPUInformation() noexcept
        : numCpus (0), hasMMX (false), hasSSE (false),
          hasSSE2 (false), hasSSE3 (false), has3DNow (false),
          hasSSSE3 (false), hasSSE41 (false), hasSSE42 (false),
          hasAVX (false), hasAVX2 (false)
    {
        initialise();
    }

    void initialise() noexcept;

    int numCpus;
    bool hasMMX, hasSSE, hasSSE2, hasSSE3, has3DNow, hasSSSE3, hasSSE41, hasSSE42, hasAVX, hasAVX2;
};

static const CPUInformation& getCPUInformation() noexcept
{
    static CPUInformation info;
    return info;
}

int SystemStats::getNumCpus() noexcept        { return getCPUInformation().numCpus; }
bool SystemStats::hasMMX() noexcept           { return getCPUInformation().hasMMX; }
bool SystemStats::has3DNow() noexcept         { return getCPUInformation().has3DNow; }
bool SystemStats::hasSSE() noexcept           { return getCPUInformation().hasSSE; }
bool SystemStats::hasSSE2() noexcept          { return getCPUInformation().hasSSE2; }
bool SystemStats::hasSSE3() noexcept          { return getCPUInformation().hasSSE3; }
bool SystemStats::hasSSSE3() noexcept         { return getCPUInformation().hasSSSE3; }
bool SystemStats::hasSSE41() noexcept         { return getCPUInformation().hasSSE41; }
bool SystemStats::hasSSE42() noexcept         { return getCPUInformation().hasSSE42; }
bool SystemStats::hasAVX() noexcept           { return getCPUInformation().hasAVX; }
bool SystemStats::hasAVX2() noexcept          { return getCPUInformation().hasAVX2; }


//==============================================================================
String SystemStats::getStackBacktrace()
{
    String result;

   #if JUCE_ANDROID || JUCE_MINGW
    jassertfalse; // sorry, not implemented yet!

   #elif JUCE_WINDOWS
    HANDLE process = GetCurrentProcess();
    SymInitialize (process, nullptr, TRUE);

    void* stack[128];
    int frames = (int) CaptureStackBackTrace (0, numElementsInArray (stack), stack, nullptr);

    HeapBlock<SYMBOL_INFO> symbol;
    symbol.calloc (sizeof (SYMBOL_INFO) + 256, 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof (SYMBOL_INFO);

    for (int i = 0; i < frames; ++i)
    {
        DWORD64 displacement = 0;

        if (SymFromAddr (process, (DWORD64) stack[i], &displacement, symbol))
        {
            result << i << ": ";

            IMAGEHLP_MODULE64 moduleInfo;
            zerostruct (moduleInfo);
            moduleInfo.SizeOfStruct = sizeof (moduleInfo);

            if (::SymGetModuleInfo64 (process, symbol->ModBase, &moduleInfo))
                result << moduleInfo.ModuleName << ": ";

            result << symbol->Name << " + 0x" << String::toHexString ((int64) displacement) << newLine;
        }
    }

   #else
    void* stack[128];
    int frames = backtrace (stack, numElementsInArray (stack));
    char** frameStrings = backtrace_symbols (stack, frames);

    for (int i = 0; i < frames; ++i)
        result << frameStrings[i] << newLine;

    ::free (frameStrings);
   #endif

    return result;
}

//==============================================================================
static SystemStats::CrashHandlerFunction globalCrashHandler = nullptr;

#if JUCE_WINDOWS
static LONG WINAPI handleCrash (LPEXCEPTION_POINTERS)
{
    globalCrashHandler();
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
static void handleCrash (int)
{
    globalCrashHandler();
    kill (getpid(), SIGKILL);
}

int juce_siginterrupt (int sig, int flag);
#endif

void SystemStats::setApplicationCrashHandler (CrashHandlerFunction handler)
{
    jassert (handler != nullptr); // This must be a valid function.
    globalCrashHandler = handler;

   #if JUCE_WINDOWS
    SetUnhandledExceptionFilter (handleCrash);
   #else
    const int signals[] = { SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGABRT, SIGSYS };

    for (int i = 0; i < numElementsInArray (signals); ++i)
    {
        ::signal (signals[i], handleCrash);
        juce_siginterrupt (signals[i], 1);
    }
   #endif
}

bool SystemStats::isRunningInAppExtensionSandbox() noexcept
{
   #if JUCE_MAC || JUCE_IOS
    static bool firstQuery = true;
    static bool isRunningInAppSandbox = false;

    if (firstQuery)
    {
        firstQuery = false;

        File bundle = File::getSpecialLocation (File::invokedExecutableFile).getParentDirectory();

       #if JUCE_MAC
        bundle = bundle.getParentDirectory().getParentDirectory();
       #endif

        if (bundle.isDirectory())
            isRunningInAppSandbox = (bundle.getFileExtension() == ".appex");
    }

    return isRunningInAppSandbox;
   #else
    return false;
   #endif
}
