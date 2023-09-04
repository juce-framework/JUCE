/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::detail
{

struct ScalingHelpers
{
    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (float scale, PointOrRect pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? pos / scale : pos;
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (float scale, PointOrRect pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? pos * scale : pos;
    }

    // For these, we need to avoid getSmallestIntegerContainer being used, which causes
    // judder when moving windows
    static Rectangle<int> unscaledScreenPosToScaled (float scale, Rectangle<int> pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? Rectangle<int> (roundToInt ((float) pos.getX() / scale),
                                                                    roundToInt ((float) pos.getY() / scale),
                                                                    roundToInt ((float) pos.getWidth() / scale),
                                                                    roundToInt ((float) pos.getHeight() / scale)) : pos;
    }

    static Rectangle<int> scaledScreenPosToUnscaled (float scale, Rectangle<int> pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? Rectangle<int> (roundToInt ((float) pos.getX() * scale),
                                                                    roundToInt ((float) pos.getY() * scale),
                                                                    roundToInt ((float) pos.getWidth() * scale),
                                                                    roundToInt ((float) pos.getHeight() * scale)) : pos;
    }

    static Rectangle<float> unscaledScreenPosToScaled (float scale, Rectangle<float> pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? Rectangle<float> (pos.getX() / scale,
                                                                      pos.getY() / scale,
                                                                      pos.getWidth() / scale,
                                                                      pos.getHeight() / scale) : pos;
    }

    static Rectangle<float> scaledScreenPosToUnscaled (float scale, Rectangle<float> pos) noexcept
    {
        return ! approximatelyEqual (scale, 1.0f) ? Rectangle<float> (pos.getX() * scale,
                                                                      pos.getY() * scale,
                                                                      pos.getWidth() * scale,
                                                                      pos.getHeight() * scale) : pos;
    }

    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (PointOrRect pos) noexcept
    {
        return unscaledScreenPosToScaled (Desktop::getInstance().getGlobalScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (PointOrRect pos) noexcept
    {
        return scaledScreenPosToUnscaled (Desktop::getInstance().getGlobalScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect unscaledScreenPosToScaled (const Component& comp, PointOrRect pos) noexcept
    {
        return unscaledScreenPosToScaled (comp.getDesktopScaleFactor(), pos);
    }

    template <typename PointOrRect>
    static PointOrRect scaledScreenPosToUnscaled (const Component& comp, PointOrRect pos) noexcept
    {
        return scaledScreenPosToUnscaled (comp.getDesktopScaleFactor(), pos);
    }

    static Point<int>       addPosition      (Point<int> p,       const Component& c) noexcept  { return p + c.getPosition(); }
    static Rectangle<int>   addPosition      (Rectangle<int> p,   const Component& c) noexcept  { return p + c.getPosition(); }
    static Point<float>     addPosition      (Point<float> p,     const Component& c) noexcept  { return p + c.getPosition().toFloat(); }
    static Rectangle<float> addPosition      (Rectangle<float> p, const Component& c) noexcept  { return p + c.getPosition().toFloat(); }
    static Point<int>       subtractPosition (Point<int> p,       const Component& c) noexcept  { return p - c.getPosition(); }
    static Rectangle<int>   subtractPosition (Rectangle<int> p,   const Component& c) noexcept  { return p - c.getPosition(); }
    static Point<float>     subtractPosition (Point<float> p,     const Component& c) noexcept  { return p - c.getPosition().toFloat(); }
    static Rectangle<float> subtractPosition (Rectangle<float> p, const Component& c) noexcept  { return p - c.getPosition().toFloat(); }

    static Point<float> screenPosToLocalPos (Component& comp, Point<float> pos)
    {
        if (auto* peer = comp.getPeer())
        {
            pos = peer->globalToLocal (pos);
            auto& peerComp = peer->getComponent();
            return comp.getLocalPoint (&peerComp, unscaledScreenPosToScaled (peerComp, pos));
        }

        return comp.getLocalPoint (nullptr, unscaledScreenPosToScaled (comp, pos));
    }
};

} // namespace juce::detail
