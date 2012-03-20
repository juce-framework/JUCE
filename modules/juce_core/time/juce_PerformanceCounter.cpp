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

PerformanceCounter::PerformanceCounter (const String& name_,
                                        const int runsPerPrintout,
                                        const File& loggingFile)
    : name (name_),
      numRuns (0),
      runsPerPrint (runsPerPrintout),
      totalTime (0),
      outputFile (loggingFile)
{
    if (outputFile != File::nonexistent)
    {
        String s ("**** Counter for \"");
        s << name_ << "\" started at: "
          << Time::getCurrentTime().toString (true, true)
          << newLine;

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
        String s ("Performance count for \"");
        s << name << "\" - average over " << numRuns << " run(s) = ";

        const int micros = (int) (totalTime * (1000.0 / numRuns));

        if (micros > 10000)
            s << (micros/1000) << " millisecs";
        else
            s << micros << " microsecs";

        s << ", total = " << String (totalTime / 1000, 5) << " seconds";

        Logger::outputDebugString (s);

        s << newLine;

        if (outputFile != File::nonexistent)
            outputFile.appendText (s, false, false);

        numRuns = 0;
        totalTime = 0;
    }
}
