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

DrawableButton::DrawableButton (const String& name,
                                const DrawableButton::ButtonStyle buttonStyle)
    : Button (name),
      style (buttonStyle),
      currentImage (nullptr),
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
}

//==============================================================================
void DrawableButton::setImages (const Drawable* normal,
                                const Drawable* over,
                                const Drawable* down,
                                const Drawable* disabled,
                                const Drawable* normalOn,
                                const Drawable* overOn,
                                const Drawable* downOn,
                                const Drawable* disabledOn)
{
    jassert (normal != nullptr); // you really need to give it at least a normal image..

    if (normal != nullptr)        normalImage = normal->createCopy();
    if (over != nullptr)          overImage = over->createCopy();
    if (down != nullptr)          downImage = down->createCopy();
    if (disabled != nullptr)      disabledImage = disabled->createCopy();
    if (normalOn != nullptr)      normalImageOn = normalOn->createCopy();
    if (overOn != nullptr)        overImageOn = overOn->createCopy();
    if (downOn != nullptr)        downImageOn = downOn->createCopy();
    if (disabledOn != nullptr)    disabledImageOn = disabledOn->createCopy();

    buttonStateChanged();
}

//==============================================================================
void DrawableButton::setButtonStyle (const DrawableButton::ButtonStyle newStyle)
{
    if (style != newStyle)
    {
        style = newStyle;
        buttonStateChanged();
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

const Colour& DrawableButton::getBackgroundColour() const noexcept
{
    return getToggleState() ? backgroundOn
                            : backgroundOff;
}

void DrawableButton::setEdgeIndent (const int numPixelsIndent)
{
    edgeIndent = numPixelsIndent;
    repaint();
    resized();
}

void DrawableButton::resized()
{
    Button::resized();

    if (currentImage != nullptr)
    {
        if (style == ImageRaw)
        {
            currentImage->setOriginWithOriginalSize (Point<float>());
        }
        else
        {
            Rectangle<int> imageSpace;

            if (style == ImageOnButtonBackground)
            {
                imageSpace = getLocalBounds().reduced (getWidth() / 4, getHeight() / 4);
            }
            else
            {
                const int textH = (style == ImageAboveTextLabel) ? jmin (16, proportionOfHeight (0.25f)) : 0;

                const int indentX = jmin (edgeIndent, proportionOfWidth (0.3f));
                const int indentY = jmin (edgeIndent, proportionOfHeight (0.3f));

                imageSpace.setBounds (indentX, indentY,
                                      getWidth() - indentX * 2,
                                      getHeight() - indentY * 2 - textH);
            }

            currentImage->setTransformToFit (imageSpace.toFloat(), RectanglePlacement::centred);
        }
    }
}

void DrawableButton::buttonStateChanged()
{
    repaint();

    Drawable* imageToDraw = nullptr;
    float opacity = 1.0f;

    if (isEnabled())
    {
        imageToDraw = getCurrentImage();
    }
    else
    {
        imageToDraw = getToggleState() ? disabledImageOn
                                       : disabledImage;

        if (imageToDraw == nullptr)
        {
            opacity = 0.4f;
            imageToDraw = getNormalImage();
        }
    }

    if (imageToDraw != currentImage)
    {
        removeChildComponent (currentImage);
        currentImage = imageToDraw;

        if (currentImage != nullptr)
        {
            currentImage->setInterceptsMouseClicks (false, false);
            addAndMakeVisible (currentImage);
            DrawableButton::resized();
        }
    }

    if (currentImage != nullptr)
        currentImage->setAlpha (opacity);
}

void DrawableButton::enablementChanged()
{
    Button::enablementChanged();
    buttonStateChanged();
}

void DrawableButton::paintButton (Graphics& g,
                                  bool isMouseOverButton,
                                  bool isButtonDown)
{
    if (style == ImageOnButtonBackground)
    {
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

        if (textH > 0)
        {
            g.setFont ((float) textH);

            g.setColour (findColour (DrawableButton::textColourId)
                            .withMultipliedAlpha (isEnabled() ? 1.0f : 0.4f));

            g.drawFittedText (getButtonText(),
                              2, getHeight() - textH - 1,
                              getWidth() - 4, textH,
                              Justification::centred, 1);
        }
    }
}

//==============================================================================
Drawable* DrawableButton::getCurrentImage() const noexcept
{
    if (isDown())
        return getDownImage();

    if (isOver())
        return getOverImage();

    return getNormalImage();
}

Drawable* DrawableButton::getNormalImage() const noexcept
{
    return (getToggleState() && normalImageOn != nullptr) ? normalImageOn
                                                          : normalImage;
}

Drawable* DrawableButton::getOverImage() const noexcept
{
    Drawable* d = normalImage;

    if (getToggleState())
    {
        if (overImageOn != nullptr)
            d = overImageOn;
        else if (normalImageOn != nullptr)
            d = normalImageOn;
        else if (overImage != nullptr)
            d = overImage;
    }
    else
    {
        if (overImage != nullptr)
            d = overImage;
    }

    return d;
}

Drawable* DrawableButton::getDownImage() const noexcept
{
    Drawable* d = normalImage;

    if (getToggleState())
    {
        if (downImageOn != nullptr)
            d = downImageOn;
        else if (overImageOn != nullptr)
            d = overImageOn;
        else if (normalImageOn != nullptr)
            d = normalImageOn;
        else if (downImage != nullptr)
            d = downImage;
        else
            d = getOverImage();
    }
    else
    {
        if (downImage != nullptr)
            d = downImage;
        else
            d = getOverImage();
    }

    return d;
}
