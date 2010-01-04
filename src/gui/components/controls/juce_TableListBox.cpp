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

#include "juce_TableListBox.h"
#include "../../../containers/juce_BitArray.h"
#include "../../../core/juce_Random.h"
#include "../mouse/juce_DragAndDropContainer.h"
#include "../../graphics/imaging/juce_Image.h"
#include "../../../text/juce_LocalisedStrings.h"


//==============================================================================
static const tchar* const tableColumnPropertyTag = T("_tableColumnID");

class TableListRowComp   : public Component,
                           public TooltipClient
{
public:
    TableListRowComp (TableListBox& owner_)
        : owner (owner_),
          row (-1),
          isSelected (false)
    {
    }

    ~TableListRowComp()
    {
        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
        TableListBoxModel* const model = owner.getModel();

        if (model != 0)
        {
            const TableHeaderComponent* const header = owner.getHeader();

            model->paintRowBackground (g, row, getWidth(), getHeight(), isSelected);

            const int numColumns = header->getNumColumns (true);

            for (int i = 0; i < numColumns; ++i)
            {
                if (! columnsWithComponents [i])
                {
                    const int columnId = header->getColumnIdOfIndex (i, true);
                    Rectangle columnRect (header->getColumnPosition (i));
                    columnRect.setSize (columnRect.getWidth(), getHeight());

                    g.saveState();

                    g.reduceClipRegion (columnRect);
                    g.setOrigin (columnRect.getX(), 0);

                    model->paintCell (g, row, columnId, columnRect.getWidth(), columnRect.getHeight(), isSelected);

                    g.restoreState();
                }
            }
        }
    }

    void update (const int newRow, const bool isNowSelected)
    {
        if (newRow != row || isNowSelected != isSelected)
        {
            row = newRow;
            isSelected = isNowSelected;
            repaint();
        }

        if (row < owner.getNumRows())
        {
            jassert (row >= 0);

            const tchar* const tagPropertyName = T("_tableLastUseNum");
            const int newTag = Random::getSystemRandom().nextInt();

            const TableHeaderComponent* const header = owner.getHeader();
            const int numColumns = header->getNumColumns (true);
            int i;

            columnsWithComponents.clear();

            if (owner.getModel() != 0)
            {
                for (i = 0; i < numColumns; ++i)
                {
                    const int columnId = header->getColumnIdOfIndex (i, true);

                    Component* const newComp
                        = owner.getModel()->refreshComponentForCell (row, columnId, isSelected,
                                                                     findChildComponentForColumn (columnId));

                    if (newComp != 0)
                    {
                        addAndMakeVisible (newComp);
                        newComp->setComponentProperty (tagPropertyName, newTag);
                        newComp->setComponentProperty (tableColumnPropertyTag, columnId);

                        const Rectangle columnRect (header->getColumnPosition (i));
                        newComp->setBounds (columnRect.getX(), 0, columnRect.getWidth(), getHeight());

                        columnsWithComponents.setBit (i);
                    }
                }
            }

            for (i = getNumChildComponents(); --i >= 0;)
            {
                Component* const c = getChildComponent (i);

                if (c->getComponentPropertyInt (tagPropertyName, false, 0) != newTag)
                    delete c;
            }
        }
        else
        {
            columnsWithComponents.clear();
            deleteAllChildren();
        }
    }

    void resized()
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            Component* const c = getChildComponent (i);

            const int columnId = c->getComponentPropertyInt (tableColumnPropertyTag, false, 0);

            if (columnId != 0)
            {
                const Rectangle columnRect (owner.getHeader()->getColumnPosition (owner.getHeader()->getIndexOfColumnId (columnId, true)));
                c->setBounds (columnRect.getX(), 0, columnRect.getWidth(), getHeight());
            }
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        isDragging = false;
        selectRowOnMouseUp = false;

        if (isEnabled())
        {
            if (! isSelected)
            {
                owner.selectRowsBasedOnModifierKeys (row, e.mods);

                const int columnId = owner.getHeader()->getColumnIdAtX (e.x);

                if (columnId != 0 && owner.getModel() != 0)
                    owner.getModel()->cellClicked (row, columnId, e);
            }
            else
            {
                selectRowOnMouseUp = true;
            }
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (isEnabled() && owner.getModel() != 0 && ! (e.mouseWasClicked() || isDragging))
        {
            const SparseSet <int> selectedRows (owner.getSelectedRows());

            if (selectedRows.size() > 0)
            {
                const String dragDescription (owner.getModel()->getDragSourceDescription (selectedRows));

                if (dragDescription.isNotEmpty())
                {
                    isDragging = true;
                    owner.startDragAndDrop (e, dragDescription);
                }
            }
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        if (selectRowOnMouseUp && e.mouseWasClicked() && isEnabled())
        {
            owner.selectRowsBasedOnModifierKeys (row, e.mods);

            const int columnId = owner.getHeader()->getColumnIdAtX (e.x);

            if (columnId != 0 && owner.getModel() != 0)
                owner.getModel()->cellClicked (row, columnId, e);
        }
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        const int columnId = owner.getHeader()->getColumnIdAtX (e.x);

        if (columnId != 0 && owner.getModel() != 0)
            owner.getModel()->cellDoubleClicked (row, columnId, e);
    }

    const String getTooltip()
    {
        int x, y;
        getMouseXYRelative (x, y);

        const int columnId = owner.getHeader()->getColumnIdAtX (x);

        if (columnId != 0 && owner.getModel() != 0)
            return owner.getModel()->getCellTooltip (row, columnId);

        return String::empty;
    }

    juce_UseDebuggingNewOperator

private:
    TableListBox& owner;
    int row;
    bool isSelected, isDragging, selectRowOnMouseUp;
    BitArray columnsWithComponents;

    Component* findChildComponentForColumn (const int columnId) const
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            Component* const c = getChildComponent (i);

            if (c->getComponentPropertyInt (tableColumnPropertyTag, false, 0) == columnId)
                return c;
        }

        return 0;
    }

    TableListRowComp (const TableListRowComp&);
    const TableListRowComp& operator= (const TableListRowComp&);
};


