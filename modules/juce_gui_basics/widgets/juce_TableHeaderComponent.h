/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A component that displays a strip of column headings for a table, and allows these
    to be resized, dragged around, etc.

    This is just the component that goes at the top of a table. You can use it
    directly for custom components, or to create a simple table, use the
    TableListBox class.

    To use one of these, create it and use addColumn() to add all the columns that you need.
    Each column must be given a unique ID number that's used to refer to it.

    @see TableListBox, TableHeaderComponent::Listener

    @tags{GUI}
*/
class JUCE_API  TableHeaderComponent   : public Component,
                                         private AsyncUpdater
{
public:
    //==============================================================================
    /** Creates an empty table header.
    */
    TableHeaderComponent();

    /** Destructor. */
    ~TableHeaderComponent() override;

    //==============================================================================
    /** A combination of these flags are passed into the addColumn() method to specify
        the properties of a column.
    */
    enum ColumnPropertyFlags
    {
        visible                     = 1,    /**< If this is set, the column will be shown; if not, it will be hidden until the user enables it with the pop-up menu. */
        resizable                   = 2,    /**< If this is set, the column can be resized by dragging it. */
        draggable                   = 4,    /**< If this is set, the column can be dragged around to change its order in the table. */
        appearsOnColumnMenu         = 8,    /**< If this is set, the column will be shown on the pop-up menu allowing it to be hidden/shown. */
        sortable                    = 16,   /**< If this is set, then clicking on the column header will set it to be the sort column, and clicking again will reverse the order. */
        sortedForwards              = 32,   /**< If this is set, the column is currently the one by which the table is sorted (forwards). */
        sortedBackwards             = 64,   /**< If this is set, the column is currently the one by which the table is sorted (backwards). */

        /** This set of default flags is used as the default parameter value in addColumn(). */
        defaultFlags                = (visible | resizable | draggable | appearsOnColumnMenu | sortable),

        /** A quick way of combining flags for a column that's not resizable. */
        notResizable                = (visible | draggable | appearsOnColumnMenu | sortable),

        /** A quick way of combining flags for a column that's not resizable or sortable. */
        notResizableOrSortable      = (visible | draggable | appearsOnColumnMenu),

        /** A quick way of combining flags for a column that's not sortable. */
        notSortable                 = (visible | resizable | draggable | appearsOnColumnMenu)
    };

    /** Adds a column to the table.

        This will add a column, and asynchronously call the tableColumnsChanged() method of any
        registered listeners.

        @param columnName       the name of the new column. It's ok to have two or more columns with the same name
        @param columnId         an ID for this column. The ID can be any number apart from 0, but every column must have
                                a unique ID. This is used to identify the column later on, after the user may have
                                changed the order that they appear in
        @param width            the initial width of the column, in pixels
        @param maximumWidth     a maximum width that the column can take when the user is resizing it. This only applies
                                if the 'resizable' flag is specified for this column
        @param minimumWidth     a minimum width that the column can take when the user is resizing it. This only applies
                                if the 'resizable' flag is specified for this column
        @param propertyFlags    a combination of some of the values from the ColumnPropertyFlags enum, to define the
                                properties of this column
        @param insertIndex      the index at which the column should be added. A value of 0 puts it at the start (left-hand side)
                                and -1 puts it at the end (right-hand size) of the table. Note that the index the index within
                                all columns, not just the index amongst those that are currently visible
    */
    void addColumn (const String& columnName,
                    int columnId,
                    int width,
                    int minimumWidth = 30,
                    int maximumWidth = -1,
                    int propertyFlags = defaultFlags,
                    int insertIndex = -1);

    /** Removes a column with the given ID.

        If there is such a column, this will asynchronously call the tableColumnsChanged() method of any
        registered listeners.
    */
    void removeColumn (int columnIdToRemove);

    /** Deletes all columns from the table.

        If there are any columns to remove, this will asynchronously call the tableColumnsChanged() method of any
        registered listeners.
    */
    void removeAllColumns();

    /** Returns the number of columns in the table.

        If onlyCountVisibleColumns is true, this will return the number of visible columns; otherwise it'll
        return the total number of columns, including hidden ones.

        @see isColumnVisible
    */
    int getNumColumns (bool onlyCountVisibleColumns) const;

    /** Returns the name for a column.
        @see setColumnName
    */
    String getColumnName (int columnId) const;

    /** Changes the name of a column. */
    void setColumnName (int columnId, const String& newName);

    /** Moves a column to a different index in the table.

        @param columnId             the column to move
        @param newVisibleIndex      the target index for it, from 0 to the number of columns currently visible.
    */
    void moveColumn (int columnId, int newVisibleIndex);

    /** Returns the width of one of the columns.
    */
    int getColumnWidth (int columnId) const;

    /** Changes the width of a column.

        This will cause an asynchronous callback to the tableColumnsResized() method of any registered listeners.
    */
    void setColumnWidth (int columnId, int newWidth);

    /** Shows or hides a column.

        This can cause an asynchronous callback to the tableColumnsChanged() method of any registered listeners.
        @see isColumnVisible
    */
    void setColumnVisible (int columnId, bool shouldBeVisible);

    /** Returns true if this column is currently visible.
        @see setColumnVisible
    */
    bool isColumnVisible (int columnId) const;

    /** Changes the column which is the sort column.

        This can cause an asynchronous callback to the tableSortOrderChanged() method of any registered listeners.

        If this method doesn't actually change the column ID, then no re-sort will take place (you can
        call reSortTable() to force a re-sort to happen if you've modified the table's contents).

        @see getSortColumnId, isSortedForwards, reSortTable
    */
    void setSortColumnId (int columnId, bool sortForwards);

    /** Returns the column ID by which the table is currently sorted, or 0 if it is unsorted.

        @see setSortColumnId, isSortedForwards
    */
    int getSortColumnId() const;

    /** Returns true if the table is currently sorted forwards, or false if it's backwards.
        @see setSortColumnId
    */
    bool isSortedForwards() const;

    /** Triggers a re-sort of the table according to the current sort-column.

        If you modify the table's contents, you can call this to signal that the table needs
        to be re-sorted.

        (This doesn't do any sorting synchronously - it just asynchronously sends a call to the
        tableSortOrderChanged() method of any listeners).
    */
    void reSortTable();

    //==============================================================================
    /** Returns the total width of all the visible columns in the table.
    */
    int getTotalWidth() const;

    /** Returns the index of a given column.

        If there's no such column ID, this will return -1.

        If onlyCountVisibleColumns is true, this will return the index amongst the visible columns;
        otherwise it'll return the index amongst all the columns, including any hidden ones.
    */
    int getIndexOfColumnId (int columnId, bool onlyCountVisibleColumns) const;

    /** Returns the ID of the column at a given index.

        If onlyCountVisibleColumns is true, this will count the index amongst the visible columns;
        otherwise it'll count it amongst all the columns, including any hidden ones.

        If the index is out-of-range, it'll return 0.
    */
    int getColumnIdOfIndex (int index, bool onlyCountVisibleColumns) const;

    /** Returns the rectangle containing of one of the columns.

        The index is an index from 0 to the number of columns that are currently visible (hidden
        ones are not counted). It returns a rectangle showing the position of the column relative
        to this component's top-left. If the index is out-of-range, an empty rectangle is returned.
    */
    Rectangle<int> getColumnPosition (int index) const;

    /** Finds the column ID at a given x-position in the component.
        If there is a column at this point this returns its ID, or if not, it will return 0.
    */
    int getColumnIdAtX (int xToFind) const;

    /** If set to true, this indicates that the columns should be expanded or shrunk to fill the
        entire width of the component.

        By default this is disabled. Turning it on also means that when resizing a column, those
        on the right will be squashed to fit.
    */
    void setStretchToFitActive (bool shouldStretchToFit);

    /** Returns true if stretch-to-fit has been enabled.
        @see setStretchToFitActive
    */
    bool isStretchToFitActive() const;

    /** If stretch-to-fit is enabled, this will resize all the columns to make them fit into the
        specified width, keeping their relative proportions the same.

        If the minimum widths of the columns are too wide to fit into this space, it may
        actually end up wider.
    */
    void resizeAllColumnsToFit (int targetTotalWidth);

    //==============================================================================
    /** Enables or disables the pop-up menu.

        The default menu allows the user to show or hide columns. You can add custom
        items to this menu by overloading the addMenuItems() and reactToMenuItem() methods.

        By default the menu is enabled.

        @see isPopupMenuActive, addMenuItems, reactToMenuItem
    */
    void setPopupMenuActive (bool hasMenu);

    /** Returns true if the pop-up menu is enabled.
        @see setPopupMenuActive
    */
    bool isPopupMenuActive() const;

    //==============================================================================
    /** Returns a string that encapsulates the table's current layout.

        This can be restored later using restoreFromString(). It saves the order of
        the columns, the currently-sorted column, and the widths.

        @see restoreFromString
    */
    String toString() const;

    /** Restores the state of the table, based on a string previously created with
        toString().

        @see toString
    */
    void restoreFromString (const String& storedVersion);

    //==============================================================================
    /**
        Receives events from a TableHeaderComponent when columns are resized, moved, etc.

        You can register one of these objects for table events using TableHeaderComponent::addListener()
        and TableHeaderComponent::removeListener().

        @see TableHeaderComponent
    */
    class JUCE_API  Listener
    {
    public:
        //==============================================================================
        Listener() = default;

        /** Destructor. */
        virtual ~Listener() = default;

        //==============================================================================
        /** This is called when some of the table's columns are added, removed, hidden,
            or rearranged.
        */
        virtual void tableColumnsChanged (TableHeaderComponent* tableHeader) = 0;

        /** This is called when one or more of the table's columns are resized. */
        virtual void tableColumnsResized (TableHeaderComponent* tableHeader) = 0;

        /** This is called when the column by which the table should be sorted is changed. */
        virtual void tableSortOrderChanged (TableHeaderComponent* tableHeader) = 0;

        /** This is called when the user begins or ends dragging one of the columns around.

            When the user starts dragging a column, this is called with the ID of that
            column. When they finish dragging, it is called again with 0 as the ID.
        */
        virtual void tableColumnDraggingChanged (TableHeaderComponent* tableHeader,
                                                 int columnIdNowBeingDragged);
    };

    /** Adds a listener to be informed about things that happen to the header. */
    void addListener (Listener* newListener);

    /** Removes a previously-registered listener. */
    void removeListener (Listener* listenerToRemove);

    //==============================================================================
    /** This can be overridden to handle a mouse-click on one of the column headers.

        The default implementation will use this click to call getSortColumnId() and
        change the sort order.
    */
    virtual void columnClicked (int columnId, const ModifierKeys& mods);

    /** This can be overridden to add custom items to the pop-up menu.

        If you override this, you should call the superclass's method to add its
        column show/hide items, if you want them on the menu as well.

        Then to handle the result, override reactToMenuItem().

        @see reactToMenuItem
    */
    virtual void addMenuItems (PopupMenu& menu, int columnIdClicked);

    /** Override this to handle any custom items that you have added to the
        pop-up menu with an addMenuItems() override.

        If the menuReturnId isn't one of your own custom menu items, you'll need to
        call TableHeaderComponent::reactToMenuItem() to allow the base class to
        handle the items that it had added.

        @see addMenuItems
    */
    virtual void reactToMenuItem (int menuReturnId, int columnIdClicked);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the TableHeaderComponent.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        textColourId                   = 0x1003800, /**< The colour for the text in the header. */
        backgroundColourId             = 0x1003810, /**< The colour of the table header background.
                                                         It's up to the LookAndFeel how this is used. */
        outlineColourId                = 0x1003820, /**< The colour of the table header's outline. */
        highlightColourId              = 0x1003830, /**< The colour of the table header background when
                                                         the mouse is over or down above the the table
                                                         header. It's up to the LookAndFeel to use a
                                                         variant of this colour to distinguish between
                                                         the down and hover state. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void drawTableHeaderBackground (Graphics&, TableHeaderComponent&) = 0;

        virtual void drawTableHeaderColumn (Graphics&, TableHeaderComponent&,
                                            const String& columnName, int columnId,
                                            int width, int height,
                                            bool isMouseOver, bool isMouseDown, int columnFlags) = 0;
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void mouseMove (const MouseEvent&) override;
    /** @internal */
    void mouseEnter (const MouseEvent&) override;
    /** @internal */
    void mouseExit (const MouseEvent&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    MouseCursor getMouseCursor() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

    /** Can be overridden for more control over the pop-up menu behaviour. */
    virtual void showColumnChooserMenu (int columnIdClicked);

private:
    struct ColumnInfo : public Component
    {
        ColumnInfo() { setInterceptsMouseClicks (false, false); }
        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

        int id, propertyFlags, width, minimumWidth, maximumWidth;
        double lastDeliberateWidth;
    };

    OwnedArray<ColumnInfo> columns;
    Array<Listener*> listeners;
    std::unique_ptr<Component> dragOverlayComp;
    class DragOverlayComp;

    bool columnsChanged = false, columnsResized = false, sortChanged = false;
    bool menuActive = true, stretchToFit = false;
    int columnIdBeingResized = 0, columnIdBeingDragged = 0, initialColumnWidth = 0;
    int columnIdUnderMouse = 0, draggingColumnOffset = 0, draggingColumnOriginalIndex = 0, lastDeliberateWidth = 0;

    ColumnInfo* getInfoForId (int columnId) const;
    int visibleIndexToTotalIndex (int visibleIndex) const;
    void sendColumnsChanged();
    void handleAsyncUpdate() override;
    void beginDrag (const MouseEvent&);
    void endDrag (int finalIndex);
    int getResizeDraggerAt (int mouseX) const;
    void updateColumnUnderMouse (const MouseEvent&);
    void setColumnUnderMouse (int columnId);
    void resizeColumnsToFit (int firstColumnIndex, int targetTotalWidth);
    void drawColumnHeader (Graphics&, LookAndFeel&, const ColumnInfo&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableHeaderComponent)
};


} // namespace juce
