/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ShapeButton::ShapeButton (const String& t, Colour n, Colour o, Colour d)
  : Button (t),
    normalColour   (n), overColour   (o), downColour   (d),
    normalColourOn (n), overColourOn (o), downColourOn (d),
    useOnColours(false),
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
