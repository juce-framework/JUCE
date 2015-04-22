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
    g.drawImageWithin (image, 0, 0, getWidth(), getHeight(), placement, false);
}
