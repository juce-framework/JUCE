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

//==============================================================================
/**
    Some shared helpers methods for using the high-performance audio paths on
    Android devices (OpenSL and Oboe).

    @tags{Audio}
*/
namespace juce::AndroidHighPerformanceAudioHelpers
{
    //==============================================================================
    static double getNativeSampleRate()
    {
        return audioManagerGetProperty ("android.media.property.OUTPUT_SAMPLE_RATE").getDoubleValue();
    }

    static int getNativeBufferSizeHint()
    {
        // This property is a hint of a native buffer size but it does not guarantee the size used.
        auto deviceBufferSize = audioManagerGetProperty ("android.media.property.OUTPUT_FRAMES_PER_BUFFER").getIntValue();

        if (deviceBufferSize == 0)
            return 192;

        return deviceBufferSize;
    }

    static bool isProAudioDevice()
    {
        static bool isSapaSupported = SystemStats::getDeviceManufacturer().containsIgnoreCase ("SAMSUNG")
                                     && DynamicLibrary().open ("libapa_jni.so");

        return androidHasSystemFeature ("android.hardware.audio.pro") || isSapaSupported;
    }

    static bool hasLowLatencyAudioPath()
    {
        return androidHasSystemFeature ("android.hardware.audio.low_latency");
    }

    static bool canUseHighPerformanceAudioPath (int nativeBufferSize, int requestedBufferSize, int requestedSampleRate)
    {
        return ((requestedBufferSize % nativeBufferSize) == 0)
               && approximatelyEqual ((double) requestedSampleRate, getNativeSampleRate())
               && isProAudioDevice();
    }

    //==============================================================================
    static int getMinimumBuffersToEnqueue (int nativeBufferSize, double requestedSampleRate)
    {
        if (canUseHighPerformanceAudioPath (nativeBufferSize, nativeBufferSize, (int) requestedSampleRate))
        {
            // see https://developer.android.com/ndk/guides/audio/opensl/opensl-prog-notes.html#sandp
            // > Beginning with Android 4.3 (API level 18), a buffer count of one is sufficient for lower latency.
            return 1;
        }

        // not using low-latency path so we can use the absolute minimum number of buffers to queue
        return 1;
    }

    static int buffersToQueueForBufferDuration (int nativeBufferSize, int bufferDurationInMs, double sampleRate) noexcept
    {
        auto maxBufferFrames = static_cast<int> (std::ceil (bufferDurationInMs * sampleRate / 1000.0));
        auto maxNumBuffers   = static_cast<int> (std::ceil (static_cast<double> (maxBufferFrames)
                                                  / static_cast<double> (nativeBufferSize)));

        return jmax (getMinimumBuffersToEnqueue (nativeBufferSize, sampleRate), maxNumBuffers);
    }

    static int getMaximumBuffersToEnqueue (int nativeBufferSize, double maximumSampleRate) noexcept
    {
        static constexpr int maxBufferSizeMs = 200;

        return jmax (8, buffersToQueueForBufferDuration (nativeBufferSize, maxBufferSizeMs, maximumSampleRate));
    }

    static Array<int> getAvailableBufferSizes (int nativeBufferSize, Array<double> availableSampleRates)
    {
        auto minBuffersToQueue = getMinimumBuffersToEnqueue (nativeBufferSize, getNativeSampleRate());
        auto maxBuffersToQueue = getMaximumBuffersToEnqueue (nativeBufferSize, findMaximum (availableSampleRates.getRawDataPointer(),
                                                                                            availableSampleRates.size()));

        Array<int> bufferSizes;

        for (int i = minBuffersToQueue; i <= maxBuffersToQueue; ++i)
            bufferSizes.add (i * nativeBufferSize);

        return bufferSizes;
    }

    static int getDefaultBufferSize (int nativeBufferSize, double currentSampleRate)
    {
        static constexpr int defaultBufferSizeForLowLatencyDeviceMs = 40;
        static constexpr int defaultBufferSizeForStandardLatencyDeviceMs = 100;

        auto defaultBufferLength = (hasLowLatencyAudioPath() ? defaultBufferSizeForLowLatencyDeviceMs
                                                             : defaultBufferSizeForStandardLatencyDeviceMs);

        auto defaultBuffersToEnqueue = buffersToQueueForBufferDuration (nativeBufferSize, defaultBufferLength, currentSampleRate);
        return defaultBuffersToEnqueue * nativeBufferSize;
    }

} // namespace juce::AndroidHighPerformanceAudioHelpers
