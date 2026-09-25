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

WaylandSurfaceScale::Update WaylandSurfaceScale::setPreferredFractionalScale120 (int scale120)
{
    return updateScaleState ([&]
    {
        // Buffer memory grows with the square of the scale
        if (1 <= scale120 && scale120 <= maximumScale120)
            preferredFractionalScale120 = scale120;
    });
}

WaylandSurfaceScale::Update WaylandSurfaceScale::setPreferredBufferScale (int scale)
{
    return updateScaleState ([&]
    {
        // Buffer memory grows with the square of the scale
        if (1 <= scale && scale <= maximumScale120 / scaleDenominator)
            preferredBufferScale = scale;
    });
}

WaylandSurfaceScale::Update WaylandSurfaceScale::setOutputScale (int scale)
{
    return updateScaleState ([&]
    {
        outputScale = jlimit (1, maximumScale120 / scaleDenominator, scale);
    });
}

WaylandSurfaceScale::Update WaylandSurfaceScale::setOverrideScale (std::optional<double> scale)
{
    return updateScaleState ([&]
    {
        // A non-positive scale cannot be rendered, so it clears the override.
        if (scale.has_value() && *scale > 0.0)
            overrideScale = *scale;
        else
            overrideScale = {};
    });
}

std::optional<double> WaylandSurfaceScale::getOverrideScale() const
{
    return overrideScale;
}

double WaylandSurfaceScale::getScaleFactor() const
{
    return getRenderScaleValue().factor;
}

int WaylandSurfaceScale::getIntegerCompositorScale() const
{
    return preferredBufferScale.value_or (outputScale);
}

double WaylandSurfaceScale::getLogicalToSurfaceScale (BufferMappingMethod method) const
{
    const auto compositorScale = method == BufferMappingMethod::viewport
                               ? getCompositorScaleValue().factor
                               : (double) getIntegerCompositorScale();
    return getRenderScaleValue().factor / compositorScale;
}

WaylandSurfaceScale::BufferGeometry WaylandSurfaceScale::getBufferGeometry (Rectangle<int> logicalSize,
                                                                            BufferMappingMethod method) const
{
    BufferGeometry result;

    if (method == BufferMappingMethod::viewport)
    {
        // set_buffer_scale stays at 1 because the viewport defines the mapping
        // from the whole-pixel buffer to surface coordinates.
        const auto renderScale = getRenderScaleValue();
        result.bufferBounds = getScaledBounds (logicalSize, renderScale);

        if (overrideScale.has_value())
        {
            const auto compositorScale = getCompositorScaleValue().factor;
            const ScaleValue surfaceScale { renderScale.factor / compositorScale, {} };
            result.viewportDestination = getScaledBounds (logicalSize, surfaceScale);
        }
        else
        {
            result.viewportDestination = logicalSize.withZeroOrigin();
        }

        return result;
    }

    // Without a viewport, buffer dimensions must be multiples of the compositor's
    // integer scale. Round the surface size before scaling it.
    const auto compositorScale = getIntegerCompositorScale();
    const ScaleValue surfaceScale { overrideScale.value_or ((double) compositorScale) / compositorScale, {} };
    result.bufferBounds = getScaledBounds (logicalSize, surfaceScale) * compositorScale;
    result.bufferScale = compositorScale;
    return result;
}

int WaylandSurfaceScale::convertSurfaceExtentToLogical (int surfaceExtent, BufferMappingMethod method) const
{
    if (surfaceExtent == 0)
        return 0;

    // Configure sizes describe new geometry, so invert the ideal scale before
    // rounded buffer geometry exists.
    const auto compositorScale = method == BufferMappingMethod::viewport
                               ? getCompositorScaleValue().factor
                               : (double) getIntegerCompositorScale();
    return jmax (1, (int) std::round (surfaceExtent * compositorScale / getRenderScaleValue().factor));
}

// The size the surface occupies in surface coordinates, which is the buffer mapped through
// either the viewport destination or the buffer scale.
Point<double> WaylandSurfaceScale::getSurfaceSize (Rectangle<int> logicalSize, BufferMappingMethod method) const
{
    const auto geometry = getBufferGeometry (logicalSize, method);
    const auto surfaceBounds = geometry.viewportDestination.value_or (geometry.bufferBounds);
    const auto divisor = geometry.viewportDestination.has_value() ? 1.0 : (double) geometry.bufferScale;

    return { surfaceBounds.getWidth() / divisor, surfaceBounds.getHeight() / divisor };
}

