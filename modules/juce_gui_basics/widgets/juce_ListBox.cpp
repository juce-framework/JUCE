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

template <typename RowComponentType>
static AccessibilityActions getListRowAccessibilityActions (RowComponentType& rowComponent)
{
    auto onFocus = [&rowComponent]
    {
        rowComponent.getOwner().scrollToEnsureRowIsOnscreen (rowComponent.getRow());
        rowComponent.getOwner().selectRow (rowComponent.getRow());
    };

    auto onPress = [&rowComponent, onFocus]
    {
        onFocus();
        rowComponent.getOwner().keyPressed (KeyPress (KeyPress::returnKey));
    };

    auto onToggle = [&rowComponent]
    {
        rowComponent.getOwner().flipRowSelection (rowComponent.getRow());
    };

    return AccessibilityActions().addAction (AccessibilityActionType::focus,  std::move (onFocus))
                                 .addAction (AccessibilityActionType::press,  std::move (onPress))
                                 .addAction (AccessibilityActionType::toggle, std::move (onToggle));
}

void ListBox::checkModelPtrIsValid() const
{
   #if ! JUCE_DISABLE_ASSERTIONS
    // If this is hit, the model was destroyed while the ListBox was still using it.
    // You should ensure that the model remains alive for as long as the ListBox holds a pointer to it.
    // If this assertion is hit in the destructor of a ListBox instance, do one of the following:
    // - Adjust the order in which your destructors run, so that the ListBox destructor runs
    //   before the destructor of your ListBoxModel, or
    // - Call ListBox::setModel (nullptr) before destroying your ListBoxModel.
    jassert ((model == nullptr) == (weakModelPtr.lock() == nullptr));
   #endif
}

//==============================================================================
/*  The ListBox and TableListBox rows both have similar mouse behaviours, which are implemented here. */
template <typename Base>
class ComponentWithListRowMouseBehaviours : public Component
{
    auto& getOwner() const { return asBase().getOwner(); }

public:
    void updateRowAndSelection (const int newRow, const bool nowSelected)
    {
        const auto rowChanged       = std::exchange (row,      newRow)      != newRow;
        const auto selectionChanged = std::exchange (selected, nowSelected) != nowSelected;

        if (rowChanged || selectionChanged)
            repaint();
    }

    void mouseDown (const MouseEvent& e) override
    {
        isDragging = false;
        isDraggingToScroll = false;
        selectRowOnMouseUp = false;

        if (! asBase().isEnabled())
            return;

        const auto select = getOwner().getRowSelectedOnMouseDown()
                            && ! selected
                            && ! detail::ViewportHelpers::wouldScrollOnEvent (getOwner().getViewport(), e.source) ;
        if (select)
            asBase().performSelection (e, false);
        else
            selectRowOnMouseUp = true;
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (asBase().isEnabled() && selectRowOnMouseUp && ! (isDragging || isDraggingToScroll))
            asBase().performSelection (e, true);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (auto* m = getModel (getOwner()))
        {
            if (asBase().isEnabled() && e.mouseWasDraggedSinceMouseDown() && ! isDragging)
            {
                SparseSet<int> rowsToDrag;

                if (getOwner().getRowSelectedOnMouseDown() || getOwner().isRowSelected (row))
                    rowsToDrag = getOwner().getSelectedRows();
                else
                    rowsToDrag.addRange (Range<int>::withStartAndLength (row, 1));

                if (! rowsToDrag.isEmpty())
                {
                    auto dragDescription = m->getDragSourceDescription (rowsToDrag);

                    if (! (dragDescription.isVoid() || (dragDescription.isString() && dragDescription.toString().isEmpty())))
                    {
                        isDragging = true;
                        getOwner().startDragAndDrop (e, rowsToDrag, dragDescription, m->mayDragToExternalWindows());
                    }
                }
            }
        }

        if (! isDraggingToScroll)
            if (auto* vp = getOwner().getViewport())
                isDraggingToScroll = vp->isCurrentlyScrollingOnDrag();
    }

    int getRow()            const { return row; }
    bool isSelected()       const { return selected; }

private:
    const Base& asBase()    const { return *static_cast<const Base*> (this); }
          Base& asBase()          { return *static_cast<      Base*> (this); }

    static TableListBoxModel* getModel (TableListBox& x)     { return x.getTableListBoxModel(); }
    static ListBoxModel*      getModel (ListBox& x)          { return x.getListBoxModel(); }

