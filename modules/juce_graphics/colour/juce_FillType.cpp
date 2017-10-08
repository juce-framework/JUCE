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

FillType::FillType() noexcept
    : colour (0xff000000)
{
}

FillType::FillType (Colour c) noexcept
    : colour (c)
{
}

FillType::FillType (const ColourGradient& gradient_)
    : colour (0xff000000), gradient (new ColourGradient (gradient_))
{
}

FillType::FillType (const Image& image_, const AffineTransform& transform_) noexcept
    : colour (0xff000000), image (image_), transform (transform_)
{
}

FillType::FillType (const FillType& other)
    : colour (other.colour),
      gradient (other.gradient.createCopy()),
      image (other.image),
      transform (other.transform)
{
}

FillType& FillType::operator= (const FillType& other)
{
    if (this != &other)
    {
        colour = other.colour;
        gradient = other.gradient.createCopy();
        image = other.image;
        transform = other.transform;
    }

    return *this;
}

FillType::FillType (FillType&& other) noexcept
    : colour (other.colour),
      gradient (other.gradient.release()),
      image (static_cast<Image&&> (other.image)),
      transform (other.transform)
{
}

FillType& FillType::operator= (FillType&& other) noexcept
{
    jassert (this != &other); // hopefully the compiler should make this situation impossible!

    colour = other.colour;
    gradient = other.gradient.release();
    image = static_cast<Image&&> (other.image);
    transform = other.transform;
    return *this;
}

FillType::~FillType() noexcept
{
}

bool FillType::operator== (const FillType& other) const
{
    return colour == other.colour && image == other.image
            && transform == other.transform
            && (gradient == other.gradient
                 || (gradient != nullptr && other.gradient != nullptr && *gradient == *other.gradient));
}

bool FillType::operator!= (const FillType& other) const
{
    return ! operator== (other);
}

void FillType::setColour (Colour newColour) noexcept
{
    gradient = nullptr;
    image = Image();
    colour = newColour;
}

void FillType::setGradient (const ColourGradient& newGradient)
{
    if (gradient != nullptr)
    {
        *gradient = newGradient;
    }
    else
    {
        image = Image();
        gradient = new ColourGradient (newGradient);
        colour = Colours::black;
    }
}

void FillType::setTiledImage (const Image& image_, const AffineTransform& transform_) noexcept
{
    gradient = nullptr;
    image = image_;
    transform = transform_;
    colour = Colours::black;
}

void FillType::setOpacity (const float newOpacity) noexcept
{
    colour = colour.withAlpha (newOpacity);
}

bool FillType::isInvisible() const noexcept
{
    return colour.isTransparent() || (gradient != nullptr && gradient->isInvisible());
}

FillType FillType::transformed (const AffineTransform& t) const
{
    FillType f (*this);
    f.transform = f.transform.followedBy (t);
    return f;
}

} // namespace juce
