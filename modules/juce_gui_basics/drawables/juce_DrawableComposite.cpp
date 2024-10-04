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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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
