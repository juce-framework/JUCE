/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

static int getItemDepth (const TreeViewItem* item)
{
    if (item == nullptr || item->getOwnerView() == nullptr)
        return 0;

    auto depth = item->getOwnerView()->isRootItemVisible() ? 0 : -1;

    for (auto* parent = item->getParentItem(); parent != nullptr; parent = parent->getParentItem())
        ++depth;

    return depth;
}

//==============================================================================
class TreeView::ItemComponent  : public Component,
                                 public TooltipClient
{
public:
    explicit ItemComponent (TreeViewItem& itemToRepresent)
        : item (itemToRepresent),
          customComponent (item.createItemComponent())
    {
        if (hasCustomComponent())
            addAndMakeVisible (*customComponent);
    }

    void paint (Graphics& g) override
    {
        item.draw (g, getWidth(), mouseIsOverButton);
    }

    void resized() override
    {
        if (hasCustomComponent())
        {
            auto itemPosition = item.getItemPosition (false);

            customComponent->setBounds (getLocalBounds().withX (itemPosition.getX())
                                                        .withWidth (itemPosition.getWidth()));
        }
    }

    void setMouseIsOverButton (bool isOver)
    {
        mouseIsOverButton = isOver;
        repaint();
    }

    TreeViewItem& getRepresentedItem() const noexcept
    {
        return item;
    }

    String getTooltip() override
    {
        return item.getTooltip();
    }

private:
    //==============================================================================
    class ItemAccessibilityHandler  : public AccessibilityHandler
    {
    public:
        explicit ItemAccessibilityHandler (ItemComponent& comp)
            : AccessibilityHandler (comp,
                                    AccessibilityRole::treeItem,
                                    getAccessibilityActions (comp),
                                    { std::make_unique<ItemCellInterface> (comp) }),
              itemComponent (comp)
        {
        }

        String getTitle() const override
        {
            return itemComponent.getRepresentedItem().getAccessibilityName();
        }

        String getHelp() const override
        {
            return itemComponent.getRepresentedItem().getTooltip();
        }

        AccessibleState getCurrentState() const override
        {
            auto& treeItem = itemComponent.getRepresentedItem();

            auto state = AccessibilityHandler::getCurrentState().withAccessibleOffscreen();

            if (auto* tree = treeItem.getOwnerView())
            {
                if (tree->isMultiSelectEnabled())
                    state = state.withMultiSelectable();
                else
                    state = state.withSelectable();
            }

            if (treeItem.mightContainSubItems())
            {
                state = state.withExpandable();

                if (treeItem.isOpen())
                    state = state.withExpanded();
                else
                    state = state.withCollapsed();
            }

            if (treeItem.isSelected())
                state = state.withSelected();

            return state;
        }

        class ItemCellInterface  : public AccessibilityCellInterface
        {
        public:
            explicit ItemCellInterface (ItemComponent& c)  : itemComponent (c)  {}

            int getDisclosureLevel() const override
            {
                return getItemDepth (&itemComponent.getRepresentedItem());
            }

            std::vector<const AccessibilityHandler*> getDisclosedRows() const override
            {
                const auto& representedItem = itemComponent.getRepresentedItem();
                const auto* tree = representedItem.getOwnerView();

                if (tree == nullptr)
                    return {};

                const auto numSubItems = representedItem.isOpen() ? representedItem.getNumSubItems() : 0;

                std::vector<const AccessibilityHandler*> result;
                result.reserve ((size_t) numSubItems);

                for (auto i = 0; i < numSubItems; ++i)
                {
                    result.push_back ([&]() -> const AccessibilityHandler*
                    {
                        if (auto* subItem = representedItem.getSubItem (i))
                            if (auto* component = tree->getItemComponent (subItem))
                                return component->getAccessibilityHandler();

                        return nullptr;
                    }());
                }

                return result;
            }

            const AccessibilityHandler* getTableHandler() const override
            {
                if (auto* tree = itemComponent.getRepresentedItem().getOwnerView())
                    return tree->getAccessibilityHandler();

                return nullptr;
            }

        private:
            ItemComponent& itemComponent;
        };

    private:
        static AccessibilityActions getAccessibilityActions (ItemComponent& itemComponent)
        {
            auto onFocus = [&itemComponent]
            {
                auto& treeItem = itemComponent.getRepresentedItem();

                if (auto* tree = treeItem.getOwnerView())
                    tree->scrollToKeepItemVisible (&treeItem);
            };

            auto onPress = [&itemComponent]
            {
                itemComponent.getRepresentedItem().itemClicked (generateMouseEvent (itemComponent, { ModifierKeys::leftButtonModifier }));
            };

            auto onShowMenu = [&itemComponent]
            {
                itemComponent.getRepresentedItem().itemClicked (generateMouseEvent (itemComponent, { ModifierKeys::popupMenuClickModifier }));
            };

            auto onToggle = [&itemComponent, onFocus]
            {
                if (auto* handler = itemComponent.getAccessibilityHandler())
                {
                    auto isSelected = handler->getCurrentState().isSelected();

                    if (! isSelected)
                        onFocus();

                    itemComponent.getRepresentedItem().setSelected (! isSelected, true);
                }
            };

            auto actions = AccessibilityActions().addAction (AccessibilityActionType::focus,    std::move (onFocus))
                                                 .addAction (AccessibilityActionType::press,    std::move (onPress))
                                                 .addAction (AccessibilityActionType::showMenu, std::move (onShowMenu))
                                                 .addAction (AccessibilityActionType::toggle,   std::move (onToggle));

            return actions;
        }

        ItemComponent& itemComponent;

        static MouseEvent generateMouseEvent (ItemComponent& itemComp, ModifierKeys mods)
        {
            auto topLeft = itemComp.getRepresentedItem().getItemPosition (false).toFloat().getTopLeft();

            return { Desktop::getInstance().getMainMouseSource(), topLeft, mods,
                     MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                     MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                     &itemComp, &itemComp, Time::getCurrentTime(), topLeft, Time::getCurrentTime(), 0, false };
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemAccessibilityHandler)
    };

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        if (hasCustomComponent() && customComponent->getAccessibilityHandler() != nullptr)
            return createIgnoredAccessibilityHandler (*this);

        return std::make_unique<ItemAccessibilityHandler> (*this);
    }

    bool hasCustomComponent() const noexcept  { return customComponent.get() != nullptr; }

    TreeViewItem& item;
    std::unique_ptr<Component> customComponent;

    bool mouseIsOverButton = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};

