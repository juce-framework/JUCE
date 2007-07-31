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

#include "juce_TreeView.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../../../juce_core/containers/juce_BitArray.h"
#include "../mouse/juce_DragAndDropContainer.h"
#include "../../graphics/imaging/juce_Image.h"


//==============================================================================
class TreeViewContentComponent  : public Component
{
public:
    TreeViewContentComponent (TreeView* const owner_)
        : owner (owner_),
          isDragging (false)
    {
    }

    ~TreeViewContentComponent()
    {
        deleteAllChildren();
    }

    void mouseDown (const MouseEvent& e)
    {
        isDragging = false;
        needSelectionOnMouseUp = false;

        Rectangle pos;
        TreeViewItem* const item = findItemAt (e.y, pos);

        if (item != 0 && e.x >= pos.getX())
        {
            if (! owner->isMultiSelectEnabled())
                item->setSelected (true, true);
            else if (item->isSelected())
                needSelectionOnMouseUp = ! e.mods.isPopupMenu();
            else
                selectBasedOnModifiers (item, e.mods);

            MouseEvent e2 (e);
            e2.x -= pos.getX();
            e2.y -= pos.getY();
            item->itemClicked (e2);
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        Rectangle pos;
        TreeViewItem* const item = findItemAt (e.y, pos);

        if (item != 0 && e.mouseWasClicked())
        {
            if (needSelectionOnMouseUp)
            {
                selectBasedOnModifiers (item, e.mods);
            }
            else if (e.mouseWasClicked())
            {
                if (e.x >= pos.getX() - owner->getIndentSize()
                     && e.x < pos.getX())
                {
                    item->setOpen (! item->isOpen());
                }
            }
        }
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        if (e.getNumberOfClicks() != 3)  // ignore triple clicks
        {
            Rectangle pos;
            TreeViewItem* const item = findItemAt (e.y, pos);

            if (item != 0 && e.x >= pos.getX())
            {
                MouseEvent e2 (e);
                e2.x -= pos.getX();
                e2.y -= pos.getY();
                item->itemDoubleClicked (e2);
            }
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (isEnabled() && ! (e.mouseWasClicked() || isDragging))
        {
            isDragging = true;

            Rectangle pos;
            TreeViewItem* const item = findItemAt (e.getMouseDownY(), pos);

            if (item != 0 && e.getMouseDownX() >= pos.getX())
            {
                const String dragDescription (item->getDragSourceDescription());

                if (dragDescription.isNotEmpty())
                {
                    DragAndDropContainer* const dragContainer
                        = DragAndDropContainer::findParentDragContainerFor (this);

                    if (dragContainer != 0)
                    {
                        pos.setSize (pos.getWidth(), item->itemHeight);
                        Image* dragImage = Component::createComponentSnapshot (pos, true);
                        dragImage->multiplyAllAlphas (0.6f);

                        dragContainer->startDragging (dragDescription, owner, dragImage);
                    }
                    else
                    {
                        // to be able to do a drag-and-drop operation, the treeview needs to
                        // be inside a component which is also a DragAndDropContainer.
                        jassertfalse
                    }
                }
            }
        }
    }

    void paint (Graphics& g);
    TreeViewItem* findItemAt (int y, Rectangle& itemPosition) const;

    void updateComponents()
    {
        int xAdjust = 0, yAdjust = 0;

        if ((! owner->rootItemVisible) && owner->rootItem != 0)
        {
            yAdjust = owner->rootItem->itemHeight;
            xAdjust = owner->getIndentSize();
        }

        const int visibleTop = -getY();
        const int visibleBottom = visibleTop + getParentHeight();

        BitArray itemsToKeep;
        TreeViewItem* item = owner->rootItem;
        int y = -yAdjust;

        while (item != 0 && y < visibleBottom)
        {
            y += item->itemHeight;

            if (y >= visibleTop)
            {
                const int index = rowComponentIds.indexOf (item->uid);

                if (index < 0)
                {
                    Component* const comp = item->createItemComponent();

                    if (comp != 0)
                    {
                        addAndMakeVisible (comp);
                        itemsToKeep.setBit (rowComponentItems.size());
                        rowComponentItems.add (item);
                        rowComponentIds.add (item->uid);
                        rowComponents.add (comp);
                    }
                }
                else
                {
                    itemsToKeep.setBit (index);
                }
            }

            item = item->getNextVisibleItem (true);
        }

        for (int i = rowComponentItems.size(); --i >= 0;)
        {
            Component* const comp = (Component*) (rowComponents.getUnchecked(i));

            bool keep = false;

            if ((itemsToKeep[i] || (comp == Component::getComponentUnderMouse() && comp->isMouseButtonDown()))
                && isParentOf (comp))
            {
                if (itemsToKeep[i])
                {
                    const TreeViewItem* const item = (TreeViewItem*) rowComponentItems.getUnchecked(i);

                    Rectangle pos (item->getItemPosition (false));
                    pos.translate (-xAdjust, -yAdjust);
                    pos.setSize (pos.getWidth() + xAdjust, item->itemHeight);

                    if (pos.getBottom() >= visibleTop && pos.getY() < visibleBottom)
                    {
                        keep = true;
                        comp->setBounds (pos);
                    }
                }
                else
                {
                    comp->setSize (0, 0);
                }
            }

            if (! keep)
            {
                delete comp;
                rowComponents.remove (i);
                rowComponentIds.remove (i);
                rowComponentItems.remove (i);
            }
        }
    }

    void resized()
    {
        owner->itemsChanged();
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    TreeView* const owner;

    VoidArray rowComponentItems;
    Array <int> rowComponentIds;
    VoidArray rowComponents;
    bool isDragging, needSelectionOnMouseUp;

    TreeViewContentComponent (const TreeViewContentComponent&);
    const TreeViewContentComponent& operator= (const TreeViewContentComponent&);

    static void selectBasedOnModifiers (TreeViewItem* const item, const ModifierKeys& modifiers)
    {
        const bool shft = modifiers.isShiftDown();
        const bool cmd  = modifiers.isCommandDown();

        item->setSelected (shft || (! cmd) || (cmd && ! item->isSelected()),
                           ! (shft || cmd));
    }
};

//==============================================================================
class TreeViewport  : public Viewport
{
public:
    TreeViewport() throw()      {}
    ~TreeViewport() throw()     {}

    void updateComponents()
    {
        if (getViewedComponent() != 0)
            ((TreeViewContentComponent*) getViewedComponent())->updateComponents();

        repaint();
    }

    void visibleAreaChanged (int, int, int, int)
    {
        updateComponents();
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    TreeViewport (const TreeViewport&);
    const TreeViewport& operator= (const TreeViewport&);
};


//==============================================================================
TreeView::TreeView (const String& componentName)
    : Component (componentName),
      rootItem (0),
      indentSize (24),
      defaultOpenness (false),
      needsRecalculating (true),
      rootItemVisible (true),
      multiSelectEnabled (false)
{
    addAndMakeVisible (viewport = new TreeViewport());
    viewport->setViewedComponent (new TreeViewContentComponent (this));
    viewport->setWantsKeyboardFocus (false);
    setWantsKeyboardFocus (true);
}

TreeView::~TreeView()
{
    if (rootItem != 0)
        rootItem->setOwnerView (0);

    deleteAllChildren();
}

void TreeView::setRootItem (TreeViewItem* const newRootItem)
{
    if (rootItem != newRootItem)
    {
        if (newRootItem != 0)
        {
            jassert (newRootItem->ownerView == 0); // can't use a tree item in more than one tree at once..

            if (newRootItem->ownerView != 0)
                newRootItem->ownerView->setRootItem (0);
        }

        if (rootItem != 0)
            rootItem->setOwnerView (0);

        rootItem = newRootItem;

        if (newRootItem != 0)
            newRootItem->setOwnerView (this);

        needsRecalculating = true;
        handleAsyncUpdate();

        if (rootItem != 0 && (defaultOpenness || ! rootItemVisible))
        {
            rootItem->setOpen (false); // force a re-open
            rootItem->setOpen (true);
        }
    }
}

void TreeView::setRootItemVisible (const bool shouldBeVisible)
{
    rootItemVisible = shouldBeVisible;

    if (rootItem != 0 && (defaultOpenness || ! rootItemVisible))
    {
        rootItem->setOpen (false); // force a re-open
        rootItem->setOpen (true);
    }

    itemsChanged();
}

void TreeView::colourChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
    repaint();
}

void TreeView::setIndentSize (const int newIndentSize)
{
    if (indentSize != newIndentSize)
    {
        indentSize = newIndentSize;
        resized();
    }
}

void TreeView::setDefaultOpenness (const bool isOpenByDefault)
{
    if (defaultOpenness != isOpenByDefault)
    {
        defaultOpenness = isOpenByDefault;
        itemsChanged();
    }
}

void TreeView::setMultiSelectEnabled (const bool canMultiSelect)
{
    multiSelectEnabled = canMultiSelect;
}

//==============================================================================
void TreeView::clearSelectedItems()
{
    if (rootItem != 0)
        rootItem->deselectAllRecursively();
}

int TreeView::getNumSelectedItems() const throw()
{
    return (rootItem != 0) ? rootItem->countSelectedItemsRecursively() : 0;
}

TreeViewItem* TreeView::getSelectedItem (const int index) const throw()
{
    return (rootItem != 0) ? rootItem->getSelectedItemWithIndex (index) : 0;
}

int TreeView::getNumRowsInTree() const
{
    if (rootItem != 0)
        return rootItem->getNumRows() - (rootItemVisible ? 0 : 1);

    return 0;
}

TreeViewItem* TreeView::getItemOnRow (int index) const
{
    if (! rootItemVisible)
        ++index;

    if (rootItem != 0 && index >= 0)
        return rootItem->getItemOnRow (index);

    return 0;
}

//==============================================================================
XmlElement* TreeView::getOpennessState (const bool alsoIncludeScrollPosition) const
{
    XmlElement* e = 0;

    if (rootItem != 0)
    {
        e = rootItem->createXmlOpenness();

        if (e != 0 && alsoIncludeScrollPosition)
            e->setAttribute (T("scrollPos"), viewport->getViewPositionY());
    }

    return e;
}

void TreeView::restoreOpennessState (const XmlElement& newState)
{
    if (rootItem != 0)
    {
        rootItem->restoreFromXml (newState);

        if (newState.hasAttribute (T("scrollPos")))
            viewport->setViewPosition (viewport->getViewPositionX(),
                                       newState.getIntAttribute (T("scrollPos")));
    }
}

//==============================================================================
void TreeView::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));
}

