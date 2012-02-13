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

#ifndef __JUCE_PERFORMANCECOUNTER_JUCEHEADER__
#define __JUCE_PERFORMANCECOUNTER_JUCEHEADER__

#include "../files/juce_File.h"


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
                        const File& loggingFile = File::nonexistent);

    /** Destructor. */
    ~PerformanceCounter();

    //==============================================================================
    /** Starts timing.

        @see stop
    */
    void start();

    /** Stops timing and prints out the results.

        The number of iterations before doing a printout of the
        results is set in the constructor.

        @see start
    */
    void stop();

    /** Dumps the current metrics to the debugger output and to a file.

        As well as using Logger::outputDebugString to print the results,
        this will write then to the file specified in the constructor (if
        this was valid).
    */
    void printStatistics();

private:
    //==============================================================================
    String name;
    int numRuns, runsPerPrint;
    double totalTime;
    int64 started;
    File outputFile;
};

#endif   // __JUCE_PERFORMANCECOUNTER_JUCEHEADER__
