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

//==============================================================================
namespace
{
    template <class RectType>
    Rectangle<int> convertToRectInt (RectType r) noexcept
    {
        return Rectangle<int> ((int) r.origin.x, (int) r.origin.y, (int) r.size.width, (int) r.size.height);
    }

    template <class RectType>
    Rectangle<float> convertToRectFloat (RectType r) noexcept
    {
        return Rectangle<float> (r.origin.x, r.origin.y, r.size.width, r.size.height);
    }

    template <class RectType>
    CGRect convertToCGRect (RectType r) noexcept
    {
        return CGRectMake ((CGFloat) r.getX(), (CGFloat) r.getY(), (CGFloat) r.getWidth(), (CGFloat) r.getHeight());
    }

    template <typename PointType>
    CGPoint convertToCGPoint (PointType p) noexcept
    {
        return CGPointMake ((CGFloat) p.x, (CGFloat) p.y);
    }
}

CGImageRef juce_createCoreGraphicsImage (const Image&, CGColorSpaceRef, bool mustOutliveSource);
CGContextRef juce_getImageContext (const Image&);

#if JUCE_IOS
 Image juce_createImageFromUIImage (UIImage*);
#endif

#if JUCE_MAC
 NSImage* imageToNSImage (const Image& image, float scaleFactor = 1.0f);
#endif

} // namespace juce
