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
/*
    A collection of methods and types for breaking down text into a unicode representation.
*/
class Unicode
{
    using Key = String;

public:
    Unicode() = delete;

    //==============================================================================
    /*  A unicode Codepoint, from this you can infer various Unicode properties such
        as direction, logical string index and breaking type, etc.
    */
    struct Codepoint
    {
        uint32_t codepoint;

        TextBreakType breaking;  // Breaking characteristics of this codepoint

        TextScript script;       // Script class for this codepoint
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

    //==============================================================================
    /*  Performs unicode analysis on a piece of text and returns an array of Codepoints
        in logical order.
    */
    static Array<Codepoint> performAnalysis (const String& string)
    {
        if (string.isEmpty())
            return {};

        thread_local LruCache<Key, Array<Unicode::Codepoint>> cache;

        return cache.get (string, analysisCallback);
    }

    //==============================================================================
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

private:
    static Array<Unicode::Codepoint> analysisCallback (const Key& key)
    {
        auto analysisBuffer = [&key]
        {
            std::vector<UnicodeAnalysisPoint> points;

            const auto data   = key.toUTF32();
            const auto length = data.length();

            points.reserve (length);

            std::transform (data.getAddress(), data.getAddress() + length, std::back_inserter (points), [] (uint32_t cp)
            {
                UnicodeAnalysisPoint p { cp, UnicodeDataTable::getDataForCodepoint (cp) };

                // Define this to enable TR9 debugging. All upper case
                // characters will be interpreted as right-to-left.
               #if defined (JUCE_TR9_UPPERCASE_IS_RTL)
                if (65 <= cp && cp <= 90)
                    p.data.bidi = BidiType::al;
               #endif

                return p;
            });

            return points;
        }();

        Array<Unicode::Codepoint> result;
        result.resize ((int) analysisBuffer.size());

        for (size_t i = 0; i < analysisBuffer.size(); i++)
            result.getReference ((int) i).codepoint = analysisBuffer[i].character;

        TR24::analyseScripts (analysisBuffer, [&result] (int index, TextScript script)
        {
            result.getReference (index).script = script;
        });

        TR14::analyseLineBreaks (analysisBuffer, [&result] (int index, TextBreakType type)
        {
            result.getReference (index).breaking = type;
        });

        return result;
    }
};

namespace detail
{

std::vector<int> UnicodeHelpers::getLineBreaks (const String& data)
{
    std::vector<int> lineBreaks;
    const auto analysis = Unicode::performAnalysis (data);

    for (const auto [index, codepoint] : enumerate (analysis, int{}))
    {
        if (codepoint.breaking == TextBreakType::hard)
            lineBreaks.push_back (index);
    }

    return lineBreaks;
}

} // namespace detail

} // namespace juce
