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

class TreeView::ContentComponent  : public Component,
                                    public TooltipClient,
                                    public AsyncUpdater
{
public:
    ContentComponent (TreeView& tree)
        : owner (tree),
          buttonUnderMouse (nullptr),
          isDragging (false),
          needSelectionOnMouseUp (false)
    {
    }

    void mouseDown (const MouseEvent& e) override
    {
        updateButtonUnderMouse (e);

        isDragging = false;
        needSelectionOnMouseUp = false;
        Rectangle<int> pos;

        if (TreeViewItem* const item = findItemAt (e.y, pos))
        {
            if (isEnabled())
            {
                // (if the open/close buttons are hidden, we'll treat clicks to the left of the item
                // as selection clicks)
                if (e.x < pos.getX() && owner.openCloseButtonsVisible)
                {
                    if (e.x >= pos.getX() - owner.getIndentSize())
                        item->setOpen (! item->isOpen());

                    // (clicks to the left of an open/close button are ignored)
                }
                else
                {
                    // mouse-down inside the body of the item..
                    if (! owner.isMultiSelectEnabled())
                        item->setSelected (true, true);
                    else if (item->isSelected())
                        needSelectionOnMouseUp = ! e.mods.isPopupMenu();
                    else
                        selectBasedOnModifiers (item, e.mods);

                    if (e.x >= pos.getX())
                        item->itemClicked (e.withNewPosition (e.getPosition() - pos.getPosition()));
                }
            }
        }
    }

    void mouseUp (const MouseEvent& e) override
    {
        updateButtonUnderMouse (e);

        if (needSelectionOnMouseUp && e.mouseWasClicked() && isEnabled())
        {
            Rectangle<int> pos;
            if (TreeViewItem* const item = findItemAt (e.y, pos))
                selectBasedOnModifiers (item, e.mods);
        }
    }

    void mouseDoubleClick (const MouseEvent& e) override
    {
        if (e.getNumberOfClicks() != 3 && isEnabled())  // ignore triple clicks
        {
            Rectangle<int> pos;
            if (TreeViewItem* const item = findItemAt (e.y, pos))
                if (e.x >= pos.getX() || ! owner.openCloseButtonsVisible)
                    item->itemDoubleClicked (e.withNewPosition (e.getPosition() - pos.getPosition()));
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (isEnabled()
             && ! (isDragging || e.mouseWasClicked()
                    || e.getDistanceFromDragStart() < 5
                    || e.mods.isPopupMenu()))
        {
            isDragging = true;

            Rectangle<int> pos;
            TreeViewItem* const item = findItemAt (e.getMouseDownY(), pos);

            if (item != nullptr && e.getMouseDownX() >= pos.getX())
            {
                const var dragDescription (item->getDragSourceDescription());

                if (! (dragDescription.isVoid() || (dragDescription.isString() && dragDescription.toString().isEmpty())))
                {
                    if (DragAndDropContainer* const dragContainer = DragAndDropContainer::findParentDragContainerFor (this))
                    {
                        pos.setSize (pos.getWidth(), item->itemHeight);
                        Image dragImage (Component::createComponentSnapshot (pos, true));
                        dragImage.multiplyAllAlphas (0.6f);

                        Point<int> imageOffset (pos.getPosition() - e.getPosition());
                        dragContainer->startDragging (dragDescription, &owner, dragImage, true, &imageOffset);
                    }
                    else
                    {
                        // to be able to do a drag-and-drop operation, the treeview needs to
                        // be inside a component which is also a DragAndDropContainer.
                        jassertfalse;
                    }
                }
            }
        }
    }

    void mouseMove (const MouseEvent& e) override    { updateButtonUnderMouse (e); }
    void mouseExit (const MouseEvent& e) override    { updateButtonUnderMouse (e); }

    void paint (Graphics& g) override
    {
        if (owner.rootItem != nullptr)
        {
            owner.recalculateIfNeeded();

            if (! owner.rootItemVisible)
                g.setOrigin (0, -owner.rootItem->itemHeight);

            owner.rootItem->paintRecursively (g, getWidth());
        }
    }

    TreeViewItem* findItemAt (int y, Rectangle<int>& itemPosition) const
    {
        if (owner.rootItem != nullptr)
        {
            owner.recalculateIfNeeded();

            if (! owner.rootItemVisible)
                y += owner.rootItem->itemHeight;

            if (TreeViewItem* const ti = owner.rootItem->findItemRecursively (y))
            {
                itemPosition = ti->getItemPosition (false);
                return ti;
            }
        }

        return nullptr;
    }

    void updateComponents()
    {
        const int visibleTop = -getY();
        const int visibleBottom = visibleTop + getParentHeight();

        for (int i = items.size(); --i >= 0;)
            items.getUnchecked(i)->shouldKeep = false;

        {
            TreeViewItem* item = owner.rootItem;
            int y = (item != nullptr && ! owner.rootItemVisible) ? -item->itemHeight : 0;

            while (item != nullptr && y < visibleBottom)
            {
                y += item->itemHeight;

                if (y >= visibleTop)
                {
                    if (RowItem* const ri = findItem (item->uid))
                    {
                        ri->shouldKeep = true;
                    }
                    else if (Component* const comp = item->createItemComponent())
                    {
                        items.add (new RowItem (item, comp, item->uid));
                        addAndMakeVisible (comp);
                    }
                }

                item = item->getNextVisibleItem (true);
            }
        }

        for (int i = items.size(); --i >= 0;)
        {
            RowItem* const ri = items.getUnchecked(i);
            bool keep = false;

            if (isParentOf (ri->component))
            {
                if (ri->shouldKeep)
                {
                    Rectangle<int> pos (ri->item->getItemPosition (false));
                    pos.setSize (pos.getWidth(), ri->item->itemHeight);

                    if (pos.getBottom() >= visibleTop && pos.getY() < visibleBottom)
                    {
                        keep = true;
                        ri->component->setBounds (pos);
                    }
                }

                if ((! keep) && isMouseDraggingInChildCompOf (ri->component))
                {
                    keep = true;
                    ri->component->setSize (0, 0);
                }
            }

            if (! keep)
                items.remove (i);
        }
    }

    bool isMouseOverButton (TreeViewItem* const item) const noexcept
    {
        return item == buttonUnderMouse;
    }

    void resized() override
    {
        owner.itemsChanged();
    }

    String getTooltip() override
    {
        Rectangle<int> pos;
        if (TreeViewItem* const item = findItemAt (getMouseXYRelative().y, pos))
            return item->getTooltip();

        return owner.getTooltip();
    }

private:
    //==============================================================================
    TreeView& owner;

    struct RowItem
    {
        RowItem (TreeViewItem* const it, Component* const c, const int itemUID)
            : component (c), item (it), uid (itemUID), shouldKeep (true)
        {
        }

        ~RowItem()
        {
            delete component.get();
        }

        WeakReference<Component> component;
        TreeViewItem* item;
        int uid;
        bool shouldKeep;
    };

    OwnedArray <RowItem> items;

    TreeViewItem* buttonUnderMouse;
    bool isDragging, needSelectionOnMouseUp;

    void selectBasedOnModifiers (TreeViewItem* const item, const ModifierKeys modifiers)
    {
        TreeViewItem* firstSelected = nullptr;

        if (modifiers.isShiftDown() && ((firstSelected = owner.getSelectedItem (0)) != nullptr))
        {
            TreeViewItem* const lastSelected = owner.getSelectedItem (owner.getNumSelectedItems() - 1);
            jassert (lastSelected != nullptr);

            int rowStart = firstSelected->getRowNumberInTree();
            int rowEnd = lastSelected->getRowNumberInTree();
            if (rowStart > rowEnd)
                std::swap (rowStart, rowEnd);

            int ourRow = item->getRowNumberInTree();
            int otherEnd = ourRow < rowEnd ? rowStart : rowEnd;

            if (ourRow > otherEnd)
                std::swap (ourRow, otherEnd);

            for (int i = ourRow; i <= otherEnd; ++i)
                owner.getItemOnRow (i)->setSelected (true, false);
        }
        else
        {
            const bool cmd = modifiers.isCommandDown();
            item->setSelected ((! cmd) || ! item->isSelected(), ! cmd);
        }
    }

    bool containsItem (TreeViewItem* const item) const noexcept
    {
        for (int i = items.size(); --i >= 0;)
            if (items.getUnchecked(i)->item == item)
                return true;

        return false;
    }

    RowItem* findItem (const int uid) const noexcept
    {
        for (int i = items.size(); --i >= 0;)
        {
            RowItem* const ri = items.getUnchecked(i);
            if (ri->uid == uid)
                return ri;
        }

        return nullptr;
    }

    void updateButtonUnderMouse (const MouseEvent& e)
    {
        TreeViewItem* newItem = nullptr;

        if (owner.openCloseButtonsVisible)
        {
            Rectangle<int> pos;
            TreeViewItem* item = findItemAt (e.y, pos);

            if (item != nullptr && e.x < pos.getX() && e.x >= pos.getX() - owner.getIndentSize())
            {
                newItem = item;

                if (! newItem->mightContainSubItems())
                    newItem = nullptr;
            }
        }

        if (buttonUnderMouse != newItem)
        {
            repaintButtonUnderMouse();
            buttonUnderMouse = newItem;
            repaintButtonUnderMouse();
        }
    }

    void repaintButtonUnderMouse()
    {
        if (buttonUnderMouse != nullptr && containsItem (buttonUnderMouse))
        {
            const Rectangle<int> r (buttonUnderMouse->getItemPosition (false));
            repaint (0, r.getY(), r.getX(), buttonUnderMouse->getItemHeight());
        }
    }

    static bool isMouseDraggingInChildCompOf (Component* const comp)
    {
        const Array<MouseInputSource>& mouseSources = Desktop::getInstance().getMouseSources();

        for (MouseInputSource* mi = mouseSources.begin(), * const e = mouseSources.end(); mi != e; ++mi)
        {
            if (mi->isDragging())
                if (Component* const underMouse = mi->getComponentUnderMouse())
                    if (comp == underMouse || comp->isParentOf (underMouse))
                        return true;
        }

        return false;
    }

    void handleAsyncUpdate() override
    {
        owner.recalculateIfNeeded();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentComponent)
};

//==============================================================================
class TreeView::TreeViewport  : public Viewport
{
public:
    TreeViewport() noexcept : lastX (-1)    {}

    void updateComponents (const bool triggerResize)
    {
        if (ContentComponent* const tvc = getContentComp())
        {
            if (triggerResize)
                tvc->resized();
            else
                tvc->updateComponents();
        }

        repaint();
    }

    void visibleAreaChanged (const Rectangle<int>& newVisibleArea) override
    {
        const bool hasScrolledSideways = (newVisibleArea.getX() != lastX);
        lastX = newVisibleArea.getX();
        updateComponents (hasScrolledSideways);
    }

    ContentComponent* getContentComp() const noexcept
    {
        return static_cast <ContentComponent*> (getViewedComponent());
    }

    bool keyPressed (const KeyPress& key) override
    {
        Component* const tree = getParentComponent();

        return (tree != nullptr && tree->keyPressed (key))
                 || Viewport::keyPressed (key);
    }

private:
    int lastX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeViewport)
};


