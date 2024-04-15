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

#if JUCE_DIRECT2D_METRICS

namespace juce
{

String Direct2DMetricsHub::getProcessString() noexcept
{
    auto processID = GetCurrentProcessId();
    return String::toHexString ((pointer_sized_int) processID);
}

void Direct2DMetricsHub::HubPipeServer::messageReceived (const MemoryBlock& message)
{
    int requestType = *(int*) message.getData();
    switch (requestType)
    {
        case getValuesRequest:
        {
            ScopedLock locker { owner.lock };

            auto foregroundWindow = GetForegroundWindow();
            Direct2DMetrics::Ptr metrics = nullptr;
            for (int i = 0; i < owner.metricsArray.size(); ++i)
            {
                auto arrayEntry = owner.metricsArray[i];
                if (arrayEntry->windowHandle && arrayEntry->windowHandle == foregroundWindow)
                {
                    metrics = arrayEntry;
                    break;
                }
            }

            if (! metrics)
            {
                if (owner.lastMetrics && owner.metricsArray.contains (owner.lastMetrics))
                    metrics = owner.lastMetrics;
            }

            if (metrics)
            {
                MemoryBlock block { sizeof (GetValuesResponse), true };

                auto* response = (GetValuesResponse*) block.getData();
                response->responseType = getValuesRequest;
                response->windowHandle = metrics->windowHandle;

                for (size_t i = 0; i <= Direct2DMetrics::drawGlyphRunTime; ++i)
                {
                    auto& accumulator = metrics->getAccumulator (i);
                    response->values[i].count = accumulator.getCount();
                    response->values[i].total = metrics->getSum (i);
                    response->values[i].average = accumulator.getAverage();
                    response->values[i].minimum = accumulator.getMinValue();
                    response->values[i].maximum = accumulator.getMaxValue();
                    response->values[i].stdDev = accumulator.getStandardDeviation();
                }

                // Track bitmap operations common to all device contexts
                for (size_t i = Direct2DMetrics::createBitmapTime; i <= Direct2DMetrics::unmapBitmapTime; ++i)
                {
                    auto& accumulator = owner.imageContextMetrics->getAccumulator (i);
                    response->values[i].count = accumulator.getCount();
                    response->values[i].total = metrics->getSum (i);
                    response->values[i].average = accumulator.getAverage();
                    response->values[i].minimum = accumulator.getMinValue();
                    response->values[i].maximum = accumulator.getMaxValue();
                    response->values[i].stdDev = accumulator.getStandardDeviation();
                }

                sendMessage (block);

                owner.lastMetrics = metrics.get();
            }
            break;
        }

        case resetValuesRequest:
        {
            owner.resetAll();
            break;
        }
    }
}

void Direct2DMetricsHub::resetAll()
{
    ScopedLock locker { lock };

    imageContextMetrics->reset();
    for (auto metrics : metricsArray)
    {
        metrics->reset();
    }
}

} // namespace juce

#endif
