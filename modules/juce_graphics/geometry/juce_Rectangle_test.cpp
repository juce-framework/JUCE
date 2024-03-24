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

struct RectangleUnitTest final : public UnitTest
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
