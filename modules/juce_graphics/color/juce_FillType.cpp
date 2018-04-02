/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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
    : color (0xff000000)
{
}

FillType::FillType (Color c) noexcept
    : color (c)
{
}

FillType::FillType (const ColorGradient& g)
    : color (0xff000000), gradient (new ColorGradient (g))
{
}

FillType::FillType (ColorGradient&& g)
    : color (0xff000000), gradient (new ColorGradient (static_cast<ColorGradient&&> (g)))
{
}

FillType::FillType (const Image& im, const AffineTransform& t) noexcept
    : color (0xff000000), image (im), transform (t)
{
}

FillType::FillType (const FillType& other)
    : color (other.color),
      gradient (createCopyIfNotNull (other.gradient.get())),
      image (other.image),
      transform (other.transform)
{
}

FillType& FillType::operator= (const FillType& other)
{
    if (this != &other)
    {
        color = other.color;
        gradient.reset (createCopyIfNotNull (other.gradient.get()));
        image = other.image;
        transform = other.transform;
    }

    return *this;
}

FillType::FillType (FillType&& other) noexcept
    : color (other.color),
      gradient (static_cast<ScopedPointer<ColorGradient>&&> (other.gradient)),
      image (static_cast<Image&&> (other.image)),
      transform (other.transform)
{
}

FillType& FillType::operator= (FillType&& other) noexcept
{
    jassert (this != &other); // hopefully the compiler should make this situation impossible!

    color = other.color;
    gradient = static_cast<ScopedPointer<ColorGradient>&&> (other.gradient);
    image = static_cast<Image&&> (other.image);
    transform = other.transform;
    return *this;
}

FillType::~FillType() noexcept
{
}

bool FillType::operator== (const FillType& other) const
{
    return color == other.color && image == other.image
            && transform == other.transform
            && (gradient == other.gradient
                 || (gradient != nullptr && other.gradient != nullptr && *gradient == *other.gradient));
}

bool FillType::operator!= (const FillType& other) const
{
    return ! operator== (other);
}

void FillType::setColor (Color newColor) noexcept
{
    gradient.reset();
    image = {};
    color = newColor;
}

void FillType::setGradient (const ColorGradient& newGradient)
{
    if (gradient != nullptr)
    {
        *gradient = newGradient;
    }
    else
    {
        image = {};
        gradient.reset (new ColorGradient (newGradient));
        color = Colors::black;
    }
}

void FillType::setTiledImage (const Image& newImage, const AffineTransform& newTransform) noexcept
{
    gradient.reset();
    image = newImage;
    transform = newTransform;
    color = Colors::black;
}

void FillType::setOpacity (const float newOpacity) noexcept
{
    color = color.withAlpha (newOpacity);
}

bool FillType::isInvisible() const noexcept
{
    return color.isTransparent() || (gradient != nullptr && gradient->isInvisible());
}

FillType FillType::transformed (const AffineTransform& t) const
{
    FillType f (*this);
    f.transform = f.transform.followedBy (t);
    return f;
}

} // namespace juce
