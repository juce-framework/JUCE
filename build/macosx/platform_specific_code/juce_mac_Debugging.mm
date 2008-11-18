/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE

//==============================================================================
void Logger::outputDebugString (const String& text) throw()
{
    const ScopedAutoReleasePool pool;
    NSLog (juceStringToNS (text + T("\n")));
}

void Logger::outputDebugPrintf (const tchar* format, ...) throw()
{
    String text;
    va_list args;
    va_start (args, format);
    text.vprintf(format, args);
    outputDebugString (text);
}

bool JUCE_CALLTYPE juce_isRunningUnderDebugger() throw()
{
    static char testResult = 0;

    if (testResult == 0)
    {
        struct kinfo_proc info;
        int m[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
        size_t sz = sizeof (info);
        sysctl (m, 4, &info, &sz, 0, 0);
        testResult = ((info.kp_proc.p_flag & P_TRACED) != 0) ? 1 : -1;
    }

    return testResult > 0;
}

bool JUCE_CALLTYPE Process::isRunningUnderDebugger() throw()
{
    return juce_isRunningUnderDebugger();
}


#endif
