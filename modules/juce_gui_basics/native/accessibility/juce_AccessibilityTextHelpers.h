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

struct AccessibilityTextHelpers
{
    /*  Wraps a CharPtr into a stdlib-compatible iterator.

        MSVC's std::reverse_iterator requires the wrapped iterator to be default constructible
        when building in C++20 mode, but I don't really want to add public default constructors to
        the CharPtr types. Instead, we add a very basic default constructor here which sets the
        wrapped CharPtr to nullptr.
    */
    template <typename CharPtr>
    class CharPtrIteratorAdapter
    {
    public:
        using difference_type = int;
        using value_type = decltype (*std::declval<CharPtr>());
        using pointer = value_type*;
        using reference = value_type;
        using iterator_category = std::bidirectional_iterator_tag;

        CharPtrIteratorAdapter() = default;
        constexpr explicit CharPtrIteratorAdapter (CharPtr arg) : ptr (arg) {}

        constexpr auto operator*() const { return *ptr; }

        constexpr CharPtrIteratorAdapter& operator++()
        {
            ++ptr;
            return *this;
        }

        constexpr CharPtrIteratorAdapter& operator--()
        {
            --ptr;
            return *this;
        }

        constexpr bool operator== (const CharPtrIteratorAdapter& other) const { return ptr == other.ptr; }
        constexpr bool operator!= (const CharPtrIteratorAdapter& other) const { return ptr != other.ptr; }

        constexpr auto operator+ (difference_type offset) const { return CharPtrIteratorAdapter { ptr + offset }; }
        constexpr auto operator- (difference_type offset) const { return CharPtrIteratorAdapter { ptr - offset }; }

    private:
        CharPtr ptr { {} };
    };

    template <typename CharPtr>
    static auto makeCharPtrIteratorAdapter (CharPtr ptr)
    {
        return CharPtrIteratorAdapter<CharPtr> { ptr };
    }

    enum class BoundaryType
    {
        character,
        word,
        line,
        document
    };

    enum class Direction
    {
        forwards,
        backwards
    };

    enum class ExtendSelection
    {
        no,
        yes
    };

    /*  Indicates whether a function may return the current text position, in the case that the
        position already falls on a text unit boundary.
    */
    enum class IncludeThisBoundary
    {
        no,     //< Always search for the following boundary, even if the current position falls on a boundary
        yes     //< Return the current position if it falls on a boundary
    };

    /*  Indicates whether a word boundary should include any whitespaces that follow the
        non-whitespace characters.
    */
    enum class IncludeWhitespaceAfterWords
    {
        no,     //< The word ends on the first whitespace character
        yes     //< The word ends after the last whitespace character
    };

    /*  Like std::distance, but always does an O(N) count rather than an O(1) count, and doesn't
        require the iterators to have any member type aliases.
    */
    template <typename Iter>
    static int countDifference (Iter from, Iter to)
    {
        int distance = 0;

        while (from != to)
        {
            ++from;
            ++distance;
        }

        return distance;
    }

    /*  Returns the number of characters between ptr and the next word end in a specific
        direction.

        If ptr is inside a word, the result will be the distance to the end of the same
        word.
    */
    template <typename CharPtr>
    static int findNextWordEndOffset (CharPtr beginIn,
                                      CharPtr endIn,
                                      CharPtr ptrIn,
                                      Direction direction,
                                      IncludeThisBoundary includeBoundary,
                                      IncludeWhitespaceAfterWords includeWhitespace)
    {
        const auto begin = makeCharPtrIteratorAdapter (beginIn);
        const auto end   = makeCharPtrIteratorAdapter (endIn);
        const auto ptr   = makeCharPtrIteratorAdapter (ptrIn);

        const auto move = [&] (auto b, auto e, auto iter)
        {
            const auto isSpace = [] (juce_wchar c) { return CharacterFunctions::isWhitespace (c); };

            const auto start = [&]
            {
                if (iter == b && includeBoundary == IncludeThisBoundary::yes)
                    return b;

                const auto nudged = iter - (iter != b && includeBoundary == IncludeThisBoundary::yes ? 1 : 0);

                return includeWhitespace == IncludeWhitespaceAfterWords::yes
                       ? std::find_if (nudged, e, isSpace)
                       : std::find_if_not (nudged, e, isSpace);
            }();

            const auto found = includeWhitespace == IncludeWhitespaceAfterWords::yes
                             ? std::find_if_not (start, e, isSpace)
                             : std::find_if (start, e, isSpace);

            return countDifference (iter, found);
        };

        return direction == Direction::forwards ? move (begin, end, ptr)
                                                : -move (std::make_reverse_iterator (end),
                                                         std::make_reverse_iterator (begin),
                                                         std::make_reverse_iterator (ptr));
    }