    int row = -1;
    bool selected = false, isDragging = false, isDraggingToScroll = false, selectRowOnMouseUp = false;
};

//==============================================================================
class ListBox::RowComponent final  : public TooltipClient,
                                     public ComponentWithListRowMouseBehaviours<RowComponent>
{
public:
    explicit RowComponent (ListBox& lb) : owner (lb) {}

    void paint (Graphics& g) override
    {
        if (auto* m = owner.getListBoxModel())
            m->paintListBoxItem (getRow(), g, getWidth(), getHeight(), isSelected());
    }

    void update (const int newRow, const bool nowSelected)
    {
        updateRowAndSelection (newRow, nowSelected);

        if (auto* m = owner.getListBoxModel())
        {
            setMouseCursor (m->getMouseCursorForRow (getRow()));

            customComponent.reset (m->refreshComponentForRow (newRow, nowSelected, customComponent.release()));

            if (customComponent != nullptr)
            {
                addAndMakeVisible (customComponent.get());
                customComponent->setBounds (getLocalBounds());

                setFocusContainerType (FocusContainerType::focusContainer);
            }
            else
            {
                setFocusContainerType (FocusContainerType::none);
            }
        }
    }

    void performSelection (const MouseEvent& e, bool isMouseUp)
    {
        owner.selectRowsBasedOnModifierKeys (getRow(), e.mods, isMouseUp);

        if (auto* m = owner.getListBoxModel())
            m->listBoxItemClicked (getRow(), e);
    }

    void mouseDoubleClick (const MouseEvent& e) override
    {
        if (isEnabled())
            if (auto* m = owner.getListBoxModel())
                m->listBoxItemDoubleClicked (getRow(), e);
    }

    void resized() override
    {
        if (customComponent != nullptr)
            customComponent->setBounds (getLocalBounds());
    }

    String getTooltip() override
    {
        if (auto* m = owner.getListBoxModel())
            return m->getTooltipForRow (getRow());

        return {};
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<RowAccessibilityHandler> (*this);
    }

    ListBox& getOwner() const { return owner; }

    Component* getCustomComponent() const { return customComponent.get(); }

private:
    //==============================================================================
    class RowAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        explicit RowAccessibilityHandler (RowComponent& rowComponentToWrap)
            : AccessibilityHandler (rowComponentToWrap,
                                    AccessibilityRole::listItem,
                                    getListRowAccessibilityActions (rowComponentToWrap),
                                    { std::make_unique<RowCellInterface> (*this) }),
              rowComponent (rowComponentToWrap)
        {
        }

        String getTitle() const override
        {
            if (auto* m = rowComponent.owner.getListBoxModel())
                return m->getNameForRow (rowComponent.getRow());

            return {};
        }

        String getHelp() const override  { return rowComponent.getTooltip(); }

        AccessibleState getCurrentState() const override
        {
            if (auto* m = rowComponent.owner.getListBoxModel())
                if (rowComponent.getRow() >= m->getNumRows())
                    return AccessibleState().withIgnored();

            auto state = AccessibilityHandler::getCurrentState().withAccessibleOffscreen();

            if (rowComponent.owner.multipleSelection)
                state = state.withMultiSelectable();
            else
                state = state.withSelectable();

            if (rowComponent.isSelected())
                state = state.withSelected();

            return state;
        }

    private:
        class RowCellInterface final : public AccessibilityCellInterface
        {
        public:
            explicit RowCellInterface (RowAccessibilityHandler& h)  : handler (h)  {}

            int getDisclosureLevel() const override  { return 0; }

            const AccessibilityHandler* getTableHandler() const override
            {
                return handler.rowComponent.owner.getAccessibilityHandler();
            }

        private:
            RowAccessibilityHandler& handler;
        };

        RowComponent& rowComponent;
    };

    //==============================================================================
    ListBox& owner;
    std::unique_ptr<Component> customComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowComponent)
};


