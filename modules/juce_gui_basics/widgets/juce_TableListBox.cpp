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

static const Identifier tableColumnProperty { "_tableColumnId" };
static const Identifier tableAccessiblePlaceholderProperty { "_accessiblePlaceholder" };

class TableListBox::RowComp final : public TooltipClient,
                                    public ComponentWithListRowMouseBehaviours<RowComp>
{
public:
    explicit RowComp (TableListBox& tlb)
        : owner (tlb)
    {
        setFocusContainerType (FocusContainerType::focusContainer);
    }

    void paint (Graphics& g) override
    {
        if (auto* tableModel = owner.getTableListBoxModel())
        {
            tableModel->paintRowBackground (g, getRow(), getWidth(), getHeight(), isSelected());

            auto& headerComp = owner.getHeader();
            const auto numColumns = jmin ((int) columnComponents.size(), headerComp.getNumColumns (true));
            const auto clipBounds = g.getClipBounds();

            for (int i = 0; i < numColumns; ++i)
            {
                if (columnComponents[(size_t) i]->getProperties().contains (tableAccessiblePlaceholderProperty))
                {
                    auto columnRect = headerComp.getColumnPosition (i).withHeight (getHeight());

                    if (columnRect.getX() >= clipBounds.getRight())
                        break;

                    if (columnRect.getRight() > clipBounds.getX())
                    {
                        Graphics::ScopedSaveState ss (g);

                        if (g.reduceClipRegion (columnRect))
                        {
                            g.setOrigin (columnRect.getX(), 0);
                            tableModel->paintCell (g, getRow(), headerComp.getColumnIdOfIndex (i, true),
                                                   columnRect.getWidth(), columnRect.getHeight(), isSelected());
                        }
                    }
                }
            }
        }
    }

    void update (int newRow, bool isNowSelected)
    {
        jassert (newRow >= 0);

        updateRowAndSelection (newRow, isNowSelected);

        auto* tableModel = owner.getTableListBoxModel();

        if (tableModel != nullptr && getRow() < owner.getNumRows())
        {
            const ComponentDeleter deleter { columnForComponent };
            const auto numColumns = owner.getHeader().getNumColumns (true);

            while (numColumns < (int) columnComponents.size())
                columnComponents.pop_back();

            while ((int) columnComponents.size() < numColumns)
                columnComponents.emplace_back (nullptr, deleter);

            for (int i = 0; i < numColumns; ++i)
            {
                auto columnId = owner.getHeader().getColumnIdOfIndex (i, true);
                auto originalComp = std::move (columnComponents[(size_t) i]);
                auto oldCustomComp = originalComp != nullptr && ! originalComp->getProperties().contains (tableAccessiblePlaceholderProperty)
                                   ? std::move (originalComp)
                                   : std::unique_ptr<Component, ComponentDeleter> { nullptr, deleter };
                auto compToRefresh = oldCustomComp != nullptr && columnId == static_cast<int> (oldCustomComp->getProperties()[tableColumnProperty])
                                   ? std::move (oldCustomComp)
                                   : std::unique_ptr<Component, ComponentDeleter> { nullptr, deleter };

                columnForComponent.erase (compToRefresh.get());
                std::unique_ptr<Component, ComponentDeleter> newCustomComp { tableModel->refreshComponentForCell (getRow(),
                                                                                                                  columnId,
                                                                                                                  isSelected(),
                                                                                                                  compToRefresh.release()),
                                                                             deleter };

                auto columnComp = [&]
                {
                    // We got a result from refreshComponentForCell, so use that
                    if (newCustomComp != nullptr)
                        return std::move (newCustomComp);

                    // There was already a placeholder component for this column
                    if (originalComp != nullptr)
                        return std::move (originalComp);

                    // Create a new placeholder component to use
                    std::unique_ptr<Component, ComponentDeleter> comp { new Component, deleter };
                    comp->setInterceptsMouseClicks (false, false);
                    comp->getProperties().set (tableAccessiblePlaceholderProperty, true);
                    return comp;
                }();

                columnForComponent.emplace (columnComp.get(), i);

                // In order for navigation to work correctly on macOS, the number of child
                // accessibility elements on each row must match the number of header accessibility
                // elements.
                columnComp->setFocusContainerType (FocusContainerType::focusContainer);
                columnComp->getProperties().set (tableColumnProperty, columnId);
                addAndMakeVisible (*columnComp);

                columnComponents[(size_t) i] = std::move (columnComp);
                resizeCustomComp (i);
            }
        }
        else
        {
            columnComponents.clear();
        }
    }

    void resized() override
    {
        for (auto i = (int) columnComponents.size(); --i >= 0;)
            resizeCustomComp (i);
    }

    void resizeCustomComp (int index)
    {
        if (auto& c = columnComponents[(size_t) index])
        {
            c->setBounds (owner.getHeader()
                               .getColumnPosition (index)
                               .withY (0)
                               .withHeight (getHeight()));
        }
    }

    void performSelection (const MouseEvent& e, bool isMouseUp)
    {
        owner.selectRowsBasedOnModifierKeys (getRow(), e.mods, isMouseUp);

        auto columnId = owner.getHeader().getColumnIdAtX (e.x);

        if (columnId != 0)
            if (auto* m = owner.getTableListBoxModel())
                m->cellClicked (getRow(), columnId, e);
    }

    void mouseDoubleClick (const MouseEvent& e) override
    {
        if (! isEnabled())
            return;

        const auto columnId = owner.getHeader().getColumnIdAtX (e.x);

        if (columnId != 0)
            if (auto* m = owner.getTableListBoxModel())
                m->cellDoubleClicked (getRow(), columnId, e);
    }

    String getTooltip() override
    {
        auto columnId = owner.getHeader().getColumnIdAtX (getMouseXYRelative().getX());

        if (columnId != 0)
            if (auto* m = owner.getTableListBoxModel())
                return m->getCellTooltip (getRow(), columnId);

        return {};
    }

    Component* findChildComponentForColumn (int columnId) const
    {
        const auto index = (size_t) owner.getHeader().getIndexOfColumnId (columnId, true);

        if (isPositiveAndBelow (index, columnComponents.size()))
            return columnComponents[index].get();

        return nullptr;
    }

    int getColumnNumberOfComponent (const Component* comp) const
    {
        const auto iter = columnForComponent.find (comp);
        return iter != columnForComponent.cend() ? iter->second : -1;
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<RowAccessibilityHandler> (*this);
    }

    TableListBox& getOwner() const { return owner; }

private:
    //==============================================================================
    class RowAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        RowAccessibilityHandler (RowComp& rowComp)
            : AccessibilityHandler (rowComp,
                                    AccessibilityRole::row,
                                    getListRowAccessibilityActions (rowComp),
                                    { std::make_unique<RowComponentCellInterface> (*this) }),
              rowComponent (rowComp)
        {
        }

        String getTitle() const override
        {
            if (auto* m = rowComponent.owner.ListBox::model)
                return m->getNameForRow (rowComponent.getRow());

            return {};
        }

        String getHelp() const override  { return rowComponent.getTooltip(); }

        AccessibleState getCurrentState() const override
        {
            if (auto* m = rowComponent.owner.getTableListBoxModel())
                if (rowComponent.getRow() >= m->getNumRows())
                    return AccessibleState().withIgnored();

            auto state = AccessibilityHandler::getCurrentState();

            if (rowComponent.owner.multipleSelection)
                state = state.withMultiSelectable();
            else
                state = state.withSelectable();

            if (rowComponent.isSelected())
                return state.withSelected();

            return state;
        }

    private:
        class RowComponentCellInterface final : public AccessibilityCellInterface
        {
        public:
            RowComponentCellInterface (RowAccessibilityHandler& handler)
                : owner (handler)
            {
            }

            int getDisclosureLevel() const override  { return 0; }

            const AccessibilityHandler* getTableHandler() const override  { return owner.rowComponent.owner.getAccessibilityHandler(); }

        private:
            RowAccessibilityHandler& owner;
        };

    private:
        RowComp& rowComponent;
    };

    //==============================================================================
    class ComponentDeleter
    {
    public:
        explicit ComponentDeleter (std::map<const Component*, int>& locations)
            : columnForComponent (&locations) {}

        void operator() (Component* comp) const
        {
            columnForComponent->erase (comp);

            if (comp != nullptr)
                delete comp;
        }

    private:
        std::map<const Component*, int>* columnForComponent;
    };

    TableListBox& owner;
    std::map<const Component*, int> columnForComponent;
    std::vector<std::unique_ptr<Component, ComponentDeleter>> columnComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowComp)
};


