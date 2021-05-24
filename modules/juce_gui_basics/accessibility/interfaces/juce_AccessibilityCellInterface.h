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