Point<float> WaylandSurfaceScale::convertSurfacePointToLogical (Point<float> point,
                                                                Rectangle<int> logicalSize,
                                                                BufferMappingMethod method) const
{
    // Input follows the rounded geometry used by the committed buffer.
    const auto surfaceSize = getSurfaceSize (logicalSize, method);

    if (surfaceSize.x <= 0.0 || surfaceSize.y <= 0.0)
        return point;

    return Point<double> { point.x * (logicalSize.getWidth() / surfaceSize.x),
                           point.y * (logicalSize.getHeight() / surfaceSize.y) }.toFloat();
}

// Rounding the buffer size can make its ratio to the logical size differ from the scale factor.
// Round start coordinates down and end coordinates up so the result covers the entire scaled rectangle.
Rectangle<int> WaylandSurfaceScale::mapLogicalRectToBuffer (Rectangle<int> logicalRect,
                                                            Rectangle<int> logicalBounds,
                                                            Rectangle<int> bufferBounds)
{
    [[maybe_unused]] const auto isNonNegative = [] (Rectangle<int> rect)
    {
        return rect.getX() >= 0 && rect.getY() >= 0 && rect.getWidth() >= 0 && rect.getHeight() >= 0;
    };

    // Callers must pass regions clipped to the component bounds
    jassert (isNonNegative (logicalRect) && isNonNegative (logicalBounds) && isNonNegative (bufferBounds));

    if (logicalBounds.isEmpty())
        return {};

    const auto mapStart = [] (int position, int bufferExtent, int logicalExtent)
    {
        return (int) (((int64) position * bufferExtent) / logicalExtent);
    };

    const auto mapEnd = [] (int position, int bufferExtent, int logicalExtent)
    {
        return (int) (((int64) position * bufferExtent + logicalExtent - 1) / logicalExtent);
    };

    const auto left = mapStart (logicalRect.getX(), bufferBounds.getWidth(), logicalBounds.getWidth());
    const auto top = mapStart (logicalRect.getY(), bufferBounds.getHeight(), logicalBounds.getHeight());
    const auto right = mapEnd (logicalRect.getRight(), bufferBounds.getWidth(), logicalBounds.getWidth());
    const auto bottom = mapEnd (logicalRect.getBottom(), bufferBounds.getHeight(), logicalBounds.getHeight());

    return { left, top, right - left, bottom - top };
}

Rectangle<int> WaylandSurfaceScale::mapLogicalRectToSurface (Rectangle<int> logicalRect,
                                                             Rectangle<int> logicalBounds,
                                                             Rectangle<int> surfaceBounds)
{
    if (logicalBounds.isEmpty())
        return {};

    // Round each shared logical boundary once so adjacent child surfaces meet without gaps or overlaps.
    const auto mapPosition = [] (int position, int logicalStart, int logicalExtent,
                                 int surfaceStart, int surfaceExtent)
    {
        return surfaceStart + roundToInt ((double) (position - logicalStart)
                                          * surfaceExtent / logicalExtent);
    };

    const auto left = mapPosition (logicalRect.getX(), logicalBounds.getX(), logicalBounds.getWidth(),
                                   surfaceBounds.getX(), surfaceBounds.getWidth());
    const auto top = mapPosition (logicalRect.getY(), logicalBounds.getY(), logicalBounds.getHeight(),
                                  surfaceBounds.getY(), surfaceBounds.getHeight());
    const auto right = mapPosition (logicalRect.getRight(), logicalBounds.getX(), logicalBounds.getWidth(),
                                    surfaceBounds.getX(), surfaceBounds.getWidth());
    const auto bottom = mapPosition (logicalRect.getBottom(), logicalBounds.getY(), logicalBounds.getHeight(),
                                     surfaceBounds.getY(), surfaceBounds.getHeight());

    return { left, top, right - left, bottom - top };
}

WaylandSurfaceScale::ScaleValue WaylandSurfaceScale::getRenderScaleValue() const
{
    if (overrideScale.has_value())
        return { *overrideScale, {} };

    return getCompositorScaleValue();
}

WaylandSurfaceScale::ScaleValue WaylandSurfaceScale::getCompositorScaleValue() const
{
    // The fractional protocol gives a more precise preference.
    const auto scale120 = preferredFractionalScale120.value_or (getIntegerCompositorScale()
                                                                * scaleDenominator);
    return { scale120 / (double) scaleDenominator, scale120 };
}

