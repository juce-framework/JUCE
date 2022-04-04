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

ImageComponent::ImageComponent (const String& name)
    : Component (name),
      placement (RectanglePlacement::centred)
{
}

ImageComponent::~ImageComponent()
{
}

void ImageComponent::setImage (const Image& newImage)
{
    if (image != newImage)
    {
        image = newImage;
        repaint();
    }
}

void ImageComponent::setImage (const Image& newImage, RectanglePlacement placementToUse)
{
    if (image != newImage || placement != placementToUse)
    {
        image = newImage;
        placement = placementToUse;
        repaint();
    }
}

void ImageComponent::setImagePlacement (RectanglePlacement newPlacement)
{
    if (placement != newPlacement)
    {
        placement = newPlacement;
        repaint();
    }
}

const Image& ImageComponent::getImage() const
{
    return image;
}

RectanglePlacement ImageComponent::getImagePlacement() const
{
    return placement;
}

void ImageComponent::paint (Graphics& g)
{
    g.setOpacity (1.0f);
    g.drawImage (image, getLocalBounds().toFloat(), placement);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ImageComponent::createAccessibilityHandler()
{
    class ImageComponentAccessibilityHandler  : public AccessibilityHandler
    {
    public:
        explicit ImageComponentAccessibilityHandler (ImageComponent& imageComponentToWrap)
            : AccessibilityHandler (imageComponentToWrap, AccessibilityRole::image),
              imageComponent (imageComponentToWrap)
        {
        }

        String getHelp() const override   { return imageComponent.getTooltip(); }

    private:
        ImageComponent& imageComponent;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageComponentAccessibilityHandler)
    };

    return std::make_unique<ImageComponentAccessibilityHandler> (*this);
}

} // namespace juce
