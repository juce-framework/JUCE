/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
