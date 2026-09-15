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

DrawableComponent::DrawableComponent (Drawable& drawableIn, BoundsToEnclose boundsToEncloseIn)
    : drawable (drawableIn), boundsToEnclose (boundsToEncloseIn)
{
    drawable.addListener (this);
    resetComponentBoundsTo (boundsToEnclose == BoundsToEnclose::drawableBounds ? drawable.getDrawableBounds()
                                                                               : drawable.getContentBounds());

    setInterceptsMouseClicks (false, false);
    setPaintingIsUnclipped (true);
    setAccessible (false);
}

DrawableComponent::DrawableComponent (Drawable& drawableIn)
    : DrawableComponent (drawableIn, BoundsToEnclose::drawableBounds)
{
}

DrawableComponent::~DrawableComponent()
{
    drawable.removeListener (this);
}

void DrawableComponent::setTransformToFit (const Rectangle<float>& areaInParent, RectanglePlacement placement)
{
    if (areaInParent.isEmpty())
        return;

    const auto boundsToFit = boundsToEnclose == BoundsToEnclose::drawableBounds ? drawable.getDrawableBounds()
                                                                                : drawable.getContentBounds();
    setTransform (placement.getTransformToFit (boundsToFit, areaInParent));
}

Path DrawableComponent::getOutlineAsPath() const
{
    auto p = drawable.getOutlineAsPath();
    p.applyTransform (getTransform());
    return p;
}

void DrawableComponent::paint (Graphics& g)
{
    g.setOrigin (originRelativeToComponent);
    drawable.draw (g, 1.0f);
}

bool DrawableComponent::hitTest (int x, int y)
{
    if (Component::hitTest (x, y))
        return true;

    bool allowsClicksOnThisComponent, allowsClicksOnChildComponents;
    getInterceptsMouseClicks (allowsClicksOnThisComponent, allowsClicksOnChildComponents);

    if (! allowsClicksOnChildComponents)
        return false;

    const Point<int> p { x - originRelativeToComponent.x, y - originRelativeToComponent.y };
    return drawable.hitTest (p.toFloat());
}

class DrawableTextAccessibilityHandler final : public AccessibilityHandler
{
public:
    DrawableTextAccessibilityHandler (Component& c, const String& textToWrap)
        : AccessibilityHandler (c, AccessibilityRole::staticText),
          text (textToWrap)
    {
    }

    String getTitle() const override
    {
        return text;
    }

private:
    String text;
};

std::unique_ptr<AccessibilityHandler> DrawableComponent::createAccessibilityHandler()
{
    if (drawable.isImage())
        return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::image);

    if (auto t = drawable.getContainedText(); t.has_value())
        return std::make_unique<DrawableTextAccessibilityHandler> (*this, *t);

    return Component::createAccessibilityHandler();
}

void DrawableComponent::resetComponentBoundsTo (Rectangle<float> bounds)
{
    const auto drawableBounds = bounds.getSmallestIntegerContainer();
    originRelativeToComponent = -drawableBounds.getPosition();
    setBounds (drawableBounds);
}

void DrawableComponent::drawableBoundsChanged (Drawable* d)
{
    if (boundsToEnclose == BoundsToEnclose::drawableBounds)
        resetComponentBoundsTo (d->getDrawableBounds());
}

} // namespace juce