void TreeView::resized()
{
    viewport->setBounds (0, 0, getWidth(), getHeight());
    itemsChanged();
}

void TreeView::moveSelectedRow (int delta)
{
    int rowSelected = 0;

    TreeViewItem* const firstSelected = getSelectedItem (0);
    if (firstSelected != 0)
        rowSelected = firstSelected->getRowNumberInTree();

    rowSelected = jlimit (0, getNumRowsInTree() - 1, rowSelected + delta);

    TreeViewItem* item = getItemOnRow (rowSelected);

    if (item != 0)
    {
        item->setSelected (true, true);

        scrollToKeepItemVisible (item);
    }
}

void TreeView::scrollToKeepItemVisible (TreeViewItem* item)
{
    if (item != 0 && item->ownerView == this)
    {
        handleAsyncUpdate();

        item = item->getDeepestOpenParentItem();

        int y = item->y;
        if (! rootItemVisible)
            y -= rootItem->itemHeight;

        int viewTop = viewport->getViewPositionY();

        if (y < viewTop)
        {
            viewport->setViewPosition (viewport->getViewPositionX(), y);
        }
        else if (y + item->itemHeight > viewTop + viewport->getViewHeight())
        {
            viewport->setViewPosition (viewport->getViewPositionX(),
                                       (y + item->itemHeight) - viewport->getViewHeight());
        }
    }
}

