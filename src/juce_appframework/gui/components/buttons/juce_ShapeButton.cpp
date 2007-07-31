/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ShapeButton.h"


//==============================================================================
ShapeButton::ShapeButton (const String& text,
                          const Colour& normalColour_,
                          const Colour& overColour_,
                          const Colour& downColour_)
  : Button (text),
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

    shadow.setShadowProperties (3.0f, 0.5f, 0, 0);
    setComponentEffect ((hasShadow) ? &shadow : 0);

    if (resizeNowToFitThisShape)
    {
        float x, y, w, h;
        shape.getBounds (x, y, w, h);
        shape.applyTransform (AffineTransform::translation (-x, -y));

        if (hasShadow)
        {
            w += 4.0f;
            h += 4.0f;
            shape.applyTransform (AffineTransform::translation (2.0f, 2.0f));
        }

        setSize (1 + (int) (w + outlineWidth),
                 1 + (int) (h + outlineWidth));
    }
}

void ShapeButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    if (! isEnabled())
    {
        isMouseOverButton = false;
        isButtonDown = false;
    }

    g.setColour ((isButtonDown) ? downColour
                                : (isMouseOverButton) ? overColour
                                                      : normalColour);

    int w = getWidth();
    int h = getHeight();

    if (getComponentEffect() != 0)
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

END_JUCE_NAMESPACE
