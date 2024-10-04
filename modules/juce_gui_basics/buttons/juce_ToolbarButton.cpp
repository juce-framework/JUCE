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
