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

namespace juce::tr9
{

enum EmbeddingLevel
{
    left  = 0,
    right = 1
};

static inline auto isOdd (int level)                 { return bool (level & 1); }
static inline auto computeLeastEven (int level)      { return isOdd (level) ? level + 1 : level + 2; }
static inline auto computeLeastOdd (int level)       { return isOdd (level) ? level + 2 : level + 1; }
static inline auto getEmbeddingDirection (int level) { return isOdd (level) ? BidiType::rtl : BidiType::ltr; }

struct Atom final : UnicodeAnalysisPoint
{
    auto getCharacter()              const { return character; }
    auto getType()                   const { return data.bidi; }
    auto getLevel()                  const { return bidi.level; }

    bool isIsolate()                 const { return isIsolateInitiator() || isIsolateTerminator(); }
    bool isIsolateInitiator()        const { return any (getType(), BidiType::lri, BidiType::rli, BidiType::fsi); }
    bool isIsolateTerminator()       const { return any (getType(), BidiType::pdi); }
    bool isStrong()                  const { return any (getType(), BidiType::rtl,   BidiType::ltr,   BidiType::al); }
    bool isNeutral()                 const { return any (getType(), BidiType::b,   BidiType::s,   BidiType::ws,  BidiType::on); }
    bool isWeak()                    const { return any (getType(), BidiType::en,  BidiType::es,  BidiType::et,  BidiType::an,
                                                                    BidiType::cs,  BidiType::nsm, BidiType::bn); }
    operator BidiType()              const { return getType(); }

    bool operator== (BidiType other) const { return getType() == other; }
    bool operator== (Atom other)     const { return getType() == other.getType(); }

    void setType (BidiType newType)        { data.bidi  = newType; }
    void setLevel (uint16_t newLevel)      { bidi.level = newLevel; }

    static Atom make (BidiType type)
    {
        Atom a{};
        a.data.bidi = type;
        return a;
    }

    static Atom none()
    {
        return make (BidiType::none);
    }
};

struct RunIterator
{
    RunIterator (Span<Atom> span) : data (span) {}

    std::optional<Range<int>> next()
    {
        if (data.size() <= head)
            return {};

        const auto start = head;
        const auto currentLevel = data[start].getLevel();

        const auto iter = std::find_if_not (data.begin() + start, data.end(),
                                            [currentLevel] (const Atom& atom)
        {
            return atom.getLevel() != currentLevel;
        });

        const auto end = iter != data.end() ? (size_t) std::distance (data.begin(), iter) : data.size();
        head = end + 1;

        return Range { (int) start, (int) end };
    }

private:
    Span<Atom> data;
    size_t head = 0;
};

template<typename T, int Capacity>
struct FixedStack
{
    constexpr FixedStack() = default;

    constexpr auto depth() const { return head; }
    constexpr bool full()  const { return head == Capacity; }
    constexpr bool empty() const { return head == 0; }
    constexpr void reset()       { head = 0; }

    void push (T val)
    {
        jassert (head < Capacity);
        data[head++] = val;
    }

    T pop()
    {
        jassert (head > 0);
        return data[--head];
    }

    constexpr T operator[] (int index) const { return data[index]; }

    constexpr T&   peek()        { return data[head - 1]; }
    constexpr auto begin() const { return data + 0; }
    constexpr auto end()   const { return data + head; }

private:
    T data[(size_t) Capacity] {};
    int head {};

