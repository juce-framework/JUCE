/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

Drawable::Drawable()
{
    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);
    setAccessible (false);
}

Drawable::Drawable (const Drawable& other)
    : Component (other.getName())
{
    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);
    setAccessible (false);

    setComponentID (other.getComponentID());
    setTransform (other.getTransform());

    if (auto* clipPath = other.drawableClipPath.get())
        setClipPath (clipPath->createCopy());
}

Drawable::~Drawable()
{
}

void Drawable::applyDrawableClipPath (Graphics& g)
{
    if (drawableClipPath != nullptr)
    {
        auto clipPath = drawableClipPath->getOutlineAsPath();

        if (! clipPath.isEmpty())
            g.getInternalContext().clipToPath (clipPath, {});
    }
}

//==============================================================================
void Drawable::draw (Graphics& g, float opacity, const AffineTransform& transform) const
{
    const_cast<Drawable*> (this)->nonConstDraw (g, opacity, transform);
}

void Drawable::nonConstDraw (Graphics& g, float opacity, const AffineTransform& transform)
{
    Graphics::ScopedSaveState ss (g);

    g.addTransform (AffineTransform::translation ((float) -(originRelativeToComponent.x),
                                                  (float) -(originRelativeToComponent.y))
                        .followedBy (getTransform())
                        .followedBy (transform));

    applyDrawableClipPath (g);

    if (! g.isClipEmpty())
    {
        if (opacity < 1.0f)
        {
            g.beginTransparencyLayer (opacity);
            paintEntireComponent (g, true);
            g.endTransparencyLayer();
        }
        else
        {
            paintEntireComponent (g, true);
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
DrawableComposite* Drawable::getParent() const
{
    return dynamic_cast<DrawableComposite*> (getParentComponent());
}

void Drawable::setClipPath (std::unique_ptr<Drawable> clipPath)
{
    if (drawableClipPath != clipPath)
    {
        drawableClipPath = std::move (clipPath);
        repaint();
    }
}

void Drawable::transformContextToCorrectOrigin (Graphics& g)
{
    g.setOrigin (originRelativeToComponent);
}

void Drawable::parentHierarchyChanged()
{
    setBoundsToEnclose (getDrawableBounds());
}

void Drawable::setBoundsToEnclose (Rectangle<float> area)
{
    Point<int> parentOrigin;

    if (auto* parent = getParent())
        parentOrigin = parent->originRelativeToComponent;

    auto newBounds = area.getSmallestIntegerContainer() + parentOrigin;
    originRelativeToComponent = parentOrigin - newBounds.getPosition();
    setBounds (newBounds);
}

//==============================================================================
bool Drawable::replaceColour (Colour original, Colour replacement)
{
    bool changed = false;

    for (auto* c : getChildren())
        if (auto* d = dynamic_cast<Drawable*> (c))
            changed = d->replaceColour (original, replacement) || changed;

    return changed;
}

//==============================================================================
void Drawable::setOriginWithOriginalSize (Point<float> originWithinParent)
{
    setTransform (AffineTransform::translation (originWithinParent.x, originWithinParent.y));
}

void Drawable::setTransformToFit (const Rectangle<float>& area, RectanglePlacement placement)
{
    if (! area.isEmpty())
        setTransform (placement.getTransformToFit (getDrawableBounds(), area));
}

//==============================================================================
std::unique_ptr<Drawable> Drawable::createFromImageData (const void* data, const size_t numBytes)
{
    auto image = ImageFileFormat::loadFrom (data, numBytes);

    if (image.isValid())
        return std::make_unique<DrawableImage> (image);

    if (auto svg = parseXMLIfTagMatches (String::createStringFromData (data, (int) numBytes), "svg"))
        return Drawable::createFromSVG (*svg);

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

} // namespace juce