//==============================================================================
class ListBox::ListViewport final : public Viewport,
                                    private Timer
{
public:
    ListViewport (ListBox& lb)  : owner (lb)
    {
        setWantsKeyboardFocus (false);

        struct IgnoredComponent final : public Component
        {
            std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
            {
                return createIgnoredAccessibilityHandler (*this);
            }
        };

        auto content = std::make_unique<IgnoredComponent>();
        content->setWantsKeyboardFocus (false);

        setViewedComponent (content.release());
    }

    int getIndexOfFirstVisibleRow() const { return jmax (0, firstIndex - 1); }

    RowComponent* getComponentForRowIfOnscreen (int row) const noexcept
    {
        const auto startIndex = getIndexOfFirstVisibleRow();

        return (startIndex <= row && row < startIndex + (int) rows.size())
                 ? rows[(size_t) (row % jmax (1, (int) rows.size()))].get()
                 : nullptr;
    }

    int getRowNumberOfComponent (const Component* const rowComponent) const noexcept
    {
        const auto iter = std::find_if (rows.begin(), rows.end(), [=] (auto& ptr) { return ptr.get() == rowComponent; });

        if (iter == rows.end())
            return -1;

        const auto index = (int) std::distance (rows.begin(), iter);
        const auto mod = jmax (1, (int) rows.size());
        const auto startIndex = getIndexOfFirstVisibleRow();

        return index + mod * ((startIndex / mod) + (index < (startIndex % mod) ? 1 : 0));
    }

    void visibleAreaChanged (const Rectangle<int>&) override
    {
        updateVisibleArea (true);

        if (auto* m = owner.getListBoxModel())
            m->listWasScrolled();

        startTimer (50);
    }

    void updateVisibleArea (const bool makeSureItUpdatesContent)
    {
        hasUpdated = false;

        auto& content = *getViewedComponent();
        auto newX = content.getX();
        auto newY = content.getY();
        auto newW = jmax (owner.minimumRowWidth, getMaximumVisibleWidth());
        auto newH = owner.totalItems * owner.getRowHeight();

        if (newY + newH < getMaximumVisibleHeight() && newH > getMaximumVisibleHeight())
            newY = getMaximumVisibleHeight() - newH;

        content.setBounds (newX, newY, newW, newH);

        if (makeSureItUpdatesContent && ! hasUpdated)
            updateContents();
    }

    void updateContents()
    {
        hasUpdated = true;
        auto rowH = owner.getRowHeight();
        auto& content = *getViewedComponent();

        if (rowH > 0)
        {
            auto y = getViewPositionY();
            auto w = content.getWidth();

            const auto numNeeded = (size_t) (4 + getMaximumVisibleHeight() / rowH);
            rows.resize (jmin (numNeeded, rows.size()));

            while (numNeeded > rows.size())
            {
                rows.emplace_back (new RowComponent (owner));
                content.addAndMakeVisible (*rows.back());
            }

            firstIndex = y / rowH;
            firstWholeIndex = (y + rowH - 1) / rowH;
            lastWholeIndex = (y + getMaximumVisibleHeight() - 1) / rowH;

            const auto startIndex = getIndexOfFirstVisibleRow();
            const auto lastIndex = startIndex + (int) rows.size();

            for (auto row = startIndex; row < lastIndex; ++row)
            {
                if (auto* rowComp = getComponentForRowIfOnscreen (row))
                {
                    rowComp->setBounds (0, row * rowH, w, rowH);
                    rowComp->update (row, owner.isRowSelected (row));
                }
                else
                {
                    jassertfalse;
                }
            }
        }

        if (owner.headerComponent != nullptr)
            owner.headerComponent->setBounds (owner.outlineThickness + content.getX(),
                                              owner.outlineThickness,
                                              jmax (owner.getWidth() - owner.outlineThickness * 2,
                                                    content.getWidth()),
                                              owner.headerComponent->getHeight());
    }

    void selectRow (const int row, const int rowH, const bool dontScroll,
                    const int lastSelectedRow, const int totalRows, const bool isMouseClick)
    {
        hasUpdated = false;

        if (row < firstWholeIndex && ! dontScroll)
        {
            setViewPosition (getViewPositionX(), row * rowH);
        }
        else if (row >= lastWholeIndex && ! dontScroll)
        {
            const int rowsOnScreen = lastWholeIndex - firstWholeIndex;

            if (row >= lastSelectedRow + rowsOnScreen
                 && rowsOnScreen < totalRows - 1
                 && ! isMouseClick)
            {
                setViewPosition (getViewPositionX(),
                                 jlimit (0, jmax (0, totalRows - rowsOnScreen), row) * rowH);
            }
            else
            {
                setViewPosition (getViewPositionX(),
                                 jmax (0, (row  + 1) * rowH - getMaximumVisibleHeight()));
            }
        }

        if (! hasUpdated)
            updateContents();
    }

    void scrollToEnsureRowIsOnscreen (const int row, const int rowH)
    {
        if (row < firstWholeIndex)
        {
            setViewPosition (getViewPositionX(), row * rowH);
        }
        else if (row >= lastWholeIndex)
        {
            setViewPosition (getViewPositionX(),
                             jmax (0, (row  + 1) * rowH - getMaximumVisibleHeight()));
        }
    }

    void paint (Graphics& g) override
    {
        if (isOpaque())
            g.fillAll (owner.findColour (ListBox::backgroundColourId));
    }

    bool keyPressed (const KeyPress& key) override
    {
        if (Viewport::respondsToKey (key))
        {
            const int allowableMods = owner.multipleSelection ? ModifierKeys::shiftModifier : 0;

            if ((key.getModifiers().getRawFlags() & ~allowableMods) == 0)
            {
                // we want to avoid these keypresses going to the viewport, and instead allow
                // them to pass up to our listbox..
                return false;
            }
        }

        return Viewport::keyPressed (key);
    }

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    void timerCallback() override
    {
        stopTimer();

        if (auto* handler = owner.getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::structureChanged);
    }

    ListBox& owner;
    std::vector<std::unique_ptr<RowComponent>> rows;
    int firstIndex = 0, firstWholeIndex = 0, lastWholeIndex = 0;
    bool hasUpdated = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListViewport)
};

