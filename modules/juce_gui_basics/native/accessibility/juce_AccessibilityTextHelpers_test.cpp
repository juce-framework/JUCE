/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct AccessibilityTextHelpersTest : public UnitTest
{
    AccessibilityTextHelpersTest()
        : UnitTest ("AccessibilityTextHelpers", UnitTestCategories::gui) {}

    void runTest() override
    {
        using ATH = AccessibilityTextHelpers;

        beginTest ("Android find word end");
        {
            const auto testMultiple = [this] (String str,
                                              int start,
                                              const std::vector<int>& collection)
            {
                auto it = collection.begin();

                for (const auto direction : { ATH::Direction::forwards, ATH::Direction::backwards })
                {
                    for (const auto includeBoundary : { ATH::IncludeThisBoundary::no, ATH::IncludeThisBoundary::yes })
                    {
                        for (const auto includeWhitespace : { ATH::IncludeWhitespaceAfterWords::no, ATH::IncludeWhitespaceAfterWords::yes })
                        {
                            const auto actual = ATH::findNextWordEndOffset (str.begin(), str.end(), str.begin() + start, direction, includeBoundary, includeWhitespace);
                            const auto expected = *it++;
                            expect (expected == actual);
                        }
                    }
                }
            };

            // Character Indices         0  3 56       13                                                                    50          51
            //                           |  | ||       |                                                                     |           |
            const auto string = String ("hello world \r\n with some  spaces in this sentence ") + String (CharPointer_UTF8 ("\xe2\x88\xae E\xe2\x8b\x85""da = Q"));
            // Direction                 forwards    forwards    forwards    forwards    backwards   backwards   backwards   backwards
            // IncludeBoundary           no          no          yes         yes         no          no          yes         yes
            // IncludeWhitespace         no          yes         no          yes         no          yes         no          yes
            testMultiple (string,   0, { 5,          6,          5,          0,           0,          0,          0,          0 });
            testMultiple (string,   3, { 2,          3,          2,          3,          -3,         -3,         -3,         -3 });
            testMultiple (string,   5, { 6,          1,          0,          1,          -5,         -5,         -5,          0 });
            testMultiple (string,   6, { 5,          9,          5,          0,          -6,         -1,          0,         -1 });
            testMultiple (string,  13, { 6,          2,          6,          2,          -7,         -2,         -7,         -2 });
            testMultiple (string,  50, { 1,          2,          1,          0,          -9,         -1,          0,         -1 });
            testMultiple (string,  51, { 5,          1,          0,          1,          -1,         -2,         -1,          0 });

            testMultiple ("  a b ", 0, { 3,          2,          0,          2,           0,          0,          0,          0 });
            testMultiple ("  a b ", 1, { 2,          1,          2,          1,          -1,         -1,         -1,         -1 });
        }

        beginTest ("Android text range adjustment");
        {
            const auto testMultiple = [this] (String str,
                                              Range<int> initial,
                                              auto boundary,
                                              const std::vector<Range<int>>& collection)
            {
                auto it = collection.begin();

                for (auto extend : { ATH::ExtendSelection::no, ATH::ExtendSelection::yes })
                {
                    for (auto direction : { ATH::Direction::forwards, ATH::Direction::backwards })
                    {
                        for (auto insert : { CursorPosition::begin, CursorPosition::end })
                        {
                            const MockAccessibilityTextInterface mock { str, initial, insert };
                            const auto actual = ATH::findNewSelectionRangeAndroid (mock, boundary, extend, direction);
                            const auto expected = *it++;
                            expect (expected == actual);
                        }
                    }
                }
            };

            // Extend                                                                       no              no              no              no              yes             yes             yes             yes
            // Direction                                                                    forwards        forwards        backwards       backwards       forwards        forwards        backwards       backwards
            // Insert                                                                       begin           end             begin           end             begin           end             begin           end
            testMultiple ("hello world", {  5,  5 }, ATH::BoundaryType::character,        { { 6,  6 },      { 6, 6 },       { 4, 4 },       { 4, 4 },       { 5, 6 },       { 5, 6 },       { 4, 5 },       { 4, 5 } });
            testMultiple ("hello world", {  0,  0 }, ATH::BoundaryType::character,        { { 1, 1 },       { 1, 1 },       { 0, 0 },       { 0, 0 },       { 0, 1 },       { 0, 1 },       { 0, 0 },       { 0, 0 } });
            testMultiple ("hello world", { 11, 11 }, ATH::BoundaryType::character,        { { 11, 11 },     { 11, 11 },     { 10, 10 },     { 10, 10 },     { 11, 11 },     { 11, 11 },     { 10, 11 },     { 10, 11 } });
            testMultiple ("hello world", {  4,  5 }, ATH::BoundaryType::character,        { { 5, 5 },       { 6, 6 },       { 3, 3 },       { 4, 4 },       { 5, 5 },       { 4, 6 },       { 3, 5 },       { 4, 4 } });
            testMultiple ("hello world", {  0,  1 }, ATH::BoundaryType::character,        { { 1, 1 },       { 2, 2 },       { 0, 0 },       { 0, 0 },       { 1, 1 },       { 0, 2 },       { 0, 1 },       { 0, 0 } });
            testMultiple ("hello world", { 10, 11 }, ATH::BoundaryType::character,        { { 11, 11 },     { 11, 11 },     { 9, 9 },       { 10, 10 },     { 11, 11 },     { 10, 11 },     { 9, 11 },      { 10, 10 } });

            testMultiple ("foo  bar  baz", { 0, 0 }, ATH::BoundaryType::word,             { { 3, 3 },       { 3, 3 },       { 0, 0 },       { 0, 0 },       { 0, 3 },       { 0, 3 },       { 0, 0 },       { 0, 0 } });
            testMultiple ("foo  bar  baz", { 1, 6 }, ATH::BoundaryType::word,             { { 3, 3 },       { 8, 8 },       { 0, 0 },       { 5, 5 },       { 3, 6 },       { 1, 8 },       { 0, 6 },       { 1, 5 } });
            testMultiple ("foo  bar  baz", { 3, 3 }, ATH::BoundaryType::word,             { { 8, 8 },       { 8, 8 },       { 0, 0 },       { 0, 0 },       { 3, 8 },       { 3, 8 },       { 0, 3 },       { 0, 3 } });
            testMultiple ("foo  bar  baz", { 3, 5 }, ATH::BoundaryType::word,             { { 8, 8 },       { 8, 8 },       { 0, 0 },       { 0, 0 },       { 5, 8 },       { 3, 8 },       { 0, 5 },       { 0, 3 } });

            testMultiple ("foo bar\n\n\na b\nc d e", { 0, 0 }, ATH::BoundaryType::line,   { { 8, 8 },       { 8, 8 },       { 0, 0 },       { 0, 0 },       { 0, 8 },       { 0, 8 },       { 0, 0 },       { 0, 0 } });
            testMultiple ("foo bar\n\n\na b\nc d e", { 7, 7 }, ATH::BoundaryType::line,   { { 8, 8 },       { 8, 8 },       { 0, 0 },       { 0, 0 },       { 7, 8 },       { 7, 8 },       { 0, 7 },       { 0, 7 } });
            testMultiple ("foo bar\n\n\na b\nc d e", { 8, 8 }, ATH::BoundaryType::line,   { { 9, 9 },       { 9, 9 },       { 0, 0 },       { 0, 0 },       { 8, 9 },       { 8, 9 },       { 0, 8 },       { 0, 8 } });

            testMultiple ("foo bar\r\na b\r\nxyz", {  0,  0 }, ATH::BoundaryType::line,   { { 9, 9 },       { 9, 9 },       { 0, 0 },       { 0, 0 },       { 0, 9 },       { 0, 9 },       { 0, 0 },       { 0, 0 } });
            testMultiple ("foo bar\r\na b\r\nxyz", { 10, 10 }, ATH::BoundaryType::line,   { { 14, 14 },     { 14, 14 },     { 9, 9 },       { 9, 9 },       { 10, 14 },     { 10, 14 },     { 9, 10 },      { 9, 10 } });
        }
    }

    enum class CursorPosition { begin, end };

    class MockAccessibilityTextInterface : public AccessibilityTextInterface
    {
    public:
        MockAccessibilityTextInterface (String stringIn, Range<int> selectionIn, CursorPosition insertIn)
            : string (stringIn), selection (selectionIn), insert (insertIn) {}

        bool isDisplayingProtectedText()                  const override { return false; }
        bool isReadOnly()                                 const override { return false; }
        int getTotalNumCharacters()                       const override { return string.length(); }
        Range<int> getSelection()                         const override { return selection; }
        int getTextInsertionOffset()                      const override { return insert == CursorPosition::begin ? selection.getStart() : selection.getEnd(); }
        String getText (Range<int> range)                 const override { return string.substring (range.getStart(), range.getEnd()); }
        RectangleList<int> getTextBounds (Range<int>)     const override { return {}; }
        int getOffsetAtPoint (Point<int>)                 const override { return 0; }

        void setSelection (Range<int> newRange)                 override { selection = newRange; }
        void setText (const String& newText)                    override { string = newText; }

    private:
        String string;
        Range<int> selection;
        CursorPosition insert;
    };
};

static AccessibilityTextHelpersTest accessibilityTextHelpersTest;

} // namespace juce
