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

#pragma once

#if JUCE_DIRECT2D_METRICS

namespace juce
{

struct Direct2DMetrics : public ReferenceCountedObject
{
    using Ptr = ReferenceCountedObjectPtr<Direct2DMetrics>;

#define DIRECT2D_PAINT_STAT_LIST                          \
    DIRECT2D_PAINT_STAT (messageThreadPaintDuration)      \
    DIRECT2D_PAINT_STAT (swapChainThreadTime)             \
    DIRECT2D_PAINT_STAT (frameInterval)                   \
    DIRECT2D_PAINT_STAT (endDrawDuration)                 \
    DIRECT2D_PAINT_STAT (present1Duration)                \
    DIRECT2D_PAINT_STAT (createGeometryTime)              \
    DIRECT2D_PAINT_STAT (drawGeometryTime)                \
    DIRECT2D_PAINT_STAT (fillGeometryTime)                \
    DIRECT2D_PAINT_STAT (createFilledGRTime)              \
    DIRECT2D_PAINT_STAT (createStrokedGRTime)             \
    DIRECT2D_PAINT_STAT (drawGRTime)                      \
    DIRECT2D_PAINT_STAT (createGradientTime)              \
    DIRECT2D_PAINT_STAT (pushAliasedAxisAlignedLayerTime) \
    DIRECT2D_PAINT_STAT (pushGeometryLayerTime)           \
    DIRECT2D_PAINT_STAT (fillTranslatedRectTime)          \
    DIRECT2D_PAINT_STAT (fillAxisAlignedRectTime)         \
    DIRECT2D_PAINT_STAT (fillTransformedRectTime)         \
    DIRECT2D_PAINT_STAT (fillRectListTime)                \
    DIRECT2D_PAINT_STAT (drawImageTime)                   \
    DIRECT2D_PAINT_STAT (spriteBatchTime)                 \
    DIRECT2D_PAINT_STAT (spriteBatchSetupTime)            \
    DIRECT2D_PAINT_STAT (createSpriteSourceTime)          \
    DIRECT2D_PAINT_STAT (setSpritesTime)                  \
    DIRECT2D_PAINT_STAT (addSpritesTime)                  \
    DIRECT2D_PAINT_STAT (clearSpritesTime)                \
    DIRECT2D_PAINT_STAT (drawSpritesTime)                 \
    DIRECT2D_PAINT_STAT (drawGlyphRunTime)                \
    DIRECT2D_PAINT_STAT (createBitmapTime)                \
    DIRECT2D_PAINT_STAT (mapBitmapTime)                   \
    DIRECT2D_LAST_PAINT_STAT (unmapBitmapTime)

#define DIRECT2D_PAINT_STAT(name) name,
#define DIRECT2D_LAST_PAINT_STAT(name) name
    enum
    {
        DIRECT2D_PAINT_STAT_LIST,
        numStats
    };
#undef DIRECT2D_PAINT_STAT
#undef DIRECT2D_LAST_PAINT_STAT

#define DIRECT2D_PAINT_STAT(name) #name,
#define DIRECT2D_LAST_PAINT_STAT(name) #name
    StringArray const accumulatorNames { DIRECT2D_PAINT_STAT_LIST };
#undef DIRECT2D_PAINT_STAT
#undef DIRECT2D_LAST_PAINT_STAT

    CriticalSection& lock;
    String const name;
    void* const windowHandle;
    int64 const creationTime = Time::getMillisecondCounter();
    double const millisecondsPerTick = 1000.0 / (double) Time::getHighResolutionTicksPerSecond();
    int paintCount = 0;
    int presentCount = 0;
    int present1Count = 0;
    int64 lastPaintStartTicks = 0;
    uint64 lockAcquireMaxTicks = 0;

    Direct2DMetrics (CriticalSection& lockIn, String nameIn, void* windowHandleIn)
        : lock (lockIn),
          name (nameIn),
          windowHandle (windowHandleIn)
    {
    }

    ~Direct2DMetrics() = default;

    void startFrame()
    {
        ScopedLock locker { lock };
        zerostruct (sums);
    }

    void finishFrame()
    {
    }

    void reset()
    {
        ScopedLock locker { lock };

        for (auto& accumulator : runningAccumulators)
            accumulator.reset();

        lastPaintStartTicks = 0;
        paintCount = 0;
        present1Count = 0;
        lockAcquireMaxTicks = 0;
    }

