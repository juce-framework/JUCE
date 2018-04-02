/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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

ShapeButton::ShapeButton (const String& t, Color n, Color o, Color d)
  : Button (t),
    normalColor   (n), overColor   (o), downColor   (d),
    normalColorOn (n), overColorOn (o), downColorOn (d),
    useOnColors(false),
    maintainShapeProportions (false),
    outlineWidth (0.0f)
{
}

ShapeButton::~ShapeButton() {}

void ShapeButton::setColors (Color newNormalColor, Color newOverColor, Color newDownColor)
{
    normalColor = newNormalColor;
    overColor   = newOverColor;
    downColor   = newDownColor;
}

void ShapeButton::setOnColors (Color newNormalColorOn, Color newOverColorOn, Color newDownColorOn)
{
    normalColorOn = newNormalColorOn;
    overColorOn   = newOverColorOn;
    downColorOn   = newDownColorOn;
}

void ShapeButton::shouldUseOnColors (bool shouldUse)
{
    useOnColors = shouldUse;
}

void ShapeButton::setOutline (Color newOutlineColor, const float newOutlineWidth)
{
    outlineColor = newOutlineColor;
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

    shadow.setShadowProperties (DropShadow (Colors::black.withAlpha (0.5f), 3, Point<int>()));
    setComponentEffect (hasShadow ? &shadow : nullptr);

    if (resizeNowToFitThisShape)
    {
        Rectangle<float> newBounds (shape.getBounds());

        if (hasShadow)
            newBounds = newBounds.expanded (4.0f);

        shape.applyTransform (AffineTransform::translation (-newBounds.getX(),
                                                            -newBounds.getY()));

        setSize (1 + (int) (newBounds.getWidth()  + outlineWidth) + border.getLeftAndRight(),
                 1 + (int) (newBounds.getHeight() + outlineWidth) + border.getTopAndBottom());
    }

    repaint();
}

void ShapeButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    if (! isEnabled())
    {
        isMouseOverButton = false;
        isButtonDown = false;
    }

    Rectangle<float> r (border.subtractedFrom (getLocalBounds()).toFloat().reduced (outlineWidth * 0.5f));

    if (getComponentEffect() != nullptr)
        r = r.reduced (2.0f);

    if (isButtonDown)
    {
        const float sizeReductionWhenPressed = 0.04f;

        r = r.reduced (sizeReductionWhenPressed * r.getWidth(),
                       sizeReductionWhenPressed * r.getHeight());
    }

    const AffineTransform trans (shape.getTransformToScaleToFit (r, maintainShapeProportions));

    if      (isButtonDown)      g.setColor (getToggleState() && useOnColors ? downColorOn   : downColor);
    else if (isMouseOverButton) g.setColor (getToggleState() && useOnColors ? overColorOn   : overColor);
    else                        g.setColor (getToggleState() && useOnColors ? normalColorOn : normalColor);

    g.fillPath (shape, trans);

    if (outlineWidth > 0.0f)
    {
        g.setColor (outlineColor);
        g.strokePath (shape, PathStrokeType (outlineWidth), trans);
    }
}

} // namespace juce