bool TreeView::keyPressed (const KeyPress& key)
{
    if (key.isKeyCode (KeyPress::upKey))
    {
        moveSelectedRow (-1);
    }
    else if (key.isKeyCode (KeyPress::downKey))
    {
        moveSelectedRow (1);
    }
    else if (key.isKeyCode (KeyPress::pageDownKey) || key.isKeyCode (KeyPress::pageUpKey))
    {
        if (rootItem != 0)
        {
            int rowsOnScreen = getHeight() / jmax (1, rootItem->itemHeight);

            if (key.isKeyCode (KeyPress::pageUpKey))
                rowsOnScreen = -rowsOnScreen;

            moveSelectedRow (rowsOnScreen);
        }
    }
    else if (key.isKeyCode (KeyPress::homeKey))
    {
        moveSelectedRow (-0x3fffffff);
    }
    else if (key.isKeyCode (KeyPress::endKey))
    {
        moveSelectedRow (0x3fffffff);
    }
    else if (key.isKeyCode (KeyPress::returnKey))
    {
        TreeViewItem* const firstSelected = getSelectedItem (0);
        if (firstSelected != 0)
            firstSelected->setOpen (! firstSelected->isOpen());
    }
    else if (key.isKeyCode (KeyPress::leftKey))
    {
        TreeViewItem* const firstSelected = getSelectedItem (0);

        if (firstSelected != 0)
        {
            if (firstSelected->isOpen())
            {
                firstSelected->setOpen (false);
            }
            else
            {
                TreeViewItem* parent = firstSelected->parentItem;

                if ((! rootItemVisible) && parent == rootItem)
                    parent = 0;

                if (parent != 0)
                {
                    parent->setSelected (true, true);
                    scrollToKeepItemVisible (parent);
                }
            }
        }
    }
    else if (key.isKeyCode (KeyPress::rightKey))
    {
        TreeViewItem* const firstSelected = getSelectedItem (0);

        if (firstSelected != 0)
        {
            if (firstSelected->isOpen() || ! firstSelected->mightContainSubItems())
                moveSelectedRow (1);
            else
                firstSelected->setOpen (true);
        }
    }
    else
    {
        return false;
    }

    return true;
}

