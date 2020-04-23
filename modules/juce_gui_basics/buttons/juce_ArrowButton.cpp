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
    p.applyTransform (path.getTransformToScaleToFit (offset, offset, getWidth() - 3.0f, getHeight() - 3.0f, false));

    DropShadow (Colours::black.withAlpha (0.3f), shouldDrawButtonAsDown ? 2 : 4, Point<int>()).drawForPath (g, p);

    g.setColour (colour);
    g.fillPath (p);
}

} // namespace juce
