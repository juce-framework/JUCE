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

// Scale state for a surface and the buffer geometry derived from it.
// wp_fractional_scale_v1 expresses scales in 120ths of a unit, so the arithmetic
// stays in whole 120ths until the final rounding to avoid floating point drift.
class WaylandSurfaceScale final
{
public:
    enum class BufferMappingMethod
    {
        integerBufferScale,
        viewport
    };

    struct BufferGeometry
    {
        Rectangle<int> bufferBounds;
        int bufferScale = 1;
        std::optional<Rectangle<int>> viewportDestination;
    };

    struct Update
    {
        std::optional<double> scaleFactorToReport;
        bool bufferGeometryChanged = false;
    };

    Update setPreferredFractionalScale120 (int scale120);
    Update setPreferredBufferScale (int scale);
    Update setOutputScale (int scale);
    Update setOverrideScale (std::optional<double> scale);

    std::optional<double> getOverrideScale() const;
    double getScaleFactor() const;
    int getIntegerCompositorScale() const;
    double getLogicalToSurfaceScale (BufferMappingMethod method) const;

    BufferGeometry getBufferGeometry (Rectangle<int> logicalSize, BufferMappingMethod method) const;
    int convertSurfaceExtentToLogical (int surfaceExtent, BufferMappingMethod method) const;
    Point<double> getSurfaceSize (Rectangle<int> logicalSize, BufferMappingMethod method) const;
    Point<float> convertSurfacePointToLogical (Point<float> point,
                                               Rectangle<int> logicalSize,
                                               BufferMappingMethod method) const;

    static Rectangle<int> mapLogicalRectToBuffer (Rectangle<int> logicalRect,
                                                  Rectangle<int> logicalBounds,
                                                  Rectangle<int> bufferBounds);
    static Rectangle<int> mapLogicalRectToSurface (Rectangle<int> logicalRect,
                                                   Rectangle<int> logicalBounds,
                                                   Rectangle<int> surfaceBounds);

private:
    struct ScaleValue
    {
        double factor = 1.0;
        std::optional<int> exactScale120;
    };

    // preferred_scale values are 120ths of a scale unit.
    static constexpr int scaleDenominator = 120;

    // Values above 16x are not plausible for a display.
    static constexpr int maximumScale120 = scaleDenominator * 16;

    template <typename Callback>
    Update updateScaleState (Callback&& callback)
    {
        const auto previousRenderScale = getScaleFactor();
        const auto previousCompositorScale = getCompositorScaleValue().factor;
        callback();
        const auto nextRenderScale = getScaleFactor();
        const auto nextCompositorScale = getCompositorScaleValue().factor;
        const auto renderScaleChanged = ! approximatelyEqual (previousRenderScale, nextRenderScale);

        Update result;
        result.bufferGeometryChanged = renderScaleChanged
                                    || ! approximatelyEqual (previousCompositorScale, nextCompositorScale);

        if (renderScaleChanged)
            result.scaleFactorToReport = nextRenderScale;

        return result;
    }

    ScaleValue getRenderScaleValue() const;
    ScaleValue getCompositorScaleValue() const;
    static Rectangle<int> getScaledBounds (Rectangle<int> logicalSize, const ScaleValue& scale);
    static int scaledExtent (int logicalExtent, const ScaleValue& scale);

    std::optional<int> preferredFractionalScale120;
    std::optional<int> preferredBufferScale;
    std::optional<double> overrideScale;
    int outputScale = 1;
};

} // namespace juce
