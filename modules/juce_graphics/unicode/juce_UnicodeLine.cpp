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

namespace juce::tr14
{

using EAWType    = EastAsianWidthType;
using BreakClass = LineBreakType;

// This compiles down to the same machine code as an array matrix and
// gives us enum value compile time safety. This way we dont need to worry
// about the enum being modified and the table being out of sync.
template <size_t Size, typename ValueType, typename ResultType>
struct LookUpTable
{
    inline constexpr void set (ValueType v1, ValueType v2, ResultType result)
    {
        cols[index (v1)][index (v2)] = result;
    }

    inline constexpr ResultType get (ValueType v1, ValueType v2) const
    {
        return cols[index (v1)][index (v2)];
    }

private:
    static inline constexpr size_t index (ValueType value)
    {
        return (size_t) value;
    }

    ResultType cols[Size][Size];
};

enum class BreakOppurtunity
{
    direct,
    indirect,
    prohibited,
    combinedProhibited,
    combinedIndirect
};

struct BreakAtom final : public UnicodeAnalysisPoint
{
    constexpr bool operator== (BreakClass type) const { return data.bt == type; }
    constexpr bool operator== (EAWType type)    const { return data.asian == type; }

    constexpr auto getBreakClass()              const { return data.bt; }
    constexpr auto getEastAsianWidthType()      const { return data.asian; }
};

static_assert (sizeof (BreakAtom) == sizeof (UnicodeAnalysisPoint), "BreakAtom - UnicodeAnalysisPoint size mismatch");

struct BreakPairTable final : LookUpTable<42, BreakClass, BreakOppurtunity>
{
    BreakPairTable()
    {
        #include "juce_LineBreakTable.inl"
    }
};

static inline BreakClass resolve (BreakClass bc, [[maybe_unused]] bool mnMc = false)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
    switch (bc)
    {
        case BreakClass::ai:
        case BreakClass::sg:
        case BreakClass::xx:
            return BreakClass::al;

        case BreakClass::sa:
            return BreakClass::al;

        case BreakClass::cj:
            return BreakClass::ns;

        default: break;
    }
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    return bc;
}

static inline BreakClass resolveSOT (BreakClass bc)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
    switch (bc)
    {
        case BreakClass::lf:
        case BreakClass::nl:
            return BreakClass::bk;

        case BreakClass::sp:
            return BreakClass::wj;

        default: break;
    }
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    return bc;
}

static const BreakPairTable pairTable;

template <typename Callback>
size_t analyseLineBreaks (Span<const UnicodeAnalysisPoint> span, Callback&& callback)
{
    uint32_t resultIndex     = 0;
    uint32_t regionalCounter = 0;
    std::optional<BreakClass> lb9;
    bool lb21a{};

    const auto data = (const BreakAtom*) span.data();
    const auto len  = span.size();

    const auto emit = [&] (auto op) { callback ((int) resultIndex++, op); };

    #define step()   i++; continue

    for (size_t i = 0; i < len;)
    {
        const auto isSOT = i == 0;
        const auto isEOT = i == len - 1;

        const auto prev = isSOT ? resolveSOT (resolve (data[i].getBreakClass()))
                                : lb9.value_or (resolve (data[i].getBreakClass()));
        const auto next = isEOT ? BreakClass::cm
                                : resolve (data[i + 1].getBreakClass());

        lb9.reset();

        if (prev == BreakClass::cr && next == BreakClass::lf)
        {
            emit (TextBreakType::none);
            step();
        }

        // LB4
        if (any (prev, BreakClass::bk, BreakClass::lf, BreakClass::nl))
        {
            emit (TextBreakType::hard);
            step();
        }

        // LB6
        if (any (next, BreakClass::bk, BreakClass::lf, BreakClass::nl))
        {
            emit (TextBreakType::none);
            step();
        }

        // LB7
        if (any (next, BreakClass::sp, BreakClass::zw))
        {
            emit (TextBreakType::none);
            step();
        }

        // LB13
        if (any (next, BreakClass::cl, BreakClass::cp, BreakClass::ex, BreakClass::is, BreakClass::sy))
        {
            emit (TextBreakType::none);
            step();
        }

        // lb21a
        if (lb21a && any (prev, BreakClass::hy, BreakClass::ba))
        {
            emit (TextBreakType::none);
            step();
        }

        lb21a = prev == BreakClass::hl;

        // LB30a
        if (prev == BreakClass::ri)
        {
            regionalCounter++;

            if (next == BreakClass::ri && regionalCounter % 2 == 0)
            {
                regionalCounter = 0;
                emit (TextBreakType::soft);
                step();
            }
        }
        else
        {
            regionalCounter = 0;
        }

        const auto bt = pairTable.get (prev, next);

        switch (bt)
        {
            case BreakOppurtunity::direct:
            {
                emit (TextBreakType::soft);
                step();
            } break;

            case BreakOppurtunity::prohibited:
            {
                emit (TextBreakType::none);
                step();
            } break;

            case BreakOppurtunity::indirect:
            {
                while (i < len)
                {
                    emit (TextBreakType::none);

                    if (resolve (data[i].getBreakClass()) != BreakClass::sp)
                        break;

                    i++;
                }

                step();
            } break;

            case BreakOppurtunity::combinedIndirect:
            {
                lb9 = std::make_optional (prev);

                while (i < len)
                {
                    emit (TextBreakType::none);

                    if (! any (resolve (data[i].getBreakClass()), BreakClass::cm, BreakClass::zwj))
                        break;

                    i++;
                }

                step();
            } break;

            case BreakOppurtunity::combinedProhibited:
            {
                emit (TextBreakType::none);
                step();
            } break;
        }

        step();
    }

    //emit (TextBreakType::soft);

    #undef emit
    #undef step

    return resultIndex;
}

}
