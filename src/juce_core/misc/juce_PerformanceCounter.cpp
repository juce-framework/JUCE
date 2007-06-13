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

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_PerformanceCounter.h"
#include "../basics/juce_Time.h"


//==============================================================================
PerformanceCounter::PerformanceCounter (const String& name_,
                                        int runsPerPrintout,
                                        const File& loggingFile)
    : name (name_),
      numRuns (0),
      runsPerPrint (runsPerPrintout),
      totalTime (0),
      outputFile (loggingFile)
{
    if (outputFile != File::nonexistent)
    {
        String s (T("**** Counter for \""));
        s += name_ + T("\" started at: ");
        s += Time::getCurrentTime().toString (true, true) + T("\r\n");
        outputFile.appendText (s, false, false);
    }
}

PerformanceCounter::~PerformanceCounter()
{
    printStatistics();
}

void PerformanceCounter::start()
{
    started = Time::getHighResolutionTicks();
}

void PerformanceCounter::stop()
{
    const int64 now = Time::getHighResolutionTicks();

    totalTime += 1000.0 * Time::highResolutionTicksToSeconds (now - started);

    if (++numRuns == runsPerPrint)
        printStatistics();
}

void PerformanceCounter::printStatistics()
{
    if (numRuns > 0)
    {
        String s (T("Performance count for \""));
        s << name << T("\" - average over ") << numRuns << T(" run(s) = ");

        const int micros = (int)(totalTime * (1000.0 / numRuns));

        if (micros > 10000)
            s << (micros/1000) << T(" millisecs");
        else
            s << micros << T(" microsecs");

        s << (", total = ") << String (totalTime / 1000, 5) << T(" seconds");

        Logger::outputDebugString (s);

        if (outputFile != File::nonexistent)
            outputFile.appendText (s + T("\r\n"), false, false);

        numRuns = 0;
        totalTime = 0;
    }
}

END_JUCE_NAMESPACE
