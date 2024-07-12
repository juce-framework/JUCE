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
        const auto paragraphLevel = directionOverride.has_value() ? (*directionOverride == TextDirection::rtl ? 1 : 0)
                                                                  : resolveParagraphEmbeddingLevel (stream);

        for (auto& atom : stream)
            atom.embeddingLevel = (uint16_t) paragraphLevel;

        // X1-8
        resolveExplicitLevels (stream, paragraphLevel);

        // X9
        for (auto& atom : stream)
            if (contains ({ BidiType::rle, BidiType::lre, BidiType::rlo, BidiType::lro, BidiType::pdf }, atom.getBidiType()))
                atom.setBidiType (BidiType::bn);

        // W1-W7
        resolveWeakTypes (stream, paragraphLevel);

        // N1-N2
        resolveNeutralTypes (stream, paragraphLevel);

        // I1-I2
        resolveImplicitTypes (stream);

        output.embeddingLevel = paragraphLevel;

        output.resolvedLevels.clear();

        for (const auto& atom : stream)
            output.resolvedLevels.push_back (atom.embeddingLevel);

        resolveReorderedIndices (output.visualOrder, stream, paragraphLevel);
    }

    static auto isOdd (int level)            { return bool (level & 1); }
    static auto computeLeastEven (int level) { return isOdd (level) ? level + 1 : level + 2; }
    static auto computeLeastOdd (int level)  { return isOdd (level) ? level + 2 : level + 1; }

    static BidiType getEmbeddingDirection (int level)
    {
        return isOdd (level) ? BidiType::rtl : BidiType::ltr;
    }

    static bool isNeutralIsolate (BidiType x)
    {
        return isNeutral (x) || isIsolate (x);
    }

    static bool isNeutral (BidiType x)
    {
        return contains ({ BidiType::b, BidiType::s, BidiType::ws, BidiType::on }, x);
    }

    static bool isIsolate (BidiType x)
    {
        return isIsolateInitiator (x) || isIsolateTerminator (x);
    }

    static bool isStrong (const UnicodeAnalysisPoint& x)
    {
        return contains ({ BidiType::rtl, BidiType::ltr, BidiType::al }, x.getBidiType());
    }

    static bool isIsolateInitiator (BidiType x)
    {
        return contains ({ BidiType::lri, BidiType::rli, BidiType::fsi }, x);
    }

    static bool isIsolateTerminator (BidiType x)
    {
        return contains ({ BidiType::pdi }, x);
    }

    static void resolveNeutralTypes (Span<UnicodeAnalysisPoint> buffer, int paragraphLevel)
    {
        n1 (buffer, paragraphLevel);
        n2 (buffer);
    }

    static void n1 (Span<UnicodeAnalysisPoint> buffer, int paragraphLevel)
    {
        static auto getStrongType = [] (BidiType bt)
        {
            if (bt == BidiType::rtl || bt == BidiType::en || bt == BidiType::an)
                return BidiType::rtl;

            return BidiType::ltr;
        };

        const auto begin = buffer.begin();
        const auto end   = buffer.end();

        const auto iso = getEmbeddingDirection (paragraphLevel);

        for (auto iter = begin; iter != end;)
        {
            const auto predicate = [] (const UnicodeAnalysisPoint& uap) { return isNeutralIsolate (uap.getBidiType()); };
            const auto niBegin   = std::find_if (iter, end, predicate);

            if (niBegin == end)
                break;

            const auto niEnd = std::find_if_not (niBegin, end, predicate);
            const auto pre   = niBegin == begin ? iso : getStrongType ((niBegin - 1)->getBidiType());
            const auto post  = niEnd   == end   ? iso : getStrongType ((niEnd)->getBidiType());

            if (pre == post)
                std::for_each (niBegin, niEnd, [pre] (auto& uap) { uap.setBidiType (pre); });

            iter = niEnd;
        }
    }

    static void n2 (Span<UnicodeAnalysisPoint> buffer)
    {
        for (auto& uap : buffer)
            if (isNeutralIsolate (uap.getBidiType()))
                uap.setBidiType (getEmbeddingDirection (uap.embeddingLevel));
    }

    static int resolveParagraphEmbeddingLevel (const Span<UnicodeAnalysisPoint> buffer)
    {
        auto seek = [buffer] (BidiType type, Range<int> seekRange) -> int
        {
            for (int i = seekRange.getStart(); i < seekRange.getEnd(); i++)
            {
                if (buffer[(size_t) i].data.bidi == type)
                    return i;
            }

            return seekRange.getEnd();
        };

        const auto bufferSize = (int) buffer.size();

        for (int i = 0; i < bufferSize; i++)
        {
            const auto& atom = buffer[(size_t) i];

            if (isStrong (atom))
                return atom == BidiType::ltr ? 0 : 1;

            if (isIsolateInitiator (atom.getBidiType()))
            {
                // skip to past matching PDI or EOS
                const auto end = seek (BidiType::pdi, { i, bufferSize });
                i = end != bufferSize ? end + 1 : bufferSize;
            }
        }

        return 0;
    }

    struct WeakContext
    {
        size_t position;
        Span<UnicodeAnalysisPoint> buffer;
        BidiType prev, next;
        int paragraphLevel;
    };

    static void w1 (const WeakContext& context)
    {
        const auto prev  = context.prev;
        auto& curr = context.buffer[context.position];

        if (curr == BidiType::nsm)
        {
            if (context.position == 0)
                curr.setBidiType (getEmbeddingDirection (context.paragraphLevel));
            else
                curr.setBidiType (isIsolate (prev) ? BidiType::on : prev);
        }
    }

    static void w2 (const WeakContext& context)
    {
        const auto& buffer = context.buffer;
        auto& curr = context.buffer[context.position];

        if (curr == BidiType::en)
        {
            for (int j = (int) context.position - 1; j >= 0; j--)
            {
                if (buffer[(size_t) j] == BidiType::al)
                {
                    curr.setBidiType (BidiType::an);
                    break;
                }

                if (isStrong (buffer[(size_t) j]))
                    break;
            }
        }
    }

    static void w3 (const WeakContext& context)
    {
        auto& curr = context.buffer[context.position];

        if (curr == BidiType::al)
            curr.setBidiType (BidiType::rtl);
    }

    static void w4 (const WeakContext& context)
    {
        const auto prevBidiType = context.prev;
        const auto nextBidiType = context.next;

        auto& curr = context.buffer[context.position];

        if (curr == BidiType::es || curr == BidiType::cs)
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
    }

    static void w5 (const WeakContext& context)
    {
        const auto& buffer = context.buffer;
        const auto& curr = buffer[context.position];

        if (curr == BidiType::en)
        {
            // seek backwards
            for (int j = (int) context.position - 1; j >= 0; j--)
            {
                if (buffer[(size_t) j] == BidiType::et)
                    buffer[(size_t) j].setBidiType (BidiType::en);
                else
                    break;
            }

            // seek forwards
            for (size_t j = context.position + 1; j < buffer.size(); j++)
            {
                if (buffer[j] == BidiType::et)
                    buffer[j].setBidiType (BidiType::en);
                else
                    break;
            }
        }
    }

    static void w6 (const WeakContext& context)
    {
        auto& curr = context.buffer[context.position];

        if (contains ({ BidiType::et, BidiType::cs, BidiType::es }, curr.getBidiType()))
            curr.setBidiType (BidiType::on);
    }

    static void w7 (const WeakContext& context)
    {
        const auto& buffer = context.buffer;
        auto& curr = buffer[context.position];

        if (curr == BidiType::en)
        {
            bool strongFound = false;

            for (int j = (int) context.position - 1; j >= 0; j--)
            {
                if (buffer[(size_t) j] == BidiType::ltr)
                {
                    curr.setBidiType (BidiType::ltr);
                    strongFound = true;
                    break;
                }

                if (isStrong (buffer[(size_t) j]))
                    break;
            }

            if (! strongFound && getEmbeddingDirection (context.paragraphLevel) == BidiType::ltr)
                curr.setBidiType (BidiType::ltr);
        }
    }

    static void resolveWeakTypes (Span<UnicodeAnalysisPoint> buffer, int paragraphLevel)
    {
        for (size_t i = 0; i < buffer.size(); i++)
        {
            const auto sos = i == 0;
            const auto eos = i == buffer.size() - 1;

            auto context = WeakContext { i,
                                         buffer,
                                         sos ? BidiType::on : buffer[i - 1].getBidiType(),
                                         eos ? BidiType::on : buffer[i + 1].getBidiType(),
                                         paragraphLevel };

            w1 (context);
            w2 (context);
            w3 (context);
            w4 (context);
            w5 (context);
            w6 (context);
            w7 (context);
        }
    }

    static void resolveImplicitTypes (Span<UnicodeAnalysisPoint> buffer)
    {
        // I1, I2
        // https://www.unicode.org/reports/tr9/#Resolving_Implicit_Levels
        for (auto& point : buffer)
        {
            const auto level  = point.embeddingLevel;
            const auto isEven = ! isOdd (level);

            if (point.getGeneralCategory() != GeneralCategory::pc)
            {
                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
                switch (point.getBidiType())
                {
                    case BidiType::ltr: point.embeddingLevel = (uint16_t) (isEven ? level     : level + 1); break;
                    case BidiType::rtl: point.embeddingLevel = (uint16_t) (isEven ? level + 1 : level    ); break;
                    case BidiType::an:  point.embeddingLevel = (uint16_t) (isEven ? level + 2 : level + 1); break;
                    case BidiType::en:  point.embeddingLevel = (uint16_t) (isEven ? level + 2 : level + 1); break;

                    default: break;
                }
                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            }
        }
    }

    static void resolveExplicitLevels (Span<UnicodeAnalysisPoint> buffer, int paragraphLevel)
    {
        struct DirectionalState
        {
            enum DirectionalOverride { Neutral, rtl, ltr };

            DirectionalState (int level, DirectionalOverride dir, bool isolate)
                : embeddingLevel (level),
                  directionalOverride (dir),
                  isolateStatus (isolate)
            {
            }

            auto getEmbeddingLevel()      const { return embeddingLevel; }
            auto getDirectionalOverride() const { return directionalOverride; }
            auto getIsolateStatus()       const { return isolateStatus; }

            void setEmbeddingLevel (int level)                    { embeddingLevel = level; }
            void setDirectionalOverride (DirectionalOverride dir) { directionalOverride = dir; }
            void setIsolateStatus (bool status)                   { isolateStatus = status; }

        private:
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
        std::vector<DirectionalState> stack;
        stack.reserve (maxStackSize);
        OverflowState overflowCounter;
        int validIsolate = 0;
        int previousEmbeddingLevel = paragraphLevel;

        static auto getEmbeddingDirection = [] (BidiType bt)
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
            switch (bt)
            {
                case BidiType::lre: return BidiType::ltr;
                case BidiType::lro: return BidiType::ltr;
                case BidiType::lri: return BidiType::ltr;
                case BidiType::rle: return BidiType::rtl;
                case BidiType::rlo: return BidiType::rtl;
                case BidiType::rli: return BidiType::rtl;

                default: break;
            }
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            jassertfalse;
            return BidiType::ltr;
        };

        [[maybe_unused]] const auto canPush = [&stack] { return stack.size() < maxStackSize; };
        const auto isValid = [&] (auto value) { return (value < maxStackSize) && (overflowCounter.isolate == 0 && overflowCounter.embedded == 0); };

        stack.push_back ({ paragraphLevel, DirectionalState::Neutral, false });

        for (auto& atom : buffer)
        {
            // X2-X5b
            if (contains ({ BidiType::lre, BidiType::lro, BidiType::rle, BidiType::rlo, BidiType::rli, BidiType::lri }, atom.getBidiType()))
            {
                if (stack.empty())
                    break;

                const auto isIsolate  = contains ({ BidiType::rli, BidiType::lri }, atom.getBidiType());
                const auto isOverride = contains ({ BidiType::lro, BidiType::rlo }, atom.getBidiType());

                auto head = stack.back();

                // This has to be applied regardless of the new embeddeding level's validity
                if (isIsolate)
                {
                    // X5ab:
                    // Set the RLIs embedding level to the embedding level of the last entry on the directional status stack.
                    atom.embeddingLevel = (uint16_t) head.getEmbeddingLevel();

                    // If the directional override status of the last entry on the directional status stack is not neutral, reset
                    // the current character type from RLI to L if the override status is left-to-right, and to R if the override
                    // status is right-to-left.
                    if (head.getDirectionalOverride() != DirectionalState::Neutral)
                        atom.setBidiType (head.getDirectionalOverride() == DirectionalState::ltr ? BidiType::ltr : BidiType::rtl);
                }
                else
                {
                    atom.embeddingLevel = (uint16_t) previousEmbeddingLevel;
                }

                // X2-5b:
                // Compute the least odd/even embedding level greater than the embedding level of the last entry on the directional
                // status stack.
                const auto isRTL    = getEmbeddingDirection (atom.getBidiType()) == BidiType::rtl;
                const auto newLevel = isRTL ? computeLeastOdd (head.getEmbeddingLevel())
                                            : computeLeastEven (head.getEmbeddingLevel());

                // X2-5b:
                // If this new level would be valid, and the overflow isolate count and overflow embedding count are both zero,
                // then this is valid.
                if (isValid (newLevel))
                {
                    // Push an entry consisting of the new embedding level.
                    head.setEmbeddingLevel (newLevel);

                    // X5ab:
                    // Increment the valid isolate count by one and true directional isolate status.
                    validIsolate += isIsolate ? 1 : 0;
                    head.setIsolateStatus (isIsolate);

                    // X4-X5:
                    // Push an entry consisting of the new embedding level, RTL/LTR directional override status
                    if (isOverride)
                        head.setDirectionalOverride (isRTL ? DirectionalState::rtl : DirectionalState::ltr);
                    else
                        head.setDirectionalOverride (DirectionalState::Neutral);

                    jassert (canPush());
                    stack.push_back (head);
                }

                // Otherwise, this is an overflow. If the overflow isolate count is zero, increment the overflow
                // embedding count by one. Leave all other variables unchanged.
                else if (overflowCounter.isolate == 0)
                {
                    overflowCounter.embedded++;
                }
            }

            // X6
            //B, BN, RLE, LRE, RLO, LRO, PDF, RLI, LRI, FSI, and PDI
            if (! contains ({ BidiType::b,   BidiType::bn,  BidiType::rle, BidiType::lre, BidiType::rlo, BidiType::lro, BidiType::pdf,
                              BidiType::rli, BidiType::lri, BidiType::fsi, BidiType::pdi }, atom.getBidiType()))
            {
                if (stack.empty())
                    break;

                auto head = stack.back();
                atom.embeddingLevel = (uint16_t) head.getEmbeddingLevel();

                previousEmbeddingLevel = stack.empty() ? paragraphLevel : stack.back().getEmbeddingLevel();

                if (head.getDirectionalOverride() != DirectionalState::Neutral)
                    atom.setBidiType (head.getDirectionalOverride() == DirectionalState::ltr ? BidiType::ltr : BidiType::rtl);
            }

            // X6a: Terminating Isolates
            if (atom == BidiType::pdi)
            {
                // If the overflow isolate count is greater than zero, this PDI matches an overflow isolate
                // initiator. Decrement the overflow isolate count by one.
                if (overflowCounter.isolate > 0)
                {
                    overflowCounter.isolate--;
                }
                // Otherwise, if the valid isolate count is > 0, this PDI matches a valid isolate initiator.
                else if (validIsolate > 0)
                {
                    // Reset the overflow embedding count to zero.
                    overflowCounter.embedded = 0;

                    // While the directional isolate status of the last entry on the stack is false, pop the
                    // last entry from the directional status stack.
                    while (! stack.empty() && ! stack.back().getIsolateStatus())
                        stack.pop_back();

                    // Pop the last entry from the directional status stack and decrement the valid isolate
                    // count by one.
                    if (! stack.empty())
                        stack.pop_back();

                    validIsolate--;
                }

                if (stack.empty())
                    break;

                // In all cases, look up the last entry on the directional status stack left after the steps above and:
                auto& head = stack.back();

                // Set the PDIs level to the entry's embedding level.
                atom.embeddingLevel = (uint16_t) head.getEmbeddingLevel();

                // If the entry's directional override status is not neutral, reset the current character type from PDI
                // to L if the override status is left-to-right, and to R if the override status is right-to-left.
                if (head.getDirectionalOverride() != DirectionalState::Neutral)
                    atom.setBidiType (head.getDirectionalOverride() == DirectionalState::ltr ? BidiType::ltr : BidiType::rtl);
            }

            // X7
            else if (atom == BidiType::pdf)
            {
                atom.embeddingLevel = (uint16_t) previousEmbeddingLevel;

                // If the overflow isolate count is greater than zero, do nothing.
                // Otherwise, if the overflow embedding count is greater than zero, decrement it by one.
                if (overflowCounter.isolate == 0 && overflowCounter.embedded > 0)
                {
                    overflowCounter.embedded--;
                }
                else if (stack.size() >= 2 && ! stack.back().getIsolateStatus())
                {
                    stack.pop_back();
                }
            }

            // X8
            else if (atom == BidiType::b)
            {
                if (stack.empty())
                    break;

                atom.embeddingLevel = (uint16_t) stack.back().getEmbeddingLevel();

                overflowCounter.embedded = 0;
                overflowCounter.isolate = 0;
                validIsolate = 0;
                previousEmbeddingLevel = paragraphLevel;

                stack.clear();
                stack.push_back ({ paragraphLevel, DirectionalState::Neutral, false });
            }
        }
    }

    static void l1 (Span<UnicodeAnalysisPoint> buffer, int paragraphLevel)
    {
        auto reset = [buffer, paragraphLevel] (size_t position)
        {
            for (int i = (int) position; i >= 0; i--)
            {
                auto& atom = buffer[(size_t) i];

                if (contains ({ BidiType::s, BidiType::b, BidiType::ws, BidiType::fsi, BidiType::lri, BidiType::rli, BidiType::pdi }, atom.getBidiType()))
                    atom.embeddingLevel = (uint16_t) paragraphLevel;
                else
                    break;
            }
        };

        // On each line, reset the embedding level of the following characters to the paragraph embedding level:
        for (size_t i = 0; i < buffer.size(); i++)
        {
            // Segment separators, Paragraph separators.
            // Any sequence of whitespace characters and/or isolate formatting characters (FSI, LRI, RLI, and PDI).
            if (contains ({ BidiType::s, BidiType::b }, buffer[i].getBidiType()))
                reset (i);
        }

        // Any sequence of whitespace characters and/or isolate formatting characters (FSI, LRI, RLI, and PDI) at the end of the line.
        reset (buffer.size() - 1);
    }

    static void l2 (Span<int> result, Span<int> levels)
    {
        jassert (levels.size() == result.size());

        std::iota (result.begin(), result.end(), 0);
        const auto high = *std::max_element (levels.begin(), levels.end());

        for (int level = high; level > 0; level--)
        {
            for (size_t i = 0; i < levels.size();)
            {
                if (levels[i] >= level)
                {
                    // Find the longest consecutive run of the current level and above
                    // 1111 = 4
                    // 1001 = 1
                    // 1123 = 4
                    const auto start = i;

                    while (i < levels.size() && levels[i] >= level)
                        i++;

                    std::reverse (result.begin() + start, result.begin() + i);
                    continue;
                }

                i++;
            }
        }
    }

    static void resolveReorderedIndices (std::vector<int>& result,
                                         const Span<UnicodeAnalysisPoint> buffer,
                                         int paragraphLevel)
    {
        std::vector<int> levels;

        result.resize (buffer.size());
        levels.resize (buffer.size());
        std::transform (buffer.begin(), buffer.end(), levels.begin(), [] (auto& point) { return (int) point.embeddingLevel; });

        l1 (buffer, paragraphLevel);
        l2 (result, levels);
    }
};

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class BidiTests : public UnitTest
{
public:
    BidiTests() : UnitTest ("Unicode Bidi", UnitTestCategories::text) {}

    void runTest() override
    {
        using BT = BidiType;

        const String LRE { L"\u202A" }; // LTR embed
        const String RLE { L"\u202B" }; // RTL embed
        const String PDF { L"\u202C" }; // Pop directional embedding
        const String RLO { L"\u202E" }; // RTL override
        const String RLI { L"\u2067" }; // RTL isolate
        const String PDI { L"\u2069" }; // Pop Directional isolate

        beginTest ("TR9 N1");
        {
            // Examples from TR9
            expect (n1 (0, { BT::ltr, BT::b, BT::ltr }, { BT::ltr, BT::ltr, BT::ltr }));
            expect (n1 (0, { BT::rtl, BT::b, BT::rtl }, { BT::rtl, BT::rtl, BT::rtl }));
            expect (n1 (0, { BT::rtl, BT::b, BT::an  }, { BT::rtl, BT::rtl, BT::an  }));
            expect (n1 (0, { BT::rtl, BT::b, BT::en  }, { BT::rtl, BT::rtl, BT::en  }));
            expect (n1 (0, { BT::an,  BT::b, BT::rtl }, { BT::an,  BT::rtl, BT::rtl }));
            expect (n1 (0, { BT::an,  BT::b, BT::an  }, { BT::an,  BT::rtl, BT::an  }));
            expect (n1 (0, { BT::an,  BT::b, BT::en  }, { BT::an,  BT::rtl, BT::en  }));
            expect (n1 (0, { BT::en,  BT::b, BT::rtl }, { BT::en,  BT::rtl, BT::rtl }));
            expect (n1 (0, { BT::en,  BT::b, BT::an  }, { BT::en,  BT::rtl, BT::an  }));
            expect (n1 (0, { BT::en,  BT::b, BT::en  }, { BT::en,  BT::rtl, BT::en  }));

            // NI affected by sos/eos
            expect (n1 (0, { BT::b,   BT::ltr }, { BT::ltr, BT::ltr }));
            expect (n1 (1, { BT::b,   BT::rtl }, { BT::rtl, BT::rtl }));

            expect (n1 (0, { BT::ltr, BT::b   }, { BT::ltr, BT::ltr }));
            expect (n1 (0, { BT::b,   BT::b   }, { BT::ltr, BT::ltr }));

            // NI not surrounded by similar types should not change.
            expect (n1 (0, { BT::rtl, BT::b, BT::ltr }, { BT::rtl, BT::b, BT::ltr }));
            expect (n1 (0, { BT::ltr, BT::b, BT::rtl }, { BT::ltr, BT::b, BT::rtl }));
        }

        beginTest ("TR9 N2");
        {
            // Examples from TR9
            expect (n2 (0, { BT::ltr, BT::b   }, { BT::ltr, BT::ltr }));
            expect (n2 (1, { BT::ltr, BT::b   }, { BT::ltr, BT::rtl }));
            expect (n2 (0, { BT::rtl, BT::b   }, { BT::rtl, BT::ltr }));
            expect (n2 (1, { BT::rtl, BT::b   }, { BT::rtl, BT::rtl }));
            expect (n2 (0, { BT::b,   BT::ltr }, { BT::ltr, BT::ltr }));
            expect (n2 (1, { BT::b,   BT::ltr }, { BT::rtl, BT::ltr }));
            expect (n2 (0, { BT::b,   BT::rtl }, { BT::ltr, BT::rtl }));
            expect (n2 (1, { BT::b,   BT::rtl }, { BT::rtl, BT::rtl }));
        }

        beginTest ("TR9 Paragraph Embedding Level");
        {
            // Examples from TR9
            expect (resolveParagraphEmbeddingLevel ({ BT::ltr, BT::ltr, BT::ltr }, 0));
            expect (resolveParagraphEmbeddingLevel ({ BT::rtl, BT::ltr, BT::ltr }, 1));
            expect (resolveParagraphEmbeddingLevel ({ BT::ltr, BT::rtl, BT::ltr, BT::rtl }, 0));
            expect (resolveParagraphEmbeddingLevel ({ BT::rtl, BT::ltr, BT::rtl, BT::ltr }, 1));
            expect (resolveParagraphEmbeddingLevel ({}, 0));
            expect (resolveParagraphEmbeddingLevel ({ BT::ltr }, 0));
            expect (resolveParagraphEmbeddingLevel ({ BT::rtl }, 1));
            expect (resolveParagraphEmbeddingLevel ({ BT::ltr, BT::rtl, BT::rtl }, 0));
            expect (resolveParagraphEmbeddingLevel ({ BT::rtl, BT::ltr, BT::ltr }, 1));
            expect (resolveParagraphEmbeddingLevel ({ BT::ltr, BT::rtl, BT::ltr, BT::rtl, BT::ltr }, 0));
            expect (resolveParagraphEmbeddingLevel ({ BT::rtl, BT::ltr, BT::rtl, BT::ltr, BT::rtl }, 1));
        }

        {
            constexpr auto neutral = BT::bn;

            beginTest ("TR9 W1");
            expect (resolveWeakTypes (TR9::w1, 1, { BT::al,  BT::nsm, BT::nsm }, { BT::al,  BT::al,  BT::al }));
            expect (resolveWeakTypes (TR9::w1, 1, { BT::nsm },                   { BT::rtl }));
            expect (resolveWeakTypes (TR9::w1, 1, { BT::lri, BT::nsm },          { BT::lri, BT::on  }));
            expect (resolveWeakTypes (TR9::w1, 1, { BT::pdi, BT::nsm },          { BT::pdi, BT::on  }));

            beginTest ("TR9 W2");
            expect (resolveWeakTypes (TR9::w2, 0, { BT::al,  BT::en },          { BT::al,  BT::an }));
            expect (resolveWeakTypes (TR9::w2, 0, { BT::al,  neutral, BT::en }, { BT::al,  neutral, BT::an }));
            expect (resolveWeakTypes (TR9::w2, 0, { neutral,  BT::en },         { neutral,  BT::en }));
            expect (resolveWeakTypes (TR9::w2, 0, { BT::ltr, neutral, BT::en }, { BT::ltr, neutral, BT::en }));
            expect (resolveWeakTypes (TR9::w2, 0, { BT::rtl, neutral, BT::en }, { BT::rtl, neutral, BT::en }));

            beginTest ("TR9 W3");
            expect (resolveWeakTypes (TR9::w3, 0, { BT::al }, { BT::rtl }));

            beginTest ("TR9 W4");
            expect (resolveWeakTypes (TR9::w4, 0, { BT::en, BT::es, BT::en }, { BT::en, BT::en, BT::en }));
            expect (resolveWeakTypes (TR9::w4, 0, { BT::en, BT::cs, BT::en }, { BT::en, BT::en, BT::en }));
            expect (resolveWeakTypes (TR9::w4, 0, { BT::an, BT::cs, BT::an }, { BT::an, BT::an, BT::an }));

            beginTest ("TR9 W5");
            expect (resolveWeakTypes (TR9::w5, 0, { BT::et, BT::et, BT::en }, { BT::en, BT::en, BT::en }));
            expect (resolveWeakTypes (TR9::w5, 0, { BT::en, BT::et, BT::et }, { BT::en, BT::en, BT::en }));
            expect (resolveWeakTypes (TR9::w5, 0, { BT::an, BT::et, BT::en }, { BT::an, BT::en, BT::en }));

            beginTest ("TR9 W6");
            expect (resolveWeakTypes (TR9::w6, 0, { BT::an,  BT::et },         { BT::an,  BT::on }));
            expect (resolveWeakTypes (TR9::w6, 0, { BT::ltr, BT::es, BT::en }, { BT::ltr, BT::on, BT::en }));
            expect (resolveWeakTypes (TR9::w6, 0, { BT::en,  BT::cs, BT::an }, { BT::en,  BT::on, BT::an }));
            expect (resolveWeakTypes (TR9::w6, 0, { BT::et,  BT::an },         { BT::on,  BT::an }));

            beginTest ("TR9 W7");
            expect (resolveWeakTypes (TR9::w7, 0, { BT::ltr, neutral, BT::en }, { BT::ltr, neutral, BT::ltr }));
            expect (resolveWeakTypes (TR9::w7, 1, { BT::rtl, neutral, BT::en }, { BT::rtl, neutral, BT::en }));
        }

        beginTest ("TR9 I1");
        {
            expect (resolveImplicitTypes (0, { BT::rtl, BT::an,  BT::en  }, { 1, 2, 2 }));
            expect (resolveImplicitTypes (0, { BT::ltr, BT::ltr, BT::ltr }, { 0, 0, 0 }));
            expect (resolveImplicitTypes (0, { BT::an,  BT::ltr, BT::rtl }, { 2, 0, 1 }));
        }

        beginTest ("TR9 I2");
        {
            expect (resolveImplicitTypes (1, { BT::ltr, BT::en,  BT::an  }, { 2, 2, 2 }));
            expect (resolveImplicitTypes (1, { BT::rtl, BT::rtl, BT::rtl }, { 1, 1, 1 }));
            expect (resolveImplicitTypes (1, { BT::an }, { 2 }));
        }

        beginTest ("X1 - X8");
        {
            expect (resolveExplicitLevels (0, String { "zero" } + RLE + "one" + PDF + "zero",
                                                       "0000"     "0"   "111"   "1"   "0000"));

            expect (resolveExplicitLevels (1, String { "one" } + PDF + "one",
                                                       "111"     "1"   "111"));

            // Nested embedding:
            expect (resolveExplicitLevels (0, String { "zero" } + RLE + "one" + RLE + "333" + PDF + PDF + "zero",
                                                       "0000"     "0"   "111"   "1"   "333"   "3"   "3"   "0000"));

            // Directional override:
            expect (resolveExplicitLevels (0, String { "abc" } + RLO + "def" + PDF + "ghi",
                                                       "000"     "0"   "111"   "1"   "000"));
            // Mixed embedding and overrides:
            expect (resolveExplicitLevels (0, String { "abc" } + LRE + "lmn" + PDF + "def" + RLO + "ghi" + PDF + "jkl",
                                                       "000"     "0"   "222"   "2"   "000"   "0"   "111"   "1"    "000"));

            // Multiple PDFs:
            expect (resolveExplicitLevels (0, String { "abc" } + RLE + "def" + RLE + "ghi" + PDF  + PDF  + "jkl",
                                                       "000"     "0"   "111"   "1"   "333"   "3"    "3"    "000"));

            // Isolates:
            expect (resolveExplicitLevels (0, String { "abc" } + RLI + "def" + PDI + "ghi",
                                                       "000"     "0"   "111"   "0"   "000"));

            // Overflows and isolates:
            // PDIs are not removed from the output string
            expect (resolveExplicitLevels (0, String { "abc" } + LRE + "rlm" + RLE + "def" + PDF + "ghi" + PDI + "jkl",
                                                       "000"     "0"   "222"   "2"   "333"   "3"   "222"   "2"   "222"));

            // Paragraph separator:
            expect (resolveExplicitLevels (0, String { "abc" } + RLE + "def" + PDF + "\nxyz",
                                                       "000"     "0"   "111"   "1"    "0000"));

            // Complex nesting:
            expect (resolveExplicitLevels (0, String { "abc" } + RLE + "lmn" + RLO + "opq" + PDF + "xyz" + PDF + "jkl",
                                                       "000"     "0"   "111"   "1"   "333"   "3"   "111"   "1"   "000"));

            // Multiple embeddings:
            expect (resolveExplicitLevels (0, String { "abc" } + RLE + "lmn" + RLE + "lpq" + PDF + "rs" + PDF + "xyz",
                                                       "000"     "0"   "111"   "1"   "333"   "3"   "11"   "1"   "000"));
        }

        beginTest ("TR9 L1");
        {
            expect (l1 (0, { BidiType::ws, BidiType::s },                "10",  "00"));
            expect (l1 (1, { BidiType::ws, BidiType::s },                "10",  "11"));
            expect (l1 (4, { BidiType::ws, BidiType::ws, BidiType::ws }, "000", "444"));

            expect (l1 (0, { BidiType::fsi, BidiType::pdi, BidiType::rli, BidiType::lri }, "1111", "0000"));
            expect (l1 (0, { BidiType::fsi, BidiType::pdi, BidiType::ws, BidiType::lri },  "1111", "0000"));

            expect (l1 (0, { BidiType::fsi, BidiType::an, BidiType::rli, BidiType::lri }, "1100",  "1100"));
        }

        beginTest ("TR9 L2");
        {
            expect (l2 ("abc", "000", "abc"));
            expect (l2 ("abc", "111", "cba"));

            expect (l2 ("car MEANS CAR.",
                        "22211111111111",
                        ".RAC SNAEM car"));

            expect (l2 ("he said \"car MEANS CAR.\" \"IT DOES,\" she agreed.",
                        "00000000022211111111110000111111100000000000000",
                        "he said \"RAC SNAEM car.\" \"SEOD TI,\" she agreed."));

            expect (l2 ("DID YOU SAY \'he said \"car MEANS CAR\"\'?",
                        "11111111111112222222224443333333333211",
                        "?\'he said \"RAC SNAEM car\"\' YAS UOY DID"));
        }
    }

    static std::vector<UnicodeAnalysisPoint> generateTestUAPs (std::initializer_list<BidiType> bts, int embeddingLevel = 0)
    {
        static const std::unordered_map<BidiType, char32_t> map =
        {
            { BidiType::ltr, 0x0041 },
            { BidiType::rtl, 0x05d0 },
            { BidiType::b,   0x2029 },
            { BidiType::s,   0x001F },
            { BidiType::en,  0x0032 },
            { BidiType::an,  0x0664 },
            { BidiType::es,  '+' },
            { BidiType::et,  '%' },
            { BidiType::cs,  '.' },
            { BidiType::ws,  ' ' },
            { BidiType::nsm, 0x0300 },
            { BidiType::al,  0x0642 },
            { BidiType::bn,  0x0000 },
            { BidiType::pdi, 0x2069 },
            { BidiType::lri, 0x2066 },
            { BidiType::rli, 0x2067 },
            { BidiType::fsi, 0x2068 }
        };

        std::vector<UnicodeAnalysisPoint> out;
        out.reserve (bts.size());

        for (const auto& bt : bts)
        {
            const auto character = map.find (bt);

            if (character == map.end())
            {
                jassertfalse;
                return {};
            }

            // Sanity check!
            const auto realBT = UnicodeDataTable::getDataForCodepoint (character->second).bidi;

            if (bt != realBT)
            {
                jassertfalse;
                return {};
            }

            UnicodeEntry entry{};
            entry.bidi = bt;

            out.emplace_back (character->second, std::move (entry));
        }

        for (auto& p : out)
            p.embeddingLevel = (uint16_t) embeddingLevel;

        return out;
    }

    static bool n1 (int paragraphLevel, std::initializer_list<BidiType> input, std::initializer_list<BidiType> expected)
    {
        auto uaps = generateTestUAPs (input);

        if (uaps.empty())
            return false;

        TR9::n1 (uaps, paragraphLevel);
        return checkUAPs (uaps, expected);
    }

    static bool n2 (int embeddingLevel, std::initializer_list<BidiType> input, std::initializer_list<BidiType> expected)
    {
        auto uaps = generateTestUAPs (input, embeddingLevel);

        if (uaps.empty())
            return false;

        TR9::n2 (uaps);
        return checkUAPs (uaps, expected);
    }

    static bool l1 (int paragraphLevel, std::initializer_list<BidiType> bts, const String& levels, const String& expected)
    {
        auto uaps = generateTestUAPs (bts);

        if (uaps.empty())
            return false;

        if ((int) uaps.size() != levels.length() || (int) uaps.size() != expected.length())
        {
            jassertfalse;
            return false;
        }

        for (int i = 0; i < (int) uaps.size(); i++)
            uaps[(size_t) i].embeddingLevel = (uint16_t) levels.substring (i, i + 1).getIntValue();

        TR9::l1 (uaps, paragraphLevel);

        const auto result = [&uaps]
        {
            String s;

            for (auto uap : uaps)
                s << (int) uap.embeddingLevel;

            return s;
        }();

        return result == expected;
    }

    static bool l2 (const String& text, const String& levels, const String& expected)
    {
        const auto utf32  = text.toUTF32();
        const auto length = utf32.length();

        std::vector<int> levelVec;
        std::vector<int> reorderVec;

        levelVec.resize (length);
        reorderVec.resize (length);

        for (size_t i = 0; i < length; i++)
        {
            reorderVec[0] = 0;
            levelVec[i] = (uint16_t) levels.substring ((int) i, (int) i + 1).getIntValue();
        }

        TR9::l2 (reorderVec, levelVec);

        const auto result = [&reorderVec, text]
        {
            String s;

            for (auto level : reorderVec)
                s << text[level];

            return s;
        }();

        return result == expected;
    }

    static bool resolveParagraphEmbeddingLevel (std::initializer_list<BidiType> bts, int expected)
    {
        auto uaps = generateTestUAPs (bts);

        if (bts.size() > 0 && uaps.empty())
            return false;

        return TR9::resolveParagraphEmbeddingLevel (uaps) == expected;
    }

    static bool resolveWeakTypes (void (*func) (const TR9::WeakContext&), int paragraphLevel, std::initializer_list<BidiType> bts,
                                  std::initializer_list<BidiType> expected)
    {
        auto uaps = generateTestUAPs (bts);

        if (uaps.empty())
            return false;

        for (size_t i = 0; i < uaps.size(); i++)
        {
            const auto sos = i == 0;
            const auto eos = i == uaps.size() - 1;

            auto context = TR9::WeakContext { i,
                                              uaps,
                                              sos ? BidiType::on : uaps[i - 1].getBidiType(),
                                              eos ? BidiType::on : uaps[i + 1].getBidiType(),
                                              paragraphLevel };

            func (context);
        }

        return checkUAPs (uaps, expected);
    }

    static bool resolveImplicitTypes (int embeddingLevel, std::initializer_list<BidiType> bts, std::initializer_list<uint16_t> expected)
    {
        auto uaps = generateTestUAPs (bts, embeddingLevel);

        if (uaps.empty())
            return false;

        TR9::resolveImplicitTypes (uaps);

        for (size_t i = 0; i < uaps.size(); i++)
            if (uaps[i].embeddingLevel != expected.begin()[i])
                return false;

        return true;
    }

    static bool resolveExplicitLevels (int embeddingLevel, const String& input, const String& expectedLevels)
    {
        std::vector<UnicodeAnalysisPoint> uaps;

        const auto inputChars = input.toUTF32();
        uaps.reserve (inputChars.length());

        for (size_t i = 0; i < inputChars.length(); i++)
        {
            const auto codepoint = inputChars[(int) i];
            auto data = UnicodeDataTable::getDataForCodepoint ((uint32_t) codepoint);

            UnicodeAnalysisPoint uap { (char32_t) codepoint, data };
            uap.embeddingLevel = (uint16_t) embeddingLevel;
            uaps.push_back (uap);
        }

        TR9::resolveExplicitLevels (uaps, embeddingLevel);

        const auto levels = [&] {
            String s;

            for (auto point : uaps)
                s << (int) point.embeddingLevel;

            return s;
        }();

        return levels == expectedLevels;
    }

    static bool checkUAPs (Span<const UnicodeAnalysisPoint> uaps, std::initializer_list<BidiType> expected)
    {
        return std::equal (uaps.begin(), uaps.end(), expected.begin(), [] (UnicodeAnalysisPoint uap, BidiType bt)
        {
            return uap.getBidiType() == bt;
        });
    }
};

static BidiTests bidiTests;

#endif

} // namespace juce
