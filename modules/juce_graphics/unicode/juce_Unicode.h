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
enum class TextBreakType { none, soft, hard };
enum class TextDirection { ltr, rtl };

struct Unicode
{
    struct Codepoint
    {
        auto getLogicalIndex() const { return logicalIndex; }
        auto getVisualIndex()  const { return visualIndex; }

        uint32_t codepoint;

        // Index of the character in the source string
        size_t logicalIndex;
        size_t visualIndex;

        // Breaking characteristics of this codepoint
        TextBreakType breaking;

        // Direction of this codepoint
        TextDirection direction;

        // Script class for this codepoint
        TextScript script;
    };

    template <typename Value>
    static auto prefix (Span<Value> v, size_t num)
    {
        return Span { v.data(), std::min (v.size(), num) };
    }

    template <typename Value>
    static auto removePrefix (Span<Value> v, size_t num)
    {
        const auto increment = std::min (v.size(), num);
        return Span { v.data() + increment, v.size() - increment };
    }

    static Array<Codepoint> performAnalysis (const String&);
    static Array<Codepoint> convertLogicalToVisual (Span<const Codepoint>);

    template <typename Traits>
    struct Iterator
    {
        using ValueType = typename Traits::ValueType;

        Iterator() = default;
        explicit Iterator (Span<ValueType> s) : data (s) {}

        std::optional<Span<ValueType>> next()
        {
            if (data.empty())
                return {};

            const auto breakpoint = std::find_if (data.begin(), data.end(), [&] (const auto& i)
            {
                return ! Traits::compare (i, data.front());
            });
            const auto lengthToBreak = (size_t) std::distance (data.begin(), breakpoint) + (Traits::includeBreakingIndex() ? 1 : 0);
            const ScopeGuard scope { [&] { data = removePrefix (data, lengthToBreak); } };
            return prefix (data, lengthToBreak);
        }

    private:
        Span<ValueType> data;
    };

    struct BidiTraits
    {
        using ValueType = const Codepoint;

        static bool compare (const Codepoint& t1, const Codepoint& t2)
        {
            return t1.direction == t2.direction;
        }

        static bool includeBreakingIndex() { return false; }
    };

    using BidiRunIterator = Iterator<BidiTraits>;

    struct LineTraits
    {
        using ValueType = const Codepoint;

        static bool compare (const Codepoint& t1, const Codepoint&)
        {
            return t1.breaking != TextBreakType::hard;
        }

        static bool includeBreakingIndex() { return true; }
    };

    using LineBreakIterator = Iterator<LineTraits>;

    struct WordTraits
    {
        using ValueType = const Codepoint;

        static bool compare (const Codepoint& t1, const Codepoint&)
        {
            return t1.breaking != TextBreakType::soft;
        }

        static bool includeBreakingIndex() { return false; }
    };

    using WordBreakIterator = Iterator<WordTraits>;

    struct ScriptTraits
    {
        using ValueType = const Codepoint;

        static bool compare (const Codepoint& t1, const Codepoint& t2)
        {
            return t1.script == t2.script;
        }

        static bool includeBreakingIndex() { return false; }
    };

    using ScriptRunIterator = Iterator<ScriptTraits>;
};

struct UnicodeFunctions
{
    static bool isRenderableCharacter (juce_wchar character);
    static bool isBreakableWhitespace (juce_wchar character);
    static bool isEmoji (juce_wchar character);
    static bool shouldVerticalGlyphRotate (juce_wchar character);
};
} // namespace juce
