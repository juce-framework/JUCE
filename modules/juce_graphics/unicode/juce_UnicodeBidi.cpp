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

class BidiLine
{
public:
    using ParagraphPtr = std::unique_ptr<std::remove_pointer_t<SBParagraphRef>, FunctionPointerDestructor<SBParagraphRelease>>;
    using LinePtr = std::unique_ptr<std::remove_pointer_t<SBLineRef>, FunctionPointerDestructor<SBLineRelease>>;

    explicit BidiLine (ParagraphPtr p, LinePtr l) : paragraph (std::move (p)), line (std::move (l)) {}

    Span<const SBRun> getRuns() const
    {
        return { SBLineGetRunsPtr (line.get()), SBLineGetRunCount (line.get()) };
    }

    void computeVisualOrder (std::vector<size_t>& result) const
    {
        result.clear();

        const auto runs = getRuns();

        if (runs.empty())
            return;

        thread_local std::vector<size_t> codepointIndicesInVisualOrder;
        codepointIndicesInVisualOrder.clear();
        codepointIndicesInVisualOrder.reserve ((size_t) SBLineGetLength (line.get()));

        jassert (SBLineGetOffset (line.get()) == 0);

        for (const auto& run : runs)
        {
            const auto ltr = run.level % 2 == 0;
            const auto increment = ltr ? 1 : -1;
            auto start = (int) (ltr ? run.offset : run.offset + run.length - 1);

            for (SBUInteger i = 0; i < run.length; ++i)
            {
                codepointIndicesInVisualOrder.push_back ((size_t) start);
                start += increment;
            }
        }

        result.assign (codepointIndicesInVisualOrder.size(), 0);

        if (std::any_of (codepointIndicesInVisualOrder.begin(),
                         codepointIndicesInVisualOrder.end(),
                         [s = result.size()] (auto i) { return i >= s; }))
        {
            jassertfalse;
            return;
        }

        for (const auto [i, index] : enumerate (codepointIndicesInVisualOrder, size_t{}))
            result[index] = i;
    }

private:
    ParagraphPtr paragraph;
    LinePtr line;
};

class BidiParagraph
{
public:
    using ParagraphPtr = BidiLine::ParagraphPtr;

    explicit BidiParagraph (ParagraphPtr p)
        : paragraph (std::move (p))
    {
    }

    size_t getLength() const
    {
        return SBParagraphGetLength (paragraph.get());
    }

    Span<const SBLevel> getResolvedLevels() const
    {
        return { SBParagraphGetLevelsPtr (paragraph.get()), getLength() };
    }

    BidiLine createLine (size_t length) const
    {
        jassert (SBParagraphGetOffset (paragraph.get()) == 0);
        jassert (length <= getLength());
        return BidiLine { ParagraphPtr { SBParagraphRetain (paragraph.get()) },
                          BidiLine::LinePtr { SBParagraphCreateLine (paragraph.get(), 0, length) } };
    }

private:
    ParagraphPtr paragraph;
};

class BidiAlgorithm
{
public:
    using AlgorithmPtr = std::unique_ptr<std::remove_pointer_t<SBAlgorithmRef>, FunctionPointerDestructor<SBAlgorithmRelease>>;

    explicit BidiAlgorithm (Span<const juce_wchar> t)
        : text (t.begin(), t.end())
    {
    }

    size_t getLength() const
    {
        return text.size();
    }

    BidiParagraph createParagraph (std::optional<detail::TextDirection> d = {}) const
    {
        BidiParagraph::ParagraphPtr result { SBAlgorithmCreateParagraph (algorithm.get(), 0, text.size(), [&]() -> SBLevel
        {
            if (! d.has_value())
                return SBLevelDefaultLTR;
            return *d == detail::TextDirection::rtl ? SBLevelDefaultRTL : SBLevelDefaultLTR;
        }()) };

        jassert (result != nullptr);

        return BidiParagraph { std::move (result) };
    }

private:
    std::vector<juce_wchar> text;
    AlgorithmPtr algorithm { [&]
    {
        SBCodepointSequence sequence { SBStringEncodingUTF32, text.data(), text.size() };
        return SBAlgorithmCreate (&sequence);
    }() };
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
        beginTest ("visual order rtl");
        {
            const CharPointer_UTF8 text ("\xd9\x85\xd9\x85\xd9\x85 colour "
                                         "\xd9\x85\xd9\x85\xd9\x85\xd9\x85\xd9\x85\xd9\x85\xd9\x85\xd9\x85\n");
            const std::vector<size_t> result { 19, 18, 17, 16, 10, 11, 12, 13, 14, 15, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

            expect (computeVisualOrder (text) == result);
        }

        beginTest ("visual order ltr");
        {
            const CharPointer_UTF8 text ("hello \xd9\x85\xd9\x85\xd9\x85 world\n");
            const std::vector<size_t> result { 0, 1, 2, 3, 4, 5, 8, 7, 6, 9, 10, 11, 12, 13, 14, 15 };
            expect (computeVisualOrder (text) == result);
        }

        beginTest ("multi-level bidi text");
        {
            const CharPointer_UTF8 text ("LOOPS 4 \xd7\xa1X \xd7\xa1""4");
            const std::vector<size_t> result { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 11 };
            expect (computeVisualOrder (text) == result);
        }

        beginTest ("bidi text with 5 embedding levels");
        {
            // pop directional formatting
            String PDF { CharPointer_UTF8 { "\xe2\x80\xac" } };

            // left to right override
            String LRO { CharPointer_UTF8 { "\xe2\x80\xad" } };

            // right to left override
            String RLO { CharPointer_UTF8 { "\xe2\x80\xae" } };

            const auto replacements = std::array {
                std::make_pair (String { "[PDF]" }, PDF),
                std::make_pair (String { "[LRO]" }, LRO),
                std::make_pair (String { "[RLO]" }, RLO),
            };

            auto getText = [&] (StringRef templateText)
            {
                String text = templateText;

                for (const auto& [placeholder, replacement] : replacements)
                    text = text.replace (placeholder, replacement);

                return text;
            };

            const CharPointer_UTF8 templ { "[RLO]DID YOU SAY '[LRO]he said \"[RLO][LRO]car[PDF] MEANS CAR[PDF]\"[PDF]'?[PDF]" };
            const auto text = getText (templ);
            const auto lookup = computeVisualOrder (text);

            std::vector<juce_wchar> reorderedText ((size_t) text.length());

            for (const auto [i, c] : enumerate (text, size_t{}))
                reorderedText[lookup[i]] = c;

            // The visual order of the control characters is not defined, so we can only compare the
            // ascii part.
            std::vector<juce_wchar> asciiPartOfReorderedText;

            for (const auto c : reorderedText)
            {
                if (c >= 0x20 && c <= 0x7e)
                    asciiPartOfReorderedText.push_back (c);
            }

            const String expected { "?'he said \"RAC SNAEM car\"' YAS UOY DID" };
            const String result { CharPointer_UTF32 { asciiPartOfReorderedText.data() }, asciiPartOfReorderedText.size() };

            expect (result == expected);
        }
    }

    static std::vector<size_t> computeVisualOrder (const String& text)
    {
        std::vector<juce_wchar> chars;

        for (const auto t : text)
            chars.push_back (t);

        BidiAlgorithm algorithm { chars };
        auto paragraph = algorithm.createParagraph();
        auto line = paragraph.createLine (paragraph.getLength());

        std::vector<size_t> order;
        line.computeVisualOrder (order);
        return order;
    }
};

static BidiTests bidiTests;

#endif

} // namespace juce
