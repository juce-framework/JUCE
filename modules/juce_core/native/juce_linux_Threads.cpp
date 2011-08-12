/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
void Process::setPriority (const ProcessPriority prior)
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

void Process::terminate()
{
    exit (0);
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger()
{
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
}

JUCE_API bool JUCE_CALLTYPE Process::isRunningUnderDebugger()
{
    return juce_isRunningUnderDebugger();
}

void Process::raisePrivilege()
{
    // If running suid root, change effective user to root
    if (geteuid() != 0 && getuid() == 0)
    {
        setreuid (geteuid(), getuid());
        setregid (getegid(), getgid());
    }
}

void Process::lowerPrivilege()
{
    // If runing suid root, change effective user back to real user
    if (geteuid() == 0 && getuid() != 0)
    {
        setreuid (geteuid(), getuid());
        setregid (getegid(), getgid());
    }
}