    auto& getAccumulator (size_t index) noexcept
    {
        return runningAccumulators[index];
    }

    auto getSum (size_t index) const noexcept
    {
        return sums[index];
    }

    void addValueTicks (size_t index, int64 ticks)
    {
        addValueMsec (index, Time::highResolutionTicksToSeconds (ticks) * 1000.0);
    }

    void addValueMsec (size_t index, double value)
    {
        ScopedLock locker { lock };

        auto& accumulator = runningAccumulators[index];

        switch (index)
        {
            case frameInterval:
                if (accumulator.getCount() > 100)
                {
                    accumulator.reset();
                }
                break;
        }
        accumulator.addValue (value);

        sums[index] += value;
    }

private:
    std::array<StatisticsAccumulator<double>, numStats> runningAccumulators;
    std::array<double, numStats> sums;
};

struct Direct2DScopedElapsedTime
{
    Direct2DScopedElapsedTime (Direct2DMetrics::Ptr& metricsIn, size_t accumulatorIndexIn)
        : metrics (metricsIn.get()),
          accumulatorIndex (accumulatorIndexIn)
    {
    }

    Direct2DScopedElapsedTime (Direct2DMetrics* metricsIn, size_t accumulatorIndexIn)
        : metrics (metricsIn),
          accumulatorIndex (accumulatorIndexIn)
    {
    }

    ~Direct2DScopedElapsedTime()
    {
        auto finishTicks = Time::getHighResolutionTicks();
        metrics->addValueTicks (accumulatorIndex, finishTicks - startTicks);
    }

    int64 startTicks = Time::getHighResolutionTicks();
    Direct2DMetrics* metrics;
    size_t accumulatorIndex;
};

class Direct2DMetricsHub : public DeletedAtShutdown
{
public:
    Direct2DMetricsHub()
    {
        imageContextMetrics = new Direct2DMetrics { lock, "Image " + getProcessString(), nullptr };
        add (imageContextMetrics);
    }

    ~Direct2DMetricsHub() override
    {
        clearSingletonInstance();
    }

    void add (Direct2DMetrics::Ptr metrics)
    {
        metricsArray.insert (0, metrics);
    }

    void remove (Direct2DMetrics::Ptr metrics)
    {
        metricsArray.removeObject (metrics);
    }

    Direct2DMetrics::Ptr getMetricsForWindowHandle (void* windowHandle) noexcept
    {
        for (auto& metrics : metricsArray)
            if (metrics->windowHandle == windowHandle)
                return metrics;

        return nullptr;
    }

    enum
    {
        getValuesRequest,
        resetValuesRequest
    };

    struct MetricValues
    {
        size_t count;
        double total;
        double average;
        double minimum;
        double maximum;
        double stdDev;
    };

    struct GetValuesResponse
    {
        int responseType;
        void* windowHandle;
        MetricValues values[Direct2DMetrics::numStats];
    };

    CriticalSection lock;
    Direct2DMetrics::Ptr imageContextMetrics;

    static constexpr int magicNumber = 0xd2d1;

    JUCE_DECLARE_SINGLETON_INLINE (Direct2DMetricsHub, false)

private:
    static String getProcessString() noexcept;

    void resetAll();

    struct HubPipeServer : public InterprocessConnection
    {
        explicit HubPipeServer (Direct2DMetricsHub& ownerIn)
            : InterprocessConnection (false, magicNumber),
              owner (ownerIn)
        {
            createPipe ("JUCEDirect2DMetricsHub:" + owner.getProcessString(), -1, true);
        }

        ~HubPipeServer() override
        {
            disconnect();
        }

        void connectionMade() override
        {
        }

        void connectionLost() override
        {
        }

        void messageReceived (const MemoryBlock& message) override;

        Direct2DMetricsHub& owner;
    };

    HubPipeServer hubPipeServer { *this };
    ReferenceCountedArray<Direct2DMetrics> metricsArray;
    Direct2DMetrics* lastMetrics = nullptr;
};

} // namespace juce

#define JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, name) juce::Direct2DScopedElapsedTime scopedElapsedTime_##name { metrics, juce::Direct2DMetrics::name };

#else

namespace juce
{

struct Direct2DMetrics : public ReferenceCountedObject
{
    using Ptr = ReferenceCountedObjectPtr<Direct2DMetrics>;
};

} // namespace juce

#define JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, name)

#endif
