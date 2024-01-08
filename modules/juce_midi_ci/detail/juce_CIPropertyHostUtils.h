/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::midi_ci::detail
{

struct PropertyHostUtils
{
    PropertyHostUtils() = delete;

    static void send (BufferOutput& output,
                      uint8_t group,
                      std::byte subID2,
                      MUID targetMuid,
                      std::byte requestID,
                      Span<const std::byte> header,
                      Span<const std::byte> body,
                      int chunkSize)
    {
        MemoryInputStream stream (body.data(), body.size(), false);
        const detail::PropertyDataMessageChunker chunker { output.getOutputBuffer(),
                                                           std::min (chunkSize, 1 << 16),
                                                           subID2,
                                                           requestID,
                                                           header,
                                                           output.getMuid(),
                                                           targetMuid,
                                                           stream };

        std::for_each (chunker.begin(), chunker.end(), [&] (auto) { output.send (group); });
    }
};
} // namespace juce::midi_ci::detail
