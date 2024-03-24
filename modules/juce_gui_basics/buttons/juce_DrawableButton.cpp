/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
static std::unique_ptr<Drawable> copyDrawableIfNotNull (const Drawable* const d)
{
    if (d != nullptr)
        return d->createCopy();

    return {};
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

        if (shouldDrawButtonBackground())
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
        if (style != ImageRaw)
        {
            int transformFlags = 0;

            if (style == ImageStretched)
            {
                transformFlags |= RectanglePlacement::stretchToFit;
            }
            else
            {
                transformFlags |= RectanglePlacement::centred;

                if (style == ImageOnButtonBackgroundOriginalSize)
                    transformFlags |= RectanglePlacement::doNotResize;
            }

            currentImage->setTransformToFit (getImageBounds(), transformFlags);
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

    if (shouldDrawButtonBackground())
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