//==============================================================================
struct ListBoxMouseMoveSelector final : public MouseListener
{
    ListBoxMouseMoveSelector (ListBox& lb) : owner (lb)
    {
        owner.addMouseListener (this, true);
    }

    ~ListBoxMouseMoveSelector() override
    {
        owner.removeMouseListener (this);
    }

    void mouseMove (const MouseEvent& e) override
    {
        auto pos = e.getEventRelativeTo (&owner).position.toInt();
        owner.selectRow (owner.getRowContainingPosition (pos.x, pos.y), true);
    }

    void mouseExit (const MouseEvent& e) override
    {
        mouseMove (e);
    }

    ListBox& owner;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListBoxMouseMoveSelector)
};


//==============================================================================
ListBox::ListBox (const String& name, ListBoxModel* const m)
    : Component (name)
{
    viewport.reset (new ListViewport (*this));
    addAndMakeVisible (viewport.get());

    setWantsKeyboardFocus (true);
    setFocusContainerType (FocusContainerType::focusContainer);
    colourChanged();

    assignModelPtr (m);
}

ListBox::~ListBox()
{
    headerComponent.reset();
    viewport.reset();
}

void ListBox::assignModelPtr (ListBoxModel* const newModel)
{
    model = newModel;

   #if ! JUCE_DISABLE_ASSERTIONS
    weakModelPtr = model != nullptr ? model->sharedState : nullptr;
   #endif
}

void ListBox::setModel (ListBoxModel* const newModel)
{
    if (model != newModel)
    {
        assignModelPtr (newModel);
        repaint();
        updateContent();
    }
}

void ListBox::setMultipleSelectionEnabled (bool b) noexcept         { multipleSelection = b; }
void ListBox::setClickingTogglesRowSelection (bool b) noexcept      { alwaysFlipSelection = b; }
void ListBox::setRowSelectedOnMouseDown (bool b) noexcept           { selectOnMouseDown = b; }

void ListBox::setMouseMoveSelectsRows (bool b)
{
    if (b)
    {
        if (mouseMoveSelector == nullptr)
            mouseMoveSelector.reset (new ListBoxMouseMoveSelector (*this));
    }
    else
    {
        mouseMoveSelector.reset();
    }
}

//==============================================================================
void ListBox::paint (Graphics& g)
{
    if (! hasDoneInitialUpdate)
        updateContent();

    g.fillAll (findColour (backgroundColourId));
}

void ListBox::paintOverChildren (Graphics& g)
{
    if (outlineThickness > 0)
    {
        g.setColour (findColour (outlineColourId));
        g.drawRect (getLocalBounds(), outlineThickness);
    }
}

void ListBox::resized()
{
    viewport->setBoundsInset (BorderSize<int> (outlineThickness + (headerComponent != nullptr ? headerComponent->getHeight() : 0),
                                               outlineThickness, outlineThickness, outlineThickness));

    viewport->setSingleStepSizes (20, getRowHeight());

    viewport->updateVisibleArea (false);
}

void ListBox::visibilityChanged()
{
    viewport->updateVisibleArea (true);
}

Viewport* ListBox::getViewport() const noexcept
{
    return viewport.get();
}

