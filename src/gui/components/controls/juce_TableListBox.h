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

#ifndef __JUCE_TABLELISTBOX_JUCEHEADER__
#define __JUCE_TABLELISTBOX_JUCEHEADER__

#include "juce_TableHeaderComponent.h"
#include "juce_ListBox.h"


//==============================================================================
/**
    One of these is used by a TableListBox as the data model for the table's contents.

    The virtual methods that you override in this class take care of drawing the
    table cells, and reacting to events.

    @see TableListBox
*/
class JUCE_API  TableListBoxModel
{
public:
    //==============================================================================
    TableListBoxModel()  {}

    /** Destructor. */
    virtual ~TableListBoxModel()  {}

    //==============================================================================
    /** This must return the number of rows currently in the table.

        If the number of rows changes, you must call TableListBox::updateContent() to
        cause it to refresh the list.
    */
    virtual int getNumRows() = 0;

    /** This must draw the background behind one of the rows in the table.

        The graphics context has its origin at the row's top-left, and your method
        should fill the area specified by the width and height parameters.
    */
    virtual void paintRowBackground (Graphics& g,
                                     int rowNumber,
                                     int width, int height,
                                     bool rowIsSelected) = 0;

    /** This must draw one of the cells.

        The graphics context's origin will already be set to the top-left of the cell,
        whose size is specified by (width, height).
    */
    virtual void paintCell (Graphics& g,
                            int rowNumber,
                            int columnId,
                            int width, int height,
                            bool rowIsSelected) = 0;

    //==============================================================================
    /** This is used to create or update a custom component to go in a cell.

        Any cell may contain a custom component, or can just be drawn with the paintCell() method
        and handle mouse clicks with cellClicked().

        This method will be called whenever a custom component might need to be updated - e.g.
        when the table is changed, or TableListBox::updateContent() is called.

        If you don't need a custom component for the specified cell, then return 0.

        If you do want a custom component, and the existingComponentToUpdate is null, then
        this method must create a new component suitable for the cell, and return it.

        If the existingComponentToUpdate is non-null, it will be a pointer to a component previously created
        by this method. In this case, the method must either update it to make sure it's correctly representing
        the given cell (which may be different from the one that the component was created for), or it can
        delete this component and return a new one.
    */
    virtual Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected,
                                                Component* existingComponentToUpdate);

    //==============================================================================
    /** This callback is made when the user clicks on one of the cells in the table.

        The mouse event's coordinates will be relative to the entire table row.
        @see cellDoubleClicked, backgroundClicked
    */
    virtual void cellClicked (int rowNumber, int columnId, const MouseEvent& e);

    /** This callback is made when the user clicks on one of the cells in the table.

        The mouse event's coordinates will be relative to the entire table row.
        @see cellClicked, backgroundClicked
    */
    virtual void cellDoubleClicked (int rowNumber, int columnId, const MouseEvent& e);

    /** This can be overridden to react to the user double-clicking on a part of the list where
        there are no rows.

        @see cellClicked
    */
    virtual void backgroundClicked();

    //==============================================================================
    /** This callback is made when the table's sort order is changed.

        This could be because the user has clicked a column header, or because the
        TableHeaderComponent::setSortColumnId() method was called.

        If you implement this, your method should re-sort the table using the given
        column as the key.
    */
    virtual void sortOrderChanged (int newSortColumnId, const bool isForwards);

    //==============================================================================
    /** Returns the best width for one of the columns.

        If you implement this method, you should measure the width of all the items
        in this column, and return the best size.

        Returning 0 means that the column shouldn't be changed.

        This is used by TableListBox::autoSizeColumn() and TableListBox::autoSizeAllColumns().
    */
    virtual int getColumnAutoSizeWidth (int columnId);

    /** Returns a tooltip for a particular cell in the table.
    */
    virtual const String getCellTooltip (int rowNumber, int columnId);

    //==============================================================================
    /** Override this to be informed when rows are selected or deselected.

        @see ListBox::selectedRowsChanged()
    */
    virtual void selectedRowsChanged (int lastRowSelected);

    /** Override this to be informed when the delete key is pressed.

        @see ListBox::deleteKeyPressed()
    */
    virtual void deleteKeyPressed (int lastRowSelected);

    /** Override this to be informed when the return key is pressed.

        @see ListBox::returnKeyPressed()
    */
    virtual void returnKeyPressed (int lastRowSelected);

    /** Override this to be informed when the list is scrolled.

        This might be caused by the user moving the scrollbar, or by programmatic changes
        to the list position.
    */
    virtual void listWasScrolled();

    /** To allow rows from your table to be dragged-and-dropped, implement this method.

        If this returns a non-empty name then when the user drags a row, the table will try to
        find a DragAndDropContainer in its parent hierarchy, and will use it to trigger a
        drag-and-drop operation, using this string as the source description, and the listbox
        itself as the source component.

        @see DragAndDropContainer::startDragging
    */
    virtual const String getDragSourceDescription (const SparseSet<int>& currentlySelectedRows);
};