Rectangle<int> WaylandSurfaceScale::getScaledBounds (Rectangle<int> logicalSize, const ScaleValue& scale)
{
    return { scaledExtent (logicalSize.getWidth(), scale),
             scaledExtent (logicalSize.getHeight(), scale) };
}

// fractional-scale-v1 specifies rounding to the nearest whole pixel, with halfway
// values rounded away from zero. Use the same rule for all derived scale geometry.
int WaylandSurfaceScale::scaledExtent (int logicalExtent, const ScaleValue& scale)
{
    if (logicalExtent == 0)
        return 0;

    if (! scale.exactScale120.has_value())
        return jmax (1, (int) std::round (logicalExtent * scale.factor));

    return jmax (1, (logicalExtent * *scale.exactScale120 + scaleDenominator / 2) / scaleDenominator);
}

//==============================================================================
#if JUCE_UNIT_TESTS

class WaylandSurfaceScaleTests final : public UnitTest
{
public:
    WaylandSurfaceScaleTests()
        : UnitTest ("WaylandSurfaceScale", UnitTestCategories::gui) {}

    void runTest() override
    {
        using BufferMappingMethod = WaylandSurfaceScale::BufferMappingMethod;

        // Compositor scale
        // Compositor-driven fractional scales require both wp_viewporter and
        // wp_fractional_scale_manager_v1.

        testCase ("A viewport maps the buffer onto the logical size", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::viewport);
            expect (geometry.bufferBounds == Rectangle<int> (200, 100));
            expectEquals (geometry.bufferScale, 1);
            expect (geometry.viewportDestination == Rectangle<int> (100, 50));
        });

        testCase ("A preferred buffer scale wins over the output scale", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setPreferredBufferScale (3);
            expectEquals (state.getScaleFactor(), 3.0);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::viewport);
            expect (geometry.bufferBounds == Rectangle<int> (300, 150));
            expectEquals (geometry.bufferScale, 1);
            expect (geometry.viewportDestination == Rectangle<int> (100, 50));
        });

        testCase ("A fractional preferred scale wins over a preferred buffer scale", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredBufferScale (2);
            state.setPreferredFractionalScale120 (180);
            expectEquals (state.getScaleFactor(), 1.5);

            const auto update = state.setPreferredBufferScale (3);
            expect (! update.scaleFactorToReport.has_value());
            expect (! update.bufferGeometryChanged);
            expectEquals (state.getScaleFactor(), 1.5);
        });

        testCase ("A fractional preferred scale in 120ths determines the buffer size", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (180);
            expectEquals (state.getScaleFactor(), 1.5);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::viewport);
            expect (geometry.bufferBounds == Rectangle<int> (150, 75));
            expectEquals (geometry.bufferScale, 1);
            expect (geometry.viewportDestination == Rectangle<int> (100, 50));
        });

        testCase ("A whole-number preference from the fractional-scale protocol produces matching buffer dimensions", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (240);
            expectEquals (state.getScaleFactor(), 2.0);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::viewport);
            expect (geometry.bufferBounds == Rectangle<int> (200, 100));
            expect (geometry.viewportDestination == Rectangle<int> (100, 50));
        });

        testCase ("Fractional buffer dimensions round down below halfway", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (175);

            expect (state.getBufferGeometry ({ 3, 3 }, BufferMappingMethod::viewport).bufferBounds
                    == Rectangle<int> (4, 4));
        });

        testCase ("Fractional buffer dimensions round halfway away from zero", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (180);

            expect (state.getBufferGeometry ({ 3, 3 }, BufferMappingMethod::viewport).bufferBounds
                    == Rectangle<int> (5, 5));
        });

        testCase ("Fractional buffer dimensions round up above halfway", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (185);

            expect (state.getBufferGeometry ({ 3, 3 }, BufferMappingMethod::viewport).bufferBounds
                    == Rectangle<int> (5, 5));
        });

        testCase ("A 1x preference from the fractional-scale protocol keeps the buffer at the logical size", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (120);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::viewport);
            expect (geometry.bufferBounds == Rectangle<int> (100, 50));
            expect (geometry.viewportDestination == Rectangle<int> (100, 50));
        });

        testCase ("Empty logical bounds produce an empty buffer", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (180);

            expect (state.getBufferGeometry ({}, BufferMappingMethod::viewport).bufferBounds.isEmpty());
        });

        testCase ("Output scales outside the supported range are clamped", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (0);
            expectEquals (state.getScaleFactor(), 1.0);

            state.setOutputScale (std::numeric_limits<int>::max());
            expectEquals (state.getScaleFactor(), 16.0);
        });

        testCase ("A fractional preferred scale of zero is ignored", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (180);

            state.setPreferredFractionalScale120 (0);
            expectEquals (state.getScaleFactor(), 1.5);
        });

        testCase ("Fractional preferred scales above 16x are ignored", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (16 * 120);
            expectEquals (state.getScaleFactor(), 16.0);

            state.setPreferredFractionalScale120 (180);
            state.setPreferredFractionalScale120 (16 * 120 + 1);
            expectEquals (state.getScaleFactor(), 1.5);
        });

        testCase ("Preferred buffer scales outside the supported range are ignored", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredBufferScale (2);

            state.setPreferredBufferScale (0);
            expectEquals (state.getScaleFactor(), 2.0);

            state.setPreferredBufferScale (17);
            expectEquals (state.getScaleFactor(), 2.0);
        });

        // JUCE scale override.
        testCase ("Only changes to the JUCE scale factor are reported", [&]
        {
            WaylandSurfaceScale state;

            auto update = state.setOutputScale (2);
            expect (update.scaleFactorToReport == 2.0);

            update = state.setPreferredFractionalScale120 (240);
            expect (! update.scaleFactorToReport.has_value());

            update = state.setPreferredFractionalScale120 (180);
            expect (update.scaleFactorToReport == 1.5);

            update = state.setOutputScale (3);
            expect (! update.scaleFactorToReport.has_value());

            update = state.setOverrideScale (1.5);
            expect (! update.scaleFactorToReport.has_value());

            update = state.setPreferredFractionalScale120 (240);
            expect (! update.scaleFactorToReport.has_value());

            update = state.setOverrideScale ({});
            expect (update.scaleFactorToReport == 2.0);
        });

        testCase ("A compositor scale change updates geometry while an override keeps the JUCE scale unchanged", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setOverrideScale (1.5);

            auto update = state.setPreferredFractionalScale120 (180);
            expect (! update.scaleFactorToReport.has_value());
            expect (update.bufferGeometryChanged);

            update = state.setPreferredFractionalScale120 (180);
            expect (! update.scaleFactorToReport.has_value());
            expect (! update.bufferGeometryChanged);
        });

        testCase ("Matching render and compositor scales leave surface coordinates unchanged", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);

            expectEquals (state.getLogicalToSurfaceScale (BufferMappingMethod::viewport), 1.0);
            expectEquals (state.getLogicalToSurfaceScale (BufferMappingMethod::integerBufferScale), 1.0);
        });

        testCase ("A host scale override converts logical coordinates to the compositor surface scale", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setOverrideScale (1.5);

            expectEquals (state.getLogicalToSurfaceScale (BufferMappingMethod::viewport), 0.75);
            expectEquals (state.getLogicalToSurfaceScale (BufferMappingMethod::integerBufferScale), 0.75);
        });

        testCase ("The override scale wins over the fractional preferred scale until it is cleared", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (240);

            state.setOverrideScale (1.25);
            expectEquals (state.getScaleFactor(), 1.25);
            expect (state.getBufferGeometry ({ 100, 100 }, BufferMappingMethod::viewport).bufferBounds
                    == Rectangle<int> (125, 125));

            state.setOverrideScale ({});
            expectEquals (state.getScaleFactor(), 2.0);
            expect (state.getBufferGeometry ({ 100, 100 }, BufferMappingMethod::viewport).bufferBounds
                    == Rectangle<int> (200, 200));
        });

        testCase ("A viewport combines a fractional compositor scale with an override", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (150);
            state.setOverrideScale (1.5);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::viewport);
            expect (geometry.bufferBounds == Rectangle<int> (150, 75));
            expectEquals (geometry.bufferScale, 1);
            expect (geometry.viewportDestination == Rectangle<int> (120, 60));
        });

        testCase ("A non-positive override clears the override instead of applying it", [&]
        {
            WaylandSurfaceScale state;
            state.setPreferredFractionalScale120 (240);
            state.setOverrideScale (1.5);
            expectEquals (state.getScaleFactor(), 1.5);

            state.setOverrideScale (-1.0);
            expectEquals (state.getScaleFactor(), 2.0);
            expect (! state.getOverrideScale().has_value());

            state.setOverrideScale (1.5);
            expectEquals (state.getScaleFactor(), 1.5);

            state.setOverrideScale (0.0);
            expectEquals (state.getScaleFactor(), 2.0);
            expect (! state.getOverrideScale().has_value());
        });

        testCase ("A viewport maps an overridden buffer using the compositor scale", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setOverrideScale (1.5);

            const auto geometry = state.getBufferGeometry ({ 101, 3 }, BufferMappingMethod::viewport);
            expect (geometry.bufferBounds == Rectangle<int> (152, 5));
            expectEquals (geometry.bufferScale, 1);
            expect (geometry.viewportDestination == Rectangle<int> (76, 2));
        });

        testCase ("Surface coordinates follow the rounded viewport destination", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setOverrideScale (1.5);

            const auto logical = state.convertSurfacePointToLogical ({ 76.0f, 2.0f },
                                                                     { 101, 3 },
                                                                     BufferMappingMethod::viewport);
            expect (logical == Point<float> (101.0f, 3.0f));
        });

        testCase ("Surface extents round halfway away from zero when converted to logical coordinates", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (3);
            state.setOverrideScale (2.0);

            expectEquals (state.convertSurfaceExtentToLogical (3, BufferMappingMethod::viewport), 5);
        });

        // No-viewport fallback
        // Every compositor tested advertises wp_viewporter, but the protocol makes it optional.
        // The following cases cover integer buffer mapping in the rare case viewport is unavailable.
        testCase ("Integer buffer mapping uses one buffer pixel per logical pixel at the default scale", [&]
        {
            WaylandSurfaceScale state;
            expectEquals (state.getScaleFactor(), 1.0);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::integerBufferScale);
            expect (geometry.bufferBounds == Rectangle<int> (100, 50));
            expectEquals (geometry.bufferScale, 1);
            expect (! geometry.viewportDestination.has_value());
        });

        testCase ("Integer buffer mapping uses the output scale as the buffer scale", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            expectEquals (state.getScaleFactor(), 2.0);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::integerBufferScale);
            expect (geometry.bufferBounds == Rectangle<int> (200, 100));
            expectEquals (geometry.bufferScale, 2);
            expect (! geometry.viewportDestination.has_value());
        });

        testCase ("Integer buffer mapping uses the preferred buffer scale", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setPreferredBufferScale (3);
            expectEquals (state.getScaleFactor(), 3.0);

            const auto geometry = state.getBufferGeometry ({ 100, 50 }, BufferMappingMethod::integerBufferScale);
            expect (geometry.bufferBounds == Rectangle<int> (300, 150));
            expectEquals (geometry.bufferScale, 3);
            expect (! geometry.viewportDestination.has_value());
        });

        testCase ("A fractional override rounds buffer dimensions halfway away from zero", [&]
        {
            WaylandSurfaceScale state;
            state.setOverrideScale (1.5);

            const auto geometry = state.getBufferGeometry ({ 3, 3 }, BufferMappingMethod::integerBufferScale);
            expect (geometry.bufferBounds == Rectangle<int> (5, 5));
            expectEquals (geometry.bufferScale, 1);
        });

        testCase ("A fractional override uses the nearest buffer dimensions allowed by the output scale", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setOverrideScale (1.5);

            const auto geometry = state.getBufferGeometry ({ 101, 3 }, BufferMappingMethod::integerBufferScale);
            expect (geometry.bufferBounds == Rectangle<int> (152, 4));
            expectEquals (geometry.bufferScale, 2);
            expectEquals (geometry.bufferBounds.getWidth() % geometry.bufferScale, 0);
            expectEquals (geometry.bufferBounds.getHeight() % geometry.bufferScale, 0);
            expect (! geometry.viewportDestination.has_value());
        });

        testCase ("Surface coordinates follow the rounded integer buffer geometry", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setOverrideScale (1.5);

            const auto logical = state.convertSurfacePointToLogical ({ 76.0f, 2.0f },
                                                                     { 101, 3 },
                                                                     BufferMappingMethod::integerBufferScale);
            expect (logical == Point<float> (101.0f, 3.0f));
        });

        testCase ("Surface extents use the integer output scale when converted to logical coordinates", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setOverrideScale (1.5);

            expectEquals (state.convertSurfaceExtentToLogical (76, BufferMappingMethod::integerBufferScale), 101);
        });

        testCase ("Fallback buffer dimensions round halfway away from zero to a valid multiple", [&]
        {
            WaylandSurfaceScale state;
            state.setOutputScale (2);
            state.setOverrideScale (2.5);

            const auto geometry = state.getBufferGeometry ({ 2, 2 }, BufferMappingMethod::integerBufferScale);
            expect (geometry.bufferBounds == Rectangle<int> (6, 6));
            expectEquals (geometry.bufferScale, 2);
        });

        // Map repainted areas from logical coordinates to buffer pixels.
        testCase ("A buffer the size of the logical bounds maps rectangles unchanged", [&]
        {
            const Rectangle<int> bounds { 100, 50 };
            const Rectangle<int> area { 10, 5, 20, 15 };

            expect (WaylandSurfaceScale::mapLogicalRectToBuffer (area, bounds, bounds) == area);
        });

        testCase ("A buffer at twice the logical size doubles rectangle positions and sizes", [&]
        {
            const auto mapped = WaylandSurfaceScale::mapLogicalRectToBuffer ({ 10, 5, 20, 10 },
                                                                             { 100, 50 },
                                                                             { 200, 100 });
            expect (mapped == Rectangle<int> (20, 10, 40, 20));
        });

        testCase ("A fractionally larger buffer rounds rectangle edges outwards", [&]
        {
            const auto mapped = WaylandSurfaceScale::mapLogicalRectToBuffer ({ 10, 10, 10, 10 },
                                                                             { 100, 100 },
                                                                             { 125, 125 });
            // The left and top edges fall inside pixel 12, and the right and bottom edges end at 25.
            expect (mapped == Rectangle<int> (12, 12, 13, 13));
        });

        testCase ("Logical rectangles that tile the surface map onto every buffer pixel", [&]
        {
            const Rectangle<int> logicalBounds { 100, 100 };
            const Rectangle<int> bufferBounds { 125, 125 };

            RectangleList<int> mapped;
            mapped.add (WaylandSurfaceScale::mapLogicalRectToBuffer ({ 0, 0, 50, 100 }, logicalBounds, bufferBounds));
            mapped.add (WaylandSurfaceScale::mapLogicalRectToBuffer ({ 50, 0, 50, 100 }, logicalBounds, bufferBounds));

            expect (mapped.containsRectangle (bufferBounds));
        });

        testCase ("A rounded buffer extent maps using the ratio between the buffer and logical sizes", [&]
        {
            // A logical width of 101 at a render scale of 1.25 rounds to a buffer width of 126.
            const Rectangle<int> logicalBounds { 101, 101 };
            const Rectangle<int> bufferBounds { 126, 126 };

            const auto atRightEdge = WaylandSurfaceScale::mapLogicalRectToBuffer ({ 91, 0, 10, 10 },
                                                                                  logicalBounds,
                                                                                  bufferBounds);
            expectEquals (atRightEdge.getRight(), 126);

            const auto nearLeftEdge = WaylandSurfaceScale::mapLogicalRectToBuffer ({ 20, 0, 4, 4 },
                                                                                   logicalBounds,
                                                                                   bufferBounds);
            // The render scale of 1.25 would place this edge on pixel 25 instead.
            expectEquals (nearLeftEdge.getX(), 24);
        });

        testCase ("An empty logical size maps every rectangle to an empty rectangle", [&]
        {
            expect (WaylandSurfaceScale::mapLogicalRectToBuffer ({ 10, 10, 10, 10 }, {}, { 125, 125 }).isEmpty());
        });

        testCase ("A child surface the size of its parent maps rectangles unchanged", [&]
        {
            const Rectangle<int> bounds { 100, 50 };
            const Rectangle<int> area { 10, 5, 20, 15 };

            expect (WaylandSurfaceScale::mapLogicalRectToSurface (area, bounds, bounds) == area);
        });

        testCase ("Adjacent child surfaces share one rounded fractional boundary", [&]
        {
            const Rectangle<int> logicalBounds { 2, 1 };
            const Rectangle<int> surfaceBounds { 3, 1 };
            const auto left = WaylandSurfaceScale::mapLogicalRectToSurface ({ 0, 0, 1, 1 },
                                                                            logicalBounds,
                                                                            surfaceBounds);
            const auto right = WaylandSurfaceScale::mapLogicalRectToSurface ({ 1, 0, 1, 1 },
                                                                             logicalBounds,
                                                                             surfaceBounds);

            expectEquals (left.getRight(), right.getX());
            expectEquals (left.getWidth() + right.getWidth(), surfaceBounds.getWidth());
        });
    }
};

static WaylandSurfaceScaleTests waylandSurfaceScaleTests;

#endif

} // namespace juce
