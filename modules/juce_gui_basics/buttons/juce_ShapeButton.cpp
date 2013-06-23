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

ShapeButton::ShapeButton (const String& text_,
                          const Colour& normalColour_,
                          const Colour& overColour_,
                          const Colour& downColour_)
  : Button (text_),
    normalColour (normalColour_),
    overColour (overColour_),
    downColour (downColour_),
    maintainShapeProportions (false),
    outlineWidth (0.0f)
{
}

ShapeButton::~ShapeButton()
{
}

void ShapeButton::setColours (const Colour& newNormalColour,
                              const Colour& newOverColour,
                              const Colour& newDownColour)
{
    normalColour = newNormalColour;
    overColour = newOverColour;
    downColour = newDownColour;
}

void ShapeButton::setOutline (const Colour& newOutlineColour,
                              const float newOutlineWidth)
{
    outlineColour = newOutlineColour;
    outlineWidth = newOutlineWidth;
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
            newBounds.expand (4.0f, 4.0f);

        shape.applyTransform (AffineTransform::translation (-newBounds.getX(), -newBounds.getY()));

        setSize (1 + (int) (newBounds.getWidth() + outlineWidth),
                 1 + (int) (newBounds.getHeight() + outlineWidth));
    }
}

void ShapeButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    if (! isEnabled())
    {
        isMouseOverButton = false;
        isButtonDown = false;
    }

    g.setColour (isButtonDown ? downColour
                              : isMouseOverButton ? overColour
                                                  : normalColour);

    int w = getWidth();
    int h = getHeight();

    if (getComponentEffect() != nullptr)
    {
        w -= 4;
        h -= 4;
    }

    const float offset = (outlineWidth * 0.5f) + (isButtonDown ? 1.5f : 0.0f);

    const AffineTransform trans (shape.getTransformToScaleToFit (offset, offset,
                                                                 w - offset - outlineWidth,
                                                                 h - offset - outlineWidth,
                                                                 maintainShapeProportions));
    g.fillPath (shape, trans);

    if (outlineWidth > 0.0f)
    {
        g.setColour (outlineColour);
        g.strokePath (shape, PathStrokeType (outlineWidth), trans);
    }
}
