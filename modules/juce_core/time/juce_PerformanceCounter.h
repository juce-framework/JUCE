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

#ifndef JUCE_PERFORMANCECOUNTER_H_INCLUDED
#define JUCE_PERFORMANCECOUNTER_H_INCLUDED


//==============================================================================
/** A timer for measuring performance of code and dumping the results to a file.

    e.g. @code

        PerformanceCounter pc ("fish", 50, "/temp/myfishlog.txt");

        for (;;)
        {
            pc.start();

            doSomethingFishy();

            pc.stop();
        }
    @endcode

    In this example, the time of each period between calling start/stop will be
    measured and averaged over 50 runs, and the results printed to a file
    every 50 times round the loop.
*/
class JUCE_API  PerformanceCounter
{
public:
    //==============================================================================
    /** Creates a PerformanceCounter object.

        @param counterName      the name used when printing out the statistics
        @param runsPerPrintout  the number of start/stop iterations before calling
                                printStatistics()
        @param loggingFile      a file to dump the results to - if this is File::nonexistent,
                                the results are just written to the debugger output
    */
    PerformanceCounter (const String& counterName,
                        int runsPerPrintout = 100,
                        const File& loggingFile = File());

    /** Destructor. */
    ~PerformanceCounter();

    //==============================================================================
    /** Starts timing.
        @see stop
    */
    void start() noexcept;

    /** Stops timing and prints out the results.

        The number of iterations before doing a printout of the
        results is set in the constructor.

        @see start
    */
    bool stop();

    /** Dumps the current metrics to the debugger output and to a file.

        As well as using Logger::outputDebugString to print the results,
        this will write then to the file specified in the constructor (if
        this was valid).
    */
    void printStatistics();

    /** Holds the current statistics. */
    struct Statistics
    {
        Statistics() noexcept;

        void clear() noexcept;
        String toString() const;

        void addResult (double elapsed) noexcept;

        String name;
        double averageSeconds;
        double maximumSeconds;
        double minimumSeconds;
        double totalSeconds;
        int64 numRuns;
    };

    /** Returns a copy of the current stats, and resets the internal counter. */
    Statistics getStatisticsAndReset();

private:
    //==============================================================================
    Statistics stats;
    int64 runsPerPrint, startTime;
    File outputFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerformanceCounter)
};


#endif   // JUCE_PERFORMANCECOUNTER_H_INCLUDED
