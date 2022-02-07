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

namespace AccessibilityTextHelpers
{
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

    static int findTextBoundary (const AccessibilityTextInterface& textInterface,
                                 int currentPosition,
                                 BoundaryType boundary,
                                 Direction direction)
    {
        const auto numCharacters = textInterface.getTotalNumCharacters();
        const auto isForwards = (direction == Direction::forwards);

        const auto offsetWithDirecton = [isForwards] (int input) { return isForwards ? input : -input; };

        switch (boundary)
        {
            case BoundaryType::character:
                return jlimit (0, numCharacters, isForwards ? currentPosition + 1 : currentPosition);

            case BoundaryType::word:
            case BoundaryType::line:
            {
                const auto text = [&]() -> String
                {
                    if (isForwards)
                        return textInterface.getText ({ currentPosition, textInterface.getTotalNumCharacters() });

                    const auto str = textInterface.getText ({ 0, currentPosition });

                    auto start = str.getCharPointer();
                    auto end = start.findTerminatingNull();
                    const auto size = getAddressDifference (end.getAddress(), start.getAddress());

                    String reversed;

                    if (size > 0)
                    {
                        reversed.preallocateBytes ((size_t) size);

                        auto destPtr = reversed.getCharPointer();

                        for (;;)
                        {
                            destPtr.write (*--end);

                            if (end == start)
                                break;
                        }

                        destPtr.writeNull();
                    }

                    return reversed;
                }();

                auto tokens = (boundary == BoundaryType::line ? StringArray::fromLines (text)
                                                              : StringArray::fromTokens (text, false));

                return currentPosition + offsetWithDirecton (tokens[0].length());
            }

            case BoundaryType::document:
                return isForwards ? numCharacters : 0;
        }

        jassertfalse;
        return -1;
    }
}

} // namespace juce
