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

    /** A simple span of elements. */
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
