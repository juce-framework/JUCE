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

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
FillType::FillType (FillType&& other) noexcept
    : colour (other.colour),
      gradient (other.gradient.release()),
      image (static_cast <Image&&> (other.image)),
      transform (other.transform)
{
}

FillType& FillType::operator= (FillType&& other) noexcept
{
    jassert (this != &other); // hopefully the compiler should make this situation impossible!

    colour = other.colour;
    gradient = other.gradient.release();
    image = static_cast <Image&&> (other.image);
    transform = other.transform;
    return *this;
}
#endif

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
    image = Image::null;
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
        image = Image::null;
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
