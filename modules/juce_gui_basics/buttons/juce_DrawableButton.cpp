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

DrawableButton::DrawableButton (const String& name, const DrawableButton::ButtonStyle buttonStyle)
    : Button (name),
      style (buttonStyle),
      currentImage (nullptr),
      edgeIndent (3)
{
}

DrawableButton::~DrawableButton()
{
}

//==============================================================================
static Drawable* copyDrawableIfNotNull (const Drawable* const d)
{
    return d != nullptr ? d->createCopy() : nullptr;
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
    jassert (normal != nullptr); // you really need to give it at least a normal image..

    normalImage     = copyDrawableIfNotNull (normal);
    overImage       = copyDrawableIfNotNull (over);
    downImage       = copyDrawableIfNotNull (down);
    disabledImage   = copyDrawableIfNotNull (disabled);
    normalImageOn   = copyDrawableIfNotNull (normalOn);
    overImageOn     = copyDrawableIfNotNull (overOn);
    downImageOn     = copyDrawableIfNotNull (downOn);
    disabledImageOn = copyDrawableIfNotNull (disabledOn);

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
        else if (style == ImageStretched)
        {
            currentImage->setTransformToFit (getLocalBounds().toFloat(), RectanglePlacement::stretchToFit);
        }
        else
        {
            Rectangle<int> imageSpace;

            const int indentX = jmin (edgeIndent, proportionOfWidth  (0.3f));
            const int indentY = jmin (edgeIndent, proportionOfHeight (0.3f));

            if (style == ImageOnButtonBackground)
            {
                imageSpace = getLocalBounds().reduced (jmax (getWidth()  / 4, indentX),
                                                       jmax (getHeight() / 4, indentY));
            }
            else
            {
                const int textH = (style == ImageAboveTextLabel) ? jmin (16, proportionOfHeight (0.25f)) : 0;

                imageSpace.setBounds (indentX, indentY,
                                      getWidth()  - indentX * 2,
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

void DrawableButton::colourChanged()
{
    repaint();
}

void DrawableButton::paintButton (Graphics& g,
                                  const bool isMouseOverButton,
                                  const bool isButtonDown)
{
    LookAndFeel& lf = getLookAndFeel();

    if (style == ImageOnButtonBackground)
        lf.drawButtonBackground (g, *this,
                                 findColour (getToggleState() ? TextButton::buttonOnColourId
                                                              : TextButton::buttonColourId),
                                 isMouseOverButton, isButtonDown);
    else
        lf.drawDrawableButton (g, *this, isMouseOverButton, isButtonDown);
}

//==============================================================================
Drawable* DrawableButton::getCurrentImage() const noexcept
{
    if (isDown())  return getDownImage();
    if (isOver())  return getOverImage();

    return getNormalImage();
}

Drawable* DrawableButton::getNormalImage() const noexcept
{
    return (getToggleState() && normalImageOn != nullptr) ? normalImageOn
                                                          : normalImage;
}

Drawable* DrawableButton::getOverImage() const noexcept
{
    if (getToggleState())
    {
        if (overImageOn   != nullptr)   return overImageOn;
        if (normalImageOn != nullptr)   return normalImageOn;
    }

    return overImage != nullptr ? overImage : normalImage;
}

Drawable* DrawableButton::getDownImage() const noexcept
{
    if (Drawable* const d = getToggleState() ? downImageOn : downImage)
        return d;

    return getOverImage();
}
