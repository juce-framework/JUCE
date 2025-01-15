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

/*  A single bitmap that represents a subsection of a virtual bitmap. */
struct Direct2DPixelDataPage
{
    /*  The bounds of the stored bitmap inside the virtual bitmap. */
    Rectangle<int> getBounds() const
    {
        if (bitmap == nullptr)
            return {};

        const auto size = bitmap->GetPixelSize();
        return Rectangle { (int) size.width, (int) size.height }.withPosition (topLeft);
    }

    /*  The stored subsection bitmap. */
    ComSmartPtr<ID2D1Bitmap1> bitmap;

    /*  The top-left position of this virtual bitmap inside the virtual bitmap. */
    Point<int> topLeft;
};

} // namespace juce
