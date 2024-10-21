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

ToolbarItemFactory::ToolbarItemFactory() {}
ToolbarItemFactory::~ToolbarItemFactory() {}

//==============================================================================
ToolbarItemComponent::ToolbarItemComponent (const int itemId_,
                                            const String& labelText,
                                            const bool isBeingUsedAsAButton_)
    : Button (labelText),
      itemId (itemId_),
      mode (normalMode),
      toolbarStyle (Toolbar::iconsOnly),
      dragOffsetX (0),
      dragOffsetY (0),
      isActive (true),
      isBeingDragged (false),
      isBeingUsedAsAButton (isBeingUsedAsAButton_)
{
    // Your item ID can't be 0!
    jassert (itemId_ != 0);
}

ToolbarItemComponent::~ToolbarItemComponent()
{
    overlayComp.reset();
}

Toolbar* ToolbarItemComponent::getToolbar() const
{
    return dynamic_cast<Toolbar*> (getParentComponent());
}

bool ToolbarItemComponent::isToolbarVertical() const
{
    const Toolbar* const t = getToolbar();
    return t != nullptr && t->isVertical();
}

void ToolbarItemComponent::setStyle (const Toolbar::ToolbarItemStyle& newStyle)
{
    if (toolbarStyle != newStyle)
    {
        toolbarStyle = newStyle;
        repaint();
        resized();
    }
}

void ToolbarItemComponent::paintButton (Graphics& g, const bool over, const bool down)
{
    if (isBeingUsedAsAButton)
        getLookAndFeel().paintToolbarButtonBackground (g, getWidth(), getHeight(),
                                                       over, down, *this);

    if (toolbarStyle != Toolbar::iconsOnly)
    {
        auto indent = contentArea.getX();
        auto y = indent;
        auto h = getHeight() - indent * 2;

        if (toolbarStyle == Toolbar::iconsWithText)
        {
            y = contentArea.getBottom() + indent / 2;
            h -= contentArea.getHeight();
        }

        getLookAndFeel().paintToolbarButtonLabel (g, indent, y, getWidth() - indent * 2, h,
                                                  getButtonText(), *this);
    }

    if (! contentArea.isEmpty())
    {
        Graphics::ScopedSaveState ss (g);

        g.reduceClipRegion (contentArea);
        g.setOrigin (contentArea.getPosition());

        paintButtonArea (g, contentArea.getWidth(), contentArea.getHeight(), over, down);
    }
}

void ToolbarItemComponent::resized()
{
    if (toolbarStyle != Toolbar::textOnly)
    {
        const int indent = jmin (proportionOfWidth (0.08f),
                                 proportionOfHeight (0.08f));

        contentArea = Rectangle<int> (indent, indent,
                                      getWidth() - indent * 2,
                                      toolbarStyle == Toolbar::iconsWithText ? proportionOfHeight (0.55f)
                                                                             : (getHeight() - indent * 2));
    }
    else
    {
        contentArea = {};
    }

    contentAreaChanged (contentArea);
}

void ToolbarItemComponent::setEditingMode (const ToolbarEditingMode newMode)
{
    if (mode != newMode)
    {
        mode = newMode;
        repaint();

        if (mode == normalMode)
        {
            overlayComp.reset();
        }
        else if (overlayComp == nullptr)
        {
            overlayComp.reset (new detail::ToolbarItemDragAndDropOverlayComponent());
            addAndMakeVisible (overlayComp.get());
            overlayComp->parentSizeChanged();
        }

        resized();
    }
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ToolbarItemComponent::createAccessibilityHandler()
{
    const auto shouldItemBeAccessible = (itemId != ToolbarItemFactory::separatorBarId
                                      && itemId != ToolbarItemFactory::spacerId
                                      && itemId != ToolbarItemFactory::flexibleSpacerId);

    if (! shouldItemBeAccessible)
        return createIgnoredAccessibilityHandler (*this);

    return std::make_unique<detail::ButtonAccessibilityHandler> (*this, AccessibilityRole::button);
}

} // namespace juce
