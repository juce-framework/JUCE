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

struct RectangleUnitTest  : public UnitTest
{
    RectangleUnitTest() : UnitTest ("Rectangle", UnitTestCategories::graphics) {}

    void runTest() override
    {
        beginTest ("Rectangle/string conversions can be round-tripped");
        {
            const Rectangle<float> a (0.1f, 0.2f, 0.3f, 0.4f);
            expect (Rectangle<float>::fromString (a.toString()) == a);

            const Rectangle<double> b (0.1, 0.2, 0.3, 0.4);
            expect (Rectangle<double>::fromString (b.toString()) == b);

            const Rectangle<int> c (1, 2, 3, 4);
            expect (Rectangle<int>::fromString (c.toString()) == c);
        }
    }
};

static RectangleUnitTest rectangleUnitTest;

} // namespace juce
