/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace
{
    struct PortIOStats
    {
        PortIOStats (const char* nm) : name (nm) {}

        const char* const name;
        int byteCount           = 0;
        int messageCount        = 0;
        int bytesPerSec         = 0;
        int largestMessageBytes = 0;
        int lastMessageBytes    = 0;

        void update (double elapsedSec)
        {
            if (byteCount > 0)
            {
                bytesPerSec = (int) (byteCount / elapsedSec);
                byteCount = 0;
                juce::Logger::writeToLog (getString());
            }
        }

        juce::String getString() const
        {
            return juce::String (name) + ": "
            + "count="    + juce::String (messageCount).paddedRight (' ', 7)
            + "rate="     + (juce::String (bytesPerSec / 1024.0f, 1) + " Kb/sec").paddedRight (' ', 11)
            + "largest="  + (juce::String (largestMessageBytes) + " bytes").paddedRight (' ', 11)
            + "last="     + (juce::String (lastMessageBytes) + " bytes").paddedRight (' ', 11);
        }

        void registerMessage (int numBytes) noexcept
        {
            byteCount += numBytes;
            ++messageCount;
            lastMessageBytes = numBytes;
            largestMessageBytes = juce::jmax (largestMessageBytes, numBytes);
        }
    };

    static PortIOStats inputStats { "Input" }, outputStats { "Output" };
    static uint32 startTime = 0;

    static inline void resetOnSecondBoundary()
    {
        auto now = juce::Time::getMillisecondCounter();
        double elapsedSec = (now - startTime) / 1000.0;

        if (elapsedSec >= 1.0)
        {
            inputStats.update (elapsedSec);
            outputStats.update (elapsedSec);
            startTime = now;
        }
    }

    static inline void registerBytesOut (int numBytes)
    {
        outputStats.registerMessage (numBytes);
        resetOnSecondBoundary();
    }

    static inline void registerBytesIn (int numBytes)
    {
        inputStats.registerMessage (numBytes);
        resetOnSecondBoundary();
    }
}

juce::String getMidiIOStats()
{
    return inputStats.getString() + "   " + outputStats.getString();
}

} // namespace juce
