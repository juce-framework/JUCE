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

ToolbarItemPalette::ToolbarItemPalette (ToolbarItemFactory& tbf, Toolbar& bar)
    : factory (tbf), toolbar (bar)
{
    Component* const itemHolder = new Component();
    viewport.setViewedComponent (itemHolder);

    Array<int> allIds;
    factory.getAllToolbarItemIds (allIds);

    for (int i = 0; i < allIds.size(); ++i)
        addComponent (allIds.getUnchecked (i), -1);

    addAndMakeVisible (viewport);
}

ToolbarItemPalette::~ToolbarItemPalette()
{
}

//==============================================================================
void ToolbarItemPalette::addComponent (const int itemId, const int index)
{
    if (ToolbarItemComponent* const tc = Toolbar::createItem (factory, itemId))
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
    const int index = items.indexOf (&comp);
    jassert (index >= 0);
    items.removeObject (&comp, false);

    addComponent (comp.getItemId(), index);
    resized();
}

void ToolbarItemPalette::resized()
{
    viewport.setBoundsInset (BorderSize<int> (1));

    Component* const itemHolder = viewport.getViewedComponent();

    const int indent = 8;
    const int preferredWidth = viewport.getWidth() - viewport.getScrollBarThickness() - indent;
    const int height = toolbar.getThickness();
    int x = indent;
    int y = indent;
    int maxX = 0;

    for (int i = 0; i < items.size(); ++i)
    {
        ToolbarItemComponent* const tc = items.getUnchecked(i);

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
