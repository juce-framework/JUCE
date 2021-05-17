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

ToolbarItemPalette::ToolbarItemPalette (ToolbarItemFactory& tbf, Toolbar& bar)
    : factory (tbf), toolbar (bar)
{
    auto* itemHolder = new Component();
    viewport.setViewedComponent (itemHolder);

    Array<int> allIds;
    factory.getAllToolbarItemIds (allIds);

    for (auto& i : allIds)
        addComponent (i, -1);

    addAndMakeVisible (viewport);
}

ToolbarItemPalette::~ToolbarItemPalette()
{
}

//==============================================================================
void ToolbarItemPalette::addComponent (const int itemId, const int index)
{
    if (auto* tc = Toolbar::createItem (factory, itemId))
    {
        items.insert (index, tc);
        viewport.getViewedComponent()->addAndMakeVisible (tc, index);
        tc->setEditingMode (ToolbarItemComponent::editableOnPalette);
    }
    else
    {
        jassertfalse;
    }
}

void ToolbarItemPalette::replaceComponent (ToolbarItemComponent& comp)
{
    auto index = items.indexOf (&comp);
    jassert (index >= 0);
    items.removeObject (&comp, false);

    addComponent (comp.getItemId(), index);
    resized();
}

void ToolbarItemPalette::resized()
{
    viewport.setBoundsInset (BorderSize<int> (1));

    auto* itemHolder = viewport.getViewedComponent();

    const int indent = 8;
    const int preferredWidth = viewport.getWidth() - viewport.getScrollBarThickness() - indent;
    const int height = toolbar.getThickness();
    auto x = indent;
    auto y = indent;
    int maxX = 0;

    for (auto* tc : items)
    {
        tc->setStyle (toolbar.getStyle());

        int preferredSize = 1, minSize = 1, maxSize = 1;

        if (tc->getToolbarItemSizes (height, false, preferredSize, minSize, maxSize))
        {
            if (x + preferredSize > preferredWidth && x > indent)
            {
                x = indent;
                y += height;
            }

            tc->setBounds (x, y, preferredSize, height);

            x += preferredSize + 8;
            maxX = jmax (maxX, x);
        }
    }

    itemHolder->setSize (maxX, y + height + 8);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ToolbarItemPalette::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace juce
