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

ShapeButton::ShapeButton (const String& t, Colour n, Colour o, Colour d)
  : Button (t),
    normalColour   (n), overColour   (o), downColour   (d),
    normalColourOn (n), overColourOn (o), downColourOn (d),
    useOnColours (false),
    maintainShapeProportions (false),
    outlineWidth (0.0f)
{
}

ShapeButton::~ShapeButton() {}

void ShapeButton::setColours (Colour newNormalColour, Colour newOverColour, Colour newDownColour)
{
    normalColour = newNormalColour;
    overColour   = newOverColour;
    downColour   = newDownColour;
}

void ShapeButton::setOnColours (Colour newNormalColourOn, Colour newOverColourOn, Colour newDownColourOn)
{
    normalColourOn = newNormalColourOn;
    overColourOn   = newOverColourOn;
    downColourOn   = newDownColourOn;
}

void ShapeButton::shouldUseOnColours (bool shouldUse)
{
    useOnColours = shouldUse;
}

void ShapeButton::setOutline (Colour newOutlineColour, const float newOutlineWidth)
{
    outlineColour = newOutlineColour;
    outlineWidth = newOutlineWidth;
}

void ShapeButton::setBorderSize (BorderSize<int> newBorder)
{
    border = newBorder;
}

void ShapeButton::setShape (const Path& newShape,
                            const bool resizeNowToFitThisShape,
                            const bool maintainShapeProportions_,
                            const bool hasShadow)
{
    shape = newShape;
    maintainShapeProportions = maintainShapeProportions_;

    shadow.setShadowProperties (DropShadow (Colours::black.withAlpha (0.5f), 3, Point<int>()));
    setComponentEffect (hasShadow ? &shadow : nullptr);

    if (resizeNowToFitThisShape)
    {
        auto newBounds = shape.getBounds();

        if (hasShadow)
            newBounds = newBounds.expanded (4.0f);

        shape.applyTransform (AffineTransform::translation (-newBounds.getX(),
                                                            -newBounds.getY()));

        setSize (1 + (int) (newBounds.getWidth()  + outlineWidth) + border.getLeftAndRight(),
                 1 + (int) (newBounds.getHeight() + outlineWidth) + border.getTopAndBottom());
    }

    repaint();
}

void ShapeButton::paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    if (! isEnabled())
    {
        shouldDrawButtonAsHighlighted = false;
        shouldDrawButtonAsDown = false;
    }

    auto r = border.subtractedFrom (getLocalBounds())
                   .toFloat()
                   .reduced (outlineWidth * 0.5f);

    if (getComponentEffect() != nullptr)
        r = r.reduced (2.0f);

    if (shouldDrawButtonAsDown)
    {
        const float sizeReductionWhenPressed = 0.04f;

        r = r.reduced (sizeReductionWhenPressed * r.getWidth(),
                       sizeReductionWhenPressed * r.getHeight());
    }

    auto trans = shape.getTransformToScaleToFit (r, maintainShapeProportions);

    if      (shouldDrawButtonAsDown)        g.setColour (getToggleState() && useOnColours ? downColourOn   : downColour);
    else if (shouldDrawButtonAsHighlighted) g.setColour (getToggleState() && useOnColours ? overColourOn   : overColour);
    else                                    g.setColour (getToggleState() && useOnColours ? normalColourOn : normalColour);

    g.fillPath (shape, trans);

    if (outlineWidth > 0.0f)
    {
        g.setColour (outlineColour);
        g.strokePath (shape, PathStrokeType (outlineWidth), trans);
    }
}

} // namespace juce
