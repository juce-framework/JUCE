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

#pragma once

//==============================================================================
class StartPageTreeHolder final : public Component
{
public:
    enum class Open { no, yes };

    StartPageTreeHolder (const String& title,
                         const StringArray& headerNames,
                         const std::vector<StringArray>& itemNames,
                         std::function<void (int, int)>&& selectedCallback,
                         Open shouldBeOpen)
        : headers (headerNames),
          items (itemNames),
          itemSelectedCallback (std::move (selectedCallback))
    {
        jassert (headers.size() == (int) items.size());

        tree.setTitle (title);
        tree.setRootItem (new TreeRootItem (*this));
        tree.setRootItemVisible (false);
        tree.setIndentSize (15);
        tree.setDefaultOpenness (shouldBeOpen == Open::yes);

        addAndMakeVisible (tree);
    }

    ~StartPageTreeHolder() override
    {
        tree.deleteRootItem();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    void resized() override
    {
        tree.setBounds (getLocalBounds());
    }

    void setSelectedItem (const String& category, int index)
    {
        auto* root = tree.getRootItem();

        for (int i = root->getNumSubItems(); --i >=0;)
        {
            if (auto* item = root->getSubItem (i))
            {
                if (item->getUniqueName() == category)
                    item->getSubItem (index)->setSelected (true, true);
            }
        }
    }

private:
    //==============================================================================
    class TreeSubItem final : public TreeViewItem
    {
    public:
        TreeSubItem (StartPageTreeHolder& o, const String& n, const StringArray& subItemsIn)
            : owner (o), name (n), isHeader (subItemsIn.size() > 0)
        {
            for (auto& s : subItemsIn)
                addSubItem (new TreeSubItem (owner, s, {}));
        }

        bool mightContainSubItems() override    { return isHeader; }
        bool canBeSelected() const override     { return ! isHeader; }

        int getItemWidth() const override       { return -1; }
        int getItemHeight() const override      { return 25; }

        String getUniqueName() const override   { return name; }
        String getAccessibilityName() override  { return getUniqueName(); }

        void paintOpenCloseButton (Graphics& g, const Rectangle<float>& area, Colour, bool isMouseOver) override
        {
            g.setColour (getOwnerView()->findColour (isSelected() ? defaultHighlightedTextColourId
                                                                  : treeIconColourId));

            TreeViewItem::paintOpenCloseButton (g, area, getOwnerView()->findColour (defaultIconColourId), isMouseOver);
        }

        void paintItem (Graphics& g, int w, int h) override
        {
            Rectangle<int> bounds (w, h);

            auto shouldBeHighlighted = isSelected();

            if (shouldBeHighlighted)
            {
                g.setColour (getOwnerView()->findColour (defaultHighlightColourId));
                g.fillRect (bounds);
            }

            g.setColour (shouldBeHighlighted ? getOwnerView()->findColour (defaultHighlightedTextColourId)
                                             : getOwnerView()->findColour (defaultTextColourId));

            g.drawFittedText (name, bounds.reduced (5).withTrimmedLeft (10), Justification::centredLeft, 1);
        }

        void itemClicked (const MouseEvent& e) override
        {
            if (isSelected())
                itemSelectionChanged (true);

            if (e.mods.isPopupMenu() && mightContainSubItems())
                setOpen (! isOpen());
        }

        void itemSelectionChanged (bool isNowSelected) override
        {
            jassert (! isHeader);

            if (isNowSelected)
                owner.itemSelectedCallback (getParentItem()->getIndexInParent(), getIndexInParent());
        }

    private:
        StartPageTreeHolder& owner;
        String name;
        bool isHeader = false;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeSubItem)
    };

    struct TreeRootItem final : public TreeViewItem
    {
        explicit TreeRootItem (StartPageTreeHolder& o)
            : owner (o)
        {
            for (int i = 0; i < owner.headers.size(); ++i)
                addSubItem (new TreeSubItem (owner, owner.headers[i], owner.items[(size_t) i]));
        }

        bool mightContainSubItems() override { return ! owner.headers.isEmpty();}

        StartPageTreeHolder& owner;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeRootItem)
    };

    //==============================================================================
    TreeView tree;
    StringArray headers;
    std::vector<StringArray> items;

    std::function<void (int, int)> itemSelectedCallback;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartPageTreeHolder)
};
