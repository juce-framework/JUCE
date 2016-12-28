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

static bool swapUserAndEffectiveUser()
{
    int result1 = setreuid (geteuid(), getuid());
    int result2 = setregid (getegid(), getgid());
    return result1 == 0 && result2 == 0;
}

JUCE_API void JUCE_CALLTYPE Process::raisePrivilege()  { if (geteuid() != 0 && getuid() == 0) swapUserAndEffectiveUser(); }
JUCE_API void JUCE_CALLTYPE Process::lowerPrivilege()  { if (geteuid() == 0 && getuid() != 0) swapUserAndEffectiveUser(); }