void TreeView::itemsChanged() throw()
{
    needsRecalculating = true;
    triggerAsyncUpdate();
    repaint();
}

void TreeView::handleAsyncUpdate()
{
    if (needsRecalculating)
    {
        needsRecalculating = false;

        const ScopedLock sl (nodeAlterationLock);

        if (rootItem != 0)
            rootItem->updatePositions (0);

        ((TreeViewport*) viewport)->updateComponents();

        if (rootItem != 0)
        {
            viewport->getViewedComponent()
                ->setSize (jmax (viewport->getMaximumVisibleWidth(), rootItem->totalWidth),
                           rootItem->totalHeight - (rootItemVisible ? 0 : rootItem->itemHeight));
        }
        else
        {
            viewport->getViewedComponent()->setSize (0, 0);
        }
    }
}


//==============================================================================
void TreeViewContentComponent::paint (Graphics& g)
{
    if (owner->rootItem != 0)
    {
        owner->handleAsyncUpdate();

        int w = getWidth();

        if (! owner->rootItemVisible)
        {
            const int indentWidth = owner->getIndentSize();

            g.setOrigin (-indentWidth, -owner->rootItem->itemHeight);
            w += indentWidth;
        }

        owner->rootItem->paintRecursively (g, w);
    }
}

TreeViewItem* TreeViewContentComponent::findItemAt (int y, Rectangle& itemPosition) const
{
    if (owner->rootItem != 0)
    {
        owner->handleAsyncUpdate();

        if (! owner->rootItemVisible)
            y += owner->rootItem->itemHeight;

        TreeViewItem* const ti = owner->rootItem->findItemRecursively (y);

        if (ti != 0)
        {
            itemPosition = ti->getItemPosition (false);

            if (! owner->rootItemVisible)
                itemPosition.translate (-owner->getIndentSize(),
                                        -owner->rootItem->itemHeight);
        }

        return ti;
    }

    return 0;
}

//==============================================================================
#define opennessDefault 0
#define opennessClosed  1
#define opennessOpen    2

TreeViewItem::TreeViewItem()
    : ownerView (0),
      parentItem (0),
      subItems (8),
      y (0),
      itemHeight (0),
      totalHeight (0),
      selected (false),
      redrawNeeded (true),
      drawLinesInside (true),
      openness (opennessDefault)
{
    static int nextUID = 0;
    uid = nextUID++;
}

TreeViewItem::~TreeViewItem()
{
}

const String TreeViewItem::getUniqueName() const
{
    return String::empty;
}

void TreeViewItem::itemOpennessChanged (bool)
{
}

