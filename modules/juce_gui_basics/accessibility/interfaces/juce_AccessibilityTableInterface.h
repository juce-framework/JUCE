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
};

} // namespace juce
