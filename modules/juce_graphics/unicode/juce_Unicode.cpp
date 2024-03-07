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

Array<Unicode::Codepoint> Unicode::performAnalysis (const String& string)
{
    if (string.isEmpty())
        return {};

    thread_local std::unordered_map<String, Array<Unicode::Codepoint>> cache;
    auto& result = cache[string];

    if (! result.isEmpty())
        return result;

    auto analysisBuffer = [&str = std::as_const (string)]
    {
        std::vector<UnicodeAnalysisPoint> points;

        const auto data   = str.toUTF32();
        const auto length = data.length();

        points.reserve (length);

        std::transform (data.getAddress(), data.getAddress() + length, std::back_inserter (points), [] (uint32_t cp)
        {
            UnicodeAnalysisPoint p;

            p.character = cp;
            p.data = getUnicodeDataForCodepoint (cp);
            p.bidi.level = 0;

           //#define JUCE_TR9_UPPERCASE_IS_RTL
           #if defined (JUCE_TR9_UPPERCASE_IS_RTL)
            if (cp >= 65 && cp <= 90)
                p.data.bidi = BidiType::al;
           #undef JUCE_TR9_UPPERCASE_IS_RTL
           #endif

            return p;
        });

        return points;
    }();

    struct ParagraphIterator
    {
        explicit ParagraphIterator (Span<UnicodeAnalysisPoint> Span) : data (Span) {}

        std::optional<Range<int>> next()
        {
            const auto start = head;
            auto end = start;

            if ((size_t) start < data.size())
            {
                while ((size_t) end < data.size())
                {
                    if (data[(size_t) end].character == 0x2029)
                        break;

                    end++;
                }


                head = end + 1;
                return std::make_optional (Range<int> { start, end });
            }

            return nullopt;
        }

        Span<UnicodeAnalysisPoint> data;
        int head = 0;
    };

    result.resize ((int) analysisBuffer.size());

    for (size_t i = 0; i < analysisBuffer.size(); i++)
        result.getReference ((int) i).codepoint = analysisBuffer[i].character;

    tr24::analyseScripts (analysisBuffer, [&result] (int index, TextScript script)
    {
        result.getReference (index).script = script;
    });

    tr14::analyseLineBreaks (analysisBuffer, [&result] (int index, TextBreakType type)
    {
        result.getReference ((int) index).breaking = type;
    });

    ParagraphIterator iter { analysisBuffer };

    while (auto range = iter.next())
    {
        const auto run  = Span { analysisBuffer.data() + (size_t) range->getStart(), (size_t) range->getLength() };
        const auto bidi = tr9::analyseBidiRun (run);

        for (size_t i = 0; i < (size_t) range->getLength(); i++)
        {
            auto& point = result.getReference ((int) i + range->getStart());

            point.direction      = bidi.resolvedLevels[i] % 2 == 0 ? TextDirection::ltr : TextDirection::rtl;
            point.logicalIndex   = (size_t) range->getStart() + i;
            point.visualIndex    = (size_t) bidi.visualOrder[i];
        }
    }

    return result;
}

// https://unicode-org.github.io/icu/userguide/transforms/bidi.html#logical-order-versus-visual-order
Array<Unicode::Codepoint> Unicode::convertLogicalToVisual (Span<const Unicode::Codepoint> codepoints)
{
    Array<Unicode::Codepoint> visual;

    visual.resize ((int) codepoints.size());

    for (const auto& codepoint : codepoints)
        visual.set ((int) codepoint.visualIndex, codepoint);

    return visual;
}

bool UnicodeFunctions::isRenderableCharacter (juce_wchar character)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
    switch (getUnicodeDataForCodepoint ((uint32_t) character).bt)
    {
        case LineBreakType::cr:
        case LineBreakType::lf:
        case LineBreakType::bk:
        case LineBreakType::nl:
        case LineBreakType::sp:
        case LineBreakType::zw:
        case LineBreakType::zwj:
        case LineBreakType::cm:
        case LineBreakType::cb:
            return false;

        default: break;
    }
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    return true;
}

