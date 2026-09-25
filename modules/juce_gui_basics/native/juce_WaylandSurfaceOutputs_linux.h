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

class WaylandSurfaceOutputs final
{
public:
    WaylandSurfaceOutputs() = default;

    void add (wl_output* output);
    void remove (wl_output* output);
    bool contains (wl_output* output) const;
    int size() const noexcept;

    template <typename ScaleForOutput>
    int getLargestScale (int fallbackScale, ScaleForOutput&& scaleForOutput) const
    {
        std::optional<int> largestScale;

        for (auto* output : outputs)
            if (const auto scale = scaleForOutput (output))
                largestScale = jmax (largestScale.value_or (*scale), *scale);

        return largestScale.value_or (fallbackScale);
    }

private:
    std::vector<wl_output*> outputs;

    JUCE_DECLARE_NON_COPYABLE (WaylandSurfaceOutputs)
};

} // namespace juce