//==============================================================================
class TableListBoxHeader  : public TableHeaderComponent
{
public:
    TableListBoxHeader (TableListBox& owner_)
        : owner (owner_)
    {
    }

    ~TableListBoxHeader()
    {
    }

    void addMenuItems (PopupMenu& menu, const int columnIdClicked)
    {
        if (owner.isAutoSizeMenuOptionShown())
        {
            menu.addItem (0xf836743, TRANS("Auto-size this column"), columnIdClicked != 0);
            menu.addItem (0xf836744, TRANS("Auto-size all columns"), owner.getHeader()->getNumColumns (true) > 0);
            menu.addSeparator();
        }

        TableHeaderComponent::addMenuItems (menu, columnIdClicked);
    }

    void reactToMenuItem (const int menuReturnId, const int columnIdClicked)
    {
        if (menuReturnId == 0xf836743)
        {
            owner.autoSizeColumn (columnIdClicked);
        }
        else if (menuReturnId == 0xf836744)
        {
            owner.autoSizeAllColumns();
        }
        else
        {
            TableHeaderComponent::reactToMenuItem (menuReturnId, columnIdClicked);
        }
    }

    juce_UseDebuggingNewOperator

private:
    TableListBox& owner;

    TableListBoxHeader (const TableListBoxHeader&);
    const TableListBoxHeader& operator= (const TableListBoxHeader&);
};

//==============================================================================
TableListBox::TableListBox (const String& name, TableListBoxModel* const model_)
    : ListBox (name, 0),
      model (model_),
      autoSizeOptionsShown (true)
{
    ListBox::model = this;

    header = new TableListBoxHeader (*this);
    header->setSize (100, 28);
    header->addListener (this);

    setHeaderComponent (header);
}

TableListBox::~TableListBox()
{
    deleteAllChildren();
}

void TableListBox::setModel (TableListBoxModel* const newModel)
{
    if (model != newModel)
    {
        model = newModel;
        updateContent();
    }
}

int TableListBox::getHeaderHeight() const
{
    return header->getHeight();
}

void TableListBox::setHeaderHeight (const int newHeight)
{
    header->setSize (header->getWidth(), newHeight);
    resized();
}

void TableListBox::autoSizeColumn (const int columnId)
{
    const int width = model != 0 ? model->getColumnAutoSizeWidth (columnId) : 0;

    if (width > 0)
        header->setColumnWidth (columnId, width);
}

