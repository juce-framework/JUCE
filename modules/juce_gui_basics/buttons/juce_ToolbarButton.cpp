/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ToolbarButton::ToolbarButton (const int iid, const String& buttonText,
                              std::unique_ptr<Drawable> normalIm,
                              std::unique_ptr<Drawable> toggledOnIm)
   : ToolbarItemComponent (iid, buttonText, true),
     normalImage (std::move (normalIm)),
     toggledOnImage (std::move (toggledOnIm))
{
    jassert (normalImage != nullptr);
}

ToolbarButton::~ToolbarButton()
{
}

//==============================================================================
bool ToolbarButton::getToolbarItemSizes (int toolbarDepth, bool /*isToolbarVertical*/, int& preferredSize, int& minSize, int& maxSize)
{
    preferredSize = minSize = maxSize = toolbarDepth;
    return true;
}

void ToolbarButton::paintButtonArea (Graphics&, int /*width*/, int /*height*/, bool /*isMouseOver*/, bool /*isMouseDown*/)
{
}

void ToolbarButton::contentAreaChanged (const Rectangle<int>&)
{
    buttonStateChanged();
}

void ToolbarButton::setCurrentImage (Drawable* const newImage)
{
    if (newImage != currentImage)
    {
        removeChildComponent (currentImage);
        currentImage = newImage;

        if (currentImage != nullptr)
        {
            enablementChanged();
            addAndMakeVisible (currentImage);
            updateDrawable();
        }
    }
}

void ToolbarButton::updateDrawable()
{
    if (currentImage != nullptr)
    {
        currentImage->setInterceptsMouseClicks (false, false);
        currentImage->setTransformToFit (getContentArea().toFloat(), RectanglePlacement::centred);
        currentImage->setAlpha (isEnabled() ? 1.0f : 0.5f);
    }
}

void ToolbarButton::resized()
{
    ToolbarItemComponent::resized();
    updateDrawable();
}

void ToolbarButton::enablementChanged()
{
    ToolbarItemComponent::enablementChanged();
    updateDrawable();
}

Drawable* ToolbarButton::getImageToUse() const
{
    if (getStyle() == Toolbar::textOnly)
        return nullptr;

    if (getToggleState() && toggledOnImage != nullptr)
        return toggledOnImage.get();

    return normalImage.get();
}

void ToolbarButton::buttonStateChanged()
{
    setCurrentImage (getImageToUse());
}

} // namespace juce
