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

#include "juce_DrawableButton.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
DrawableButton::DrawableButton (const String& name,
                                const DrawableButton::ButtonStyle buttonStyle)
    : Button (name),
      style (buttonStyle),
      edgeIndent (3)
{
    if (buttonStyle == ImageOnButtonBackground)
    {
        backgroundOff = Colour (0xffbbbbff);
        backgroundOn = Colour (0xff3333ff);
    }
    else
    {
        backgroundOff = Colours::transparentBlack;
        backgroundOn = Colour (0xaabbbbff);
    }
}

DrawableButton::~DrawableButton()
{
    deleteImages();
}

//==============================================================================
void DrawableButton::deleteImages()
{
}

void DrawableButton::setImages (const Drawable* normal,
                                const Drawable* over,
                                const Drawable* down,
                                const Drawable* disabled,
                                const Drawable* normalOn,
                                const Drawable* overOn,
                                const Drawable* downOn,
                                const Drawable* disabledOn)
{
    deleteImages();

    jassert (normal != 0); // you really need to give it at least a normal image..

    if (normal != 0)
        normalImage = normal->createCopy();

    if (over != 0)
        overImage = over->createCopy();

    if (down != 0)
        downImage = down->createCopy();

    if (disabled != 0)
        disabledImage = disabled->createCopy();


    if (normalOn != 0)
        normalImageOn = normalOn->createCopy();

    if (overOn != 0)
        overImageOn = overOn->createCopy();

    if (downOn != 0)
        downImageOn = downOn->createCopy();

    if (disabledOn != 0)
        disabledImageOn = disabledOn->createCopy();

    repaint();
}

//==============================================================================
void DrawableButton::setButtonStyle (const DrawableButton::ButtonStyle newStyle)
{
    if (style != newStyle)
    {
        style = newStyle;
        repaint();
    }
}

void DrawableButton::setBackgroundColours (const Colour& toggledOffColour,
                                           const Colour& toggledOnColour)
{
    if (backgroundOff != toggledOffColour
         || backgroundOn != toggledOnColour)
    {
        backgroundOff = toggledOffColour;
        backgroundOn = toggledOnColour;

        repaint();
    }
}

const Colour& DrawableButton::getBackgroundColour() const throw()
{
    return getToggleState() ? backgroundOn
                            : backgroundOff;
}

void DrawableButton::setEdgeIndent (const int numPixelsIndent)
{
    edgeIndent = numPixelsIndent;
    repaint();
}

void DrawableButton::paintButton (Graphics& g,
                                  bool isMouseOverButton,
                                  bool isButtonDown)
{
    Rectangle imageSpace;

    if (style == ImageOnButtonBackground)
    {
        const int insetX = getWidth() / 4;
        const int insetY = getHeight() / 4;

        imageSpace.setBounds (insetX, insetY, getWidth() - insetX * 2, getHeight() - insetY * 2);

        getLookAndFeel().drawButtonBackground (g, *this,
                                               getBackgroundColour(),
                                               isMouseOverButton,
                                               isButtonDown);
    }
    else
    {
        g.fillAll (getBackgroundColour());

        const int textH = (style == ImageAboveTextLabel)
                            ? jmin (16, proportionOfHeight (0.25f))
                            : 0;

        const int indentX = jmin (edgeIndent, proportionOfWidth (0.3f));
        const int indentY = jmin (edgeIndent, proportionOfHeight (0.3f));

        imageSpace.setBounds (indentX, indentY,
                              getWidth() - indentX * 2,
                              getHeight() - indentY * 2 - textH);

        if (textH > 0)
        {
            g.setFont ((float) textH);

            g.setColour (Colours::black.withAlpha (isEnabled() ? 1.0f : 0.4f));
            g.drawFittedText (getButtonText(),
                              2, getHeight() - textH - 1,
                              getWidth() - 4, textH,
                              Justification::centred, 1);
        }
    }

    g.setImageResamplingQuality (Graphics::mediumResamplingQuality);
    g.setOpacity (1.0f);

    const Drawable* imageToDraw = 0;

    if (isEnabled())
    {
        imageToDraw = getCurrentImage();
    }
    else
    {
        imageToDraw = getToggleState() ? disabledImageOn
                                       : disabledImage;

        if (imageToDraw == 0)
        {
            g.setOpacity (0.4f);
            imageToDraw = getNormalImage();
        }
    }

    if (imageToDraw != 0)
    {
        if (style == ImageRaw)
        {
            imageToDraw->draw (g, 1.0f);
        }
        else
        {
            imageToDraw->drawWithin (g,
                                     imageSpace.getX(),
                                     imageSpace.getY(),
                                     imageSpace.getWidth(),
                                     imageSpace.getHeight(),
                                     RectanglePlacement::centred,
                                     1.0f);
        }
    }
}

//==============================================================================
const Drawable* DrawableButton::getCurrentImage() const throw()
{
    if (isDown())
        return getDownImage();

    if (isOver())
        return getOverImage();

    return getNormalImage();
}

const Drawable* DrawableButton::getNormalImage() const throw()
{
    return (getToggleState() && normalImageOn != 0) ? normalImageOn
                                                    : normalImage;
}

const Drawable* DrawableButton::getOverImage() const throw()
{
    const Drawable* d = normalImage;

    if (getToggleState())
    {
        if (overImageOn != 0)
            d = overImageOn;
        else if (normalImageOn != 0)
            d = normalImageOn;
        else if (overImage != 0)
            d = overImage;
    }
    else
    {
        if (overImage != 0)
            d = overImage;
    }

    return d;
}

const Drawable* DrawableButton::getDownImage() const throw()
{
    const Drawable* d = normalImage;

    if (getToggleState())
    {
        if (downImageOn != 0)
            d = downImageOn;
        else if (overImageOn != 0)
            d = overImageOn;
        else if (normalImageOn != 0)
            d = normalImageOn;
        else if (downImage != 0)
            d = downImage;
        else
            d = getOverImage();
    }
    else
    {
        if (downImage != 0)
            d = downImage;
        else
            d = getOverImage();
    }

    return d;
}

END_JUCE_NAMESPACE