//==============================================================================
/**
    A table of cells, using a TableHeaderComponent as its header.

    This component makes it easy to create a table by providing a TableListBoxModel as
    the data source.


    @see TableListBoxModel, TableHeaderComponent
*/
class JUCE_API  TableListBox   : public ListBox,
                                 private ListBoxModel,
                                 private TableHeaderListener
{
public:
    //==============================================================================
    /** Creates a TableListBox.

        The model pointer passed-in can be null, in which case you can set it later
        with setModel().
    */
    TableListBox (const String& componentName,
                  TableListBoxModel* const model);

    /** Destructor. */
    ~TableListBox();

    //==============================================================================
    /** Changes the TableListBoxModel that is being used for this table.
    */
    void setModel (TableListBoxModel* const newModel);

    /** Returns the model currently in use. */
    TableListBoxModel* getModel() const                             { return model; }

    //==============================================================================
    /** Returns the header component being used in this table. */
    TableHeaderComponent* getHeader() const                         { return header; }

    /** Changes the height of the table header component.
        @see getHeaderHeight
    */
    void setHeaderHeight (const int newHeight);

    /** Returns the height of the table header.
        @see setHeaderHeight
    */
    int getHeaderHeight() const;

    //==============================================================================
    /** Resizes a column to fit its contents.

        This uses TableListBoxModel::getColumnAutoSizeWidth() to find the best width,
        and applies that to the column.

        @see autoSizeAllColumns, TableHeaderComponent::setColumnWidth
    */
    void autoSizeColumn (const int columnId);

    /** Calls autoSizeColumn() for all columns in the table. */
    void autoSizeAllColumns();

    /** Enables or disables the auto size options on the popup menu.

        By default, these are enabled.
    */
    void setAutoSizeMenuOptionShown (const bool shouldBeShown);

    /** True if the auto-size options should be shown on the menu.
        @see setAutoSizeMenuOptionsShown
    */
    bool isAutoSizeMenuOptionShown() const;

    /** Returns the position of one of the cells in the table.

        If relativeToComponentTopLeft is true, the co-ordinates are relative to
        the table component's top-left. The row number isn't checked to see if it's
        in-range, but the column ID must exist or this will return an empty rectangle.

        If relativeToComponentTopLeft is false, the co-ords are relative to the
        top-left of the table's top-left cell.
    */
    const Rectangle getCellPosition (const int columnId,
                                     const int rowNumber,
                                     const bool relativeToComponentTopLeft) const;

    /** Scrolls horizontally if necessary to make sure that a particular column is visible.

        @see ListBox::scrollToEnsureRowIsOnscreen
    */
    void scrollToEnsureColumnIsOnscreen (const int columnId);

    //==============================================================================
    /** @internal */
    int getNumRows();
    /** @internal */
    void paintListBoxItem (int, Graphics&, int, int, bool);
    /** @internal */
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);
    /** @internal */
    void selectedRowsChanged (int lastRowSelected);
    /** @internal */
    void deleteKeyPressed (int currentSelectedRow);
    /** @internal */
    void returnKeyPressed (int currentSelectedRow);
    /** @internal */
    void backgroundClicked();
    /** @internal */
    void listWasScrolled();
    /** @internal */
    void tableColumnsChanged (TableHeaderComponent*);
    /** @internal */
    void tableColumnsResized (TableHeaderComponent*);
    /** @internal */
    void tableSortOrderChanged (TableHeaderComponent*);
    /** @internal */
    void tableColumnDraggingChanged (TableHeaderComponent*, int);
    /** @internal */
    void resized();

    juce_UseDebuggingNewOperator

    //==============================================================================
private:
    TableHeaderComponent* header;
    TableListBoxModel* model;
    int columnIdNowBeingDragged;
    bool autoSizeOptionsShown;

    void updateColumnComponents() const;

    TableListBox (const TableListBox&);
    const TableListBox& operator= (const TableListBox&);
};


#endif   // __JUCE_TABLELISTBOX_JUCEHEADER__