void TableListBox::autoSizeAllColumns()
{
    for (int i = 0; i < header->getNumColumns (true); ++i)
        autoSizeColumn (header->getColumnIdOfIndex (i, true));
}

void TableListBox::setAutoSizeMenuOptionShown (const bool shouldBeShown)
{
    autoSizeOptionsShown = shouldBeShown;
}

bool TableListBox::isAutoSizeMenuOptionShown() const
{
    return autoSizeOptionsShown;
}

const Rectangle TableListBox::getCellPosition (const int columnId,
                                               const int rowNumber,
                                               const bool relativeToComponentTopLeft) const
{
    Rectangle headerCell (header->getColumnPosition (header->getIndexOfColumnId (columnId, true)));

    if (relativeToComponentTopLeft)
        headerCell.translate (header->getX(), 0);

    const Rectangle row (getRowPosition (rowNumber, relativeToComponentTopLeft));

    return Rectangle (headerCell.getX(), row.getY(),
                      headerCell.getWidth(), row.getHeight());
}

void TableListBox::scrollToEnsureColumnIsOnscreen (const int columnId)
{
    ScrollBar* const scrollbar = getHorizontalScrollBar();

    if (scrollbar != 0)
    {
        const Rectangle pos (header->getColumnPosition (header->getIndexOfColumnId (columnId, true)));

        double x = scrollbar->getCurrentRangeStart();
        const double w = scrollbar->getCurrentRangeSize();

        if (pos.getX() < x)
            x = pos.getX();
        else if (pos.getRight() > x + w)
            x += jmax (0.0, pos.getRight() - (x + w));

        scrollbar->setCurrentRangeStart (x);
    }
}

int TableListBox::getNumRows()
{
    return model != 0 ? model->getNumRows() : 0;
}

void TableListBox::paintListBoxItem (int, Graphics&, int, int, bool)
{
}

Component* TableListBox::refreshComponentForRow (int rowNumber, bool isRowSelected_, Component* existingComponentToUpdate)
{
    if (existingComponentToUpdate == 0)
        existingComponentToUpdate = new TableListRowComp (*this);

    ((TableListRowComp*) existingComponentToUpdate)->update (rowNumber, isRowSelected_);

    return existingComponentToUpdate;
}

void TableListBox::selectedRowsChanged (int row)
{
    if (model != 0)
        model->selectedRowsChanged (row);
}

void TableListBox::deleteKeyPressed (int row)
{
    if (model != 0)
        model->deleteKeyPressed (row);
}

void TableListBox::returnKeyPressed (int row)
{
    if (model != 0)
        model->returnKeyPressed (row);
}

void TableListBox::backgroundClicked()
{
    if (model != 0)
        model->backgroundClicked();
}

void TableListBox::listWasScrolled()
{
    if (model != 0)
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
    if (model != 0)
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
    const int firstRow = getRowContainingPosition (0, 0);

    for (int i = firstRow + getNumRowsOnScreen() + 2; --i >= firstRow;)
    {
        TableListRowComp* const rowComp = dynamic_cast <TableListRowComp*> (getComponentForRowNumber (i));

        if (rowComp != 0)
            rowComp->resized();
    }
}

//==============================================================================
void TableListBoxModel::cellClicked (int, int, const MouseEvent&)
{
}

void TableListBoxModel::cellDoubleClicked (int, int, const MouseEvent&)
{
}

void TableListBoxModel::backgroundClicked()
{
}

void TableListBoxModel::sortOrderChanged (int, const bool)
{
}

int TableListBoxModel::getColumnAutoSizeWidth (int)
{
    return 0;
}

void TableListBoxModel::selectedRowsChanged (int)
{
}

void TableListBoxModel::deleteKeyPressed (int)
{
}

void TableListBoxModel::returnKeyPressed (int)
{
}

void TableListBoxModel::listWasScrolled()
{
}

const String TableListBoxModel::getCellTooltip (int /*rowNumber*/, int /*columnId*/)
{
    return String::empty;
}

const String TableListBoxModel::getDragSourceDescription (const SparseSet<int>&)
{
    return String::empty;
}

Component* TableListBoxModel::refreshComponentForCell (int, int, bool, Component* existingComponentToUpdate)
{
    (void) existingComponentToUpdate;
    jassert (existingComponentToUpdate == 0); // indicates a failure in the code the recycles the components
    return 0;
}


END_JUCE_NAMESPACE
