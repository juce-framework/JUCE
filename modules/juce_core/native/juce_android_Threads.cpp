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
// sets the process to 0=low priority, 1=normal, 2=high, 3=realtime
void Process::setPriority (ProcessPriority prior)
{
    // TODO

    struct sched_param param;
    int policy, maxp, minp;

    const int p = (int) prior;

    if (p <= 1)
        policy = SCHED_OTHER;
    else
        policy = SCHED_RR;

    minp = sched_get_priority_min (policy);
    maxp = sched_get_priority_max (policy);

    if (p < 2)
        param.sched_priority = 0;
    else if (p == 2 )
        // Set to middle of lower realtime priority range
        param.sched_priority = minp + (maxp - minp) / 4;
    else
        // Set to middle of higher realtime priority range
        param.sched_priority = minp + (3 * (maxp - minp) / 4);

    pthread_setschedparam (pthread_self(), policy, &param);
}

void Process::terminate()
{
    // TODO
    exit (0);
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger()
{
    return false;
}

JUCE_API bool JUCE_CALLTYPE Process::isRunningUnderDebugger()
{
    return juce_isRunningUnderDebugger();
}

void Process::raisePrivilege() {}
void Process::lowerPrivilege() {}