bool UnicodeFunctions::isBreakableWhitespace (juce_wchar character)
{
    switch (character)
    {
        case 0x0020: case 0x1680: case 0x180E: case 0x2000:
        case 0x2001: case 0x2002: case 0x2003: case 0x2004:
        case 0x2005: case 0x2006: case 0x2007: case 0x2008:
        case 0x2009: case 0x200A: case 0x200B: case 0x202F:
        case 0x205F: case 0x3000:
            return true;

        default: break;
    }

    return false;
}

bool UnicodeFunctions::isEmoji (juce_wchar character)
{
    return getEmojiType ((uint32_t) character) != EmojiType::no;
}

bool UnicodeFunctions::shouldVerticalGlyphRotate (juce_wchar character)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
    switch (getUnicodeDataForCodepoint ((uint32_t) character).vertical)
    {
    case VerticalTransformType::R:
    case VerticalTransformType::Tr:
    case VerticalTransformType::Tu: return true;

    default: break;
    }
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    return false;
}

//#define JUCE_UNICODE_UNIT_TESTS

#if defined(JUCE_UNIT_TESTS) && defined(JUCE_UNICODE_UNIT_TESTS)
struct LineBreakTests : UnitTest
{
    LineBreakTests() : UnitTest ("UnicodeLineBreakTests", UnitTestCategories::unicode)
    {
    }

    void runTest()
    {
        static const auto data = []
        {
            using namespace generated;

            MemoryInputStream mStream {tr14TestData, sizeof (tr14TestData), false};
            GZIPDecompressorInputStream zStream {&mStream, false};

            MemoryBlock data {tr14TestDataUncompressedSize, false};
            zStream.read (data.getData(), data.getSize());

            return std::move (data);
        }();

        MemoryInputStream stream {data, false};

        int testCounter = 0;

        while (! stream.isExhausted())
        {
            uint32_t input[256]{};
            bool     output[256]{};

            const auto inputStringLength = (size_t) stream.readShort();
            jassert (inputStringLength < std::size (input));

            for (size_t i = 0; i < inputStringLength; i++)
                input[i] = (uint32_t) stream.readInt();

            for (size_t i = 0; i < inputStringLength + 1; i++)
                output[i] = (bool) stream.readBool();

            beginTest ("Test " + String (++testCounter));
            expect (runTest (Span<const uint32_t> { input,  inputStringLength },
                             Span<const bool>     { output, inputStringLength + 1 }));
        }
    }

    static bool runTest (Span<const uint32_t> input, Span<const bool> output)
    {
        UnicodeAnalysisPoint points[256]{};
        bool                 result[256]{};

        std::transform (input.begin(), input.end(), points, [] (uint32_t cp)
        {
            auto data = getUnicodePointForCodepoint (cp);
            return UnicodeAnalysisPoint {cp, data};
        });

        const auto resultCount = tr14::analyseLineBreaks ({ points, input.size() }, [&result, output] (int index, TextBreakType type)
        {
            jassert (index <= output.size());
            result[index] = type != TextBreakType::noBreak;
        });

        const auto s = std::equal (output.begin(), output.end(), std::begin (result)) &&
                       resultCount == output.size();

       #if JUCE_DEBUG
        if (! s)
        {
            String expected, actual;

            DBG ("Test Failed:");

            if (resultCount != output.size())
            {
                DBG ("\tIncorrect output size. Expected " << output.size() << " got " << resultCount);
                jassertfalse;
            }

            for (size_t i = 0; i < output.size(); i++)
            {
                expected << (output[i] ? "True" : "False") << " ";
                actual   << (result[i] ? "True" : "False") << " ";
            }

            String inputString;

            for (auto value : input)
                inputString << String::formatted ("%04X ", value);

            DBG ("\tInput:    { " << inputString << "}");
            DBG ("\tOutput:   { " << actual   << "}");
            DBG ("\tExpected: { " << expected << "}");

            DBG (tr14::debugString);
            jassertfalse;
        }
       #endif

        return s;
    }
};

static LineBreakTests lineBreakTests;

#endif

}
