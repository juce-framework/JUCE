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

class TR9
{
public:
    TR9() = delete;

    struct BidiOutput
    {
        int embeddingLevel = -1;
        std::vector<int> resolvedLevels;
        std::vector<int> visualOrder;
    };

    static void analyseBidiRun (BidiOutput& output,
                                Span<UnicodeAnalysisPoint> stream,
                                std::optional<TextDirection> directionOverride = {})
    {
        // BD1
        const auto baseLevel = directionOverride.has_value() ? (*directionOverride == TextDirection::rtl ? 1 : 0)
                                                             : resolveParagraphEmbeddingLevel (stream);

        std::for_each (stream.begin(), stream.end(), [baseLevel] (auto& atom)
        {
            atom.bidiLevel = (uint16_t) baseLevel;
        });

        resolveExplicitLevels (stream, baseLevel);

        // X9 replace override characters
        const auto pred = [] (auto atom) { return contains ({ BidiType::rle, BidiType::lre,
                                                              BidiType::lro, BidiType::rlo,
                                                              BidiType::pdf }, atom.getBidiType()); };

        std::for_each (stream.begin(),
                       stream.end(),
                       [&pred] (auto& atom)
                       {
                           if (! pred (atom))
                               return;

                           auto copy = atom;
                           copy.setBidiType (BidiType::bn);

                           atom = copy;
                       });

        // W1-W7
        resolveWeakTypes (stream, {});

        // N0-N2
        resolveNeutralTypes (stream, baseLevel, {});

        // I1-I2
        resolveImplicitTypes (stream, baseLevel, {});

        output.embeddingLevel = baseLevel;

        output.resolvedLevels.clear();

        for (const auto& atom : stream)
            output.resolvedLevels.push_back (atom.bidiLevel);

        resolveReorderedIndices (output.visualOrder, stream, baseLevel, {});
    }

private:
    enum EmbeddingLevel
    {
        left  = 0,
        right = 1
    };

    static auto isOdd (int level)                 { return bool (level & 1); }
    static auto computeLeastEven (int level)      { return isOdd (level) ? level + 1 : level + 2; }
    static auto computeLeastOdd (int level)       { return isOdd (level) ? level + 2 : level + 1; }
    static auto getEmbeddingDirection (int level) { return isOdd (level) ? BidiType::rtl : BidiType::ltr; }

    static bool isStrong (const UnicodeAnalysisPoint& x)
    {
        return contains ({ BidiType::rtl,   BidiType::ltr,   BidiType::al }, x.getBidiType());
    }

    static bool isNeutral (const UnicodeAnalysisPoint& x)
    {
        return contains ({ BidiType::b,   BidiType::s,   BidiType::ws,  BidiType::on }, x.getBidiType());
    }

    static bool isIsolateInitiator (BidiType x)
    {
        return contains ({ BidiType::lri, BidiType::rli, BidiType::fsi }, x);
    }

    static bool isIsolateTerminator (BidiType x)
    {
        return contains ({ BidiType::pdi }, x);
    }

    static bool isIsolate (BidiType x)
    {
        return isIsolateInitiator (x) || isIsolateTerminator (x);
    }

    static int resolveParagraphEmbeddingLevel (const Span<UnicodeAnalysisPoint> buffer, Range<int> range = {})
    {
        range = range.isEmpty() ? Range<int> { 0, (int) buffer.size() } : range;

        auto seek = [buffer] (BidiType type, Range<int> seekRange) -> int
        {
            for (int i = seekRange.getStart(); i < seekRange.getEnd(); i++)
            {
                if (buffer[(size_t) i].data.bidi == type)
                    return i;
            }

            return seekRange.getEnd();
        };

        for (int i = range.getStart(); i < range.getEnd(); i++)
        {
            const auto& atom = buffer[(size_t) i];

            if (isStrong (atom))
                return atom == BidiType::ltr ? EmbeddingLevel::left : EmbeddingLevel::right;

            if (isIsolateInitiator (atom.getBidiType()))
            {
                // skip to past matching PDI or EOS
                const auto end = seek (BidiType::pdi, { i, range.getEnd() });
                i = end != range.getEnd() ? end + 1 : range.getEnd();
            }
        }

        return EmbeddingLevel::left;
    }