//==============================================================================
class TreeView::ContentComponent  : public Component,
                                    public TooltipClient,
                                    public AsyncUpdater
{
public:
    ContentComponent (TreeView& tree)  : owner (tree)
    {
    }

    //==============================================================================
    void resized() override
    {
        triggerAsyncUpdate();
    }

    String getTooltip() override
    {
        if (auto* itemComponent = getItemComponentAt (getMouseXYRelative()))
            return itemComponent->getRepresentedItem().getTooltip();

        return owner.getTooltip();
    }

    void mouseDown (const MouseEvent& e) override         { mouseDownInternal        (e.getEventRelativeTo (this)); }
    void mouseUp (const MouseEvent& e) override           { mouseUpInternal          (e.getEventRelativeTo (this)); }
    void mouseDoubleClick (const MouseEvent& e) override  { mouseDoubleClickInternal (e.getEventRelativeTo (this));}
    void mouseDrag (const MouseEvent& e) override         { mouseDragInternal        (e.getEventRelativeTo (this));}
    void mouseMove (const MouseEvent& e) override         { mouseMoveInternal        (e.getEventRelativeTo (this)); }
    void mouseExit (const MouseEvent& e) override         { mouseExitInternal        (e.getEventRelativeTo (this)); }

    //==============================================================================
    ItemComponent* getItemComponentAt (Point<int> p)
    {
        auto iter = std::find_if (itemComponents.cbegin(), itemComponents.cend(),
                                  [p] (const auto& c)
                                  {
                                      return c->getBounds().contains (p);
                                  });

        if (iter != itemComponents.cend())
            return iter->get();

        return nullptr;
    }

    ItemComponent* getComponentForItem (const TreeViewItem* item) const
    {
        const auto iter = std::find_if (itemComponents.begin(), itemComponents.end(),
                                        [item] (const auto& c)
                                        {
                                            return &c->getRepresentedItem() == item;
                                        });

        if (iter != itemComponents.end())
            return iter->get();

        return nullptr;
    }

    void itemBeingDeleted (const TreeViewItem* item)
    {
        const auto iter = std::find_if (itemComponents.begin(), itemComponents.end(),
                                        [item] (const auto& c)
                                        {
                                            return &c->getRepresentedItem() == item;
                                        });

        if (iter != itemComponents.end())
        {
            if (itemUnderMouse == iter->get())
                itemUnderMouse = nullptr;

            if (isMouseDraggingInChildComp (*(iter->get())))
                owner.hideDragHighlight();

            itemComponents.erase (iter);
        }
    }

    const TreeViewItem* getItemForItemComponent (const Component* comp) const
    {
        const auto iter = itemForItemComponent.find (comp);
        return iter != itemForItemComponent.cend() ? iter->second : nullptr;
    }

    void updateComponents()
    {
        std::set<ItemComponent*> componentsToKeep;

        for (auto* treeItem : getAllVisibleItems())
        {
            if (auto* itemComp = getComponentForItem (treeItem))
            {
                componentsToKeep.insert (itemComp);
            }
            else
            {
                std::unique_ptr<ItemComponent, Deleter> newComp { new ItemComponent (*treeItem), Deleter { itemForItemComponent } };
                itemForItemComponent.emplace (newComp.get(), treeItem);

                addAndMakeVisible (*newComp);
                newComp->addMouseListener (this, treeItem->customComponentUsesTreeViewMouseHandler());
                componentsToKeep.insert (newComp.get());

                itemComponents.push_back (std::move (newComp));
            }
        }

        auto removePredicate = [&] (auto& item)
        {
            if (item == nullptr)
                return true;

            return componentsToKeep.find (item.get()) == componentsToKeep.end()
                    && ! isMouseDraggingInChildComp (*item);
        };

        const auto iter = std::remove_if (itemComponents.begin(), itemComponents.end(), std::move (removePredicate));
        itemComponents.erase (iter, itemComponents.end());

        for (auto& comp : itemComponents)
        {
            auto& treeItem = comp->getRepresentedItem();
            comp->setBounds ({ 0, treeItem.y, getWidth(), treeItem.itemHeight });
        }
    }

private:
    //==============================================================================
    struct ScopedDisableViewportScroll
    {
        explicit ScopedDisableViewportScroll (ItemComponent& c)
            : item (&c)
        {
            item->setViewportIgnoreDragFlag (true);
        }

        ~ScopedDisableViewportScroll()
        {
            if (item != nullptr)
                item->setViewportIgnoreDragFlag (false);
        }

        SafePointer<ItemComponent> item;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedDisableViewportScroll)
    };

    //==============================================================================
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    void mouseDownInternal (const MouseEvent& e)
    {
        updateItemUnderMouse (e);

        isDragging = false;
        scopedScrollDisabler = nullopt;
        needSelectionOnMouseUp = false;

        if (! isEnabled())
            return;

        if (auto* itemComponent = getItemComponentAt (e.getPosition()))
        {
            auto& item = itemComponent->getRepresentedItem();
            auto pos = item.getItemPosition (false);

            // (if the open/close buttons are hidden, we'll treat clicks to the left of the item
            // as selection clicks)
            if (e.x < pos.getX() && owner.openCloseButtonsVisible)
            {
                // (clicks to the left of an open/close button are ignored)
                if (e.x >= pos.getX() - owner.getIndentSize())
                    item.setOpen (! item.isOpen());
            }
            else
            {
                // mouse-down inside the body of the item..
                if (! owner.isMultiSelectEnabled())
                    item.setSelected (true, true);
                else if (item.isSelected())
                    needSelectionOnMouseUp = ! e.mods.isPopupMenu();
                else
                    selectBasedOnModifiers (item, e.mods);

                if (e.x >= pos.getX())
                    item.itemClicked (e.withNewPosition (e.position - pos.getPosition().toFloat()));
            }
        }
    }

    void mouseUpInternal (const MouseEvent& e)
    {
        updateItemUnderMouse (e);

        if (isEnabled() && needSelectionOnMouseUp && e.mouseWasClicked())
            if (auto* itemComponent = getItemComponentAt (e.getPosition()))
                selectBasedOnModifiers (itemComponent->getRepresentedItem(), e.mods);
    }

    void mouseDoubleClickInternal (const MouseEvent& e)
    {
        if (isEnabled() && e.getNumberOfClicks() != 3)  // ignore triple clicks
        {
            if (auto* itemComponent = getItemComponentAt (e.getPosition()))
            {
                auto& item = itemComponent->getRepresentedItem();
                auto pos = item.getItemPosition (false);

                if (e.x >= pos.getX() || ! owner.openCloseButtonsVisible)
                    item.itemDoubleClicked (e.withNewPosition (e.position - pos.getPosition().toFloat()));
            }
        }
    }

    void mouseDragInternal (const MouseEvent& e)
    {
        if (isEnabled()
             && ! (isDragging || e.mouseWasClicked()
                    || e.getDistanceFromDragStart() < 5
                    || e.mods.isPopupMenu()))
        {
            isDragging = true;

            if (auto* itemComponent = getItemComponentAt (e.getMouseDownPosition()))
            {
                auto& item = itemComponent->getRepresentedItem();
                auto pos = item.getItemPosition (false);

                if (e.getMouseDownX() >= pos.getX())
                {
                    auto dragDescription = item.getDragSourceDescription();

                    if (! (dragDescription.isVoid() || (dragDescription.isString() && dragDescription.toString().isEmpty())))
                    {
                        if (auto* dragContainer = DragAndDropContainer::findParentDragContainerFor (this))
                        {
                            pos.setSize (pos.getWidth(), item.itemHeight);

                            const auto additionalScale = 2.0f;
                            auto dragImage = Component::createComponentSnapshot (pos,
                                                                                 true,
                                                                                 Component::getApproximateScaleFactorForComponent (itemComponent) * additionalScale);

                            dragImage.multiplyAllAlphas (0.6f);

                            auto imageOffset = pos.getPosition() - e.getPosition();
                            dragContainer->startDragging (dragDescription, &owner, { dragImage, additionalScale }, true, &imageOffset, &e.source);

                            scopedScrollDisabler.emplace (*itemComponent);
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
    }

    void mouseMoveInternal (const MouseEvent& e)  { updateItemUnderMouse (e); }
    void mouseExitInternal (const MouseEvent& e)  { updateItemUnderMouse (e); }

    static bool isMouseDraggingInChildComp (const Component& comp)
    {
        for (auto& ms : Desktop::getInstance().getMouseSources())
            if (ms.isDragging())
                if (auto* underMouse = ms.getComponentUnderMouse())
                    return (&comp == underMouse || comp.isParentOf (underMouse));

        return false;
    }

    void updateItemUnderMouse (const MouseEvent& e)
    {
        if (! owner.openCloseButtonsVisible)
            return;

        auto* newItem = [this, &e]() -> ItemComponent*
        {
            if (auto* itemComponent = getItemComponentAt (e.getPosition()))
            {
                auto& item = itemComponent->getRepresentedItem();

                if (item.mightContainSubItems())
                {
                    const auto xPos = item.getItemPosition (false).getX();

                    if (xPos - owner.getIndentSize() <= e.x && e.x < xPos)
                        return itemComponent;
                }
            }

            return nullptr;
        }();

        if (itemUnderMouse != newItem)
        {
            if (itemUnderMouse != nullptr)
                itemUnderMouse->setMouseIsOverButton (false);

            if (newItem != nullptr)
                newItem->setMouseIsOverButton (true);

            itemUnderMouse = newItem;
        }
    }

    void handleAsyncUpdate() override
    {
        owner.updateVisibleItems();
    }

    //==============================================================================
    void selectBasedOnModifiers (TreeViewItem& item, const ModifierKeys modifiers)
    {
        TreeViewItem* firstSelected = nullptr;

        if (modifiers.isShiftDown() && ((firstSelected = owner.getSelectedItem (0)) != nullptr))
        {
            auto* lastSelected = owner.getSelectedItem (owner.getNumSelectedItems() - 1);

            if (lastSelected == nullptr)
            {
                jassertfalse;
                return;
            }

            auto rowStart = firstSelected->getRowNumberInTree();
            auto rowEnd = lastSelected->getRowNumberInTree();

            if (rowStart > rowEnd)
                std::swap (rowStart, rowEnd);

            auto ourRow = item.getRowNumberInTree();
            auto otherEnd = ourRow < rowEnd ? rowStart : rowEnd;

            if (ourRow > otherEnd)
                std::swap (ourRow, otherEnd);

            for (int i = ourRow; i <= otherEnd; ++i)
                owner.getItemOnRow (i)->setSelected (true, false);
        }
        else
        {
            const auto cmd = modifiers.isCommandDown();
            item.setSelected ((! cmd) || ! item.isSelected(), ! cmd);
        }
    }

    static TreeViewItem* getNextVisibleItem (TreeViewItem* item, bool forwards)
    {
        if (item == nullptr || item->ownerView == nullptr)
            return nullptr;

        auto* nextItem = item->ownerView->getItemOnRow (item->getRowNumberInTree() + (forwards ? 1 : -1));

        return nextItem == item->ownerView->rootItem && ! item->ownerView->rootItemVisible ? nullptr
                                                                                           : nextItem;
    }

    std::vector<TreeViewItem*> getAllVisibleItems() const
    {
        if (owner.rootItem == nullptr)
            return {};

        const auto visibleTop = -getY();
        const auto visibleBottom = visibleTop + getParentHeight();

        std::vector<TreeViewItem*> visibleItems;

        auto* item = [&]
        {
            auto* i = owner.rootItemVisible ? owner.rootItem
                                            : owner.rootItem->subItems.getFirst();

            while (i != nullptr && i->y < visibleTop)
                i = getNextVisibleItem (i, true);

            return i;
        }();

        auto addOffscreenItemBuffer = [&visibleItems] (TreeViewItem* i, int num, bool forwards)
        {
            while (--num >= 0)
            {
                i = getNextVisibleItem (i, forwards);

                if (i == nullptr)
                    return;

                visibleItems.push_back (i);
            }
        };

        addOffscreenItemBuffer (item, 2, false);

        while (item != nullptr && item->y < visibleBottom)
        {
            visibleItems.push_back (item);
            item = getNextVisibleItem (item, true);
        }

        if (item != nullptr)
            visibleItems.push_back (item);

        addOffscreenItemBuffer (item, 2, true);

        return visibleItems;
    }

    //==============================================================================
    class Deleter
    {
    public:
        explicit Deleter (std::map<const Component*, const TreeViewItem*>& map)
            : itemForItemComponent (&map) {}

        void operator() (ItemComponent* ptr) const
        {
            itemForItemComponent->erase (ptr);

            if (ptr != nullptr)
                delete ptr;
        }

    private:
        std::map<const Component*, const TreeViewItem*>* itemForItemComponent = nullptr;
    };

    //==============================================================================
    TreeView& owner;

    std::map<const Component*, const TreeViewItem*> itemForItemComponent;
    std::vector<std::unique_ptr<ItemComponent, Deleter>> itemComponents;
    ItemComponent* itemUnderMouse = nullptr;
    Optional<ScopedDisableViewportScroll> scopedScrollDisabler;
    bool isDragging = false, needSelectionOnMouseUp = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentComponent)
};

//==============================================================================
class TreeView::TreeViewport  : public Viewport,
                                private AsyncUpdater
{
public:
    explicit TreeViewport (TreeView& treeView)  : owner (treeView)  {}

    void visibleAreaChanged (const Rectangle<int>& newVisibleArea) override
    {
        const auto hasScrolledSideways = (newVisibleArea.getX() != lastX);

        lastX = newVisibleArea.getX();
        updateComponents (hasScrolledSideways);

        structureChanged = true;
        triggerAsyncUpdate();
    }

    bool keyPressed (const KeyPress& key) override
    {
        if (auto* tree = getParentComponent())
            if (tree->keyPressed (key))
                return true;

        return Viewport::keyPressed (key);
    }

    ContentComponent* getContentComp() const noexcept
    {
        return static_cast<ContentComponent*> (getViewedComponent());
    }

    enum class Async { yes, no };

    void recalculatePositions (Async useAsyncUpdate, std::optional<Point<int>> viewportPosition)
    {
        needsRecalculating = true;
        viewportAfterRecalculation = std::move (viewportPosition);

        if (useAsyncUpdate == Async::yes)
            triggerAsyncUpdate();
        else
            handleAsyncUpdate();
    }

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    void handleAsyncUpdate() override
    {
        if (std::exchange (structureChanged, false))
        {
            if (auto* handler = owner.getAccessibilityHandler())
                handler->notifyAccessibilityEvent (AccessibilityEvent::structureChanged);
        }

        if (std::exchange (needsRecalculating, false))
        {
            if (auto* root = owner.rootItem)
            {
                const auto startY = owner.rootItemVisible ? 0 : -root->itemHeight;

                root->updatePositions (startY);
                getViewedComponent()->setSize (jmax (getMaximumVisibleWidth(), root->totalWidth + 50),
                                               root->totalHeight + startY);
            }
            else
            {
                getViewedComponent()->setSize (0, 0);
            }

            updateComponents (false);

            if (const auto viewportPosition = std::exchange (viewportAfterRecalculation, {}))
                setViewPosition (viewportPosition->getX(), viewportPosition->getY());
        }
    }

    void updateComponents (bool triggerResize)
    {
        if (auto* content = getContentComp())
        {
            if (triggerResize)
                content->resized();
            else
                content->updateComponents();
        }

        repaint();
    }

    TreeView& owner;
    int lastX = -1;
    bool structureChanged = false, needsRecalculating = false;
    std::optional<Point<int>> viewportAfterRecalculation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeViewport)
};

//==============================================================================
TreeView::TreeView (const String& name)  : Component (name)
{
    viewport = std::make_unique<TreeViewport> (*this);
    addAndMakeVisible (viewport.get());
    viewport->setViewedComponent (new ContentComponent (*this));

    setWantsKeyboardFocus (true);
    setFocusContainerType (FocusContainerType::focusContainer);
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
            // can't use a tree item in more than one tree at once..
            jassert (newRootItem->ownerView == nullptr);

            if (newRootItem->ownerView != nullptr)
                newRootItem->ownerView->setRootItem (nullptr);
        }

        if (rootItem != nullptr)
            rootItem->setOwnerView (nullptr);

        rootItem = newRootItem;

        if (newRootItem != nullptr)
            newRootItem->setOwnerView (this);

        if (rootItem != nullptr && (defaultOpenness || ! rootItemVisible))
        {
            rootItem->setOpen (false); // force a re-open
            rootItem->setOpen (true);
        }

        viewport->recalculatePositions (TreeViewport::Async::no, {});
    }
}

