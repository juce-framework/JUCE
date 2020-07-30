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

DrawableComposite::DrawableComposite()
    : bounds ({ 0.0f, 0.0f, 100.0f, 100.0f })
{
    setContentArea ({ 0.0f, 0.0f, 100.0f, 100.0f });
}

DrawableComposite::DrawableComposite (const DrawableComposite& other)
    : Drawable (other),
      bounds (other.bounds),
      contentArea (other.contentArea)
{
    for (auto* c : other.getChildren())
        if (auto* d = dynamic_cast<const Drawable*> (c))
            addAndMakeVisible (d->createCopy().release());
}

DrawableComposite::~DrawableComposite()
{
    deleteAllChildren();
}

std::unique_ptr<Drawable> DrawableComposite::createCopy() const
{
    return std::make_unique<DrawableComposite> (*this);
}

//==============================================================================
Rectangle<float> DrawableComposite::getDrawableBounds() const
{
    Rectangle<float> r;

    for (auto* c : getChildren())
        if (auto* d = dynamic_cast<const Drawable*> (c))
            r = r.getUnion (d->isTransformed() ? d->getDrawableBounds().transformedBy (d->getTransform())
                                               : d->getDrawableBounds());

    return r;
}

void DrawableComposite::setContentArea (Rectangle<float> newArea)
{
    contentArea = newArea;
}

void DrawableComposite::setBoundingBox (Rectangle<float> newBounds)
{
    setBoundingBox (Parallelogram<float> (newBounds));
}

void DrawableComposite::setBoundingBox (Parallelogram<float> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;

        auto t = AffineTransform::fromTargetPoints (contentArea.getTopLeft(),     bounds.topLeft,
                                                    contentArea.getTopRight(),    bounds.topRight,
                                                    contentArea.getBottomLeft(),  bounds.bottomLeft);

        if (t.isSingularity())
            t = {};

        setTransform (t);
    }
}

void DrawableComposite::resetBoundingBoxToContentArea()
{
    setBoundingBox (contentArea);
}

void DrawableComposite::resetContentAreaAndBoundingBoxToFitChildren()
{
    setContentArea (getDrawableBounds());
    resetBoundingBoxToContentArea();
}

void DrawableComposite::parentHierarchyChanged()
{
    if (auto* parent = getParent())
        originRelativeToComponent = parent->originRelativeToComponent - getPosition();
}

void DrawableComposite::childBoundsChanged (Component*)
{
    updateBoundsToFitChildren();
}

void DrawableComposite::childrenChanged()
{
    updateBoundsToFitChildren();
}

void DrawableComposite::updateBoundsToFitChildren()
{
    if (! updateBoundsReentrant)
    {
        const ScopedValueSetter<bool> setter (updateBoundsReentrant, true, false);

        Rectangle<int> childArea;

        for (auto* c : getChildren())
            childArea = childArea.getUnion (c->getBoundsInParent());

        auto delta = childArea.getPosition();
        childArea += getPosition();

        if (childArea != getBounds())
        {
            if (! delta.isOrigin())
            {
                originRelativeToComponent -= delta;

                for (auto* c : getChildren())
                    c->setBounds (c->getBounds() - delta);
            }

            setBounds (childArea);
        }
    }
}

//==============================================================================
Path DrawableComposite::getOutlineAsPath() const
{
    Path p;

    for (auto* c : getChildren())
        if (auto* d = dynamic_cast<Drawable*> (c))
            p.addPath (d->getOutlineAsPath());

    p.applyTransform (getTransform());
    return p;
}

} // namespace juce
