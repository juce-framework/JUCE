/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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
        const auto offsetWithDirection = [isForwards] (auto num) { return isForwards ? num : -num; };

        switch (boundary)
        {
            case BoundaryType::character:
                return jlimit (0, numCharacters, currentPosition + offsetWithDirection (1));

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

                return currentPosition + offsetWithDirection (tokens[0].length());
            }

            case BoundaryType::document:
                return isForwards ? numCharacters : 0;
        }

        jassertfalse;
        return -1;
    }
}

} // namespace juce
