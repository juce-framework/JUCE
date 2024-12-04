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

class ParallelogramTest : public UnitTest
{
public:
    ParallelogramTest() : UnitTest ("Parallelogram", UnitTestCategories::graphics) {}

    void runTest() override
    {
        beginTest ("isEmpty");
        {
            expect (! Parallelogram (Rectangle<int> (10, 10, 20, 20)).isEmpty());
            expect (Parallelogram (Rectangle<int> (10, 10, 0, 20)).isEmpty());
            expect (Parallelogram (Rectangle<int> (10, 10, 20, 0)).isEmpty());

            expect (! Parallelogram (Point<int> (0, 0), Point<int> (10, 10), Point<int> (20, 0)).isEmpty());
            expect (Parallelogram (Point<int> (0, 0), Point<int> (0, 0), Point<int> (20, 0)).isEmpty());
            expect (Parallelogram (Point<int> (0, 0), Point<int> (10, 10), Point<int> (10, 10)).isEmpty());
            expect (Parallelogram (Point<int> (20, 0), Point<int> (10, 10), Point<int> (20, 0)).isEmpty());
        }

        beginTest ("operators");
        {
            Parallelogram p (Rectangle<int> (10, 10, 20, 20));
            p += Point<int> (5, 10);
            expect (p.topLeft == Point<int> (15, 20));
            expect (p.topRight == Point<int> (35, 20));
            expect (p.bottomLeft == Point<int> (15, 40));

            p -= Point<int> (10, 5);
            expect (p.topLeft == Point<int> (5, 15));
            expect (p.topRight == Point<int> (25, 15));
            expect (p.bottomLeft == Point<int> (5, 35));
        }
    }
};

static ParallelogramTest parallelogramTest;

} // namespace juce
