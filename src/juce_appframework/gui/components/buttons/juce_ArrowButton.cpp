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
