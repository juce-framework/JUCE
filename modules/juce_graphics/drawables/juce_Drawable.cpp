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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

Drawable::Drawable (const Drawable& other)
    : transform (other.transform),
      clipPath (other.clipPath != nullptr ? other.clipPath->createCopy() : nullptr),
      name (other.name),
      visible (other.visible)
{
}

void Drawable::applyDrawableClipPath (Graphics& g) const
{
    if (auto* drawable = clipPath.get())
    {
        auto path = drawable->getOutlineAsPath();

        if (! path.isEmpty())
            g.getInternalContext().clipToPath (path, {});
    }
}

//==============================================================================
void Drawable::draw (Graphics& g, float opacity, const AffineTransform& drawTransform) const
{
    Graphics::ScopedSaveState ss (g);

    g.addTransform (getTransform().followedBy (drawTransform));

    applyDrawableClipPath (g);

    if (! g.isClipEmpty())
    {
        if (opacity < 1.0f)
        {
            g.beginTransparencyLayer (opacity);
            paint (g);
            g.endTransparencyLayer();
        }
        else
        {
            paint (g);
        }
    }
}

void Drawable::drawAt (Graphics& g, float x, float y, float opacity) const
{
    draw (g, opacity, AffineTransform::translation (x, y));
}

void Drawable::drawWithin (Graphics& g, Rectangle<float> destArea,
                           RectanglePlacement placement, float opacity) const
{
    draw (g, opacity, placement.getTransformToFit (getDrawableBounds(), destArea));
}

//==============================================================================
void Drawable::setClipPath (std::unique_ptr<Drawable> clipPathIn)
{
    clipPath = std::move (clipPathIn);
}

//==============================================================================
bool Drawable::replaceColour (Colour, Colour)
{
    return false;
}

void Drawable::setDrawableTransform (const AffineTransform& newTransform)
{
    transform = newTransform;
}

void Drawable::setDrawableTransformToFit (const Rectangle<float>& area, RectanglePlacement placement)
{
    if (! area.isEmpty())
        setDrawableTransform (placement.getTransformToFit (getDrawableBoundsUntransformed(), area));
}

AffineTransform Drawable::getDrawableTransform() const
{
    return transform;
}

AffineTransform Drawable::getTransform() const
{
    return transform;
}

//==============================================================================
std::unique_ptr<Drawable> Drawable::createFromImageData (const void* data, const size_t numBytes)
{
    auto image = ImageFileFormat::loadFrom (data, numBytes);

    if (image.isValid())
        return std::make_unique<DrawableImage> (image);

    if (const auto s = String::createStringFromData (data, (int) numBytes); s.contains ("<svg"))
        return Drawable::createFromSVGString (s);

    return {};
}

std::unique_ptr<Drawable> Drawable::createFromImageDataStream (InputStream& dataSource)
{
    MemoryOutputStream mo;
    mo << dataSource;

    return createFromImageData (mo.getData(), mo.getDataSize());
}

std::unique_ptr<Drawable> Drawable::createFromImageFile (const File& file)
{
    FileInputStream fin (file);

    if (fin.openedOk())
        return createFromImageDataStream (fin);

    return {};
}

Rectangle<float> Drawable::getDrawableBounds() const
{
    return getDrawableBoundsUntransformed().transformedBy (getDrawableTransform());
}

Rectangle<float> Drawable::getContentBounds() const
{
    return getContentBoundsUntransformed().transformedBy (getDrawableTransform());
}

Rectangle<float> Drawable::getContentBoundsUntransformed() const
{
    return getDrawableBoundsUntransformed();
}

} // namespace juce