//==============================================================================
void ListBox::updateContent()
{
    checkModelPtrIsValid();
    hasDoneInitialUpdate = true;
    totalItems = (model != nullptr) ? model->getNumRows() : 0;

    bool selectionChanged = false;

    if (selected.size() > 0 && selected [selected.size() - 1] >= totalItems)
    {
        selected.removeRange ({ totalItems, std::numeric_limits<int>::max() });
        lastRowSelected = getSelectedRow (0);
        selectionChanged = true;
    }

    viewport->updateVisibleArea (isVisible());
    viewport->resized();

    if (selectionChanged)
    {
        if (model != nullptr)
            model->selectedRowsChanged (lastRowSelected);

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
    }
}

//==============================================================================
void ListBox::selectRow (int row, bool dontScroll, bool deselectOthersFirst)
{
    selectRowInternal (row, dontScroll, deselectOthersFirst, false);
}

void ListBox::selectRowInternal (const int row,
                                 bool dontScroll,
                                 bool deselectOthersFirst,
                                 bool isMouseClick)
{
    checkModelPtrIsValid();

    if (! multipleSelection)
        deselectOthersFirst = true;

    if ((! isRowSelected (row))
         || (deselectOthersFirst && getNumSelectedRows() > 1))
    {
        if (isPositiveAndBelow (row, totalItems))
        {
            if (deselectOthersFirst)
                selected.clear();

            selected.addRange ({ row, row + 1 });

            if (getHeight() == 0 || getWidth() == 0)
                dontScroll = true;

            viewport->selectRow (row, getRowHeight(), dontScroll,
                                 lastRowSelected, totalItems, isMouseClick);

            lastRowSelected = row;
            model->selectedRowsChanged (row);

            if (auto* handler = getAccessibilityHandler())
                handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
        }
        else
        {
            if (deselectOthersFirst)
                deselectAllRows();
        }
    }
}

void ListBox::deselectRow (const int row)
{
    checkModelPtrIsValid();

    if (selected.contains (row))
    {
        selected.removeRange ({ row, row + 1 });

        if (row == lastRowSelected)
            lastRowSelected = getSelectedRow (0);

        viewport->updateContents();
        model->selectedRowsChanged (lastRowSelected);

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
    }
}

void ListBox::setSelectedRows (const SparseSet<int>& setOfRowsToBeSelected,
                               const NotificationType sendNotificationEventToModel)
{
    checkModelPtrIsValid();

    selected = setOfRowsToBeSelected;
    selected.removeRange ({ totalItems, std::numeric_limits<int>::max() });

    if (! isRowSelected (lastRowSelected))
        lastRowSelected = getSelectedRow (0);

    viewport->updateContents();

    if (model != nullptr && sendNotificationEventToModel == sendNotification)
        model->selectedRowsChanged (lastRowSelected);

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
}

SparseSet<int> ListBox::getSelectedRows() const
{
    return selected;
}

void ListBox::selectRangeOfRows (int firstRow, int lastRow, bool dontScrollToShowThisRange)
{
    if (multipleSelection && (firstRow != lastRow))
    {
        const int numRows = totalItems - 1;
        firstRow = jlimit (0, jmax (0, numRows), firstRow);
        lastRow  = jlimit (0, jmax (0, numRows), lastRow);

        selected.addRange ({ jmin (firstRow, lastRow),
                             jmax (firstRow, lastRow) + 1 });

        selected.removeRange ({ lastRow, lastRow + 1 });
    }

    selectRowInternal (lastRow, dontScrollToShowThisRange, false, true);
}

void ListBox::flipRowSelection (const int row)
{
    if (isRowSelected (row))
        deselectRow (row);
    else
        selectRowInternal (row, false, false, true);
}

void ListBox::deselectAllRows()
{
    checkModelPtrIsValid();

    if (! selected.isEmpty())
    {
        selected.clear();
        lastRowSelected = -1;

        viewport->updateContents();

        if (model != nullptr)
            model->selectedRowsChanged (lastRowSelected);

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
    }
}

void ListBox::selectRowsBasedOnModifierKeys (const int row,
                                             ModifierKeys mods,
                                             const bool isMouseUpEvent)
{
    if (multipleSelection && (mods.isCommandDown() || alwaysFlipSelection))
    {
        flipRowSelection (row);
    }
    else if (multipleSelection && mods.isShiftDown() && lastRowSelected >= 0)
    {
        selectRangeOfRows (lastRowSelected, row);
    }
    else if ((! mods.isPopupMenu()) || ! isRowSelected (row))
    {
        selectRowInternal (row, false, ! (multipleSelection && (! isMouseUpEvent) && isRowSelected (row)), true);
    }
}

