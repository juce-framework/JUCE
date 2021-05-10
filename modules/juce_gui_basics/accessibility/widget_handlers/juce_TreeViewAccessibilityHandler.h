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

/** Basic accessible interface for a TreeView.

    @tags{Accessibility}
*/
class JUCE_API  TreeViewAccessibilityHandler  : public AccessibilityHandler
{
public:
    explicit TreeViewAccessibilityHandler (TreeView& treeViewToWrap)
        : AccessibilityHandler (treeViewToWrap,
                                AccessibilityRole::tree,
                                {},
                                { std::make_unique<TreeViewTableInterface> (treeViewToWrap) })
    {
    }

private:
    class TreeViewTableInterface  : public AccessibilityTableInterface
    {
    public:
        explicit TreeViewTableInterface (TreeView& treeViewToWrap)
            : treeView (treeViewToWrap)
        {
        }

        int getNumRows() const override
        {
            return treeView.getNumRowsInTree();
        }

        int getNumColumns() const override
        {
            return 1;
        }

        const AccessibilityHandler* getCellHandler (int row, int) const override
        {
            if (auto* itemComp = treeView.getItemComponent (treeView.getItemOnRow (row)))
                return itemComp->getAccessibilityHandler();

            return nullptr;
        }

    private:
        TreeView& treeView;
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeViewAccessibilityHandler)
};

} // namespace juce