    static void resolveNeutralTypes (Span<UnicodeAnalysisPoint> buffer, int embeddingLevel, Range<int> range)
    {
        range = range.isEmpty() ? Range<int> { 0, (int) buffer.size() } : range;

        // BD13:
        const auto bracketRanges = [buffer, range]
        {
            struct Bracket
            {
                int position;
                uint32_t character;
                Brackets::Kind type;
            };

            // https://www.unicode.org/reports/tr9/#BD16
            constexpr auto maxStackSize = 63;
            std::vector<Bracket> stack;
            stack.reserve (maxStackSize);
            std::vector<Range<int>> brackets;

            for (int i = range.getStart(); i < range.getEnd(); i++)
            {
                const auto& curr = buffer[(size_t) i];

                if (curr == BidiType::on)
                {
                    const auto type = Brackets::getKind (curr.character);

                    if (type == Brackets::Kind::open)
                    {
                        if (stack.size() == stack.capacity())
                            return brackets;

                        stack.push_back ({ i, curr.character, Brackets::Kind::open });
                    }
                    else if (type == Brackets::Kind::close)
                    {
                        while (! stack.empty())
                        {
                            const auto head = stack.back();
                            stack.pop_back();

                            if (head.type == Brackets::Kind::open)
                            {
                                if (Brackets::isMatchingPair (head.character, curr.character))
                                {
                                    brackets.push_back ({ head.position, i });
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            return brackets;
        }();

        // N0:
        for (auto bracketRange : bracketRanges)
        {
            const auto dir = getEmbeddingDirection (embeddingLevel);
            const Span span { buffer.data() + bracketRange.getStart(), (size_t) bracketRange.getLength() };
            const auto strong = std::find_if (span.begin(), span.end(), [] (const UnicodeAnalysisPoint& atom)
            {
                return isStrong (atom);
            });

            if (strong != span.end())
            {
                if (*strong == dir)
                {
                    // B:
                    buffer[(size_t) bracketRange.getStart()].setBidiType (dir);
                    buffer[(size_t) bracketRange.getEnd()].setBidiType (dir);
                }
                else
                {
                    // C:
                    const auto strongContext = [&buf = std::as_const (buffer),
                            start = bracketRange.getStart(),
                            dir]
                    {
                        for (int i = start; i >= 0; i--)
                        {
                            if (isStrong (buf[(size_t) i]))
                                return buf[(size_t) i].getBidiType();
                        }

                        return dir;
                    }();

                    buffer[(size_t) bracketRange.getStart()].setBidiType (strongContext);
                    buffer[(size_t) bracketRange.getEnd()].setBidiType (strongContext);
                }
            }
        }

        // N1:
        for (int i = range.getStart(); i < range.getEnd(); i++)
        {
            const auto curr = buffer[(size_t) i];
            const auto prevBidiType = i > 0 ? buffer[(size_t) i - 1].getBidiType() : BidiType::none; // SOS type?

            if (isNeutral (curr) || isIsolate (curr.getBidiType()))
            {
                const auto endIndex = [buffer, start = i, end = range.getEnd()]
                {
                    for (int j = start; j < end; j++)
                    {
                        const auto atom = buffer[(size_t) j];

                        if (! (isNeutral (atom) || isIsolate (atom.getBidiType())))
                            return j;
                    }

                    return end - 1;
                }();

                auto isNumber    = [] (BidiType type) { return contains ({ BidiType::an, BidiType::en }, type); };
                const auto start = isNumber (prevBidiType) ? BidiType::rtl : prevBidiType;
                const auto end   = isNumber (buffer[(size_t) endIndex].getBidiType()) ? BidiType::rtl : buffer[(size_t) endIndex].getBidiType();

                const auto type  = start == end ? start : getEmbeddingDirection (embeddingLevel);
                std::for_each (buffer.begin() + i, buffer.begin() + endIndex, [type] (UnicodeAnalysisPoint& atom)
                {
                    atom.setBidiType (type);
                });

                i = endIndex;
            }
        }
    }

    static void resolveWeakTypes (Span<UnicodeAnalysisPoint> buffer, Range<int> range)
    {
        range = range.isEmpty() ? Range<int> { 0, static_cast<int> (buffer.size()) } : range;

        for (int i = range.getStart(); i < range.getEnd(); i++)
        {
            auto& curr = buffer[(size_t) i];
            const auto prevBidiType = i > 0 ? buffer[(size_t) i - 1].getBidiType() : BidiType::none;
            const auto nextBidiType = i < range.getEnd() - 1 ? buffer[(size_t) i + 1].getBidiType() : BidiType::none;

            // W1:
            if (curr == BidiType::nsm)
                curr.setBidiType (isIsolate (prevBidiType) ? BidiType::on : prevBidiType);

            // W2:
            else if (curr == BidiType::en)
            {
                for (int j = i - 1; j >= 1; j--)
                {
                    if (buffer[(size_t) j] == BidiType::al)
                        curr.setBidiType (BidiType::al);

                    if (isStrong (buffer[(size_t) j]))
                        break;
                }
            }

            // W3:
            else if (curr == BidiType::al)
                curr.setBidiType (BidiType::rtl);

            // W4:
            else if (curr == BidiType::es || curr == BidiType::cs)
            {
                if (prevBidiType == BidiType::en && nextBidiType == BidiType::en)
                {
                    curr.setBidiType (BidiType::en);
                }
                else if (curr == BidiType::cs)
                {
                    if (prevBidiType == BidiType::an && nextBidiType == BidiType::an)
                        curr.setBidiType (BidiType::an);
                }
            }

            // W5:
            else if (curr == BidiType::et)
            {
                if (prevBidiType == BidiType::en || nextBidiType == BidiType::en)
                    curr.setBidiType (BidiType::en);
            }

            // W6
            if (contains ({ BidiType::es, BidiType::cs }, curr.getBidiType()))
                curr.setBidiType (BidiType::on);

            // W7:
            else if (curr == BidiType::en)
            {
                for (int j = i - 1; j >= 1; j--)
                {
                    if (buffer[(size_t) j] == BidiType::ltr)
                        curr.setBidiType (BidiType::ltr);

                    if (isStrong (buffer[(size_t) j]))
                        break;
                }
            }
        }
    }

    static void resolveImplicitTypes (Span<UnicodeAnalysisPoint> buffer, int embeddingLevel, Range<int> range)
    {
        range = range.isEmpty() ? Range<int> { 0, static_cast<int> (buffer.size()) } : range;

        // I1, I2
        // https://www.unicode.org/reports/tr9/#Resolving_Implicit_Levels
        for (int i = range.getStart(); i < range.getEnd(); i++)
        {
            auto& curr = buffer[(size_t) i];

            const auto level = (uint16_t) embeddingLevel;
            const auto isEven = isOdd (level) == false;

            if (curr.getGeneralCategory() != GeneralCategory::pc)
            {
                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
                switch (curr.getBidiType())
                {
                    case BidiType::ltr: curr.bidiLevel = (isEven ? level     : level + 1); break;
                    case BidiType::rtl: curr.bidiLevel = (isEven ? level + 1 : level    ); break;

                    default: break;
                }
                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            }
        }
    }

    static void resolveExplicitLevels (Span<UnicodeAnalysisPoint> buffer, int embeddingLevel)
    {
        struct State
        {
            enum DirectionalOverride { Neutral, rtl, ltr };
            int embeddingLevel;
            DirectionalOverride directionalOverride;
            bool isolateStatus;
        };

        // X1
        struct OverflowState
        {
            int isolate  = 0;
            int embedded = 0;
        };

        // https://www.unicode.org/reports/tr9/#BD2
        static constexpr auto maxStackSize = 125;
        std::vector<State> stack;
        stack.reserve (maxStackSize);
        OverflowState overflow;

        [[maybe_unused]] const auto canPush = [&stack] { return stack.size() < maxStackSize; };
        const auto isValid = [&] (auto value) { return (value < maxStackSize) && (overflow.isolate == 0 && overflow.embedded == 0); };

        stack.push_back ({ embeddingLevel, State::Neutral, false });

        // X2-X6a
        for (auto& atom : buffer)
        {
            // X2-X3: Explicit embeddings
            if (atom == BidiType::rle || atom == BidiType::lre)
            {
                if (stack.empty())
                    break;

                auto& head = stack.back();
                head.embeddingLevel = atom == BidiType::rle ? computeLeastOdd (head.embeddingLevel)
                                                            : computeLeastEven (head.embeddingLevel);

                if (isValid (head.embeddingLevel))
                {
                    head.directionalOverride = State::Neutral;
                    head.isolateStatus = false;

                    jassert (canPush());

                    stack.push_back (stack.back());
                }
                else if (overflow.isolate == 0)
                {
                    overflow.embedded++;
                }
            }

            // X4-X5: Explicit Overrides
            else if (atom == BidiType::rlo || atom == BidiType::lro)
            {
                if (stack.empty())
                    break;

                auto& head = stack.back();
                head.embeddingLevel = atom == BidiType::rlo ? computeLeastOdd (head.embeddingLevel)
                                                            : computeLeastEven (head.embeddingLevel);

                if (isValid (head.embeddingLevel))
                {
                    head.directionalOverride = atom == BidiType::rlo ? State::rtl : State::ltr;
                    head.isolateStatus = false;

                    jassert (canPush());
                    stack.push_back (stack.back());
                }
                else if (overflow.isolate == 0)
                {
                    overflow.embedded++;
                }
            }

            // X5a-X5b: Isolates
            else if (atom == BidiType::rli || atom == BidiType::lri)
            {
                if (stack.empty())
                    break;

                auto& head = stack.back();
                head.embeddingLevel = atom == BidiType::rli ? computeLeastOdd (head.embeddingLevel)
                                                            : computeLeastEven (head.embeddingLevel);

                if (head.directionalOverride == State::ltr)
                    atom.setBidiType (BidiType::ltr);
                else if (head.directionalOverride == State::rtl)
                    atom.setBidiType (BidiType::rtl);

                if (isValid (head.embeddingLevel))
                {
                    head.directionalOverride = State::Neutral;
                    head.isolateStatus = true;

                    jassert (canPush());
                    stack.push_back (stack.back());
                }
                else
                {
                    overflow.isolate++;
                }
            }

            // X6a: Terminating Isolates
            else if (atom == BidiType::pdi)
            {
                if (overflow.isolate > 0)
                {
                    overflow.isolate--;
                }
                else
                {
                    overflow.embedded = 0;

                    while (! stack.empty() && ! stack.back().isolateStatus)
                        stack.pop_back();

                    if (! stack.empty())
                        stack.pop_back();
                }

                if (stack.empty())
                    break;

                atom.bidiLevel = (uint16_t) stack.back().embeddingLevel;
            }

            // X7
            else if (atom == BidiType::pdf)
            {
                if (overflow.isolate > 0)
                {
                    overflow.isolate--;
                }
                else if (overflow.embedded > 0)
                {
                    overflow.embedded--;
                }
                else if (stack.size() >= 2 && stack.back().isolateStatus == false)
                {
                    stack.pop_back();
                }
            }

            // X8
            else if (atom == BidiType::b)
            {
                if (stack.empty())
                    break;

                atom.bidiLevel = (uint16_t) stack.back().embeddingLevel;

                overflow.embedded = 0;
                overflow.isolate = 0;
                stack.clear();
                stack.push_back ({ embeddingLevel, State::Neutral, false });
            }

            // X6: Everything else
            // ! (B | BN | RLE | LRE | RLO | LRO | PDF | RLI | LRI | FSI | PDI)
            else
            {
                if (stack.empty())
                    break;

                atom.bidiLevel = (uint16_t) stack.back().embeddingLevel;
            }
        }
    }

    static void resolveReorderedIndices (std::vector<int>& result,
                                         const Span<UnicodeAnalysisPoint> buffer,
                                         int embeddingLevel,
                                         Range<int> range = {})
    {
        range = range.isEmpty() ? Range<int> { 0, static_cast<int> (buffer.size()) } : range;

        std::vector<int> levels ((size_t) range.getLength());

        for (int i = range.getStart(); i < range.getEnd(); i++)
            levels[(size_t) i] = buffer[(size_t) i].bidiLevel;

        // L1:
        for (int i = range.getStart(); i < range.getEnd(); i++)
        {
            auto curr = buffer[(size_t) i];

            if (contains ({ BidiType::s, BidiType::b }, curr.getBidiType()))
                levels[(size_t) i] = embeddingLevel;

            for (int j = i - 1; j >= 0; j--)
            {
                curr = buffer[(size_t) j];

                if (! isIsolate (curr.getBidiType()))
                    break;

                levels[(size_t) j] = embeddingLevel;
            }
        }

        // L2:
        result.resize ((size_t) range.getLength());
        std::iota (result.begin(), result.end(), 0);
        const auto high = *std::max_element (levels.begin(), levels.end());

        for (int level = high; level > 0; level--)
        {
            for (int i = 0; i < range.getLength(); i++)
            {
                const auto indexLevel = levels[(size_t) i];

                if (level > 0 && (indexLevel >= level))
                {
                    // Find the longest consecutive run of the current level and above
                    // 1111 = 4
                    // 1001 = 1
                    // 1123 = 4
                    const auto start = i;
                    const auto end = [start, levels, level]
                    {
                        auto e = (size_t) start + 1;

                        while (e < levels.size() && levels[e] >= level)
                            e++;

                        return e;
                    }();

                    std::reverse (result.begin() + start, result.begin() + (int) end);
                    i = (int) end;
                }
            }
        }
    }
};

} // namespace juce