int ListBox::getNumSelectedRows() const
{
    return selected.size();
}

int ListBox::getSelectedRow (const int index) const
{
    return (isPositiveAndBelow (index, selected.size()))
                ? selected [index] : -1;
}

bool ListBox::isRowSelected (const int row) const
{
    return selected.contains (row);
}

int ListBox::getLastRowSelected() const
{
    return isRowSelected (lastRowSelected) ? lastRowSelected : -1;
}

//==============================================================================
int ListBox::getRowContainingPosition (const int x, const int y) const noexcept
{
    if (isPositiveAndBelow (x, getWidth()))
    {
        const int row = (viewport->getViewPositionY() + y - viewport->getY()) / rowHeight;

        if (isPositiveAndBelow (row, totalItems))
            return row;
    }

    return -1;
}

int ListBox::getInsertionIndexForPosition (const int x, const int y) const noexcept
{
    if (isPositiveAndBelow (x, getWidth()))
        return jlimit (0, totalItems, (viewport->getViewPositionY() + y + rowHeight / 2 - viewport->getY()) / rowHeight);

    return -1;
}

Component* ListBox::getComponentForRowNumber (const int row) const noexcept
{
    if (auto* listRowComp = viewport->getComponentForRowIfOnscreen (row))
        return listRowComp->getCustomComponent();

    return nullptr;
}

int ListBox::getRowNumberOfComponent (const Component* const rowComponent) const noexcept
{
    return viewport->getRowNumberOfComponent (rowComponent);
}

Rectangle<int> ListBox::getRowPosition (int rowNumber, bool relativeToComponentTopLeft) const noexcept
{
    auto y = viewport->getY() + rowHeight * rowNumber;

    if (relativeToComponentTopLeft)
        y -= viewport->getViewPositionY();

    return { viewport->getX(), y,
             viewport->getViewedComponent()->getWidth(), rowHeight };
}

void ListBox::setVerticalPosition (const double proportion)
{
    auto offscreen = viewport->getViewedComponent()->getHeight() - viewport->getHeight();

    viewport->setViewPosition (viewport->getViewPositionX(),
                               jmax (0, roundToInt (proportion * offscreen)));
}

double ListBox::getVerticalPosition() const
{
    auto offscreen = viewport->getViewedComponent()->getHeight() - viewport->getHeight();

    return offscreen > 0 ? viewport->getViewPositionY() / (double) offscreen
                         : 0;
}

int ListBox::getVisibleRowWidth() const noexcept
{
    return viewport->getViewWidth();
}

void ListBox::scrollToEnsureRowIsOnscreen (const int row)
{
    viewport->scrollToEnsureRowIsOnscreen (row, getRowHeight());
}

//==============================================================================
bool ListBox::keyPressed (const KeyPress& key)
{
    checkModelPtrIsValid();

    const int numVisibleRows = viewport->getHeight() / getRowHeight();

    const bool multiple = multipleSelection
                            && lastRowSelected >= 0
                            && key.getModifiers().isShiftDown();

    if (key.isKeyCode (KeyPress::upKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected - 1);
        else
            selectRow (jmax (0, lastRowSelected - 1));
    }
    else if (key.isKeyCode (KeyPress::downKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected + 1);
        else
            selectRow (jmin (totalItems - 1, jmax (0, lastRowSelected + 1)));
    }
    else if (key.isKeyCode (KeyPress::pageUpKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected - numVisibleRows);
        else
            selectRow (jmax (0, jmax (0, lastRowSelected) - numVisibleRows));
    }
    else if (key.isKeyCode (KeyPress::pageDownKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected + numVisibleRows);
        else
            selectRow (jmin (totalItems - 1, jmax (0, lastRowSelected) + numVisibleRows));
    }
    else if (key.isKeyCode (KeyPress::homeKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, 0);
        else
            selectRow (0);
    }
    else if (key.isKeyCode (KeyPress::endKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, totalItems - 1);
        else
            selectRow (totalItems - 1);
    }
    else if (key.isKeyCode (KeyPress::returnKey) && isRowSelected (lastRowSelected))
    {
        if (model != nullptr)
            model->returnKeyPressed (lastRowSelected);
    }
    else if ((key.isKeyCode (KeyPress::deleteKey) || key.isKeyCode (KeyPress::backspaceKey))
               && isRowSelected (lastRowSelected))
    {
        if (model != nullptr)
            model->deleteKeyPressed (lastRowSelected);
    }
    else if (multipleSelection && key == KeyPress ('a', ModifierKeys::commandModifier, 0))
    {
        selectRangeOfRows (0, std::numeric_limits<int>::max());
    }
    else
    {
        return false;
    }

    return true;
}

