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

class RectangleListTests : public UnitTest
{
public:
    RectangleListTests() : UnitTest ("RectangleList", UnitTestCategories::graphics) {}

    void runTest() override
    {
        beginTest ("Avoid infinite loops when adding rectangles");
        {
            const Rectangle<float> rectanglesA[]
            {
                { { -9.15555f, 5.06667f }, { 12.f, 5.15556f } },
                { { -9.11111f, 5.11111f }, { 12.f, 5.2f } },
            };

            RectangleList<float> a;

            for (const auto& rect : rectanglesA)
                a.add (rect);

            const std::set<Rectangle<float>> expectedA { { { -9.15555f, 5.06666994f }, { 12.0f, 5.15556f } },
                                                         { { -9.11111f, 5.15556f }, { 12.0f, 5.2f } } };

            for (const auto& r : expectedA)
                expect (std::find (a.begin(), a.end(), r) != a.end());

            const Rectangle<float> rectanglesB[]
            {
                { 565.15887451171875, 777.1043701171875, 454, 212 },
                { -1368.379150390625, 175.8321533203125, 2241.439453125, 782.1121826171875 },
            };

            RectangleList<float> b;

            for (const auto& rect : rectanglesB)
                b.add (rect);

            const std::set<Rectangle<float>> expectedB { { { 565.15887451171875, 777.1043701171875 }, { 1019.15887451171875, 989.1043701171875 } },
                                                         { { 565.15887451171875, 175.8321533203125 }, { 873.060302734375, 777.1043701171875 } },
                                                         { { -1368.379150390625, 175.8321533203125 }, { 565.158935546875, 957.9443359375 } } };

            for (const auto& r : expectedB)
                expect (std::find (b.begin(), b.end(), r) != b.end());
        }

        beginTest ("Subtracting overlapping empty rect subdivides existing rects");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });
            list.subtract ({ 50, 50, 0, 0 });

            // The overlapping rect gets subdivided on the X axis at the location of the empty rect.
            const std::set<Rectangle<float>> expected { { 10, 10, 40, 80 },
                                                        { 50, 10, 40, 80 } };

            for (const auto& r : expected)
                expect (std::find (list.begin(), list.end(), r) != list.end());
        }

        beginTest ("Subtracting non-overlapping rects has no effect");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });

            list.subtract ({ 0, 0, 5, 5 });
            list.subtract ({ 0, 95, 5, 5 });
            list.subtract ({ 95, 0, 5, 5 });
            list.subtract ({ 95, 95, 5, 5 });

            expect (list.getNumRectangles() == 1);
            expect (*list.begin() == Rectangle<float> { 10, 10, 80, 80 });
        }

        beginTest ("Subtracting from corner produces two rects");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });

            list.subtract ({ 0, 0, 50, 50 });

            const std::set<Rectangle<float>> expected { { 50, 10, 40, 80 },
                                                        { 10, 50, 40, 40 } };

            for (const auto& r : expected)
                expect (std::find (list.begin(), list.end(), r) != list.end());
        }

        beginTest ("Subtracting from entire edge shrinks existing rect");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });

            list.subtract ({ 0, 0, 30, 100 });
            list.subtract ({ 30, 50, 60, 40 });

            const std::set<Rectangle<float>> expected { { 30, 10, 60, 40 } };

            for (const auto& r : expected)
                expect (std::find (list.begin(), list.end(), r) != list.end());
        }

        beginTest ("Subtracting a notch from a vertical edge produces new rects");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });
            list.subtract ({ 10, 20, 10, 60 });

            const std::set<Rectangle<float>> expected { { 20, 10, 70, 80 },
                                                        { 10, 80, 10, 10 },
                                                        { 10, 10, 10, 10} };

            for (auto r : list)
                expect (std::find (list.begin(), list.end(), r) != list.end());
        }

        beginTest ("Subtracting a notch from a horizontal edge produces new rects");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });
            list.subtract ({ 20, 10, 60, 10 });

            const std::set<Rectangle<float>> expected { { 80, 10, 10, 80 },
                                                        { 20, 20, 60, 70 },
                                                        { 10, 10, 10, 80 } };

            for (auto r : list)
                expect (std::find (list.begin(), list.end(), r) != list.end());
        }

        beginTest ("Subtracting a hole from the centre of a rect produces new rects");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });
            list.subtract ({ 20, 20, 60, 60 });

            const std::set<Rectangle<float>> expected { { 10, 10, 10, 80 },
                                                        { 20, 10, 60, 10 },
                                                        { 20, 80, 60, 10 },
                                                        { 80, 10, 10, 80 } };

            for (auto r : list)
                expect (std::find (list.begin(), list.end(), r) != list.end());
        }

        beginTest ("Subtracting a rect from itself produces an empty list");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });
            list.subtract ({ 10, 10, 80, 80 });
            expect (list.isEmpty());
        }

        beginTest ("Subtracting a larger rect from a rect list produces an empty list");
        {
            RectangleList<float> list;
            list.add ({ 10, 10, 80, 80 });
            list.subtract ({ 0, 0, 100, 100 });
            expect (list.isEmpty());
        }
    }
};

static RectangleListTests rectangleListTests;

} // namespace juce
