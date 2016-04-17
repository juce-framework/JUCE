/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

#if JUCE_IOS
bool isIOSAppActive = true;
#endif

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()
{
   if (SystemStats::isRunningInAppExtensionSandbox())
       return true;

   #if JUCE_MAC
    return [NSApp isActive];
   #else
    return isIOSAppActive;
   #endif
}

JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess()
{
   #if JUCE_MAC
    if (! SystemStats::isRunningInAppExtensionSandbox())
        [NSApp activateIgnoringOtherApps: YES];
   #endif
}

JUCE_API void JUCE_CALLTYPE Process::hide()
{
   #if JUCE_MAC
    if (! SystemStats::isRunningInAppExtensionSandbox())
        [NSApp hide: nil];
   #endif
}

JUCE_API void JUCE_CALLTYPE Process::raisePrivilege()
{
    jassertfalse;
}

JUCE_API void JUCE_CALLTYPE Process::lowerPrivilege()
{
    jassertfalse;
}

JUCE_API void JUCE_CALLTYPE Process::setPriority (ProcessPriority)
{
    // xxx
}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger() noexcept
{
    struct kinfo_proc info;
    int m[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    size_t sz = sizeof (info);
    sysctl (m, 4, &info, &sz, 0, 0);
    return (info.kp_proc.p_flag & P_TRACED) != 0;
}
