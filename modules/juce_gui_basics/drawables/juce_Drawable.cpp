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

Drawable::Drawable()
{
    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);
}

Drawable::Drawable (const Drawable& other)
    : Component (other.getName())
{
    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);

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

void Drawable::setClipPath (Drawable* clipPath)
{
    if (drawableClipPath.get() != clipPath)
    {
        drawableClipPath.reset (clipPath);
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
Drawable* Drawable::createFromImageData (const void* data, const size_t numBytes)
{
    Drawable* result = nullptr;

    auto image = ImageFileFormat::loadFrom (data, numBytes);

    if (image.isValid())
    {
        auto* di = new DrawableImage();
        di->setImage (image);
        result = di;
    }
    else
    {
        auto asString = String::createStringFromData (data, (int) numBytes);

        XmlDocument doc (asString);
        ScopedPointer<XmlElement> outer (doc.getDocumentElement (true));

        if (outer != nullptr && outer->hasTagName ("svg"))
        {
            ScopedPointer<XmlElement> svg (doc.getDocumentElement());

            if (svg != nullptr)
                result = Drawable::createFromSVG (*svg);
        }
    }

    return result;
}

Drawable* Drawable::createFromImageDataStream (InputStream& dataSource)
{
    MemoryOutputStream mo;
    mo << dataSource;

    return createFromImageData (mo.getData(), mo.getDataSize());
}

Drawable* Drawable::createFromImageFile (const File& file)
{
    FileInputStream fin (file);

    return fin.openedOk() ? createFromImageDataStream (fin) : nullptr;
}

} // namespace juce
