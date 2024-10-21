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

//==============================================================================
namespace
{
    template <class RectType>
    Rectangle<int> convertToRectInt (RectType r) noexcept
    {
        return { (int) r.origin.x,
                 (int) r.origin.y,
                 (int) r.size.width,
                 (int) r.size.height };
    }

    template <class RectType>
    Rectangle<float> convertToRectFloat (RectType r) noexcept
    {
        return { (float) r.origin.x,
                 (float) r.origin.y,
                 (float) r.size.width,
                 (float) r.size.height };
    }

    template <class RectType>
    CGRect convertToCGRect (RectType r) noexcept
    {
        return CGRectMake ((CGFloat) r.getX(), (CGFloat) r.getY(), (CGFloat) r.getWidth(), (CGFloat) r.getHeight());
    }

    template <class PointType>
    Point<float> convertToPointFloat (PointType p) noexcept
    {
        return { (float) p.x, (float) p.y };
    }

    template <typename PointType>
    CGPoint convertToCGPoint (PointType p) noexcept
    {
        return CGPointMake ((CGFloat) p.x, (CGFloat) p.y);
    }

    template <class PointType>
    Point<int> roundToIntPoint (PointType p) noexcept
    {
        return { roundToInt (p.x), roundToInt (p.y) };
    }

   #if JUCE_MAC
    inline CGFloat getMainScreenHeight() noexcept
    {
        if ([[NSScreen screens] count] == 0)
            return 0.0f;

        return [[[NSScreen screens] objectAtIndex: 0] frame].size.height;
    }

    inline NSRect flippedScreenRect (NSRect r) noexcept
    {
        r.origin.y = getMainScreenHeight() - (r.origin.y + r.size.height);
        return r;
    }

    inline NSPoint flippedScreenPoint (NSPoint p) noexcept
    {
        p.y = getMainScreenHeight() - p.y;
        return p;
    }
   #endif
}

CGImageRef juce_createCoreGraphicsImage (const Image&, CGColorSpaceRef);
CGContextRef juce_getImageContext (const Image&);

#if JUCE_IOS
 Image juce_createImageFromUIImage (UIImage*);
#endif

#if JUCE_MAC
 NSImage* imageToNSImage (const ScaledImage& image);
#endif

} // namespace juce