//==============================================================================
TreeView::TreeView (const String& name)
    : Component (name),
      viewport (new TreeViewport()),
      rootItem (nullptr),
      indentSize (-1),
      defaultOpenness (false),
      needsRecalculating (true),
      rootItemVisible (true),
      multiSelectEnabled (false),
      openCloseButtonsVisible (true)
{
    addAndMakeVisible (viewport);
    viewport->setViewedComponent (new ContentComponent (*this));
    setWantsKeyboardFocus (true);
}

TreeView::~TreeView()
{
    if (rootItem != nullptr)
        rootItem->setOwnerView (nullptr);
}

void TreeView::setRootItem (TreeViewItem* const newRootItem)
{
    if (rootItem != newRootItem)
    {
        if (newRootItem != nullptr)
        {
            jassert (newRootItem->ownerView == nullptr); // can't use a tree item in more than one tree at once..

            if (newRootItem->ownerView != nullptr)
                newRootItem->ownerView->setRootItem (nullptr);
        }

        if (rootItem != nullptr)
            rootItem->setOwnerView (nullptr);

        rootItem = newRootItem;

        if (newRootItem != nullptr)
            newRootItem->setOwnerView (this);

        needsRecalculating = true;
        recalculateIfNeeded();

        if (rootItem != nullptr && (defaultOpenness || ! rootItemVisible))
        {
            rootItem->setOpen (false); // force a re-open
            rootItem->setOpen (true);
        }
    }
}

