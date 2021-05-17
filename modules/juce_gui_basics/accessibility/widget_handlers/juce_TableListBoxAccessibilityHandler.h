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

/** Basic accessible interface for a TableListBox.

    @tags{Accessibility}
*/
class JUCE_API  TableListBoxAccessibilityHandler  : public AccessibilityHandler
{
public:
    explicit TableListBoxAccessibilityHandler (TableListBox& tableListBoxToWrap)
        : AccessibilityHandler (tableListBoxToWrap,
                                AccessibilityRole::list,
                                {},
                                { std::make_unique<TableListBoxTableInterface> (tableListBoxToWrap) })
    {
    }

private:
    class TableListBoxTableInterface  : public AccessibilityTableInterface
    {
    public:
        explicit TableListBoxTableInterface (TableListBox& tableListBoxToWrap)
            : tableListBox (tableListBoxToWrap)
        {
        }

        int getNumRows() const override
        {
            if (auto* model = tableListBox.getModel())
                return model->getNumRows();

            return 0;
        }

        int getNumColumns() const override
        {
            return tableListBox.getHeader().getNumColumns (false);
        }

        const AccessibilityHandler* getCellHandler (int row, int column) const override
        {
            if (isPositiveAndBelow (row, getNumRows()) && isPositiveAndBelow (column, getNumColumns()))
                if (auto* cellComponent = tableListBox.getCellComponent (tableListBox.getHeader().getColumnIdOfIndex (column, false), row))
                    return cellComponent->getAccessibilityHandler();

            return nullptr;
        }

    private:
        TableListBox& tableListBox;
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableListBoxAccessibilityHandler)
};

} // namespace juce