int TreeViewItem::getNumSubItems() const throw()
{
    return subItems.size();
}

TreeViewItem* TreeViewItem::getSubItem (const int index) const throw()
{
    return subItems [index];
}

void TreeViewItem::clearSubItems()
{
    if (subItems.size() > 0)
    {
        if (ownerView != 0)
        {
            const ScopedLock sl (ownerView->nodeAlterationLock);
            subItems.clear();
            treeHasChanged();
        }
        else
        {
            subItems.clear();
        }
    }
}

void TreeViewItem::addSubItem (TreeViewItem* const newItem, const int insertPosition)
{
    if (newItem != 0)
    {
        newItem->parentItem = this;
        newItem->setOwnerView (ownerView);
        newItem->y = 0;
        newItem->itemHeight = newItem->getItemHeight();
        newItem->totalHeight = 0;
        newItem->itemWidth = newItem->getItemWidth();
        newItem->totalWidth = 0;

        if (ownerView != 0)
        {
            const ScopedLock sl (ownerView->nodeAlterationLock);
            subItems.insert (insertPosition, newItem);
            treeHasChanged();

            if (newItem->isOpen())
                newItem->itemOpennessChanged (true);
        }
        else
        {
            subItems.insert (insertPosition, newItem);

            if (newItem->isOpen())
                newItem->itemOpennessChanged (true);
        }
    }
}

void TreeViewItem::removeSubItem (const int index, const bool deleteItem)
{
    if (ownerView != 0)
        ownerView->nodeAlterationLock.enter();

    if (index >= 0 && index < subItems.size())
    {
        subItems.remove (index, deleteItem);
        treeHasChanged();
    }

    if (ownerView != 0)
        ownerView->nodeAlterationLock.exit();
}

bool TreeViewItem::isOpen() const throw()
{
    if (openness == opennessDefault)
        return ownerView != 0 && ownerView->defaultOpenness;
    else
        return openness == opennessOpen;
}

void TreeViewItem::setOpen (const bool shouldBeOpen)
{
    if (isOpen() != shouldBeOpen)
    {
        openness = shouldBeOpen ? opennessOpen
                                : opennessClosed;

        treeHasChanged();

        itemOpennessChanged (isOpen());
    }
}

bool TreeViewItem::isSelected() const throw()
{
    return selected;
}

void TreeViewItem::deselectAllRecursively()
{
    setSelected (false, false);

    for (int i = 0; i < subItems.size(); ++i)
        subItems.getUnchecked(i)->deselectAllRecursively();
}

void TreeViewItem::setSelected (const bool shouldBeSelected,
                                const bool deselectOtherItemsFirst)
{
    if (deselectOtherItemsFirst)
        getTopLevelItem()->deselectAllRecursively();

    if (shouldBeSelected != selected)
    {
        selected = shouldBeSelected;
        if (ownerView != 0)
            ownerView->repaint();

        itemSelectionChanged (shouldBeSelected);
    }
}

void TreeViewItem::paintItem (Graphics&, int, int)
{
}

void TreeViewItem::itemClicked (const MouseEvent&)
{
}

void TreeViewItem::itemDoubleClicked (const MouseEvent&)
{
    if (mightContainSubItems())
        setOpen (! isOpen());
}

void TreeViewItem::itemSelectionChanged (bool)
{
}

const String TreeViewItem::getDragSourceDescription()
{
    return String::empty;
}

const Rectangle TreeViewItem::getItemPosition (const bool relativeToTreeViewTopLeft) const throw()
{
    const int indentX = getIndentX();

    int width = itemWidth;

    if (ownerView != 0 && width < 0)
        width = ownerView->viewport->getViewWidth() - indentX;

    Rectangle r (indentX, y, jmax (0, width), totalHeight);

    if (relativeToTreeViewTopLeft)
        r.setPosition (r.getX() - ownerView->viewport->getViewPositionX(),
                       r.getY() - ownerView->viewport->getViewPositionY());

    return r;
}

void TreeViewItem::treeHasChanged() const throw()
{
    if (ownerView != 0)
        ownerView->itemsChanged();
}

