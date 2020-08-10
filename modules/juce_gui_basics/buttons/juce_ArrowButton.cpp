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

ArrowButton::ArrowButton (const String& name, float arrowDirectionInRadians, Colour arrowColour)
   : Button (name), colour (arrowColour)
{
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (MathConstants<float>::twoPi * arrowDirectionInRadians, 0.5f, 0.5f));
}

ArrowButton::~ArrowButton() {}

void ArrowButton::paintButton (Graphics& g, bool /*shouldDrawButtonAsHighlighted*/, bool shouldDrawButtonAsDown)
{
    Path p (path);

    const float offset = shouldDrawButtonAsDown ? 1.0f : 0.0f;
    p.applyTransform (path.getTransformToScaleToFit (offset, offset, (float) getWidth() - 3.0f, (float) getHeight() - 3.0f, false));

    DropShadow (Colours::black.withAlpha (0.3f), shouldDrawButtonAsDown ? 2 : 4, Point<int>()).drawForPath (g, p);

    g.setColour (colour);
    g.fillPath (p);
}

} // namespace juce
