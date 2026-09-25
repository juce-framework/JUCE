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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

void WaylandSurfacePositionHistory::recordPositionForCommit (uint64 commitSequence, Point<int> position,
                                                             uint64 processedCommitSequence)
{
    const auto firstPending = std::find_if (records.begin(), records.end(), [&] (const Record& record)
    {
        return processedCommitSequence < record.commitSequence;
    });

    if (firstPending != records.begin())
        records.erase (records.begin(), std::prev (firstPending));

    // Several moves before the same commit replace the pending position.
    if (! records.empty() && records.back().commitSequence == commitSequence)
    {
        records.back().position = position;
        return;
    }

    records.push_back ({ commitSequence, position });
}

std::optional<Point<int>> WaylandSurfacePositionHistory::getPositionForCommit (uint64 commitSequence) const
{
    if (records.empty())
        return std::nullopt;

    const auto selected = std::find_if (records.rbegin(), records.rend(), [&] (const Record& record)
    {
        return record.commitSequence <= commitSequence;
    });

    return selected != records.rend() ? selected->position : records.front().position;
}

//==============================================================================
#if JUCE_UNIT_TESTS

class WaylandSurfacePositionHistoryTests final : public UnitTest
{
public:
    WaylandSurfacePositionHistoryTests()
        : UnitTest ("WaylandSurfacePositionHistory", UnitTestCategories::gui) {}

    void runTest() override
    {
        const Point<int> first { 10, 20 };
        const Point<int> second { 30, 40 };
        const Point<int> third { 50, 60 };

        testCase ("Position lookup uses commit order and the latest move recorded for each commit", [&]
        {
            WaylandSurfacePositionHistory positionHistory;
            expect (positionHistory.getPositionForCommit (5) == std::nullopt);

            positionHistory.recordPositionForCommit (3, first, 0);
            positionHistory.recordPositionForCommit (4, second, 0);
            positionHistory.recordPositionForCommit (6, third, 0);

            expect (positionHistory.getPositionForCommit (2) == std::optional { first });
            expect (positionHistory.getPositionForCommit (5) == std::optional { second });
            expect (positionHistory.getPositionForCommit (6) == std::optional { third });

            positionHistory.recordPositionForCommit (6, first, 0);
            expect (positionHistory.getPositionForCommit (6) == std::optional { first });
        });

        testCase ("Recording a move keeps the latest processed position and pending moves", [&]
        {
            const Point<int> fourth { 70, 80 };
            WaylandSurfacePositionHistory positionHistory;
            positionHistory.recordPositionForCommit (3, first, 0);
            positionHistory.recordPositionForCommit (4, second, 0);
            positionHistory.recordPositionForCommit (6, third, 0);
            positionHistory.recordPositionForCommit (7, fourth, 5);

            expect (positionHistory.getPositionForCommit (3) == std::optional { second });
            expect (positionHistory.getPositionForCommit (6) == std::optional { third });
            expect (positionHistory.getPositionForCommit (7) == std::optional { fourth });

            positionHistory.recordPositionForCommit (8, first, 7);
            expect (positionHistory.getPositionForCommit (6) == std::optional { fourth });
            expect (positionHistory.getPositionForCommit (8) == std::optional { first });
        });
    }
};

static WaylandSurfacePositionHistoryTests waylandSurfacePositionHistoryTests;

#endif

} // namespace juce
