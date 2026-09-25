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

void WaylandSurfaceOutputs::add (wl_output* output)
{
    if (output != nullptr && ! contains (output))
        outputs.push_back (output);
}

void WaylandSurfaceOutputs::remove (wl_output* output)
{
    if (const auto it = std::find (outputs.begin(), outputs.end(), output); it != outputs.end())
        outputs.erase (it);
}

bool WaylandSurfaceOutputs::contains (wl_output* output) const
{
    return std::find (outputs.begin(), outputs.end(), output) != outputs.end();
}

int WaylandSurfaceOutputs::size() const noexcept
{
    return (int) outputs.size();
}

//==============================================================================
#if JUCE_UNIT_TESTS

class WaylandSurfaceOutputsTests final : public UnitTest
{
public:
    WaylandSurfaceOutputsTests()
        : UnitTest ("WaylandSurfaceOutputs", UnitTestCategories::gui) {}

    void runTest() override
    {
        // A wl_output is an opaque handle, so these tests only need two distinct addresses.
        int firstStorage = 0;
        int secondStorage = 0;
        auto* first = reinterpret_cast<wl_output*> (&firstStorage);
        auto* second = reinterpret_cast<wl_output*> (&secondStorage);

        const auto unknownScale = [] (wl_output*) { return std::optional<int>{}; };

        testCase ("A surface that is not on an output uses the fallback scale", [&]
        {
            WaylandSurfaceOutputs surfaceOutputs;

            expectEquals (surfaceOutputs.size(), 0);
            expectEquals (surfaceOutputs.getLargestScale (3, [] (wl_output*) { return std::optional<int> (1); }), 3);
        });

        testCase ("Repeated enter events track an output once", [&]
        {
            WaylandSurfaceOutputs surfaceOutputs;
            surfaceOutputs.add (first);
            surfaceOutputs.add (first);

            expectEquals (surfaceOutputs.size(), 1);
            expect (surfaceOutputs.contains (first));
        });

        testCase ("Removing an untracked output changes nothing", [&]
        {
            WaylandSurfaceOutputs surfaceOutputs;
            surfaceOutputs.add (first);
            surfaceOutputs.remove (second);

            expectEquals (surfaceOutputs.size(), 1);
            expect (surfaceOutputs.contains (first));
        });

        testCase ("A surface uses the largest scale of the outputs that contain it", [&]
        {
            WaylandSurfaceOutputs surfaceOutputs;
            surfaceOutputs.add (first);
            surfaceOutputs.add (second);

            const auto scaleForOutput = [&] (wl_output* output) { return std::optional<int> (output == second ? 3 : 2); };
            expectEquals (surfaceOutputs.getLargestScale (1, scaleForOutput), 3);

            surfaceOutputs.remove (second);
            expectEquals (surfaceOutputs.getLargestScale (1, scaleForOutput), 2);
        });

        testCase ("A surface ignores an output that has not reported its scale", [&]
        {
            WaylandSurfaceOutputs surfaceOutputs;
            surfaceOutputs.add (first);
            surfaceOutputs.add (second);

            const auto scaleForOutput = [&] (wl_output* output)
            {
                return output == second ? std::optional<int> (2) : std::nullopt;
            };

            expectEquals (surfaceOutputs.getLargestScale (4, scaleForOutput), 2);
        });

        testCase ("A surface uses the fallback when none of its outputs has reported a scale", [&]
        {
            WaylandSurfaceOutputs surfaceOutputs;
            surfaceOutputs.add (first);
            surfaceOutputs.add (second);

            expectEquals (surfaceOutputs.getLargestScale (2, unknownScale), 2);
        });
    }
};

static WaylandSurfaceOutputsTests waylandSurfaceOutputsTests;

#endif

} // namespace juce
