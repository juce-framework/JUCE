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
