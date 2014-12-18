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

static void appendToFile (const File& f, const String& s)
{
    if (f.getFullPathName().isNotEmpty())
    {
        FileOutputStream out (f);

        if (! out.failedToOpen())
            out << s << newLine;
    }
}

PerformanceCounter::PerformanceCounter (const String& name, int runsPerPrintout, const File& loggingFile)
    : runsPerPrint (runsPerPrintout), startTime (0), outputFile (loggingFile)
{
    stats.name = name;
    appendToFile (outputFile, "**** Counter for \"" + name + "\" started at: " + Time::getCurrentTime().toString (true, true));
}

PerformanceCounter::~PerformanceCounter()
{
    printStatistics();
}

PerformanceCounter::Statistics::Statistics() noexcept
    : averageSeconds(), maximumSeconds(), minimumSeconds(), totalSeconds(), numRuns()
{
}

void PerformanceCounter::Statistics::clear() noexcept
{
    averageSeconds = maximumSeconds = minimumSeconds = totalSeconds = 0;
    numRuns = 0;
}

void PerformanceCounter::Statistics::addResult (double elapsed) noexcept
{
    if (numRuns == 0)
    {
        maximumSeconds = elapsed;
        minimumSeconds = elapsed;
    }
    else
    {
        maximumSeconds = jmax (maximumSeconds, elapsed);
        minimumSeconds = jmin (minimumSeconds, elapsed);
    }

    ++numRuns;
    totalSeconds += elapsed;
}

static String timeToString (double secs)
{
    return String ((int64) (secs * (secs < 0.01 ? 1000000.0 : 1000.0) + 0.5))
                    + (secs < 0.01 ? " microsecs" : " millisecs");
}

String PerformanceCounter::Statistics::toString() const
{
    MemoryOutputStream s;

    s << "Performance count for \"" << name << "\" over " << numRuns << " run(s)" << newLine
      << "Average = "   << timeToString (averageSeconds)
      << ", minimum = " << timeToString (minimumSeconds)
      << ", maximum = " << timeToString (maximumSeconds)
      << ", total = "   << timeToString (totalSeconds);

    return s.toString();
}

void PerformanceCounter::start() noexcept
{
    startTime = Time::getHighResolutionTicks();
}

bool PerformanceCounter::stop()
{
    stats.addResult (Time::highResolutionTicksToSeconds (Time::getHighResolutionTicks() - startTime));

    if (stats.numRuns < runsPerPrint)
        return false;

    printStatistics();
    return true;
}

void PerformanceCounter::printStatistics()
{
    const String desc (getStatisticsAndReset().toString());

    Logger::outputDebugString (desc);
    appendToFile (outputFile, desc);
}

PerformanceCounter::Statistics PerformanceCounter::getStatisticsAndReset()
{
    Statistics s (stats);
    stats.clear();

    if (s.numRuns > 0)
        s.averageSeconds = s.totalSeconds / s.numRuns;

    return s;
}