//==============================================================================
class TableListBox::Header final : public TableHeaderComponent
{
public:
    Header (TableListBox& tlb)  : owner (tlb) {}

    void addMenuItems (PopupMenu& menu, int columnIdClicked)
    {
        if (owner.isAutoSizeMenuOptionShown())
        {
            menu.addItem (autoSizeColumnId, TRANS ("Auto-size this column"), columnIdClicked != 0);
            menu.addItem (autoSizeAllId, TRANS ("Auto-size all columns"), owner.getHeader().getNumColumns (true) > 0);
            menu.addSeparator();
        }

        TableHeaderComponent::addMenuItems (menu, columnIdClicked);
    }

    void reactToMenuItem (int menuReturnId, int columnIdClicked)
    {
        switch (menuReturnId)
        {
            case autoSizeColumnId:      owner.autoSizeColumn (columnIdClicked); break;
            case autoSizeAllId:         owner.autoSizeAllColumns(); break;
            default:                    TableHeaderComponent::reactToMenuItem (menuReturnId, columnIdClicked); break;
        }
    }

private:
    TableListBox& owner;

    enum { autoSizeColumnId = 0xf836743, autoSizeAllId = 0xf836744 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Header)
};

//==============================================================================
TableListBox::TableListBox (const String& name, TableListBoxModel* const m)
    : ListBox (name, nullptr), model (m)
{
    ListBox::assignModelPtr (this);

    setHeader (std::make_unique<Header> (*this));
}