    /*  Returns the number of characters between ptr and the beginning of the next line in a
        specific direction.
    */
    template <typename CharPtr>
    static int findNextLineOffset (CharPtr beginIn,
                                   CharPtr endIn,
                                   CharPtr ptrIn,
                                   Direction direction,
                                   IncludeThisBoundary includeBoundary)
    {
        const auto begin = makeCharPtrIteratorAdapter (beginIn);
        const auto end   = makeCharPtrIteratorAdapter (endIn);
        const auto ptr   = makeCharPtrIteratorAdapter (ptrIn);

        const auto findNewline = [] (auto from, auto to) { return std::find (from, to, juce_wchar { '\n' }); };

        if (direction == Direction::forwards)
        {
            if (ptr != begin && includeBoundary == IncludeThisBoundary::yes && *(ptr - 1) == '\n')
                return 0;

            const auto newline = findNewline (ptr, end);
            return countDifference (ptr, newline) + (newline == end ? 0 : 1);
        }

        const auto rbegin   = std::make_reverse_iterator (ptr);
        const auto rend     = std::make_reverse_iterator (begin);

        return -countDifference (rbegin, findNewline (rbegin + (rbegin == rend || includeBoundary == IncludeThisBoundary::yes ? 0 : 1), rend));
    }

    /*  Unfortunately, the method of computing end-points of text units depends on context, and on
        the current platform.

        Some examples of different behaviour:
        - On Android, updating the cursor/selection always searches for the next text unit boundary;
          but on Windows, ExpandToEnclosingUnit() should not move the starting point of the
          selection if it already at a unit boundary. This means that we need both inclusive and
          exclusive methods for finding the next text boundary.
        - On Android, moving the cursor by 'words' should move to the first space following a
          non-space character in the requested direction. On Windows, a 'word' includes trailing
          whitespace, but not preceding whitespace. This means that we need a way of specifying
          whether whitespace should be included when navigating by words.
    */
    static int findTextBoundary (const AccessibilityTextInterface& textInterface,
                                 int currentPosition,
                                 BoundaryType boundary,
                                 Direction direction,
                                 IncludeThisBoundary includeBoundary,
                                 IncludeWhitespaceAfterWords includeWhitespace)
    {
        const auto numCharacters = textInterface.getTotalNumCharacters();
        const auto isForwards = (direction == Direction::forwards);
        const auto currentClamped = jlimit (0, numCharacters, currentPosition);

        switch (boundary)
        {
            case BoundaryType::character:
            {
                const auto offset = includeBoundary == IncludeThisBoundary::yes ? 0
                                                                                : (isForwards ? 1 : -1);
                return jlimit (0, numCharacters, currentPosition + offset);
            }

            case BoundaryType::word:
            {
                const auto str = textInterface.getText ({ 0, numCharacters });
                return currentClamped + findNextWordEndOffset (str.begin(),
                                                               str.end(),
                                                               str.begin() + currentClamped,
                                                               direction,
                                                               includeBoundary,
                                                               includeWhitespace);
            }

            case BoundaryType::line:
            {
                const auto str = textInterface.getText ({ 0, numCharacters });
                return currentClamped + findNextLineOffset (str.begin(),
                                                            str.end(),
                                                            str.begin() + currentClamped,
                                                            direction,
                                                            includeBoundary);
            }

            case BoundaryType::document:
                return isForwards ? numCharacters : 0;
        }

        jassertfalse;
        return -1;
    }

    /*  Adjusts the current text selection range, using an algorithm appropriate for cursor movement
        on Android.
    */
    static Range<int> findNewSelectionRangeAndroid (const AccessibilityTextInterface& textInterface,
                                                    BoundaryType boundaryType,
                                                    ExtendSelection extend,
                                                    Direction direction)
    {
        const auto oldPos = textInterface.getTextInsertionOffset();
        const auto cursorPos = findTextBoundary (textInterface,
                                                 oldPos,
                                                 boundaryType,
                                                 direction,
                                                 IncludeThisBoundary::no,
                                                 IncludeWhitespaceAfterWords::no);

        if (extend == ExtendSelection::no)
            return { cursorPos, cursorPos };

        const auto currentSelection = textInterface.getSelection();
        const auto start = currentSelection.getStart();
        const auto end   = currentSelection.getEnd();
        return Range<int>::between (cursorPos, oldPos == start ? end : start);
    }
};

} // namespace juce
