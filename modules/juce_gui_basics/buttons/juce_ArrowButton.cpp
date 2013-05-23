/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

ArrowButton::ArrowButton (const String& name, float arrowDirectionInRadians, Colour arrowColour)
   : Button (name), colour (arrowColour)
{
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (float_Pi * 2.0f * arrowDirectionInRadians, 0.5f, 0.5f));
}

ArrowButton::~ArrowButton() {}

void ArrowButton::paintButton (Graphics& g, bool /*isMouseOverButton*/, bool isButtonDown)
{
    Path p (path);

    const float offset = isButtonDown ? 1.0f : 0.0f;
    p.applyTransform (path.getTransformToScaleToFit (offset, offset, getWidth() - 3.0f, getHeight() - 3.0f, false));

    DropShadow (Colours::black.withAlpha (0.3f), isButtonDown ? 2 : 4, Point<int>()).drawForPath (g, p);

    g.setColour (colour);
    g.fillPath (p);
}