TableListBox::~TableListBox()
{
}

void TableListBox::setModel (TableListBoxModel* newModel)
{
    if (model != newModel)
    {
        model = newModel;
        updateContent();
    }
}

void TableListBox::setHeader (std::unique_ptr<TableHeaderComponent> newHeader)
{
    if (newHeader == nullptr)
    {
        jassertfalse; // you need to supply a real header for a table!
        return;
    }

    Rectangle<int> newBounds (100, 28);

    if (header != nullptr)
        newBounds = header->getBounds();

    header = newHeader.get();
    header->setBounds (newBounds);

    setHeaderComponent (std::move (newHeader));

    header->addListener (this);
}

int TableListBox::getHeaderHeight() const noexcept
{
    return header->getHeight();
}

void TableListBox::setHeaderHeight (int newHeight)
{
    header->setSize (header->getWidth(), newHeight);
    resized();
}

void TableListBox::autoSizeColumn (int columnId)
{
    auto width = model != nullptr ? model->getColumnAutoSizeWidth (columnId) : 0;

    if (width > 0)
        header->setColumnWidth (columnId, width);
}

void TableListBox::autoSizeAllColumns()
{
    for (int i = 0; i < header->getNumColumns (true); ++i)
        autoSizeColumn (header->getColumnIdOfIndex (i, true));
}

void TableListBox::setAutoSizeMenuOptionShown (bool shouldBeShown) noexcept
{
    autoSizeOptionsShown = shouldBeShown;
}

Rectangle<int> TableListBox::getCellPosition (int columnId, int rowNumber, bool relativeToComponentTopLeft) const
{
    auto headerCell = header->getColumnPosition (header->getIndexOfColumnId (columnId, true));

    if (relativeToComponentTopLeft)
        headerCell.translate (header->getX(), 0);

    return getRowPosition (rowNumber, relativeToComponentTopLeft)
            .withX (headerCell.getX())
            .withWidth (headerCell.getWidth());
}

