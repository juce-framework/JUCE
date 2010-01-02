/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Toolbar.h"
#include "juce_ToolbarItemPalette.h"
#include "juce_ToolbarItemFactory.h"
#include "../menus/juce_PopupMenu.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../layout/juce_StretchableObjectResizer.h"


//==============================================================================
ToolbarItemFactory::ToolbarItemFactory()
{
}

ToolbarItemFactory::~ToolbarItemFactory()
{
}


//==============================================================================
class ItemDragAndDropOverlayComponent    : public Component
{
public:
    ItemDragAndDropOverlayComponent()
        : isDragging (false)
    {
        setAlwaysOnTop (true);
        setRepaintsOnMouseActivity (true);
        setMouseCursor (MouseCursor::DraggingHandCursor);
    }

    ~ItemDragAndDropOverlayComponent()
    {
    }

    void paint (Graphics& g)
    {
        ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (getParentComponent());

        if (isMouseOverOrDragging()
              && tc != 0
              && tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar)
        {
            g.setColour (findColour (Toolbar::editingModeOutlineColourId, true));
            g.drawRect (0, 0, getWidth(), getHeight(),
                        jmin (2, (getWidth() - 1) / 2, (getHeight() - 1) / 2));
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        isDragging = false;
        ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (getParentComponent());

        if (tc != 0)
        {
            tc->dragOffsetX = e.x;
            tc->dragOffsetY = e.y;
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (! (isDragging || e.mouseWasClicked()))
        {
            isDragging = true;
            DragAndDropContainer* const dnd = DragAndDropContainer::findParentDragContainerFor (this);

            if (dnd != 0)
            {
                dnd->startDragging (Toolbar::toolbarDragDescriptor, getParentComponent(), 0, true);

                ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (getParentComponent());

                if (tc != 0)
                {
                    tc->isBeingDragged = true;

                    if (tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar)
                        tc->setVisible (false);
                }
            }
        }
    }

    void mouseUp (const MouseEvent&)
    {
        isDragging = false;
        ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (getParentComponent());

        if (tc != 0)
        {
            tc->isBeingDragged = false;

            Toolbar* const tb = tc->getToolbar();

            if (tb != 0)
                tb->updateAllItemPositions (true);
            else if (tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar)
                delete tc;
        }
    }

    void parentSizeChanged()
    {
        setBounds (0, 0, getParentWidth(), getParentHeight());
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    bool isDragging;

    ItemDragAndDropOverlayComponent (const ItemDragAndDropOverlayComponent&);
    const ItemDragAndDropOverlayComponent& operator= (const ItemDragAndDropOverlayComponent&);
};


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
    jassert (overlayComp == 0 || overlayComp->isValidComponent());
    overlayComp = 0;
}

Toolbar* ToolbarItemComponent::getToolbar() const
{
    return dynamic_cast <Toolbar*> (getParentComponent());
}

bool ToolbarItemComponent::isToolbarVertical() const
{
    const Toolbar* const t = getToolbar();
    return t != 0 && t->isVertical();
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

void ToolbarItemComponent::paintButton (Graphics& g, bool isMouseOver, bool isMouseDown)
{
    if (isBeingUsedAsAButton)
        getLookAndFeel().paintToolbarButtonBackground (g, getWidth(), getHeight(),
                                                       isMouseOver, isMouseDown, *this);

    if (toolbarStyle != Toolbar::iconsOnly)
    {
        const int indent = contentArea.getX();
        int y = indent;
        int h = getHeight() - indent * 2;

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
        g.saveState();
        g.setOrigin (contentArea.getX(), contentArea.getY());
        g.reduceClipRegion (0, 0, contentArea.getWidth(), contentArea.getHeight());

        paintButtonArea (g, contentArea.getWidth(), contentArea.getHeight(), isMouseOver, isMouseDown);

        g.restoreState();
    }
}

void ToolbarItemComponent::resized()
{
    if (toolbarStyle != Toolbar::textOnly)
    {
        const int indent = jmin (proportionOfWidth (0.08f),
                                 proportionOfHeight (0.08f));

        contentArea = Rectangle (indent, indent,
                                 getWidth() - indent * 2,
                                 toolbarStyle == Toolbar::iconsWithText ? proportionOfHeight (0.55f)
                                                                        : (getHeight() - indent * 2));
    }
    else
    {
        contentArea = Rectangle();
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
            jassert (overlayComp == 0 || overlayComp->isValidComponent());
            overlayComp = 0;
        }
        else if (overlayComp == 0)
        {
            addAndMakeVisible (overlayComp = new ItemDragAndDropOverlayComponent());
            overlayComp->parentSizeChanged();
        }

        resized();
    }
}


END_JUCE_NAMESPACE
