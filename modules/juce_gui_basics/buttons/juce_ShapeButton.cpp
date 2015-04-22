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

ShapeButton::ShapeButton (const String& t, Colour n, Colour o, Colour d)
  : Button (t),
    normalColour (n), overColour (o), downColour (d),
    maintainShapeProportions (false),
    outlineWidth (0.0f)
{
}

ShapeButton::~ShapeButton() {}

void ShapeButton::setColours (Colour newNormalColour, Colour newOverColour, Colour newDownColour)
{
    normalColour = newNormalColour;
    overColour = newOverColour;
    downColour = newDownColour;
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

    g.setColour (isButtonDown ? downColour
                              : isMouseOverButton ? overColour
                                                  : normalColour);
    g.fillPath (shape, trans);

    if (outlineWidth > 0.0f)
    {
        g.setColour (outlineColour);
        g.strokePath (shape, PathStrokeType (outlineWidth), trans);
    }
}