void TreeViewItem::updatePositions (int newY)
{
    y = newY;
    itemHeight = getItemHeight();
    totalHeight = itemHeight;
    itemWidth = getItemWidth();
    totalWidth = jmax (itemWidth, 0);

    if (isOpen())
    {
        const int ourIndent = getIndentX();
        newY += totalHeight;

        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const ti = subItems.getUnchecked(i);

            ti->updatePositions (newY);
            newY += ti->totalHeight;
            totalHeight += ti->totalHeight;
            totalWidth = jmax (totalWidth, ti->totalWidth + ourIndent);
        }
    }
}

TreeViewItem* TreeViewItem::getDeepestOpenParentItem() throw()
{
    TreeViewItem* result = this;
    TreeViewItem* item = this;

    while (item->parentItem != 0)
    {
        item = item->parentItem;

        if (! item->isOpen())
            result = item;
    }

    return result;
}

void TreeViewItem::setOwnerView (TreeView* const newOwner) throw()
{
    ownerView = newOwner;

    for (int i = subItems.size(); --i >= 0;)
        subItems.getUnchecked(i)->setOwnerView (newOwner);
}

int TreeViewItem::getIndentX() const throw()
{
    const int indentWidth = ownerView->getIndentSize();
    int x = indentWidth;

    TreeViewItem* p = parentItem;

    while (p != 0)
    {
        x += indentWidth;
        p = p->parentItem;
    }

    return x;
}

void TreeViewItem::paintRecursively (Graphics& g, int width)
{
    jassert (ownerView != 0);
    if (ownerView == 0)
        return;

    const int indent = getIndentX();
    const int itemW = itemWidth < 0 ? width - indent : itemWidth;

    g.setColour (ownerView->findColour (TreeView::linesColourId));

    const float halfH = itemHeight * 0.5f;
    int depth = 0;
    TreeViewItem* p = parentItem;

    while (p != 0)
    {
        ++depth;
        p = p->parentItem;
    }

    const int indentWidth = ownerView->getIndentSize();
    float x = (depth + 0.5f) * indentWidth;

    if (x > 0)
    {
        if (depth >= 0)
        {
            if (parentItem != 0 && parentItem->drawLinesInside)
                g.drawLine (x, 0, x, isLastOfSiblings() ? halfH : (float) itemHeight);

            if (parentItem == 0 || parentItem->drawLinesInside)
                g.drawLine (x, halfH, x + indentWidth / 2, halfH);
        }

        p = parentItem;
        int d = depth;

        while (p != 0 && --d >= 0)
        {
            x -= (float) indentWidth;

            if ((p->parentItem == 0 || p->parentItem->drawLinesInside)
                 && ! p->isLastOfSiblings())
            {
                g.drawLine (x, 0, x, (float) itemHeight);
            }

            p = p->parentItem;
        }

        if (mightContainSubItems())
        {
            ownerView->getLookAndFeel()
                .drawTreeviewPlusMinusBox (g,
                                           depth * indentWidth, 0,
                                           indentWidth, itemHeight,
                                           ! isOpen());
        }
    }

    {
        g.saveState();
        g.setOrigin (indent, 0);

        if (g.reduceClipRegion (0, 0, itemW, itemHeight))
            paintItem (g, itemW, itemHeight);

        g.restoreState();
    }

    if (isOpen())
    {
        const Rectangle clip (g.getClipBounds());

        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const ti = subItems.getUnchecked(i);

            const int relY = ti->y - y;

            if (relY >= clip.getBottom())
                break;

            if (relY + ti->totalHeight >= clip.getY())
            {
                g.saveState();
                g.setOrigin (0, relY);

                if (g.reduceClipRegion (0, 0, width, ti->totalHeight))
                    ti->paintRecursively (g, width);

                g.restoreState();
            }
        }
    }
}

bool TreeViewItem::isLastOfSiblings() const throw()
{
    return parentItem == 0
        || parentItem->subItems.getLast() == this;
}

TreeViewItem* TreeViewItem::getTopLevelItem() throw()
{
    return (parentItem == 0) ? this
                             : parentItem->getTopLevelItem();
}

int TreeViewItem::getNumRows() const throw()
{
    int num = 1;

    if (isOpen())
    {
        for (int i = subItems.size(); --i >= 0;)
            num += subItems.getUnchecked(i)->getNumRows();
    }

    return num;
}

