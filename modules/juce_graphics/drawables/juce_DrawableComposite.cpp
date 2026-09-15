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

DrawableComposite::DrawableComposite (const DrawableComposite& other)
    : Drawable (other),
      contentArea (other.contentArea),
      bounds (other.bounds),
      blendMode (other.blendMode),
      luminanceMask (other.luminanceMask),
      opacity (other.opacity)
{
    for (const auto& child : other.children)
        children.push_back (child->createCopy());
}

std::unique_ptr<Drawable> DrawableComposite::createCopy() const
{
    return std::make_unique<DrawableComposite> (*this);
}

//==============================================================================
Rectangle<float> DrawableComposite::getDrawableBoundsUntransformed() const
{
    Rectangle<float> r;

    for (const auto& child : children)
        r = r.getUnion (child->getDrawableBounds());

    return r;
}

Rectangle<float> DrawableComposite::getContentBoundsUntransformed() const
{
    return contentArea;
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

        auto t = AffineTransform::fromTargetPoints (contentArea.getTopLeft(),
                                                    bounds.topLeft,
                                                    contentArea.getTopRight(),
                                                    bounds.topRight,
                                                    contentArea.getBottomLeft(),
                                                    bounds.bottomLeft);

        if (t.isSingularity())
            t = {};

        setDrawableTransform (t);
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

void DrawableComposite::addChild (std::unique_ptr<Drawable> newChild)
{
    if (newChild == nullptr)
    {
        jassertfalse;
        return;
    }

    children.emplace_back (std::move (newChild));
    callListeners();
}

bool DrawableComposite::replaceColour (Colour originalColour, Colour replacementColour)
{
    bool changed = false;

    for (const auto& child : children)
        changed |= child->replaceColour (originalColour, replacementColour);

    return changed;
}

bool DrawableComposite::hitTest (Point<float> p) const
{
    return std::any_of (children.begin(), children.end(), [&] (const auto& child)
    {
        return child->hitTest (p);
    });
}

//==============================================================================
Path DrawableComposite::getOutlineAsPath() const
{
    Path p;

    for (auto& c : children)
        p.addPath (c->getOutlineAsPath());

    p.applyTransform (getDrawableTransform());
    return p;
}

void DrawableComposite::paint (Graphics& g) const
{
    const Graphics::ScopedSaveState ss (g);

    if (! requiresBlendBuffer())
    {
        for (const auto& child : children)
            child->draw (g, opacity);

        return;
    }

    ScopedBlendContext blend { g,
                               ScopedBlendContext::Options { blendMode }
                                    .withLuminanceMaskSet (luminanceMask)
                                    .withScopeClip (getDrawableBounds().getSmallestIntegerContainer()) };

    if (blend.isClipEmpty())
        return;

    for (const auto& child : children)
        child->draw (blend.getContext(), opacity);
}

bool DrawableComposite::requiresBlendBuffer() const
{
    if (blendMode != BlendMode::sourceOver || luminanceMask)
        return true;

    return std::any_of (children.begin(), children.end(), [] (const auto& child)
    {
        return child->requiresBlendBuffer();
    });
}

} // namespace juce