    JUCE_DECLARE_NON_COPYABLE (FixedStack)
    JUCE_DECLARE_NON_MOVEABLE (FixedStack)
};

static inline int resolveParagraphEmbeddingLevel (const Span<Atom> buffer, Range<int> range = {})
{
    range = range.isEmpty() ? Range<int> {0, (int) buffer.size()} : range;

    auto seek = [buffer] (BidiType type, Range<int> seekRange) -> std::optional<int>
    {
        for (int i = seekRange.getStart(); i < seekRange.getEnd(); i++)
        {
            if (buffer[(size_t) i].getType() == type)
                return std::make_optional (i);
        }

        return {};
    };

    for (int i = range.getStart(); i < range.getEnd(); i++)
    {
        const auto& atom = buffer[(size_t) i];

        if (atom.isStrong())
        {
            return atom == BidiType::ltr ? EmbeddingLevel::left : EmbeddingLevel::right;
        }
        else if (atom.isIsolateInitiator())
        {
            // skip to past matching PDI or EOS
            const auto end = seek (BidiType::pdi, { i, range.getEnd() });
            i = end.has_value() ? *end + 1 : range.getEnd();
        }
    }

    return EmbeddingLevel::left;
}

static inline void resolveNeutralTypes (Span<Atom> buffer, int embeddingLevel, Range<int> range)
{
    range = range.isEmpty() ? Range<int> {0, (int) buffer.size()} : range;

    // BD13:
    const auto bracketRanges = [buffer, range]
    {
        struct Bracket
        {
            int position;
            uint32_t character;
            BracketType type;
        };

        FixedStack<Bracket, 64 - 1> stack;
        std::vector<Range<int>> brackets;

        for (int i = range.getStart(); i < range.getEnd(); i++)
        {
            const auto& curr = buffer[(size_t) i];

            if (curr == BidiType::on)
            {
                const auto type = getBracketType (curr.getCharacter());

                if (type == BracketType::open)
                {
                    if (stack.full())
                        return brackets;

                    stack.push ({ i, curr.getCharacter(), BracketType::open });
                }
                else if (type == BracketType::close)
                {
                    while (! stack.empty())
                    {
                        const auto head = stack.pop();

                        if (head.type == BracketType::open)
                        {
                            if (isMatchingBracketPair (head.character, curr.getCharacter()))
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
        const auto dir    = getEmbeddingDirection (embeddingLevel);
        const auto span   = Span { buffer.data() + bracketRange.getStart(), (size_t) bracketRange.getLength() };
        const auto strong = std::find_if (span.begin(), span.end(), [] (const Atom& atom)
                            {
                                return atom.isStrong();
                            });

        // TODO: refactor this
        if (strong != span.end())
        {
            if (strong->getType() == dir)
            {
                // B:
                buffer[(size_t) bracketRange.getStart()].setType (dir);
                buffer[(size_t) bracketRange.getEnd()].setType (dir);
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
                        if (buf[(size_t) i].isStrong())
                            return buf[(size_t) i].getType();
                    }

                    return dir;
                }();

                buffer[(size_t) bracketRange.getStart()].setType (strongContext);
                buffer[(size_t) bracketRange.getEnd()].setType (strongContext);
            }
        }
    }

    // N1:
    for (int i = range.getStart(); i < range.getEnd(); i++)
    {
        const auto curr = buffer[(size_t) i];
        const auto prev = i > 0 ? buffer[(size_t) i - 1] : Atom::none(); // SOS type?

        if (curr.isNeutral() || curr.isIsolate())
        {
            const auto endIndex = [buffer, start = i, end = range.getEnd()]
            {
                for (int j = start; j < end; j++)
                {
                    const auto atom = buffer[(size_t) j];

                    if (! (atom.isNeutral() || atom.isIsolate()))
                        return j;
                }

                return end - 1;
            }();

            auto isNumber    = [] (BidiType type) { return any (type, BidiType::an, BidiType::en); };
            const auto start = isNumber (prev)                      ? BidiType::rtl : prev.getType();
            const auto end   = isNumber (buffer[(size_t) endIndex]) ? BidiType::rtl : buffer[(size_t) endIndex].getType();

            const auto type  = start == end ? start : getEmbeddingDirection (embeddingLevel);
            std::for_each (buffer.begin() + i, buffer.begin() + endIndex, [type] (Atom& atom)
            {
                atom.setType (type);
            });

            i = endIndex;
        }
    }
}

static inline void resolveWeakTypes (Span<Atom> buffer, Range<int> range)
{
    range = range.isEmpty() ? Range<int> { 0, static_cast<int> (buffer.size()) } : range;

    for (int i = range.getStart(); i < range.getEnd(); i++)
    {
        auto& curr = buffer[(size_t) i];
        const auto prev = i > 0                  ? buffer[(size_t) i - 1] : Atom::none();
        const auto next = i < range.getEnd() - 1 ? buffer[(size_t) i + 1] : Atom::none();

        // W1:
        if (curr == BidiType::nsm)
            curr.setType (prev.isIsolate() ? BidiType::on : prev.getType());

        // W2:
        else if (curr == BidiType::en)
        {
            for (int j = i - 1; j >= 1; j--)
            {
                if (buffer[(size_t) j] == BidiType::al)
                    curr.setType (BidiType::al);

                if (buffer[(size_t) j].isStrong())
                    break;
            }
        }

        // W3:
        else if (curr == BidiType::al)
            curr.setType (BidiType::rtl);

        // W4:
        else if (curr == BidiType::es || curr == BidiType::cs)
        {
            if (prev == BidiType::en && next == BidiType::en)
            {
                curr.setType (BidiType::en);
            }
            else if (curr == BidiType::cs)
            {
                if (prev == BidiType::an && next == BidiType::an)
                    curr.setType (BidiType::an);
            }
        }

        // W5:
        else if (curr == BidiType::et)
        {
            //jassertfalse;
            if (prev == BidiType::en || next == BidiType::en)
                curr.setType (BidiType::en);
        }

        // W6
        if (any (curr.getType(), BidiType::es, BidiType::cs))
            curr.setType (BidiType::on);

        // W7:
        else if (curr == BidiType::en)
        {
            for (int j = i - 1; j >= 1; j--)
            {
                if (buffer[(size_t) j] == BidiType::ltr)
                    curr.setType (BidiType::ltr);

                if (buffer[(size_t) j].isStrong())
                    break;
            }
        }
    }
}

static inline void resolveImplicitTypes (Span<Atom> buffer, int embeddingLevel, Range<int> range)
{
    range = range.isEmpty() ? Range<int> { 0, static_cast<int> (buffer.size()) } : range;

    // I1, I2
    // https://www.unicode.org/reports/tr9/#Resolving_Implicit_Levels
    for (int i = range.getStart(); i < range.getEnd(); i++)
    {
        auto& curr = buffer[(size_t) i];

        const auto level = (uint16_t) embeddingLevel;
        const auto isEven = isOdd (level) == false;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
        switch (curr.getType())
        {
        case BidiType::ltr: curr.setLevel (isEven ? level     : level + 1); break;
        case BidiType::rtl: curr.setLevel (isEven ? level + 1 : level    ); break;
        case BidiType::an:  curr.setLevel (isEven ? level + 2 : level + 1); break;
        case BidiType::en:  curr.setLevel (isEven ? level + 2 : level + 1); break;

        default: break;
        }
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }
}

static inline void resolveExplicitLevels (Span<Atom> buffer, int embeddingLevel)
{
    constexpr auto MAX_DEPTH = 126 - 1;

    struct State
    {
        int embeddingLevel;
        enum { Neutral, rtl, ltr } directionalOverride;
        bool isolateStatus;
    };

    // X1
    struct
    {
        int isolate = 0;
        int embedded = 0;
    } overflow {};

    FixedStack<State, MAX_DEPTH> stack;

    stack.push ({ embeddingLevel, State::Neutral, false });

    auto getHead = [&stack] { return &stack.peek(); };
    auto save    = [&stack] { stack.push (stack.peek()); };
    auto restore = [&stack] { stack.pop(); };

    auto isValid = [&] (auto value) { return (value < MAX_DEPTH) && (overflow.isolate == 0 && overflow.embedded == 0); };

    // X2-X6a
    for (size_t i = 0; i < buffer.size(); i++)
    {
        auto& atom = buffer[i];

        // X2-X3: Explicit embeddings
        if (atom == BidiType::rle || atom == BidiType::lre)
        {
            auto* state = getHead();
            state->embeddingLevel = atom == BidiType::rle ? computeLeastOdd (state->embeddingLevel)
                                                          : computeLeastEven (state->embeddingLevel);

            if (isValid (state->embeddingLevel))
            {
                state->directionalOverride = State::Neutral;
                state->isolateStatus = false;
                save();
            }
            else if (overflow.isolate == 0)
            {
                overflow.embedded++;
            }
        }

        // X4-X5: Explicit Overrides
        else if (atom == BidiType::rlo || atom == BidiType::lro)
        {
            auto* state = getHead();
            state->embeddingLevel = atom == BidiType::rlo ? computeLeastOdd (state->embeddingLevel)
                                                          : computeLeastEven (state->embeddingLevel);

            if (isValid (state->embeddingLevel))
            {
                state->directionalOverride = atom == BidiType::rlo ? State::rtl : State::ltr;
                state->isolateStatus = false;
                save();
            }
            else if (overflow.isolate == 0)
            {
                overflow.embedded++;
            }
        }

        // X5a-X5b: Isolates
        else if (atom == BidiType::rli || atom == BidiType::lri)
        {
            auto* state = getHead();
            state->embeddingLevel = atom == BidiType::rli ? computeLeastOdd (state->embeddingLevel)
                                                          : computeLeastEven (state->embeddingLevel);

            if (state->directionalOverride == State::ltr)
                atom.setType (BidiType::ltr);
            else if (state->directionalOverride == State::rtl)
                atom.setType (BidiType::rtl);

            if (isValid (state->embeddingLevel))
            {
                state->directionalOverride = State::Neutral;
                state->isolateStatus = true;
                save();
            }
            else
            {
                overflow.isolate++;
            }
        }

        // X5c
        else if (atom == BidiType::fsi)
        {
            //auto pEmbedLevel = resolveParagraphEmbeddingLevel(buffer, { i + 1, (int) buffer.size() });
            jassertfalse;
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

                while (! getHead()->isolateStatus)
                    restore();

                restore();
            }

            atom.setLevel ((uint16_t) getHead()->embeddingLevel);
        }

        // X7
        else if (atom == BidiType::pdf)
        {
            if (overflow.isolate > 0)
            {
                // Maybe???
                overflow.isolate--;
            }
            else if (overflow.embedded > 0)
            {
                overflow.embedded--;
            }
            else if (getHead()->isolateStatus == false && stack.depth() >= 2)
            {
                restore();
            }
        }

        // X8
        else if (atom == BidiType::b)
        {
            atom.setLevel ((uint16_t) getHead()->embeddingLevel);

            overflow.embedded = 0;
            overflow.isolate = 0;
            stack.reset();
            stack.push ({ embeddingLevel, State::Neutral, false });
        }

        // X6: Everything else
        // ! (B | BN | RLE | LRE | RLO | LRO | PDF | RLI | LRI | FSI | PDI)
        else
        {
            auto* head = getHead();
            atom.setLevel ((uint16_t) head->embeddingLevel);
        }
    }
}

static inline std::vector<int> resolveReorderedIndices (const Span<Atom> buffer, int embeddingLevel, Range<int> range = {})
{
    range = range.isEmpty() ? Range<int> { 0, static_cast<int> (buffer.size()) } : range;

    std::vector<int> levels;
    levels.resize ((size_t) range.getLength());

    for (int i = range.getStart(); i < range.getEnd(); i++)
        levels[(size_t) i] = buffer[(size_t) i].getLevel();

    // L1:
    for (int i = range.getStart(); i < range.getEnd(); i++)
    {
        auto curr = buffer[(size_t) i];

        if (any<BidiType> (curr, BidiType::s, BidiType::b))
            levels[(size_t) i] = embeddingLevel;

        for (int j = i - 1; j >= 0; j--)
        {
            curr = buffer[(size_t) j];

            if (! curr.isIsolate())
                break;

            levels[(size_t) j] = embeddingLevel;
        }
    }

    // TODO: line breaks

    // L2:
    std::vector<int> reordered;
    reordered.resize ((size_t) range.getLength());
    std::iota (reordered.begin(), reordered.end(), 0);
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

                std::reverse (reordered.begin() + start, reordered.begin() + (int) end);
                i = (int) end;
            }
        }
    }

