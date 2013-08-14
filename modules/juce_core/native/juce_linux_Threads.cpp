/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

//==============================================================================
JUCE_API void JUCE_CALLTYPE Process::setPriority (const ProcessPriority prior)
{
    const int policy = (prior <= NormalPriority) ? SCHED_OTHER : SCHED_RR;
    const int minp = sched_get_priority_min (policy);
    const int maxp = sched_get_priority_max (policy);

    struct sched_param param;

    switch (prior)
    {
        case LowPriority:
        case NormalPriority:    param.sched_priority = 0; break;
        case HighPriority:      param.sched_priority = minp + (maxp - minp) / 4; break;
        case RealtimePriority:  param.sched_priority = minp + (3 * (maxp - minp) / 4); break;
        default:                jassertfalse; break;
    }

    pthread_setschedparam (pthread_self(), policy, &param);
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger()
{
   #if JUCE_BSD
    return false;
   #else
    static char testResult = 0;

    if (testResult == 0)
    {
        testResult = (char) ptrace (PT_TRACE_ME, 0, 0, 0);

        if (testResult >= 0)
        {
            ptrace (PT_DETACH, 0, (caddr_t) 1, 0);
            testResult = 1;
        }
    }

    return testResult < 0;
   #endif
}

JUCE_API bool JUCE_CALLTYPE Process::isRunningUnderDebugger()
{
    return juce_isRunningUnderDebugger();
}

static void swapUserAndEffectiveUser()
{
    (void) setreuid (geteuid(), getuid());
    (void) setregid (getegid(), getgid());
}

JUCE_API void JUCE_CALLTYPE Process::raisePrivilege()  { if (geteuid() != 0 && getuid() == 0) swapUserAndEffectiveUser(); }
JUCE_API void JUCE_CALLTYPE Process::lowerPrivilege()  { if (geteuid() == 0 && getuid() != 0) swapUserAndEffectiveUser(); }
