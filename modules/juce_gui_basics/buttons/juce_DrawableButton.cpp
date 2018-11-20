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

DrawableButton::DrawableButton (const String& name, const DrawableButton::ButtonStyle buttonStyle)
    : Button (name), style (buttonStyle)
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

    normalImage     .reset (copyDrawableIfNotNull (normal));
    overImage       .reset (copyDrawableIfNotNull (over));
    downImage       .reset (copyDrawableIfNotNull (down));
    disabledImage   .reset (copyDrawableIfNotNull (disabled));
    normalImageOn   .reset (copyDrawableIfNotNull (normalOn));
    overImageOn     .reset (copyDrawableIfNotNull (overOn));
    downImageOn     .reset (copyDrawableIfNotNull (downOn));
    disabledImageOn .reset (copyDrawableIfNotNull (disabledOn));

    currentImage = nullptr;

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

Rectangle<float> DrawableButton::getImageBounds() const
{
    auto r = getLocalBounds();

    if (style != ImageStretched)
    {
        auto indentX = jmin (edgeIndent, proportionOfWidth  (0.3f));
        auto indentY = jmin (edgeIndent, proportionOfHeight (0.3f));

        if (style == ImageOnButtonBackground)
        {
            indentX = jmax (getWidth()  / 4, indentX);
            indentY = jmax (getHeight() / 4, indentY);
        }
        else if (style == ImageAboveTextLabel)
        {
            r = r.withTrimmedBottom (jmin (16, proportionOfHeight (0.25f)));
        }

        r = r.reduced (indentX, indentY);
    }

    return r.toFloat();
}

void DrawableButton::resized()
{
    Button::resized();

    if (currentImage != nullptr)
    {
        if (style == ImageRaw)
            currentImage->setOriginWithOriginalSize (Point<float>());
        else
            currentImage->setTransformToFit (getImageBounds(),
                                             style == ImageStretched ? RectanglePlacement::stretchToFit
                                                                     : RectanglePlacement::centred);
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
        imageToDraw = getToggleState() ? disabledImageOn.get()
                                       : disabledImage.get();

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
            resized();
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
                                  const bool shouldDrawButtonAsHighlighted,
                                  const bool shouldDrawButtonAsDown)
{
    auto& lf = getLookAndFeel();

    if (style == ImageOnButtonBackground)
        lf.drawButtonBackground (g, *this,
                                 findColour (getToggleState() ? TextButton::buttonOnColourId
                                                              : TextButton::buttonColourId),
                                 shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    else
        lf.drawDrawableButton (g, *this, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
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
    return (getToggleState() && normalImageOn != nullptr) ? normalImageOn.get()
                                                          : normalImage.get();
}

Drawable* DrawableButton::getOverImage() const noexcept
{
    if (getToggleState())
    {
        if (overImageOn   != nullptr)   return overImageOn.get();
        if (normalImageOn != nullptr)   return normalImageOn.get();
    }

    return overImage != nullptr ? overImage.get() : normalImage.get();
}

Drawable* DrawableButton::getDownImage() const noexcept
{
    if (auto* d = getToggleState() ? downImageOn.get() : downImage.get())
        return d;

    return getOverImage();
}

} // namespace juce