void TreeView::deleteRootItem()
{
    const ScopedPointer <TreeViewItem> deleter (rootItem);
    setRootItem (nullptr);
}

void TreeView::setRootItemVisible (const bool shouldBeVisible)
{
    rootItemVisible = shouldBeVisible;

    if (rootItem != nullptr && (defaultOpenness || ! rootItemVisible))
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

int TreeView::getIndentSize() noexcept
{
    return indentSize >= 0 ? indentSize
                           : getLookAndFeel().getTreeViewIndentSize (*this);
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

void TreeView::setOpenCloseButtonsVisible (const bool shouldBeVisible)
{
    if (openCloseButtonsVisible != shouldBeVisible)
    {
        openCloseButtonsVisible = shouldBeVisible;
        itemsChanged();
    }
}

Viewport* TreeView::getViewport() const noexcept
{
    return viewport;
}

//==============================================================================
void TreeView::clearSelectedItems()
{
    if (rootItem != nullptr)
        rootItem->deselectAllRecursively();
}

int TreeView::getNumSelectedItems (int maximumDepthToSearchTo) const noexcept
{
    return rootItem != nullptr ? rootItem->countSelectedItemsRecursively (maximumDepthToSearchTo) : 0;
}

TreeViewItem* TreeView::getSelectedItem (const int index) const noexcept
{
    return rootItem != nullptr ? rootItem->getSelectedItemWithIndex (index) : 0;
}

int TreeView::getNumRowsInTree() const
{
    return rootItem != nullptr ? (rootItem->getNumRows() - (rootItemVisible ? 0 : 1)) : 0;
}

TreeViewItem* TreeView::getItemOnRow (int index) const
{
    if (! rootItemVisible)
        ++index;

    if (rootItem != nullptr && index >= 0)
        return rootItem->getItemOnRow (index);

    return nullptr;
}

TreeViewItem* TreeView::getItemAt (int y) const noexcept
{
    ContentComponent* const tc = viewport->getContentComp();
    Rectangle<int> pos;
    return tc->findItemAt (tc->getLocalPoint (this, Point<int> (0, y)).y, pos);
}

TreeViewItem* TreeView::findItemFromIdentifierString (const String& identifierString) const
{
    if (rootItem == nullptr)
        return nullptr;

    return rootItem->findItemFromIdentifierString (identifierString);
}

//==============================================================================
static void addAllSelectedItemIds (TreeViewItem* item, XmlElement& parent)
{
    if (item->isSelected())
        parent.createNewChildElement ("SELECTED")->setAttribute ("id", item->getItemIdentifierString());

    const int numSubItems = item->getNumSubItems();

    for (int i = 0; i < numSubItems; ++i)
        addAllSelectedItemIds (item->getSubItem(i), parent);
}

XmlElement* TreeView::getOpennessState (const bool alsoIncludeScrollPosition) const
{
    XmlElement* e = nullptr;

    if (rootItem != nullptr)
    {
        e = rootItem->getOpennessState (false);

        if (e != nullptr)
        {
            if (alsoIncludeScrollPosition)
                e->setAttribute ("scrollPos", viewport->getViewPositionY());

            addAllSelectedItemIds (rootItem, *e);
        }
    }

    return e;
}

void TreeView::restoreOpennessState (const XmlElement& newState, const bool restoreStoredSelection)
{
    if (rootItem != nullptr)
    {
        rootItem->restoreOpennessState (newState);

        if (newState.hasAttribute ("scrollPos"))
            viewport->setViewPosition (viewport->getViewPositionX(),
                                       newState.getIntAttribute ("scrollPos"));

        if (restoreStoredSelection)
        {
            clearSelectedItems();

            forEachXmlChildElementWithTagName (newState, e, "SELECTED")
            {
                if (TreeViewItem* const item = rootItem->findItemFromIdentifierString (e->getStringAttribute ("id")))
                    item->setSelected (true, false);
            }
        }
    }
}

//==============================================================================
void TreeView::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));
}

void TreeView::resized()
{
    viewport->setBounds (getLocalBounds());

    itemsChanged();
    recalculateIfNeeded();
}

void TreeView::enablementChanged()
{
    repaint();
}

void TreeView::moveSelectedRow (const int delta)
{
    const int numRowsInTree = getNumRowsInTree();

    if (numRowsInTree > 0)
    {
        int rowSelected = 0;

        if (TreeViewItem* const firstSelected = getSelectedItem (0))
            rowSelected = firstSelected->getRowNumberInTree();

        rowSelected = jlimit (0, numRowsInTree - 1, rowSelected + delta);

        for (;;)
        {
            if (TreeViewItem* const item = getItemOnRow (rowSelected))
            {
                if (! item->canBeSelected())
                {
                    // if the row we want to highlight doesn't allow it, try skipping
                    // to the next item..
                    const int nextRowToTry = jlimit (0, numRowsInTree - 1, rowSelected + (delta < 0 ? -1 : 1));

                    if (rowSelected != nextRowToTry)
                    {
                        rowSelected = nextRowToTry;
                        continue;
                    }

                    break;
                }

                item->setSelected (true, true);
                scrollToKeepItemVisible (item);
            }

            break;
        }
    }
}

