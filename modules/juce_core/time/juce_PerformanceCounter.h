/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

    @tags{Core}
*/
class JUCE_API  PerformanceCounter
{
public:
    //==============================================================================
    /** Creates a PerformanceCounter object.

        @param counterName      the name used when printing out the statistics
        @param runsPerPrintout  the number of start/stop iterations before calling
                                printStatistics()
        @param loggingFile      a file to dump the results to - if this is File(),
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


//==============================================================================
/**
    Simple RAII class for measuring the time spent in a scope.

    Example:

    {
        double timeSec;

        {
            ScopedTimeMeasurement m (timeSec);
            doSomething();
        }

        Logger::writeToLog ("doSomething() took " + String (timeSec) + "seconds");
    }

    @param resultInSeconds The result of the measurement will be stored in this variable.

    @tags{Core}
*/
class JUCE_API  ScopedTimeMeasurement
{
public:
    ScopedTimeMeasurement (double& resultInSeconds) noexcept
        : result (resultInSeconds)
    {
        result = 0.0;
    }

    ~ScopedTimeMeasurement()
    {
        static auto scaler = 1.0 / static_cast<double> (Time::getHighResolutionTicksPerSecond());
        result = static_cast<double> (Time::getHighResolutionTicks() - startTimeTicks) * scaler;
    }

private:
    int64 startTimeTicks = Time::getHighResolutionTicks();
    double& result;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedTimeMeasurement)
};

} // namespace juce
