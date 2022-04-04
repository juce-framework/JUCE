/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** An abstract interface which represents a UI element that supports a cell interface.

    This typically represents a single cell inside of a UI element which implements an
    AccessibilityTableInterface.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityCellInterface
{
public:
    /** Destructor. */
    virtual ~AccessibilityCellInterface() = default;

    /** Returns the column index of the cell in the table. */
    virtual int getColumnIndex() const = 0;

    /** Returns the number of columns occupied by the cell in the table. */
    virtual int getColumnSpan() const = 0;

    /** Returns the row index of the cell in the table. */
    virtual int getRowIndex() const = 0;

    /** Returns the number of rows occupied by the cell in the table. */
    virtual int getRowSpan() const = 0;

    /** Returns the indentation level for the cell. */
    virtual int getDisclosureLevel() const = 0;

    /** Returns the AccessibilityHandler of the table which contains the cell. */
    virtual const AccessibilityHandler* getTableHandler() const = 0;
};

} // namespace juce