bool ListBox::keyStateChanged (const bool isKeyDown)
{
    return isKeyDown
            && (KeyPress::isKeyCurrentlyDown (KeyPress::upKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::pageUpKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::downKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::pageDownKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::homeKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::endKey)
                || KeyPress::isKeyCurrentlyDown (KeyPress::returnKey));
}

void ListBox::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    bool eventWasUsed = false;

    if (! approximatelyEqual (wheel.deltaX, 0.0f) && getHorizontalScrollBar().isVisible())
    {
        eventWasUsed = true;
        getHorizontalScrollBar().mouseWheelMove (e, wheel);
    }

    if (! approximatelyEqual (wheel.deltaY, 0.0f) && getVerticalScrollBar().isVisible())
    {
        eventWasUsed = true;
        getVerticalScrollBar().mouseWheelMove (e, wheel);
    }

    if (! eventWasUsed)
        Component::mouseWheelMove (e, wheel);
}

void ListBox::mouseUp (const MouseEvent& e)
{
    checkModelPtrIsValid();

    if (e.mouseWasClicked() && model != nullptr)
        model->backgroundClicked (e);
}

//==============================================================================
void ListBox::setRowHeight (const int newHeight)
{
    rowHeight = jmax (1, newHeight);
    viewport->setSingleStepSizes (20, rowHeight);
    updateContent();
}

int ListBox::getNumRowsOnScreen() const noexcept
{
    return viewport->getMaximumVisibleHeight() / rowHeight;
}

void ListBox::setMinimumContentWidth (const int newMinimumWidth)
{
    minimumRowWidth = newMinimumWidth;
    updateContent();
}

int ListBox::getVisibleContentWidth() const noexcept            { return viewport->getMaximumVisibleWidth(); }

ScrollBar& ListBox::getVerticalScrollBar() const noexcept       { return viewport->getVerticalScrollBar(); }
ScrollBar& ListBox::getHorizontalScrollBar() const noexcept     { return viewport->getHorizontalScrollBar(); }

void ListBox::colourChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
    viewport->setOpaque (isOpaque());
    repaint();
}

void ListBox::parentHierarchyChanged()
{
    colourChanged();
}

void ListBox::setOutlineThickness (int newThickness)
{
    outlineThickness = newThickness;
    resized();
}

void ListBox::setHeaderComponent (std::unique_ptr<Component> newHeaderComponent)
{
    headerComponent = std::move (newHeaderComponent);
    addAndMakeVisible (headerComponent.get());
    ListBox::resized();
    invalidateAccessibilityHandler();
}

bool ListBox::hasAccessibleHeaderComponent() const
{
    return headerComponent != nullptr
            && headerComponent->getAccessibilityHandler() != nullptr;
}

void ListBox::repaintRow (const int rowNumber) noexcept
{
    repaint (getRowPosition (rowNumber, true));
}

ScaledImage ListBox::createSnapshotOfRows (const SparseSet<int>& rows, int& imageX, int& imageY)
{
    Rectangle<int> imageArea;
    auto firstRow = getRowContainingPosition (0, viewport->getY());

    for (int i = getNumRowsOnScreen() + 2; --i >= 0;)
    {
        if (rows.contains (firstRow + i))
        {
            if (auto* rowComp = viewport->getComponentForRowIfOnscreen (firstRow + i))
            {
                auto pos = getLocalPoint (rowComp, Point<int>());

                imageArea = imageArea.getUnion ({ pos.x, pos.y, rowComp->getWidth(), rowComp->getHeight() });
            }
        }
    }

    imageArea = imageArea.getIntersection (getLocalBounds());
    imageX = imageArea.getX();
    imageY = imageArea.getY();

    const auto additionalScale = 2.0f;
    const auto listScale = Component::getApproximateScaleFactorForComponent (this) * additionalScale;
    Image snapshot (Image::ARGB,
                    roundToInt ((float) imageArea.getWidth() * listScale),
                    roundToInt ((float) imageArea.getHeight() * listScale),
                    true);

    for (int i = getNumRowsOnScreen() + 2; --i >= 0;)
    {
        if (rows.contains (firstRow + i))
        {
            if (auto* rowComp = viewport->getComponentForRowIfOnscreen (firstRow + i))
            {
                Graphics g (snapshot);
                g.setOrigin ((getLocalPoint (rowComp, Point<int>()) - imageArea.getPosition()) * additionalScale);

                const auto rowScale = Component::getApproximateScaleFactorForComponent (rowComp) * additionalScale;

                if (g.reduceClipRegion (rowComp->getLocalBounds() * rowScale))
                {
                    g.beginTransparencyLayer (0.6f);
                    g.addTransform (AffineTransform::scale (rowScale));
                    rowComp->paintEntireComponent (g, false);
                    g.endTransparencyLayer();
                }
            }
        }
    }

    return { snapshot, additionalScale };
}

