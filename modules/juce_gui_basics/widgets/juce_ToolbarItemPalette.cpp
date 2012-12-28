/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

ToolbarItemPalette::ToolbarItemPalette (ToolbarItemFactory& factory_,
                                        Toolbar* const toolbar_)
    : factory (factory_),
      toolbar (toolbar_)
{
    Component* const itemHolder = new Component();
    viewport.setViewedComponent (itemHolder);

    Array <int> allIds;
    factory.getAllToolbarItemIds (allIds);

    for (int i = 0; i < allIds.size(); ++i)
        addComponent (allIds.getUnchecked (i), -1);

    addAndMakeVisible (&viewport);
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

void ToolbarItemPalette::replaceComponent (ToolbarItemComponent* const comp)
{
    const int index = items.indexOf (comp);
    jassert (index >= 0);
    items.removeObject (comp, false);

    addComponent (comp->getItemId(), index);
    resized();
}

void ToolbarItemPalette::resized()
{
    viewport.setBoundsInset (BorderSize<int> (1));

    Component* const itemHolder = viewport.getViewedComponent();

    const int indent = 8;
    const int preferredWidth = viewport.getWidth() - viewport.getScrollBarThickness() - indent;
    const int height = toolbar->getThickness();
    int x = indent;
    int y = indent;
    int maxX = 0;

    for (int i = 0; i < items.size(); ++i)
    {
        ToolbarItemComponent* const tc = items.getUnchecked(i);

        tc->setStyle (toolbar->getStyle());

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
