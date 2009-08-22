/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ArrowButton.h"


//==============================================================================
ArrowButton::ArrowButton (const String& name,
                          float arrowDirectionInRadians,
                          const Colour& arrowColour)
   : Button (name),
     colour (arrowColour)
{
    path.lineTo (0.0f, 1.0f);
    path.lineTo (1.0f, 0.5f);
    path.closeSubPath();

    path.applyTransform (AffineTransform::rotation (float_Pi * 2.0f * arrowDirectionInRadians,
                                                    0.5f, 0.5f));

    setComponentEffect (&shadow);
    buttonStateChanged();
}

ArrowButton::~ArrowButton()
{
}

void ArrowButton::paintButton (Graphics& g,
                               bool /*isMouseOverButton*/,
                               bool /*isButtonDown*/)
{
    g.setColour (colour);

    g.fillPath (path, path.getTransformToScaleToFit ((float) offset,
                                                     (float) offset,
                                                     (float) (getWidth() - 3),
                                                     (float) (getHeight() - 3),
                                                     false));
}

void ArrowButton::buttonStateChanged()
{
    offset = (isDown()) ? 1 : 0;

    shadow.setShadowProperties ((isDown()) ? 1.2f : 3.0f,
                                0.3f, -1, 0);
}

END_JUCE_NAMESPACE