    return reordered;
}

struct BidiOutput
{
    int embeddedingLevel = -1;
    std::vector<int> resolvedLevels;
    std::vector<int> visualOrder;
};

static BidiOutput analyseBidiRun (Span<UnicodeAnalysisPoint> buffer, std::optional<bool> rtlOverride = std::nullopt)
{
    auto stream = Span {(Atom*) buffer.data(), buffer.size()};

    // BD1
    const auto baseLevel = rtlOverride.has_value() ? *rtlOverride ? 1 : 0
                                                   : resolveParagraphEmbeddingLevel (stream);

    std::for_each (stream.begin(), stream.end(), [baseLevel] (Atom& atom)
    {
        atom.setLevel ((uint16_t) baseLevel);
    });

    resolveExplicitLevels (stream, baseLevel);

    // X9 replace override characters
    const auto pred = [] (auto atom) { return any (atom.getType(), BidiType::rle, BidiType::lre,
                                                                   BidiType::lro, BidiType::rlo,
                                                                   BidiType::pdf); };
    std::replace_if (stream.begin(), stream.end(), pred, Atom::make (BidiType::bn));
    //buffer.erase (remove_if (stream.begin(), stream.end(), pred), stream.end());

    // W1-W7
    resolveWeakTypes (stream, {});

    // N0-N2
    resolveNeutralTypes (stream, baseLevel, {});

    // I1-I2
    resolveImplicitTypes (stream, baseLevel, {});

    BidiOutput output;

    output.embeddedingLevel = baseLevel;

    for (const auto& atom : stream)
        output.resolvedLevels.push_back (atom.getLevel());

    output.visualOrder = resolveReorderedIndices (stream, baseLevel, {});

    return output;
}

} // namespace juce::tr9