void TreeView::scrollToKeepItemVisible (TreeViewItem* item)
{
    if (item != nullptr && item->ownerView == this)
    {
        recalculateIfNeeded();

        item = item->getDeepestOpenParentItem();

        const int y = item->y;
        const int viewTop = viewport->getViewPositionY();

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

void TreeView::toggleOpenSelectedItem()
{
    if (TreeViewItem* const firstSelected = getSelectedItem (0))
        firstSelected->setOpen (! firstSelected->isOpen());
}

void TreeView::moveOutOfSelectedItem()
{
    if (TreeViewItem* const firstSelected = getSelectedItem (0))
    {
        if (firstSelected->isOpen())
        {
            firstSelected->setOpen (false);
        }
        else
        {
            TreeViewItem* parent = firstSelected->parentItem;

            if ((! rootItemVisible) && parent == rootItem)
                parent = nullptr;

            if (parent != nullptr)
            {
                parent->setSelected (true, true);
                scrollToKeepItemVisible (parent);
            }
        }
    }
}

void TreeView::moveIntoSelectedItem()
{
    if (TreeViewItem* const firstSelected = getSelectedItem (0))
    {
        if (firstSelected->isOpen() || ! firstSelected->mightContainSubItems())
            moveSelectedRow (1);
        else
            firstSelected->setOpen (true);
    }
}

void TreeView::moveByPages (int numPages)
{
    if (TreeViewItem* currentItem = getSelectedItem (0))
    {
        const Rectangle<int> pos (currentItem->getItemPosition (false));
        const int targetY = pos.getY() + numPages * (getHeight() - pos.getHeight());
        int currentRow = currentItem->getRowNumberInTree();

        for (;;)
        {
            moveSelectedRow (numPages);
            currentItem = getSelectedItem (0);

            if (currentItem == nullptr)
                break;

            const int y = currentItem->getItemPosition (false).getY();
            if ((numPages < 0 && y <= targetY) || (numPages > 0 && y >= targetY))
                break;

            const int newRow = currentItem->getRowNumberInTree();
            if (newRow == currentRow)
                break;

            currentRow = newRow;
        }
    }
}

bool TreeView::keyPressed (const KeyPress& key)
{
    if (rootItem != nullptr)
    {
        if (key == KeyPress::upKey)       { moveSelectedRow (-1); return true; }
        if (key == KeyPress::downKey)     { moveSelectedRow (1);  return true; }
        if (key == KeyPress::homeKey)     { moveSelectedRow (-0x3fffffff); return true; }
        if (key == KeyPress::endKey)      { moveSelectedRow (0x3fffffff);  return true; }
        if (key == KeyPress::pageUpKey)   { moveByPages (-1); return true; }
        if (key == KeyPress::pageDownKey) { moveByPages (1);  return true; }
        if (key == KeyPress::returnKey)   { toggleOpenSelectedItem(); return true; }
        if (key == KeyPress::leftKey)     { moveOutOfSelectedItem();  return true; }
        if (key == KeyPress::rightKey)    { moveIntoSelectedItem();   return true; }
    }

    return false;
}

void TreeView::itemsChanged() noexcept
{
    needsRecalculating = true;
    repaint();
    viewport->getContentComp()->triggerAsyncUpdate();
}

void TreeView::recalculateIfNeeded()
{
    if (needsRecalculating)
    {
        needsRecalculating = false;

        const ScopedLock sl (nodeAlterationLock);

        if (rootItem != nullptr)
            rootItem->updatePositions (rootItemVisible ? 0 : -rootItem->itemHeight);

        viewport->updateComponents (false);

        if (rootItem != nullptr)
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
struct TreeView::InsertPoint
{
    InsertPoint (TreeView& view, const StringArray& files,
                 const DragAndDropTarget::SourceDetails& dragSourceDetails)
        : pos (dragSourceDetails.localPosition),
          item (view.getItemAt (dragSourceDetails.localPosition.y)),
          insertIndex (0)
    {
        if (item != nullptr)
        {
            Rectangle<int> itemPos (item->getItemPosition (true));
            insertIndex = item->getIndexInParent();
            const int oldY = pos.y;
            pos.y = itemPos.getY();

            if (item->getNumSubItems() == 0 || ! item->isOpen())
            {
                if (files.size() > 0 ? item->isInterestedInFileDrag (files)
                                     : item->isInterestedInDragSource (dragSourceDetails))
                {
                    // Check if we're trying to drag into an empty group item..
                    if (oldY > itemPos.getY() + itemPos.getHeight() / 4
                         && oldY < itemPos.getBottom() - itemPos.getHeight() / 4)
                    {
                        insertIndex = 0;
                        pos.x = itemPos.getX() + view.getIndentSize();
                        pos.y = itemPos.getBottom();
                        return;
                    }
                }
            }

            if (oldY > itemPos.getCentreY())
            {
                pos.y += item->getItemHeight();

                while (item->isLastOfSiblings() && item->getParentItem() != nullptr
                        && item->getParentItem()->getParentItem() != nullptr)
                {
                    if (pos.x > itemPos.getX())
                        break;

                    item = item->getParentItem();
                    itemPos = item->getItemPosition (true);
                    insertIndex = item->getIndexInParent();
                }

                ++insertIndex;
            }

            pos.x = itemPos.getX();
            item = item->getParentItem();
        }
    }

    Point<int> pos;
    TreeViewItem* item;
    int insertIndex;
};

//==============================================================================
class TreeView::InsertPointHighlight   : public Component
{
public:
    InsertPointHighlight()
        : lastItem (nullptr), lastIndex (0)
    {
        setSize (100, 12);
        setAlwaysOnTop (true);
        setInterceptsMouseClicks (false, false);
    }

    void setTargetPosition (const InsertPoint& insertPos, const int width) noexcept
    {
        lastItem = insertPos.item;
        lastIndex = insertPos.insertIndex;
        const int offset = getHeight() / 2;
        setBounds (insertPos.pos.x - offset, insertPos.pos.y - offset,
                   width - (insertPos.pos.x - offset), getHeight());
    }

    void paint (Graphics& g) override
    {
        Path p;
        const float h = (float) getHeight();
        p.addEllipse (2.0f, 2.0f, h - 4.0f, h - 4.0f);
        p.startNewSubPath (h - 2.0f, h / 2.0f);
        p.lineTo ((float) getWidth(), h / 2.0f);

        g.setColour (findColour (TreeView::dragAndDropIndicatorColourId, true));
        g.strokePath (p, PathStrokeType (2.0f));
    }

    TreeViewItem* lastItem;
    int lastIndex;

private:
    JUCE_DECLARE_NON_COPYABLE (InsertPointHighlight)
};

//==============================================================================
class TreeView::TargetGroupHighlight   : public Component
{
public:
    TargetGroupHighlight()
    {
        setAlwaysOnTop (true);
        setInterceptsMouseClicks (false, false);
    }

    void setTargetPosition (TreeViewItem* const item) noexcept
    {
        Rectangle<int> r (item->getItemPosition (true));
        r.setHeight (item->getItemHeight());
        setBounds (r);
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (TreeView::dragAndDropIndicatorColourId, true));
        g.drawRoundedRectangle (1.0f, 1.0f, getWidth() - 2.0f, getHeight() - 2.0f, 3.0f, 2.0f);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (TargetGroupHighlight)
};

//==============================================================================
void TreeView::showDragHighlight (const InsertPoint& insertPos) noexcept
{
    beginDragAutoRepeat (100);

    if (dragInsertPointHighlight == nullptr)
    {
        addAndMakeVisible (dragInsertPointHighlight = new InsertPointHighlight());
        addAndMakeVisible (dragTargetGroupHighlight = new TargetGroupHighlight());
    }

    dragInsertPointHighlight->setTargetPosition (insertPos, viewport->getViewWidth());
    dragTargetGroupHighlight->setTargetPosition (insertPos.item);
}

void TreeView::hideDragHighlight() noexcept
{
    dragInsertPointHighlight = nullptr;
    dragTargetGroupHighlight = nullptr;
}

void TreeView::handleDrag (const StringArray& files, const SourceDetails& dragSourceDetails)
{
    const bool scrolled = viewport->autoScroll (dragSourceDetails.localPosition.x,
                                                dragSourceDetails.localPosition.y, 20, 10);

    InsertPoint insertPos (*this, files, dragSourceDetails);

    if (insertPos.item != nullptr)
    {
        if (scrolled || dragInsertPointHighlight == nullptr
             || dragInsertPointHighlight->lastItem != insertPos.item
             || dragInsertPointHighlight->lastIndex != insertPos.insertIndex)
        {
            if (files.size() > 0 ? insertPos.item->isInterestedInFileDrag (files)
                                 : insertPos.item->isInterestedInDragSource (dragSourceDetails))
                showDragHighlight (insertPos);
            else
                hideDragHighlight();
        }
    }
    else
    {
        hideDragHighlight();
    }
}

void TreeView::handleDrop (const StringArray& files, const SourceDetails& dragSourceDetails)
{
    hideDragHighlight();

    InsertPoint insertPos (*this, files, dragSourceDetails);

    if (insertPos.item == nullptr)
        insertPos.item = rootItem;

    if (insertPos.item != nullptr)
    {
        if (files.size() > 0)
        {
            if (insertPos.item->isInterestedInFileDrag (files))
                insertPos.item->filesDropped (files, insertPos.insertIndex);
        }
        else
        {
            if (insertPos.item->isInterestedInDragSource (dragSourceDetails))
                insertPos.item->itemDropped (dragSourceDetails, insertPos.insertIndex);
        }
    }
}

//==============================================================================
bool TreeView::isInterestedInFileDrag (const StringArray&)
{
    return true;
}

void TreeView::fileDragEnter (const StringArray& files, int x, int y)
{
    fileDragMove (files, x, y);
}

void TreeView::fileDragMove (const StringArray& files, int x, int y)
{
    handleDrag (files, SourceDetails (String::empty, this, Point<int> (x, y)));
}

void TreeView::fileDragExit (const StringArray&)
{
    hideDragHighlight();
}

void TreeView::filesDropped (const StringArray& files, int x, int y)
{
    handleDrop (files, SourceDetails (String::empty, this, Point<int> (x, y)));
}

bool TreeView::isInterestedInDragSource (const SourceDetails& /*dragSourceDetails*/)
{
    return true;
}

void TreeView::itemDragEnter (const SourceDetails& dragSourceDetails)
{
    itemDragMove (dragSourceDetails);
}

void TreeView::itemDragMove (const SourceDetails& dragSourceDetails)
{
    handleDrag (StringArray(), dragSourceDetails);
}

void TreeView::itemDragExit (const SourceDetails& /*dragSourceDetails*/)
{
    hideDragHighlight();
}

void TreeView::itemDropped (const SourceDetails& dragSourceDetails)
{
    handleDrop (StringArray(), dragSourceDetails);
}

//==============================================================================
enum TreeViewOpenness
{
    opennessDefault = 0,
    opennessClosed = 1,
    opennessOpen = 2
};

TreeViewItem::TreeViewItem()
    : ownerView (nullptr),
      parentItem (nullptr),
      y (0),
      itemHeight (0),
      totalHeight (0),
      itemWidth (0),
      totalWidth (0),
      selected (false),
      redrawNeeded (true),
      drawLinesInside (false),
      drawLinesSet (false),
      drawsInLeftMargin (false),
      openness (opennessDefault)
{
    static int nextUID = 0;
    uid = nextUID++;
}

TreeViewItem::~TreeViewItem()
{
}

String TreeViewItem::getUniqueName() const
{
    return String::empty;
}

void TreeViewItem::itemOpennessChanged (bool)
{
}

int TreeViewItem::getNumSubItems() const noexcept
{
    return subItems.size();
}

TreeViewItem* TreeViewItem::getSubItem (const int index) const noexcept
{
    return subItems [index];
}

void TreeViewItem::clearSubItems()
{
    if (ownerView != nullptr)
    {
        const ScopedLock sl (ownerView->nodeAlterationLock);

        if (subItems.size() > 0)
        {
            removeAllSubItemsFromList();
            treeHasChanged();
        }
    }
    else
    {
        removeAllSubItemsFromList();
    }
}

void TreeViewItem::removeAllSubItemsFromList()
{
    for (int i = subItems.size(); --i >= 0;)
        removeSubItemFromList (i, true);
}

void TreeViewItem::addSubItem (TreeViewItem* const newItem, const int insertPosition)
{
    if (newItem != nullptr)
    {
        newItem->parentItem = this;
        newItem->setOwnerView (ownerView);
        newItem->y = 0;
        newItem->itemHeight = newItem->getItemHeight();
        newItem->totalHeight = 0;
        newItem->itemWidth = newItem->getItemWidth();
        newItem->totalWidth = 0;

        if (ownerView != nullptr)
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

void TreeViewItem::removeSubItem (int index, bool deleteItem)
{
    if (ownerView != nullptr)
    {
        const ScopedLock sl (ownerView->nodeAlterationLock);

        if (removeSubItemFromList (index, deleteItem))
            treeHasChanged();
    }
    else
    {
        removeSubItemFromList (index, deleteItem);
    }
}

bool TreeViewItem::removeSubItemFromList (int index, bool deleteItem)
{
    if (TreeViewItem* child = subItems [index])
    {
        child->parentItem = nullptr;
        subItems.remove (index, deleteItem);
        return true;
    }

    return false;
}

bool TreeViewItem::isOpen() const noexcept
{
    if (openness == opennessDefault)
        return ownerView != nullptr && ownerView->defaultOpenness;

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

bool TreeViewItem::isFullyOpen() const noexcept
{
    if (! isOpen())
        return false;

    for (int i = 0; i < subItems.size(); ++i)
        if (! subItems.getUnchecked(i)->isFullyOpen())
            return false;

    return true;
}

void TreeViewItem::restoreToDefaultOpenness()
{
    if (openness != opennessDefault && ownerView != nullptr)
    {
        setOpen (ownerView->defaultOpenness);
        openness = opennessDefault;
    }
}

bool TreeViewItem::isSelected() const noexcept
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
                                const bool deselectOtherItemsFirst,
                                const NotificationType notify)
{
    if (shouldBeSelected && ! canBeSelected())
        return;

    if (deselectOtherItemsFirst)
        getTopLevelItem()->deselectAllRecursively();

    if (shouldBeSelected != selected)
    {
        selected = shouldBeSelected;
        if (ownerView != nullptr)
            ownerView->repaint();

        if (notify != dontSendNotification)
            itemSelectionChanged (shouldBeSelected);
    }
}

void TreeViewItem::paintItem (Graphics&, int, int)
{
}

void TreeViewItem::paintOpenCloseButton (Graphics& g, const Rectangle<float>& area, Colour backgroundColour, bool isMouseOver)
{
    getOwnerView()->getLookAndFeel()
       .drawTreeviewPlusMinusBox (g, area, backgroundColour, isOpen(), isMouseOver);
}

void TreeViewItem::paintHorizontalConnectingLine (Graphics& g, const Line<float>& line)
{
   g.setColour (ownerView->findColour (TreeView::linesColourId));
   g.drawLine (line);
}

void TreeViewItem::paintVerticalConnectingLine (Graphics& g, const Line<float>& line)
{
   g.setColour (ownerView->findColour (TreeView::linesColourId));
   g.drawLine (line);
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

String TreeViewItem::getTooltip()
{
    return String::empty;
}

var TreeViewItem::getDragSourceDescription()
{
    return var::null;
}

bool TreeViewItem::isInterestedInFileDrag (const StringArray&)
{
    return false;
}

void TreeViewItem::filesDropped (const StringArray& /*files*/, int /*insertIndex*/)
{
}

bool TreeViewItem::isInterestedInDragSource (const DragAndDropTarget::SourceDetails& /*dragSourceDetails*/)
{
    return false;
}

void TreeViewItem::itemDropped (const DragAndDropTarget::SourceDetails& /*dragSourceDetails*/, int /*insertIndex*/)
{
}

Rectangle<int> TreeViewItem::getItemPosition (const bool relativeToTreeViewTopLeft) const noexcept
{
    const int indentX = getIndentX();
    int width = itemWidth;

    if (ownerView != nullptr && width < 0)
        width = ownerView->viewport->getViewWidth() - indentX;

    Rectangle<int> r (indentX, y, jmax (0, width), totalHeight);

    if (relativeToTreeViewTopLeft && ownerView != nullptr)
        r -= ownerView->viewport->getViewPosition();

    return r;
}

void TreeViewItem::treeHasChanged() const noexcept
{
    if (ownerView != nullptr)
        ownerView->itemsChanged();
}

void TreeViewItem::repaintItem() const
{
    if (ownerView != nullptr && areAllParentsOpen())
    {
        Rectangle<int> r (getItemPosition (true));
        r.setLeft (0);
        ownerView->viewport->repaint (r);
    }
}

bool TreeViewItem::areAllParentsOpen() const noexcept
{
    return parentItem == nullptr
            || (parentItem->isOpen() && parentItem->areAllParentsOpen());
}

void TreeViewItem::updatePositions (int newY)
{
    y = newY;
    itemHeight = getItemHeight();
    totalHeight = itemHeight;
    itemWidth = getItemWidth();
    totalWidth = jmax (itemWidth, 0) + getIndentX();

    if (isOpen())
    {
        newY += totalHeight;

        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const ti = subItems.getUnchecked(i);

            ti->updatePositions (newY);
            newY += ti->totalHeight;
            totalHeight += ti->totalHeight;
            totalWidth = jmax (totalWidth, ti->totalWidth);
        }
    }
}

TreeViewItem* TreeViewItem::getDeepestOpenParentItem() noexcept
{
    TreeViewItem* result = this;
    TreeViewItem* item = this;

    while (item->parentItem != nullptr)
    {
        item = item->parentItem;

        if (! item->isOpen())
            result = item;
    }

    return result;
}

void TreeViewItem::setOwnerView (TreeView* const newOwner) noexcept
{
    ownerView = newOwner;

    for (int i = subItems.size(); --i >= 0;)
        subItems.getUnchecked(i)->setOwnerView (newOwner);
}

int TreeViewItem::getIndentX() const noexcept
{
    int x = ownerView->rootItemVisible ? 1 : 0;

    if (! ownerView->openCloseButtonsVisible)
        --x;

    for (TreeViewItem* p = parentItem; p != nullptr; p = p->parentItem)
        ++x;

    return x * ownerView->getIndentSize();
}

void TreeViewItem::setDrawsInLeftMargin (bool canDrawInLeftMargin) noexcept
{
    drawsInLeftMargin = canDrawInLeftMargin;
}

namespace TreeViewHelpers
{
    static int calculateDepth (const TreeViewItem* item, const bool rootIsVisible) noexcept
    {
        jassert (item != nullptr);
        int depth = rootIsVisible ? 0 : -1;

        for (const TreeViewItem* p = item->getParentItem(); p != nullptr; p = p->getParentItem())
            ++depth;

        return depth;
    }
}

bool TreeViewItem::areLinesDrawn() const
{
    return drawLinesSet ? drawLinesInside
                        : (ownerView != nullptr && ownerView->getLookAndFeel().areLinesDrawnForTreeView (*ownerView));
}

void TreeViewItem::paintRecursively (Graphics& g, int width)
{
    jassert (ownerView != nullptr);
    if (ownerView == nullptr)
        return;

    const int indent = getIndentX();
    const int itemW = itemWidth < 0 ? width - indent : itemWidth;

    {
        Graphics::ScopedSaveState ss (g);
        g.setOrigin (indent, 0);

        if (g.reduceClipRegion (drawsInLeftMargin ? -indent : 0, 0,
                                drawsInLeftMargin ? itemW + indent : itemW, itemHeight))
        {
            if (isSelected())
                g.fillAll (ownerView->findColour (TreeView::selectedItemBackgroundColourId));

            paintItem (g, itemW, itemHeight);
        }
    }

    const float halfH = itemHeight * 0.5f;
    const int indentWidth = ownerView->getIndentSize();
    const int depth = TreeViewHelpers::calculateDepth (this, ownerView->rootItemVisible);

    if (depth >= 0 && ownerView->openCloseButtonsVisible)
    {
        float x = (depth + 0.5f) * indentWidth;

        const bool parentLinesDrawn = parentItem != nullptr && parentItem->areLinesDrawn();

        if (parentLinesDrawn)
            paintVerticalConnectingLine (g, Line<float> (x, 0, x, isLastOfSiblings() ? halfH : (float) itemHeight));

        if (parentLinesDrawn || (parentItem == nullptr && areLinesDrawn()))
            paintHorizontalConnectingLine (g, Line<float> (x, halfH, x + indentWidth / 2, halfH));

        {
            TreeViewItem* p = parentItem;
            int d = depth;

            while (p != nullptr && --d >= 0)
            {
                x -= (float) indentWidth;

                if ((p->parentItem == nullptr || p->parentItem->areLinesDrawn()) && ! p->isLastOfSiblings())
                    p->paintVerticalConnectingLine (g, Line<float> (x, 0, x, (float) itemHeight));

                p = p->parentItem;
            }
        }

        if (mightContainSubItems())
            paintOpenCloseButton (g, Rectangle<float> ((float) (depth * indentWidth), 0, (float) indentWidth, (float) itemHeight),
                                  Colours::white,
                                  ownerView->viewport->getContentComp()->isMouseOverButton (this));
    }

    if (isOpen())
    {
        const Rectangle<int> clip (g.getClipBounds());

        for (int i = 0; i < subItems.size(); ++i)
        {
            TreeViewItem* const ti = subItems.getUnchecked(i);

            const int relY = ti->y - y;

            if (relY >= clip.getBottom())
                break;

            if (relY + ti->totalHeight >= clip.getY())
            {
                Graphics::ScopedSaveState ss (g);

                g.setOrigin (0, relY);

                if (g.reduceClipRegion (0, 0, width, ti->totalHeight))
                    ti->paintRecursively (g, width);
            }
        }
    }
}

bool TreeViewItem::isLastOfSiblings() const noexcept
{
    return parentItem == nullptr
            || parentItem->subItems.getLast() == this;
}

int TreeViewItem::getIndexInParent() const noexcept
{
    return parentItem == nullptr ? 0
                                 : parentItem->subItems.indexOf (this);
}

TreeViewItem* TreeViewItem::getTopLevelItem() noexcept
{
    return parentItem == nullptr ? this
                                 : parentItem->getTopLevelItem();
}

int TreeViewItem::getNumRows() const noexcept
{
    int num = 1;

    if (isOpen())
    {
        for (int i = subItems.size(); --i >= 0;)
            num += subItems.getUnchecked(i)->getNumRows();
    }

    return num;
}

TreeViewItem* TreeViewItem::getItemOnRow (int index) noexcept
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

    return nullptr;
}

TreeViewItem* TreeViewItem::findItemRecursively (int targetY) noexcept
{
    if (isPositiveAndBelow (targetY, totalHeight))
    {
        const int h = itemHeight;

        if (targetY < h)
            return this;

        if (isOpen())
        {
            targetY -= h;

            for (int i = 0; i < subItems.size(); ++i)
            {
                TreeViewItem* const ti = subItems.getUnchecked(i);

                if (targetY < ti->totalHeight)
                    return ti->findItemRecursively (targetY);

                targetY -= ti->totalHeight;
            }
        }
    }

    return nullptr;
}

int TreeViewItem::countSelectedItemsRecursively (int depth) const noexcept
{
    int total = isSelected() ? 1 : 0;

    if (depth != 0)
        for (int i = subItems.size(); --i >= 0;)
            total += subItems.getUnchecked(i)->countSelectedItemsRecursively (depth - 1);

    return total;
}

TreeViewItem* TreeViewItem::getSelectedItemWithIndex (int index) noexcept
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

            if (TreeViewItem* const found = item->getSelectedItemWithIndex (index))
                return found;

            index -= item->countSelectedItemsRecursively (-1);
        }
    }

    return nullptr;
}

int TreeViewItem::getRowNumberInTree() const noexcept
{
    if (parentItem != nullptr && ownerView != nullptr)
    {
        int n = 1 + parentItem->getRowNumberInTree();

        int ourIndex = parentItem->subItems.indexOf (this);
        jassert (ourIndex >= 0);

        while (--ourIndex >= 0)
            n += parentItem->subItems [ourIndex]->getNumRows();

        if (parentItem->parentItem == nullptr
             && ! ownerView->rootItemVisible)
            --n;

        return n;
    }

    return 0;
}

void TreeViewItem::setLinesDrawnForSubItems (const bool drawLines) noexcept
{
    drawLinesInside = drawLines;
    drawLinesSet = true;
}

TreeViewItem* TreeViewItem::getNextVisibleItem (const bool recurse) const noexcept
{
    if (recurse && isOpen() && subItems.size() > 0)
        return subItems [0];

    if (parentItem != nullptr)
    {
        const int nextIndex = parentItem->subItems.indexOf (this) + 1;

        if (nextIndex >= parentItem->subItems.size())
            return parentItem->getNextVisibleItem (false);

        return parentItem->subItems [nextIndex];
    }

    return nullptr;
}

String TreeViewItem::getItemIdentifierString() const
{
    String s;

    if (parentItem != nullptr)
        s = parentItem->getItemIdentifierString();

    return s + "/" + getUniqueName().replaceCharacter ('/', '\\');
}

TreeViewItem* TreeViewItem::findItemFromIdentifierString (const String& identifierString)
{
    const String thisId ("/" + getUniqueName());

    if (thisId == identifierString)
        return this;

    if (identifierString.startsWith (thisId + "/"))
    {
        const String remainingPath (identifierString.substring (thisId.length()));

        const bool wasOpen = isOpen();
        setOpen (true);

        for (int i = subItems.size(); --i >= 0;)
            if (TreeViewItem* item = subItems.getUnchecked(i)->findItemFromIdentifierString (remainingPath))
                return item;

        setOpen (wasOpen);
    }

    return nullptr;
}

void TreeViewItem::restoreOpennessState (const XmlElement& e)
{
    if (e.hasTagName ("CLOSED"))
    {
        setOpen (false);
    }
    else if (e.hasTagName ("OPEN"))
    {
        setOpen (true);

        Array<TreeViewItem*> items;
        items.addArray (subItems);

        forEachXmlChildElement (e, n)
        {
            const String id (n->getStringAttribute ("id"));

            for (int i = 0; i < items.size(); ++i)
            {
                TreeViewItem* const ti = items.getUnchecked(i);

                if (ti->getUniqueName() == id)
                {
                    ti->restoreOpennessState (*n);
                    items.remove (i);
                    break;
                }
            }
        }

        for (int i = 0; i < items.size(); ++i)
            items.getUnchecked(i)->restoreToDefaultOpenness();
    }
}

XmlElement* TreeViewItem::getOpennessState() const
{
    return getOpennessState (true);
}

XmlElement* TreeViewItem::getOpennessState (const bool canReturnNull) const
{
    const String name (getUniqueName());

    if (name.isNotEmpty())
    {
        XmlElement* e;

        if (isOpen())
        {
            if (canReturnNull && ownerView != nullptr && ownerView->defaultOpenness && isFullyOpen())
                return nullptr;

            e = new XmlElement ("OPEN");

            for (int i = subItems.size(); --i >= 0;)
                e->prependChildElement (subItems.getUnchecked(i)->getOpennessState (true));
        }
        else
        {
            if (canReturnNull && ownerView != nullptr && ! ownerView->defaultOpenness)
                return nullptr;

            e = new XmlElement ("CLOSED");
        }

        e->setAttribute ("id", name);
        return e;
    }

    // trying to save the openness for an element that has no name - this won't
    // work because it needs the names to identify what to open.
    jassertfalse;
    return nullptr;
}

//==============================================================================
TreeViewItem::OpennessRestorer::OpennessRestorer (TreeViewItem& item)
    : treeViewItem (item),
      oldOpenness (item.getOpennessState())
{
}

TreeViewItem::OpennessRestorer::~OpennessRestorer()
{
    if (oldOpenness != nullptr)
        treeViewItem.restoreOpennessState (*oldOpenness);
}
