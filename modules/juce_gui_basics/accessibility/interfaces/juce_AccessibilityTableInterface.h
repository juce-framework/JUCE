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

/** An abstract interface which represents a UI element that supports a table interface.

    Examples of UI elements which typically support a table interface are lists, tables,
    and trees.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityTableInterface
{
public:
    /** Destructor. */
    virtual ~AccessibilityTableInterface() = default;

    /** Returns the total number of rows in the table. */
    virtual int getNumRows() const = 0;

    /** Returns the total number of columns in the table. */
    virtual int getNumColumns() const = 0;

    /** Returns the AccessibilityHandler for one of the cells in the table, or
        nullptr if there is no cell at the specified position.
    */
    virtual const AccessibilityHandler* getCellHandler (int row, int column) const = 0;

    /** Returns the AccessibilityHandler for a row in the table, or nullptr if there is
        no row at this index.

        The row component should have a child component for each column in the table.
    */
    virtual const AccessibilityHandler* getRowHandler (int row) const = 0;

    /** Returns the AccessibilityHandler for the header, or nullptr if there is
        no header.

        If you supply a header, it must have exactly the same number of children
        as there are columns in the table.
    */
    virtual const AccessibilityHandler* getHeaderHandler() const = 0;

    struct Span { int begin, num; };

    /** Given the handler of one of the cells in the table, returns the rows covered
        by that cell, or null if the cell does not exist in the table.

        This function replaces the getRowIndex and getRowSpan
        functions from AccessibilityCellInterface. Most of the time, it's easier for the
        table itself to keep track of cell locations, than to delegate to the individual
        cells.
    */
    virtual Optional<Span> getRowSpan (const AccessibilityHandler&) const = 0;

    /** Given the handler of one of the cells in the table, returns the columns covered
        by that cell, or null if the cell does not exist in the table.

        This function replaces the getColumnIndex and getColumnSpan
        functions from AccessibilityCellInterface. Most of the time, it's easier for the
        table itself to keep track of cell locations, than to delegate to the individual
        cells.
    */
    virtual Optional<Span> getColumnSpan (const AccessibilityHandler&) const = 0;

    /** Attempts to scroll the table (if necessary) so that the cell with the given handler
        is visible.
    */
    virtual void showCell (const AccessibilityHandler&) const = 0;
};

} // namespace juce