Component* TableListBox::getCellComponent (int columnId, int rowNumber) const
{
    if (auto* rowComp = dynamic_cast<RowComp*> (getComponentForRowNumber (rowNumber)))
        return rowComp->findChildComponentForColumn (columnId);

    return nullptr;
}

void TableListBox::scrollToEnsureColumnIsOnscreen (int columnId)
{
    auto& scrollbar = getHorizontalScrollBar();
    auto pos = header->getColumnPosition (header->getIndexOfColumnId (columnId, true));

    auto x = scrollbar.getCurrentRangeStart();
    auto w = scrollbar.getCurrentRangeSize();

    if (pos.getX() < x)
        x = pos.getX();
    else if (pos.getRight() > x + w)
        x += jmax (0.0, pos.getRight() - (x + w));

    scrollbar.setCurrentRangeStart (x);
}

int TableListBox::getNumRows()
{
    return model != nullptr ? model->getNumRows() : 0;
}

void TableListBox::paintListBoxItem (int, Graphics&, int, int, bool)
{
}

Component* TableListBox::refreshComponentForRow (int rowNumber, bool rowSelected, Component* existingComponentToUpdate)
{
    if (existingComponentToUpdate == nullptr)
        existingComponentToUpdate = new RowComp (*this);

    static_cast<RowComp*> (existingComponentToUpdate)->update (rowNumber, rowSelected);

    return existingComponentToUpdate;
}

void TableListBox::selectedRowsChanged (int row)
{
    if (model != nullptr)
        model->selectedRowsChanged (row);
}

void TableListBox::deleteKeyPressed (int row)
{
    if (model != nullptr)
        model->deleteKeyPressed (row);
}

void TableListBox::returnKeyPressed (int row)
{
    if (model != nullptr)
        model->returnKeyPressed (row);
}

void TableListBox::backgroundClicked (const MouseEvent& e)
{
    if (model != nullptr)
        model->backgroundClicked (e);
}

void TableListBox::listWasScrolled()
{
    if (model != nullptr)
        model->listWasScrolled();
}

void TableListBox::tableColumnsChanged (TableHeaderComponent*)
{
    setMinimumContentWidth (header->getTotalWidth());
    repaint();
    updateColumnComponents();
}

void TableListBox::tableColumnsResized (TableHeaderComponent*)
{
    setMinimumContentWidth (header->getTotalWidth());
    repaint();
    updateColumnComponents();
}

void TableListBox::tableSortOrderChanged (TableHeaderComponent*)
{
    if (model != nullptr)
        model->sortOrderChanged (header->getSortColumnId(),
                                 header->isSortedForwards());
}

void TableListBox::tableColumnDraggingChanged (TableHeaderComponent*, int columnIdNowBeingDragged_)
{
    columnIdNowBeingDragged = columnIdNowBeingDragged_;
    repaint();
}

void TableListBox::resized()
{
    ListBox::resized();

    header->resizeAllColumnsToFit (getVisibleContentWidth());
    setMinimumContentWidth (header->getTotalWidth());
}

void TableListBox::updateColumnComponents() const
{
    auto firstRow = getRowContainingPosition (0, 0);

    for (int i = firstRow + getNumRowsOnScreen() + 2; --i >= firstRow;)
        if (auto* rowComp = dynamic_cast<RowComp*> (getComponentForRowNumber (i)))
            rowComp->resized();
}

template <typename FindIndex>
Optional<AccessibilityTableInterface::Span> findRecursively (const AccessibilityHandler& handler,
                                                             Component* outermost,
                                                             FindIndex&& findIndexOfComponent)
{
    for (auto* comp = &handler.getComponent(); comp != outermost; comp = comp->getParentComponent())
    {
        const auto result = findIndexOfComponent (comp);

        if (result != -1)
            return AccessibilityTableInterface::Span { result, 1 };
    }

    return nullopt;
}