TreeViewItem* TreeViewItem::getItemOnRow (int index) throw()
{
    if (index == 0)
        return this;

    if (index > 0 && isOpen())
    {
        --index;

        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const item = subItems.getUnchecked(i);

            if (index == 0)
                return item;

            const int numRows = item->getNumRows();

            if (numRows > index)
                return item->getItemOnRow (index);

            index -= numRows;
        }
    }

    return 0;
}

TreeViewItem* TreeViewItem::findItemRecursively (int y) throw()
{
    if (y >= 0 && y < totalHeight)
    {
        const int h = itemHeight;

        if (y < h)
            return this;

        if (isOpen())
        {
            y -= h;

            for (int i = 0; i < subItems.size(); ++i)
            {
                TreeViewItem* const ti = subItems.getUnchecked(i);

                if (ti->totalHeight >= y)
                    return ti->findItemRecursively (y);

                y -= ti->totalHeight;
            }
        }
    }

    return 0;
}

int TreeViewItem::countSelectedItemsRecursively() const throw()
{
    int total = 0;

    if (isSelected())
        ++total;

    for (int i = subItems.size(); --i >= 0;)
        total += subItems.getUnchecked(i)->countSelectedItemsRecursively();

    return total;
}

TreeViewItem* TreeViewItem::getSelectedItemWithIndex (int index) throw()
{
    if (isSelected())
    {
        if (index == 0)
            return this;

        --index;
    }

    if (index >= 0)
    {
        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const item = subItems.getUnchecked(i);

            TreeViewItem* const found = item->getSelectedItemWithIndex (index);

            if (found != 0)
                return found;

            index -= item->countSelectedItemsRecursively();
        }
    }

    return 0;
}

int TreeViewItem::getRowNumberInTree() const throw()
{
    if (parentItem != 0 && ownerView != 0)
    {
        int n = 1 + parentItem->getRowNumberInTree();

        int ourIndex = parentItem->subItems.indexOf (this);
        jassert (ourIndex >= 0);

        while (--ourIndex >= 0)
            n += parentItem->subItems [ourIndex]->getNumRows();

        if (parentItem->parentItem == 0
             && ! ownerView->rootItemVisible)
            --n;

        return n;
    }
    else
    {
        return 0;
    }
}

void TreeViewItem::setLinesDrawnForSubItems (const bool drawLines) throw()
{
    drawLinesInside = drawLines;
}

TreeViewItem* TreeViewItem::getNextVisibleItem (const bool recurse) const throw()
{
    if (recurse && isOpen() && subItems.size() > 0)
        return subItems [0];

    if (parentItem != 0)
    {
        const int nextIndex = parentItem->subItems.indexOf (this) + 1;

        if (nextIndex >= parentItem->subItems.size())
            return parentItem->getNextVisibleItem (false);

        return parentItem->subItems [nextIndex];
    }

    return 0;
}

void TreeViewItem::restoreFromXml (const XmlElement& e)
{
    if (e.hasTagName (T("CLOSED")))
    {
        setOpen (false);
    }
    else if (e.hasTagName (T("OPEN")))
    {
        setOpen (true);

        forEachXmlChildElement (e, n)
        {
            const String id (n->getStringAttribute (T("id")));

            for (int i = 0; i < subItems.size(); ++i)
            {
                TreeViewItem* const ti = subItems.getUnchecked(i);

                if (ti->getUniqueName() == id)
                {
                    ti->restoreFromXml (*n);
                    break;
                }
            }
        }
    }
}

XmlElement* TreeViewItem::createXmlOpenness() const
{
    if (openness != opennessDefault)
    {
        const String name (getUniqueName());

        if (name.isNotEmpty())
        {
            XmlElement* e;

            if (isOpen())
            {
                e = new XmlElement (T("OPEN"));

                for (int i = 0; i < subItems.size(); ++i)
                    e->addChildElement (subItems.getUnchecked(i)->createXmlOpenness());
            }
            else
            {
                e = new XmlElement (T("CLOSED"));
            }

            e->setAttribute (T("id"), name);

            return e;
        }
        else
        {
            // trying to save the openness for an element that has no name - this won't
            // work because it needs the names to identify what to open.
            jassertfalse
        }
    }

    return 0;
}

END_JUCE_NAMESPACE
