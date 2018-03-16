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

ToolbarButton::ToolbarButton (const int iid, const String& buttonText,
                              Drawable* const normalIm, Drawable* const toggledOnIm)
   : ToolbarItemComponent (iid, buttonText, true),
     normalImage (normalIm),
     toggledOnImage (toggledOnIm),
     currentImage (nullptr)
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