std::unique_ptr<AccessibilityHandler> TableListBox::createAccessibilityHandler()
{
    class TableInterface final : public AccessibilityTableInterface
    {
    public:
        explicit TableInterface (TableListBox& tableListBoxToWrap)
            : tableListBox (tableListBoxToWrap)
        {
        }

        int getNumRows() const override
        {
            if (auto* tableModel = tableListBox.getTableListBoxModel())
                return tableModel->getNumRows();

            return 0;
        }

        int getNumColumns() const override
        {
            return tableListBox.getHeader().getNumColumns (true);
        }

        const AccessibilityHandler* getRowHandler (int row) const override
        {
            if (isPositiveAndBelow (row, getNumRows()))
                if (auto* rowComp = tableListBox.getComponentForRowNumber (row))
                    return rowComp->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getCellHandler (int row, int column) const override
        {
            if (isPositiveAndBelow (row, getNumRows()) && isPositiveAndBelow (column, getNumColumns()))
                if (auto* cellComponent = tableListBox.getCellComponent (tableListBox.getHeader().getColumnIdOfIndex (column, true), row))
                    return cellComponent->getAccessibilityHandler();

            return nullptr;
        }

        const AccessibilityHandler* getHeaderHandler() const override
        {
            if (tableListBox.hasAccessibleHeaderComponent())
                return tableListBox.headerComponent->getAccessibilityHandler();

            return nullptr;
        }

        Optional<Span> getRowSpan (const AccessibilityHandler& handler) const override
        {
            if (tableListBox.isParentOf (&handler.getComponent()))
                return findRecursively (handler, &tableListBox, [&] (auto* c) { return tableListBox.getRowNumberOfComponent (c); });

            return nullopt;
        }

        Optional<Span> getColumnSpan (const AccessibilityHandler& handler) const override
        {
            if (const auto rowSpan = getRowSpan (handler))
                if (auto* rowComponent = dynamic_cast<RowComp*> (tableListBox.getComponentForRowNumber (rowSpan->begin)))
                    return findRecursively (handler, &tableListBox, [&] (auto* c) { return rowComponent->getColumnNumberOfComponent (c); });

            return nullopt;
        }

        void showCell (const AccessibilityHandler& handler) const override
        {
            const auto row = getRowSpan (handler);
            const auto col = getColumnSpan (handler);

            if (row.hasValue() && col.hasValue())
            {
                tableListBox.scrollToEnsureRowIsOnscreen (row->begin);
                tableListBox.scrollToEnsureColumnIsOnscreen (col->begin);
            }
        }

    private:
        TableListBox& tableListBox;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableInterface)
    };

    return std::make_unique<AccessibilityHandler> (*this,
                                                   AccessibilityRole::table,
                                                   AccessibilityActions{},
                                                   AccessibilityHandler::Interfaces { std::make_unique<TableInterface> (*this) });
}

//==============================================================================
void TableListBoxModel::cellClicked (int, int, const MouseEvent&)       {}
void TableListBoxModel::cellDoubleClicked (int, int, const MouseEvent&) {}
void TableListBoxModel::backgroundClicked (const MouseEvent&)           {}
void TableListBoxModel::sortOrderChanged (int, bool)                    {}
int TableListBoxModel::getColumnAutoSizeWidth (int)                     { return 0; }
void TableListBoxModel::selectedRowsChanged (int)                       {}
void TableListBoxModel::deleteKeyPressed (int)                          {}
void TableListBoxModel::returnKeyPressed (int)                          {}
void TableListBoxModel::listWasScrolled()                               {}

String TableListBoxModel::getCellTooltip (int /*rowNumber*/, int /*columnId*/)    { return {}; }
var TableListBoxModel::getDragSourceDescription (const SparseSet<int>&)           { return {}; }

Component* TableListBoxModel::refreshComponentForCell (int, int, bool, [[maybe_unused]] Component* existingComponentToUpdate)
{
    jassert (existingComponentToUpdate == nullptr); // indicates a failure in the code that recycles the components
    return nullptr;
}

} // namespace juce
