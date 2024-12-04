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
/**
    Maintains an ongoing measurement of the proportion of time which is being
    spent inside an audio callback.

    @tags{Audio}
*/
class JUCE_API  AudioProcessLoadMeasurer
{
public:
    /** */
    AudioProcessLoadMeasurer();

    /** Destructor. */
    ~AudioProcessLoadMeasurer();

    //==============================================================================
    /** Resets the state. */
    void reset();

    /** Resets the counter, in preparation for use with the given sample rate and block size. */
    void reset (double sampleRate, int blockSize);

    /** Returns the current load as a proportion 0 to 1.0 */
    double getLoadAsProportion() const;

    /** Returns the current load as a percentage 0 to 100.0 */
    double getLoadAsPercentage() const;

    /** Returns the number of over- (or under-) runs recorded since the state was reset. */
    int getXRunCount() const;

    //==============================================================================
    /** This class measures the time between its construction and destruction and
        adds it to an AudioProcessLoadMeasurer.

        e.g.
        @code
        {
            AudioProcessLoadMeasurer::ScopedTimer timer (myProcessLoadMeasurer);
            myCallback->doTheCallback();
        }
        @endcode

        @tags{Audio}
    */
    struct JUCE_API  ScopedTimer
    {
        ScopedTimer (AudioProcessLoadMeasurer&);
        ScopedTimer (AudioProcessLoadMeasurer&, int numSamplesInBlock);
        ~ScopedTimer();

    private:
        AudioProcessLoadMeasurer& owner;
        double startTime;
        int samplesInBlock;

        JUCE_DECLARE_NON_COPYABLE (ScopedTimer)
    };

    /** Can be called manually to add the time of a callback to the stats.
        Normally you probably would never call this - it's simpler and more robust to
        use a ScopedTimer to measure the time using an RAII pattern.
    */
    void registerBlockRenderTime (double millisecondsTaken);

    /** Can be called manually to add the time of a callback to the stats.
        Normally you probably would never call this - it's simpler and more robust to
        use a ScopedTimer to measure the time using an RAII pattern.
    */
    void registerRenderTime (double millisecondsTaken, int numSamples);

private:
    void registerRenderTimeLocked (double, int);

    SpinLock mutex;
    int samplesPerBlock = 0;
    double msPerSample = 0;
    std::atomic<double> cpuUsageProportion { 0 };
    std::atomic<int> xruns { 0 };
};


} // namespace juce