void ListBox::startDragAndDrop (const MouseEvent& e, const SparseSet<int>& rowsToDrag, const var& dragDescription, bool allowDraggingToOtherWindows)
{
    if (auto* dragContainer = DragAndDropContainer::findParentDragContainerFor (this))
    {
        int x, y;
        auto dragImage = createSnapshotOfRows (rowsToDrag, x, y);

        auto p = Point<int> (x, y) - e.getEventRelativeTo (this).position.toInt();
        dragContainer->startDragging (dragDescription, this, dragImage, allowDraggingToOtherWindows, &p, &e.source);
    }
    else
    {
        // to be able to do a drag-and-drop operation, the listbox needs to
        // be inside a component which is also a DragAndDropContainer.
        jassertfalse;
    }
}

std::unique_ptr<AccessibilityHandler> ListBox::createAccessibilityHandler()
{
    class TableInterface final : public AccessibilityTableInterface
    {
    public:
        explicit TableInterface (ListBox& listBoxToWrap)
            : listBox (listBoxToWrap)
        {
        }

        int getNumRows() const override
        {
            listBox.checkModelPtrIsValid();

            return listBox.model != nullptr ? listBox.model->getNumRows()
                                            : 0;
        }

        int getNumColumns() const override
        {
            return 1;
        }

        const AccessibilityHandler* getHeaderHandler() const override
        {
            if (listBox.hasAccessibleHeaderComponent())
                return listBox.headerComponent->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getRowHandler (int row) const override
        {
            if (auto* rowComponent = listBox.viewport->getComponentForRowIfOnscreen (row))
                return rowComponent->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getCellHandler (int, int) const override
        {
            return nullptr;
        }

        Optional<Span> getRowSpan (const AccessibilityHandler& handler) const override
        {
            const auto rowNumber = listBox.getRowNumberOfComponent (&handler.getComponent());

            return rowNumber != -1 ? makeOptional (Span { rowNumber, 1 })
                                   : nullopt;
        }

        Optional<Span> getColumnSpan (const AccessibilityHandler&) const override
        {
            return Span { 0, 1 };
        }

        void showCell (const AccessibilityHandler& h) const override
        {
            if (const auto row = getRowSpan (h))
                listBox.scrollToEnsureRowIsOnscreen (row->begin);
        }

    private:
        ListBox& listBox;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableInterface)
    };

    return std::make_unique<AccessibilityHandler> (*this,
                                                   AccessibilityRole::list,
                                                   AccessibilityActions{},
                                                   AccessibilityHandler::Interfaces { std::make_unique<TableInterface> (*this) });
}

//==============================================================================
Component* ListBoxModel::refreshComponentForRow (int, bool, [[maybe_unused]] Component* existingComponentToUpdate)
{
    jassert (existingComponentToUpdate == nullptr); // indicates a failure in the code that recycles the components
    return nullptr;
}

String ListBoxModel::getNameForRow (int rowNumber)                      { return "Row " + String (rowNumber + 1); }
void ListBoxModel::listBoxItemClicked (int, const MouseEvent&) {}
void ListBoxModel::listBoxItemDoubleClicked (int, const MouseEvent&) {}
void ListBoxModel::backgroundClicked (const MouseEvent&) {}
void ListBoxModel::selectedRowsChanged (int) {}
void ListBoxModel::deleteKeyPressed (int) {}
void ListBoxModel::returnKeyPressed (int) {}
void ListBoxModel::listWasScrolled() {}
var ListBoxModel::getDragSourceDescription (const SparseSet<int>&)      { return {}; }
String ListBoxModel::getTooltipForRow (int)                             { return {}; }
MouseCursor ListBoxModel::getMouseCursorForRow (int)                    { return MouseCursor::NormalCursor; }

} // namespace juce
