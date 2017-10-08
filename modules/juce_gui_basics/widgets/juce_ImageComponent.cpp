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

} // namespace juce
