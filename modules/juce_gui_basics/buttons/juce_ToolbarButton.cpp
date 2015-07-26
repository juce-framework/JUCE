/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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
        return toggledOnImage;

    return normalImage;
}

void ToolbarButton::buttonStateChanged()
{
    setCurrentImage (getImageToUse());
}