void TreeView::deleteRootItem()
{
    const std::unique_ptr<TreeViewItem> deleter (rootItem);
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

    updateVisibleItems();
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
        updateVisibleItems();
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
        updateVisibleItems();
    }
}

Viewport* TreeView::getViewport() const noexcept
{
    return viewport.get();
}

//==============================================================================
void TreeView::clearSelectedItems()
{
    if (rootItem != nullptr)
        rootItem->deselectAllRecursively (nullptr);
}

int TreeView::getNumSelectedItems (int maximumDepthToSearchTo) const noexcept
{
    return rootItem != nullptr ? rootItem->countSelectedItemsRecursively (maximumDepthToSearchTo) : 0;
}

TreeViewItem* TreeView::getSelectedItem (const int index) const noexcept
{
    return rootItem != nullptr ? rootItem->getSelectedItemWithIndex (index) : nullptr;
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
    if (auto* contentComp = viewport->getContentComp())
        if (auto* itemComponent = contentComp->getItemComponentAt (contentComp->getLocalPoint (this, Point<int> (0, y))))
            return &itemComponent->getRepresentedItem();

    return nullptr;
}

TreeViewItem* TreeView::findItemFromIdentifierString (const String& identifierString) const
{
    if (rootItem == nullptr)
        return nullptr;

    return rootItem->findItemFromIdentifierString (identifierString);
}

