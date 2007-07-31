/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ToolbarItemPalette.h"
#include "juce_ToolbarItemFactory.h"


//==============================================================================
ToolbarItemPalette::ToolbarItemPalette (ToolbarItemFactory& factory_,
                                        Toolbar* const toolbar_)
    : factory (factory_),
      toolbar (toolbar_)
{
    Component* const itemHolder = new Component();

    Array <int> allIds;
    factory_.getAllToolbarItemIds (allIds);

    for (int i = 0; i < allIds.size(); ++i)
    {
        ToolbarItemComponent* const tc = Toolbar::createItem (factory_, allIds.getUnchecked (i));

        jassert (tc != 0);
        if (tc != 0)
        {
            itemHolder->addAndMakeVisible (tc);
            tc->setEditingMode (ToolbarItemComponent::editableOnPalette);
        }
    }

    viewport = new Viewport();
    viewport->setViewedComponent (itemHolder);
    addAndMakeVisible (viewport);
}

ToolbarItemPalette::~ToolbarItemPalette()
{
    viewport->getViewedComponent()->deleteAllChildren();
    deleteAllChildren();
}


//==============================================================================
void ToolbarItemPalette::resized()
{
    viewport->setBoundsInset (BorderSize (1));

    Component* const itemHolder = viewport->getViewedComponent();

    const int indent = 8;
    const int preferredWidth = viewport->getWidth() - viewport->getScrollBarThickness() - indent;
    const int height = toolbar->getThickness();
    int x = indent;
    int y = indent;
    int maxX = 0;

    for (int i = 0; i < itemHolder->getNumChildComponents(); ++i)
    {
        ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (itemHolder->getChildComponent (i));

        if (tc != 0)
        {
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
    }

    itemHolder->setSize (maxX, y + height + 8);
}

void ToolbarItemPalette::replaceComponent (ToolbarItemComponent* const comp)
{
    ToolbarItemComponent* const tc = Toolbar::createItem (factory, comp->getItemId());

    jassert (tc != 0);

    if (tc != 0)
    {
        tc->setBounds (comp->getBounds());
        tc->setStyle (toolbar->getStyle());
        tc->setEditingMode (comp->getEditingMode());
        viewport->getViewedComponent()->addAndMakeVisible (tc, getIndexOfChildComponent (comp));
    }
}


END_JUCE_NAMESPACE
