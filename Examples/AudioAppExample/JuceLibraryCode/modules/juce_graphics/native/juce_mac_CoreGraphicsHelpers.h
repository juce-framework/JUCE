/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_MAC_COREGRAPHICSHELPERS_H_INCLUDED
#define JUCE_MAC_COREGRAPHICSHELPERS_H_INCLUDED


//==============================================================================
namespace
{
    template <class RectType>
    Rectangle<int> convertToRectInt (const RectType& r) noexcept
    {
        return Rectangle<int> ((int) r.origin.x, (int) r.origin.y, (int) r.size.width, (int) r.size.height);
    }

    template <class RectType>
    Rectangle<float> convertToRectFloat (const RectType& r) noexcept
    {
        return Rectangle<float> (r.origin.x, r.origin.y, r.size.width, r.size.height);
    }

    template <class RectType>
    CGRect convertToCGRect (const RectType& r) noexcept
    {
        return CGRectMake ((CGFloat) r.getX(), (CGFloat) r.getY(), (CGFloat) r.getWidth(), (CGFloat) r.getHeight());
    }

    template <typename PointType>
    CGPoint convertToCGPoint (const PointType& p) noexcept
    {
        return CGPointMake ((CGFloat) p.x, (CGFloat) p.y);
    }
}

extern CGImageRef juce_createCoreGraphicsImage (const Image&, CGColorSpaceRef, bool mustOutliveSource);
extern CGContextRef juce_getImageContext (const Image&);

#endif   // JUCE_MAC_COREGRAPHICSHELPERS_H_INCLUDED