Component* TreeView::getItemComponent (const TreeViewItem* item) const
{
    return viewport->getContentComp()->getComponentForItem (item);
}

//==============================================================================
static void addAllSelectedItemIds (TreeViewItem* item, XmlElement& parent)
{
    if (item->isSelected())
        parent.createNewChildElement ("SELECTED")->setAttribute ("id", item->getItemIdentifierString());

    auto numSubItems = item->getNumSubItems();

    for (int i = 0; i < numSubItems; ++i)
        addAllSelectedItemIds (item->getSubItem (i), parent);
}

std::unique_ptr<XmlElement> TreeView::getOpennessState (bool alsoIncludeScrollPosition) const
{
    if (rootItem != nullptr)
    {
        if (auto rootOpenness = rootItem->getOpennessState (false))
        {
            if (alsoIncludeScrollPosition)
                rootOpenness->setAttribute ("scrollPos", viewport->getViewPositionY());

            addAllSelectedItemIds (rootItem, *rootOpenness);
            return rootOpenness;
        }
    }

    return {};
}

void TreeView::restoreOpennessState (const XmlElement& newState, bool restoreStoredSelection)
{
    if (rootItem != nullptr)
    {
        rootItem->restoreOpennessState (newState);

        if (restoreStoredSelection)
        {
            clearSelectedItems();

            for (auto* e : newState.getChildWithTagNameIterator ("SELECTED"))
                if (auto* item = rootItem->findItemFromIdentifierString (e->getStringAttribute ("id")))
                    item->setSelected (true, false);
        }

        const auto scrollPos = newState.hasAttribute ("scrollPos")
                             ? std::make_optional<Point<int>> (viewport->getViewPositionX(), newState.getIntAttribute ("scrollPos"))
                             : std::nullopt;

        updateVisibleItems (std::move (scrollPos));
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
    updateVisibleItems();
}

void TreeView::enablementChanged()
{
    repaint();
}

void TreeView::moveSelectedRow (int delta)
{
    auto numRowsInTree = getNumRowsInTree();

    if (numRowsInTree > 0)
    {
        int rowSelected = 0;

        if (auto* firstSelected = getSelectedItem (0))
            rowSelected = firstSelected->getRowNumberInTree();

        rowSelected = jlimit (0, numRowsInTree - 1, rowSelected + delta);

        for (;;)
        {
            if (auto* item = getItemOnRow (rowSelected))
            {
                if (! item->canBeSelected())
                {
                    // if the row we want to highlight doesn't allow it, try skipping
                    // to the next item..
                    auto nextRowToTry = jlimit (0, numRowsInTree - 1, rowSelected + (delta < 0 ? -1 : 1));

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

void TreeView::scrollToKeepItemVisible (const TreeViewItem* item)
{
    if (item != nullptr && item->ownerView == this)
    {
        updateVisibleItems();

        item = item->getDeepestOpenParentItem();

        auto y = item->y;
        auto viewTop = viewport->getViewPositionY();

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

bool TreeView::toggleOpenSelectedItem()
{
    if (auto* firstSelected = getSelectedItem (0))
    {
        if (firstSelected->mightContainSubItems())
        {
            firstSelected->setOpen (! firstSelected->isOpen());
            return true;
        }
    }

    return false;
}

void TreeView::moveOutOfSelectedItem()
{
    if (auto* firstSelected = getSelectedItem (0))
    {
        if (firstSelected->isOpen())
        {
            firstSelected->setOpen (false);
        }
        else
        {
            auto* parent = firstSelected->parentItem;

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
    if (auto* firstSelected = getSelectedItem (0))
    {
        if (firstSelected->isOpen() || ! firstSelected->mightContainSubItems())
            moveSelectedRow (1);
        else
            firstSelected->setOpen (true);
    }
}

void TreeView::moveByPages (int numPages)
{
    if (auto* currentItem = getSelectedItem (0))
    {
        auto pos = currentItem->getItemPosition (false);
        auto targetY = pos.getY() + numPages * (getHeight() - pos.getHeight());
        auto currentRow = currentItem->getRowNumberInTree();

        for (;;)
        {
            moveSelectedRow (numPages);
            currentItem = getSelectedItem (0);

            if (currentItem == nullptr)
                break;

            auto y = currentItem->getItemPosition (false).getY();

            if ((numPages < 0 && y <= targetY) || (numPages > 0 && y >= targetY))
                break;

            auto newRow = currentItem->getRowNumberInTree();

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
        if (key == KeyPress::returnKey)   { return toggleOpenSelectedItem(); }
        if (key == KeyPress::leftKey)     { moveOutOfSelectedItem();  return true; }
        if (key == KeyPress::rightKey)    { moveIntoSelectedItem();   return true; }
    }

    return false;
}

void TreeView::updateVisibleItems (std::optional<Point<int>> viewportPosition)
{
    viewport->recalculatePositions (TreeViewport::Async::yes, std::move (viewportPosition));
}

//==============================================================================
struct TreeView::InsertPoint
{
    InsertPoint (TreeView& view, const StringArray& files,
                 const DragAndDropTarget::SourceDetails& dragSourceDetails)
        : pos (dragSourceDetails.localPosition),
          item (view.getItemAt (dragSourceDetails.localPosition.y))
    {
        if (item != nullptr)
        {
            auto itemPos = item->getItemPosition (true);
            insertIndex = item->getIndexInParent();
            auto oldY = pos.y;
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
        else if (auto* root = view.getRootItem())
        {
            // If they're dragging beyond the bottom of the list, then insert at the end of the root item..
            item = root;
            insertIndex = root->getNumSubItems();
            pos = root->getItemPosition (true).getBottomLeft();
            pos.x += view.getIndentSize();
        }
    }

    Point<int> pos;
    TreeViewItem* item;
    int insertIndex = 0;
};

//==============================================================================
class TreeView::InsertPointHighlight   : public Component
{
public:
    InsertPointHighlight()
    {
        setSize (100, 12);
        setAlwaysOnTop (true);
        setInterceptsMouseClicks (false, false);
    }

    void setTargetPosition (const InsertPoint& insertPos, const int width) noexcept
    {
        lastItem = insertPos.item;
        lastIndex = insertPos.insertIndex;
        auto offset = getHeight() / 2;
        setBounds (insertPos.pos.x - offset, insertPos.pos.y - offset,
                   width - (insertPos.pos.x - offset), getHeight());
    }

    void paint (Graphics& g) override
    {
        Path p;
        auto h = (float) getHeight();
        p.addEllipse (2.0f, 2.0f, h - 4.0f, h - 4.0f);
        p.startNewSubPath (h - 2.0f, h / 2.0f);
        p.lineTo ((float) getWidth(), h / 2.0f);

        g.setColour (findColour (TreeView::dragAndDropIndicatorColourId, true));
        g.strokePath (p, PathStrokeType (2.0f));
    }

    TreeViewItem* lastItem = nullptr;
    int lastIndex = 0;

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
        setBounds (item->getItemPosition (true)
                     .withHeight (item->getItemHeight()));
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (TreeView::dragAndDropIndicatorColourId, true));
        g.drawRoundedRectangle (1.0f, 1.0f, (float) getWidth() - 2.0f, (float) getHeight() - 2.0f, 3.0f, 2.0f);
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
        dragInsertPointHighlight = std::make_unique<InsertPointHighlight>();
        dragTargetGroupHighlight = std::make_unique<TargetGroupHighlight>();

        addAndMakeVisible (dragInsertPointHighlight.get());
        addAndMakeVisible (dragTargetGroupHighlight.get());
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
    const auto scrolled = viewport->autoScroll (dragSourceDetails.localPosition.x,
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
    handleDrag (files, SourceDetails (var(), this, { x, y }));
}

void TreeView::fileDragExit (const StringArray&)
{
    hideDragHighlight();
}

void TreeView::filesDropped (const StringArray& files, int x, int y)
{
    handleDrop (files, SourceDetails (var(), this, { x, y }));
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
std::unique_ptr<AccessibilityHandler> TreeView::createAccessibilityHandler()
{
    class TableInterface  : public AccessibilityTableInterface
    {
    public:
        explicit TableInterface (TreeView& treeViewToWrap)  : treeView (treeViewToWrap) {}

        int getNumRows() const override     { return treeView.getNumRowsInTree(); }
        int getNumColumns() const override  { return 1; }

        const AccessibilityHandler* getHeaderHandler() const override
        {
            return nullptr;
        }

        const AccessibilityHandler* getRowHandler (int row) const override
        {
            if (auto* itemComp = treeView.getItemComponent (treeView.getItemOnRow (row)))
                return itemComp->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getCellHandler (int, int) const override
        {
            return nullptr;
        }

        Optional<Span> getRowSpan (const AccessibilityHandler& handler) const override
        {
            auto* item = getItemForHandler (handler);

            if (item == nullptr)
                return nullopt;

            const auto rowNumber = item->getRowNumberInTree();

            return rowNumber != -1 ? makeOptional (Span { rowNumber, 1 })
                                   : nullopt;
        }

        Optional<Span> getColumnSpan (const AccessibilityHandler&) const override
        {
            return Span { 0, 1 };
        }

        void showCell (const AccessibilityHandler& cellHandler) const override
        {
            treeView.scrollToKeepItemVisible (getItemForHandler (cellHandler));
        }

    private:
        const TreeViewItem* getItemForHandler (const AccessibilityHandler& handler) const
        {
            for (auto* comp = &handler.getComponent(); comp != &treeView; comp = comp->getParentComponent())
                if (auto* result = treeView.viewport->getContentComp()->getItemForItemComponent (comp))
                    return result;

            return nullptr;
        }

        TreeView& treeView;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableInterface)
    };

    return std::make_unique<AccessibilityHandler> (*this,
                                                   AccessibilityRole::tree,
                                                   AccessibilityActions{},
                                                   AccessibilityHandler::Interfaces { std::make_unique<TableInterface> (*this) });
}

//==============================================================================
TreeViewItem::TreeViewItem()
{
    static int nextUID = 0;
    uid = nextUID++;
}

TreeViewItem::~TreeViewItem()
{
    if (ownerView != nullptr)
        ownerView->viewport->getContentComp()->itemBeingDeleted (this);
}

String TreeViewItem::getUniqueName() const
{
    return {};
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
    return subItems[index];
}

void TreeViewItem::clearSubItems()
{
    if (ownerView != nullptr)
    {
        if (! subItems.isEmpty())
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
        newItem->parentItem = nullptr;
        newItem->setOwnerView (ownerView);
        newItem->y = 0;
        newItem->itemHeight = newItem->getItemHeight();
        newItem->totalHeight = 0;
        newItem->itemWidth = newItem->getItemWidth();
        newItem->totalWidth = 0;
        newItem->parentItem = this;

        if (ownerView != nullptr)
        {
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
    if (auto* child = subItems[index])
    {
        child->parentItem = nullptr;
        subItems.remove (index, deleteItem);

        return true;
    }

    return false;
}

TreeViewItem::Openness TreeViewItem::getOpenness() const noexcept
{
    return openness;
}

void TreeViewItem::setOpenness (Openness newOpenness)
{
    auto wasOpen = isOpen();
    openness = newOpenness;
    auto isNowOpen = isOpen();

    if (isNowOpen != wasOpen)
    {
        treeHasChanged();
        itemOpennessChanged (isNowOpen);
    }
}

bool TreeViewItem::isOpen() const noexcept
{
    if (openness == Openness::opennessDefault)
        return ownerView != nullptr && ownerView->defaultOpenness;

    return openness == Openness::opennessOpen;
}

void TreeViewItem::setOpen (const bool shouldBeOpen)
{
    if (isOpen() != shouldBeOpen)
        setOpenness (shouldBeOpen ? Openness::opennessOpen
                                  : Openness::opennessClosed);
}

bool TreeViewItem::isFullyOpen() const noexcept
{
    if (! isOpen())
        return false;

    for (auto* i : subItems)
        if (! i->isFullyOpen())
            return false;

    return true;
}

void TreeViewItem::restoreToDefaultOpenness()
{
    setOpenness (Openness::opennessDefault);
}

bool TreeViewItem::isSelected() const noexcept
{
    return selected;
}

void TreeViewItem::deselectAllRecursively (TreeViewItem* itemToIgnore)
{
    if (this != itemToIgnore)
        setSelected (false, false);

    for (auto* i : subItems)
        i->deselectAllRecursively (itemToIgnore);
}

void TreeViewItem::setSelected (const bool shouldBeSelected,
                                const bool deselectOtherItemsFirst,
                                const NotificationType notify)
{
    if (shouldBeSelected && ! canBeSelected())
        return;

    if (deselectOtherItemsFirst)
        getTopLevelItem()->deselectAllRecursively (this);

    if (shouldBeSelected != selected)
    {
        selected = shouldBeSelected;

        if (ownerView != nullptr)
        {
            ownerView->repaint();

            if (selected)
            {
                if (auto* itemComponent = ownerView->getItemComponent (this))
                    if (auto* itemHandler = itemComponent->getAccessibilityHandler())
                        itemHandler->grabFocus();
            }

            if (auto* handler = ownerView->getAccessibilityHandler())
                handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
        }

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
    return {};
}

String TreeViewItem::getAccessibilityName()
{
    auto tooltipString = getTooltip();

    return tooltipString.isNotEmpty()
      ? tooltipString
      : "Level " + String (getItemDepth (this)) + " row " + String (getIndexInParent());
}

void TreeViewItem::ownerViewChanged (TreeView*)
{
}

var TreeViewItem::getDragSourceDescription()
{
    return {};
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
    auto indentX = getIndentX();
    auto width = itemWidth;

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
        ownerView->updateVisibleItems();
}

void TreeViewItem::repaintItem() const
{
    if (ownerView != nullptr && areAllParentsOpen())
        if (auto* component = ownerView->getItemComponent (this))
            component->repaint();
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

        for (auto* i : subItems)
        {
            i->updatePositions (newY);
            newY += i->totalHeight;
            totalHeight += i->totalHeight;
            totalWidth = jmax (totalWidth, i->totalWidth);
        }
    }
}

const TreeViewItem* TreeViewItem::getDeepestOpenParentItem() const noexcept
{
    auto* result = this;
    auto* item = this;

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

    for (auto* i : subItems)
    {
        i->setOwnerView (newOwner);
        i->ownerViewChanged (newOwner);
    }
}

int TreeViewItem::getIndentX() const noexcept
{
    if (ownerView == nullptr)
        return 0;

    int x = ownerView->rootItemVisible ? 1 : 0;

    if (! ownerView->openCloseButtonsVisible)
        --x;

    for (auto* p = parentItem; p != nullptr; p = p->parentItem)
        ++x;

    return x * ownerView->getIndentSize();
}

void TreeViewItem::setDrawsInLeftMargin (bool canDrawInLeftMargin) noexcept
{
    drawsInLeftMargin = canDrawInLeftMargin;
}

void TreeViewItem::setDrawsInRightMargin (bool canDrawInRightMargin) noexcept
{
    drawsInRightMargin = canDrawInRightMargin;
}

bool TreeViewItem::areLinesDrawn() const
{
    return drawLinesSet ? drawLinesInside
                        : (ownerView != nullptr && ownerView->getLookAndFeel().areLinesDrawnForTreeView (*ownerView));
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
        for (auto* i : subItems)
            num += i->getNumRows();

    return num;
}

TreeViewItem* TreeViewItem::getItemOnRow (int index) noexcept
{
    if (index == 0)
        return this;

    if (index > 0 && isOpen())
    {
        --index;

        for (auto* i : subItems)
        {
            if (index == 0)
                return i;

            auto numRows = i->getNumRows();

            if (numRows > index)
                return i->getItemOnRow (index);

            index -= numRows;
        }
    }

    return nullptr;
}

int TreeViewItem::countSelectedItemsRecursively (int depth) const noexcept
{
    int total = isSelected() ? 1 : 0;

    if (depth != 0)
        for (auto* i : subItems)
            total += i->countSelectedItemsRecursively (depth - 1);

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
        for (auto* i : subItems)
        {
            if (auto* found = i->getSelectedItemWithIndex (index))
                return found;

            index -= i->countSelectedItemsRecursively (-1);
        }
    }

    return nullptr;
}

int TreeViewItem::getRowNumberInTree() const noexcept
{
    if (parentItem != nullptr && ownerView != nullptr)
    {
        if (! parentItem->isOpen())
            return parentItem->getRowNumberInTree();

        auto n = 1 + parentItem->getRowNumberInTree();

        auto ourIndex = parentItem->subItems.indexOf (this);
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

void TreeViewItem::setLinesDrawnForSubItems (bool drawLines) noexcept
{
    drawLinesInside = drawLines;
    drawLinesSet = true;
}

static String escapeSlashesInTreeViewItemName (const String& s)
{
    return s.replaceCharacter ('/', '\\');
}

String TreeViewItem::getItemIdentifierString() const
{
    String s;

    if (parentItem != nullptr)
        s = parentItem->getItemIdentifierString();

    return s + "/" + escapeSlashesInTreeViewItemName (getUniqueName());
}

TreeViewItem* TreeViewItem::findItemFromIdentifierString (const String& identifierString)
{
    auto thisId = "/" + escapeSlashesInTreeViewItemName (getUniqueName());

    if (thisId == identifierString)
        return this;

    if (identifierString.startsWith (thisId + "/"))
    {
        auto remainingPath = identifierString.substring (thisId.length());

        const auto wasOpen = isOpen();
        setOpen (true);

        for (auto* i : subItems)
            if (auto* item = i->findItemFromIdentifierString (remainingPath))
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

        for (auto* n : e.getChildIterator())
        {
            auto id = n->getStringAttribute ("id");

            for (int i = 0; i < items.size(); ++i)
            {
                auto* ti = items.getUnchecked (i);

                if (ti->getUniqueName() == id)
                {
                    ti->restoreOpennessState (*n);
                    items.remove (i);
                    break;
                }
            }
        }

        // for any items that weren't mentioned in the XML, reset them to default:
        for (auto* i : items)
            i->restoreToDefaultOpenness();
    }
}

std::unique_ptr<XmlElement> TreeViewItem::getOpennessState() const
{
    return getOpennessState (true);
}

std::unique_ptr<XmlElement> TreeViewItem::getOpennessState (bool canReturnNull) const
{
    auto name = getUniqueName();

    if (name.isNotEmpty())
    {
        std::unique_ptr<XmlElement> e;

        if (isOpen())
        {
            if (canReturnNull && ownerView != nullptr && ownerView->defaultOpenness && isFullyOpen())
                return nullptr;

            e = std::make_unique<XmlElement> ("OPEN");

            for (int i = subItems.size(); --i >= 0;)
                e->prependChildElement (subItems.getUnchecked (i)->getOpennessState (true).release());
        }
        else
        {
            if (canReturnNull && ownerView != nullptr && ! ownerView->defaultOpenness)
                return nullptr;

            e = std::make_unique<XmlElement> ("CLOSED");
        }

        e->setAttribute ("id", name);
        return e;
    }

    // trying to save the openness for an element that has no name - this won't
    // work because it needs the names to identify what to open.
    jassertfalse;
    return {};
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

void TreeViewItem::draw (Graphics& g, int width, bool isMouseOverButton)
{
    if (ownerView == nullptr)
        return;

    const auto indent = getIndentX();
    const auto itemW = (itemWidth < 0 || drawsInRightMargin) ? width - indent : itemWidth;

    {
        Graphics::ScopedSaveState ss (g);
        g.setOrigin (indent, 0);

        if (g.reduceClipRegion (drawsInLeftMargin ? -indent : 0, 0,
                                drawsInLeftMargin ? itemW + indent : itemW, itemHeight))
        {
            if (isSelected())
                g.fillAll (ownerView->findColour (TreeView::selectedItemBackgroundColourId));
            else
                g.fillAll ((getRowNumberInTree() % 2 == 0) ? ownerView->findColour (TreeView::oddItemsColourId)
                                                           : ownerView->findColour (TreeView::evenItemsColourId));

            paintItem (g, itemWidth < 0 ? width - indent : itemWidth, itemHeight);
        }
    }

    const auto halfH = (float) itemHeight * 0.5f;
    const auto indentWidth = ownerView->getIndentSize();
    const auto depth = getItemDepth (this);

    if (depth >= 0 && ownerView->openCloseButtonsVisible)
    {
        auto x = ((float) depth + 0.5f) * (float) indentWidth;
        const auto parentLinesDrawn = parentItem != nullptr && parentItem->areLinesDrawn();

        if (parentLinesDrawn)
            paintVerticalConnectingLine (g, Line<float> (x, 0, x, isLastOfSiblings() ? halfH : (float) itemHeight));

        if (parentLinesDrawn || (parentItem == nullptr && areLinesDrawn()))
            paintHorizontalConnectingLine (g, Line<float> (x, halfH, x + (float) indentWidth * 0.5f, halfH));

        {
            auto* p = parentItem;
            auto d = depth;

            while (p != nullptr && --d >= 0)
            {
                x -= (float) indentWidth;

                if ((p->parentItem == nullptr || p->parentItem->areLinesDrawn()) && ! p->isLastOfSiblings())
                    p->paintVerticalConnectingLine (g, Line<float> (x, 0, x, (float) itemHeight));

                p = p->parentItem;
            }
        }

        if (mightContainSubItems())
        {
            auto backgroundColour = ownerView->findColour (TreeView::backgroundColourId);

            paintOpenCloseButton (g, Rectangle<float> ((float) (depth * indentWidth), 0, (float) indentWidth, (float) itemHeight),
                                  backgroundColour.isTransparent() ? Colours::white : backgroundColour,
                                  isMouseOverButton);
        }
    }
}

} // namespace juce
