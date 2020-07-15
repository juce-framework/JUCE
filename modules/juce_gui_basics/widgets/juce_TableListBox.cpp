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

class TableListBox::RowComp   : public Component,
                                public TooltipClient
{
public:
    RowComp (TableListBox& tlb) noexcept  : owner (tlb) {}

    void paint (Graphics& g) override
    {
        if (auto* tableModel = owner.getModel())
        {
            tableModel->paintRowBackground (g, row, getWidth(), getHeight(), isSelected);

            auto& headerComp = owner.getHeader();
            auto numColumns = headerComp.getNumColumns (true);
            auto clipBounds = g.getClipBounds();

            for (int i = 0; i < numColumns; ++i)
            {
                if (columnComponents[i] == nullptr)
                {
                    auto columnRect = headerComp.getColumnPosition(i).withHeight (getHeight());

                    if (columnRect.getX() >= clipBounds.getRight())
                        break;

                    if (columnRect.getRight() > clipBounds.getX())
                    {
                        Graphics::ScopedSaveState ss (g);

                        if (g.reduceClipRegion (columnRect))
                        {
                            g.setOrigin (columnRect.getX(), 0);
                            tableModel->paintCell (g, row, headerComp.getColumnIdOfIndex (i, true),
                                                   columnRect.getWidth(), columnRect.getHeight(), isSelected);
                        }
                    }
                }
            }
        }
    }

    void update (int newRow, bool isNowSelected)
    {
        jassert (newRow >= 0);

        if (newRow != row || isNowSelected != isSelected)
        {
            row = newRow;
            isSelected = isNowSelected;
            repaint();
        }

        auto* tableModel = owner.getModel();

        if (tableModel != nullptr && row < owner.getNumRows())
        {
            const Identifier columnProperty ("_tableColumnId");
            auto numColumns = owner.getHeader().getNumColumns (true);

            for (int i = 0; i < numColumns; ++i)
            {
                auto columnId = owner.getHeader().getColumnIdOfIndex (i, true);
                auto* comp = columnComponents[i];

                if (comp != nullptr && columnId != static_cast<int> (comp->getProperties() [columnProperty]))
                {
                    columnComponents.set (i, nullptr);
                    comp = nullptr;
                }

                comp = tableModel->refreshComponentForCell (row, columnId, isSelected, comp);
                columnComponents.set (i, comp, false);

                if (comp != nullptr)
                {
                    comp->getProperties().set (columnProperty, columnId);

                    addAndMakeVisible (comp);
                    resizeCustomComp (i);
                }
            }

            columnComponents.removeRange (numColumns, columnComponents.size());
        }
        else
        {
            columnComponents.clear();
        }
    }

    void resized() override
    {
        for (int i = columnComponents.size(); --i >= 0;)
            resizeCustomComp (i);
    }

    void resizeCustomComp (int index)
    {
        if (auto* c = columnComponents.getUnchecked (index))
            c->setBounds (owner.getHeader().getColumnPosition (index)
                            .withY (0).withHeight (getHeight()));
    }

    void mouseDown (const MouseEvent& e) override
    {
        isDragging = false;
        selectRowOnMouseUp = false;

        if (isEnabled())
        {
            if (! isSelected)
            {
                owner.selectRowsBasedOnModifierKeys (row, e.mods, false);

                auto columnId = owner.getHeader().getColumnIdAtX (e.x);

                if (columnId != 0)
                    if (auto* m = owner.getModel())
                        m->cellClicked (row, columnId, e);
            }
            else
            {
                selectRowOnMouseUp = true;
            }
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (isEnabled()
             && owner.getModel() != nullptr
             && e.mouseWasDraggedSinceMouseDown()
             && ! isDragging)
        {
            SparseSet<int> rowsToDrag;

            if (owner.selectOnMouseDown || owner.isRowSelected (row))
                rowsToDrag = owner.getSelectedRows();
            else
                rowsToDrag.addRange (Range<int>::withStartAndLength (row, 1));

            if (rowsToDrag.size() > 0)
            {
                auto dragDescription = owner.getModel()->getDragSourceDescription (rowsToDrag);

                if (! (dragDescription.isVoid() || (dragDescription.isString() && dragDescription.toString().isEmpty())))
                {
                    isDragging = true;
                    owner.startDragAndDrop (e, rowsToDrag, dragDescription, true);
                }
            }
        }
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (selectRowOnMouseUp && e.mouseWasClicked() && isEnabled())
        {
            owner.selectRowsBasedOnModifierKeys (row, e.mods, true);

            auto columnId = owner.getHeader().getColumnIdAtX (e.x);

            if (columnId != 0)
                if (TableListBoxModel* m = owner.getModel())
                    m->cellClicked (row, columnId, e);
        }
    }

    void mouseDoubleClick (const MouseEvent& e) override
    {
        auto columnId = owner.getHeader().getColumnIdAtX (e.x);

        if (columnId != 0)
            if (auto* m = owner.getModel())
                m->cellDoubleClicked (row, columnId, e);
    }

    String getTooltip() override
    {
        auto columnId = owner.getHeader().getColumnIdAtX (getMouseXYRelative().getX());

        if (columnId != 0)
            if (auto* m = owner.getModel())
                return m->getCellTooltip (row, columnId);

        return {};
    }

    Component* findChildComponentForColumn (int columnId) const
    {
        return columnComponents [owner.getHeader().getIndexOfColumnId (columnId, true)];
    }

private:
    TableListBox& owner;
    OwnedArray<Component> columnComponents;
    int row = -1;
    bool isSelected = false, isDragging = false, selectRowOnMouseUp = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowComp)
};


//==============================================================================
class TableListBox::Header  : public TableHeaderComponent
{
public:
    Header (TableListBox& tlb)  : owner (tlb) {}

    void addMenuItems (PopupMenu& menu, int columnIdClicked)
    {
        if (owner.isAutoSizeMenuOptionShown())
        {
            menu.addItem (autoSizeColumnId, TRANS("Auto-size this column"), columnIdClicked != 0);
            menu.addItem (autoSizeAllId, TRANS("Auto-size all columns"), owner.getHeader().getNumColumns (true) > 0);
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
    ListBox::model = this;

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

Component* TableListBoxModel::refreshComponentForCell (int, int, bool, Component* existingComponentToUpdate)
{
    ignoreUnused (existingComponentToUpdate);
    jassert (existingComponentToUpdate == nullptr); // indicates a failure in the code that recycles the components
    return nullptr;
}

} // namespace juce
