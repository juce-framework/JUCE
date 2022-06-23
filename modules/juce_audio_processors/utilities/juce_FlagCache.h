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

#if ! DOXYGEN

namespace juce
{

template <size_t requiredFlagBitsPerItem>
class FlagCache
{
    using FlagType = uint32_t;

public:
    FlagCache() = default;

    explicit FlagCache (size_t items)
        : flags (divCeil (items, groupsPerWord))
    {
        std::fill (flags.begin(), flags.end(), 0);
    }

    void set (size_t index, FlagType bits)
    {
        const auto flagIndex = index / groupsPerWord;
        jassert (flagIndex < flags.size());
        const auto groupIndex = index - (flagIndex * groupsPerWord);
        flags[flagIndex].fetch_or (moveToGroupPosition (bits, groupIndex), std::memory_order_acq_rel);
    }

    /*  Calls the supplied callback for any entries with non-zero flags, and
        sets all flags to zero.
    */
    template <typename Callback>
    void ifSet (Callback&& callback)
    {
        for (size_t flagIndex = 0; flagIndex < flags.size(); ++flagIndex)
        {
            const auto prevFlags = flags[flagIndex].exchange (0, std::memory_order_acq_rel);

            for (size_t group = 0; group < groupsPerWord; ++group)
            {
                const auto masked = moveFromGroupPosition (prevFlags, group);

                if (masked != 0)
                    callback ((flagIndex * groupsPerWord) + group, masked);
            }
        }
    }

    void clear()
    {
        std::fill (flags.begin(), flags.end(), 0);
    }

private:
    /*  Given the flags for a single item, and a group index, shifts the flags
        so that they are positioned at the appropriate location for that group
        index.

        e.g. If the flag type is a uint32_t, and there are 2 flags per item,
        then each uint32_t will hold flags for 16 items. The flags for item 0
        are the least significant two bits; the flags for item 15 are the most
        significant two bits.
    */
    static constexpr FlagType moveToGroupPosition (FlagType ungrouped, size_t groupIndex)
    {
        return (ungrouped & groupMask) << (groupIndex * bitsPerFlagGroup);
    }

    /*  Given a set of grouped flags for multiple items, and a group index,
        extracts the flags set for an item at that group index.

        e.g. If the flag type is a uint32_t, and there are 2 flags per item,
        then each uint32_t will hold flags for 16 items. Asking for groupIndex
        0 will return the least significant two bits; asking for groupIndex 15
        will return the most significant two bits.
    */
    static constexpr FlagType moveFromGroupPosition (FlagType grouped, size_t groupIndex)
    {
        return (grouped >> (groupIndex * bitsPerFlagGroup)) & groupMask;
    }

    static constexpr size_t findNextPowerOfTwoImpl (size_t n, size_t shift)
    {
        return shift == 32 ? n : findNextPowerOfTwoImpl (n | (n >> shift), shift * 2);
    }

    static constexpr size_t findNextPowerOfTwo (size_t value)
    {
        return findNextPowerOfTwoImpl (value - 1, 1) + 1;
    }

    static constexpr size_t divCeil (size_t a, size_t b)
    {
        return (a / b) + ((a % b) != 0);
    }

    static constexpr size_t bitsPerFlagGroup = findNextPowerOfTwo (requiredFlagBitsPerItem);
    static constexpr size_t groupsPerWord = (8 * sizeof (FlagType)) / bitsPerFlagGroup;
    static constexpr FlagType groupMask = ((FlagType) 1 << requiredFlagBitsPerItem) - 1;

    std::vector<std::atomic<FlagType>> flags;
};

template <size_t requiredFlagBitsPerItem>
class FlaggedFloatCache
{
public:
    FlaggedFloatCache() = default;

    explicit FlaggedFloatCache (size_t sizeIn)
        : values (sizeIn),
          flags (sizeIn)
    {
        std::fill (values.begin(), values.end(), 0.0f);
    }

    size_t size() const noexcept { return values.size(); }

    void setValue (size_t index, float value)
    {
        jassert (index < size());
        values[index].store (value, std::memory_order_relaxed);
    }

    void setBits (size_t index, uint32_t bits) { flags.set (index, bits); }

    void setValueAndBits (size_t index, float value, uint32_t bits)
    {
        setValue (index, value);
        setBits (index, bits);
    }

    float get (size_t index) const noexcept
    {
        jassert (index < size());
        return values[index].load (std::memory_order_relaxed);
    }

    /*  Calls the supplied callback for any entries which have been modified
        since the last call to this function.
    */
    template <typename Callback>
    void ifSet (Callback&& callback)
    {
        flags.ifSet ([this, &callback] (size_t groupIndex, uint32_t bits)
        {
            callback (groupIndex, values[groupIndex].load (std::memory_order_relaxed), bits);
        });
    }

private:
    std::vector<std::atomic<float>> values;
    FlagCache<requiredFlagBitsPerItem> flags;
};

} // namespace juce

#endif
