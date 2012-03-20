/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

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

void ImageComponent::setImage (const Image& newImage, const RectanglePlacement& placementToUse)
{
    if (image != newImage || placement != placementToUse)
    {
        image = newImage;
        placement = placementToUse;
        repaint();
    }
}

void ImageComponent::setImagePlacement (const RectanglePlacement& newPlacement)
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

const RectanglePlacement ImageComponent::getImagePlacement() const
{
    return placement;
}

void ImageComponent::paint (Graphics& g)
{
    g.setOpacity (1.0f);
    g.drawImageWithin (image, 0, 0, getWidth(), getHeight(), placement, false);
}

const Identifier ImageComponent::Ids::tagType ("IMAGECOMPONENT");
const Identifier ImageComponent::Ids::image ("image");
const Identifier ImageComponent::Ids::placement ("placement");

void ImageComponent::refreshFromValueTree (const ValueTree& state, ComponentBuilder& builder)
{
    ComponentBuilder::refreshBasicComponentProperties (*this, state);

    Image newImage;
    const var imageIdentifier (state [Ids::image]);

    ComponentBuilder::ImageProvider* const imageProvider = builder.getImageProvider();
    jassert (imageProvider != nullptr || imageIdentifier.isVoid());

    if (imageProvider != nullptr)
        newImage = imageProvider->getImageForIdentifier (imageIdentifier);

    setImage (newImage, getPlacement (state));
}

RectanglePlacement ImageComponent::getPlacement (const ValueTree& state)
{
    return RectanglePlacement (static_cast <int> (state [Ids::placement]));
}
