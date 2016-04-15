/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"


//==============================================================================
class ValueTreeItem  : public TreeViewItem,
                       private ValueTree::Listener
{
public:
    ValueTreeItem (const ValueTree& v, UndoManager& um)
        : tree (v), undoManager (um)
    {
        tree.addListener (this);
    }

    String getUniqueName() const override
    {
        return tree["name"].toString();
    }

    bool mightContainSubItems() override
    {
        return tree.getNumChildren() > 0;
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        g.setColour (Colours::black);
        g.setFont (15.0f);

        g.drawText (tree["name"].toString(),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen && getNumSubItems() == 0)
            refreshSubItems();
        else
            clearSubItems();
    }

    var getDragSourceDescription() override
    {
        return "Drag Demo";
    }

    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        return dragSourceDetails.description == "Drag Demo";
    }

    void itemDropped (const DragAndDropTarget::SourceDetails&, int insertIndex) override
    {
        OwnedArray<ValueTree> selectedTrees;
        getSelectedTreeViewItems (*getOwnerView(), selectedTrees);

        moveItems (*getOwnerView(), selectedTrees, tree, insertIndex, undoManager);
    }

    static void moveItems (TreeView& treeView, const OwnedArray<ValueTree>& items,
                           ValueTree newParent, int insertIndex, UndoManager& undoManager)
    {
        if (items.size() > 0)
        {
            ScopedPointer<XmlElement> oldOpenness (treeView.getOpennessState (false));

            for (int i = items.size(); --i >= 0;)
            {
                ValueTree& v = *items.getUnchecked(i);

                if (v.getParent().isValid() && newParent != v && ! newParent.isAChildOf (v))
                {
                    if (v.getParent() == newParent && newParent.indexOf(v) < insertIndex)
                        --insertIndex;

                    v.getParent().removeChild (v, &undoManager);
                    newParent.addChild (v, insertIndex, &undoManager);
                }
            }

            if (oldOpenness != nullptr)
                treeView.restoreOpennessState (*oldOpenness, false);
        }
    }

    static void getSelectedTreeViewItems (TreeView& treeView, OwnedArray<ValueTree>& items)
    {
        const int numSelected = treeView.getNumSelectedItems();

        for (int i = 0; i < numSelected; ++i)
            if (const ValueTreeItem* vti = dynamic_cast<ValueTreeItem*> (treeView.getSelectedItem (i)))
                items.add (new ValueTree (vti->tree));
    }

private:
    ValueTree tree;
    UndoManager& undoManager;

    void refreshSubItems()
    {
        clearSubItems();

        for (int i = 0; i < tree.getNumChildren(); ++i)
            addSubItem (new ValueTreeItem (tree.getChild (i), undoManager));
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override
    {
        repaintItem();
    }

    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { treeChildrenChanged (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override  { treeChildrenChanged (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override    { treeChildrenChanged (parentTree); }
    void valueTreeParentChanged (ValueTree&) override {}

    void treeChildrenChanged (const ValueTree& parentTree)
    {
        if (parentTree == tree)
        {
            refreshSubItems();
            treeHasChanged();
            setOpen (true);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreeItem)
};

//==============================================================================
class ValueTreesDemo   : public Component,
                         public DragAndDropContainer,
                         private ButtonListener,
                         private Timer
{
public:
    ValueTreesDemo()
        : undoButton ("Undo"),
          redoButton ("Redo")
    {
        addAndMakeVisible (tree);

        tree.setDefaultOpenness (true);
        tree.setMultiSelectEnabled (true);
        tree.setRootItem (rootItem = new ValueTreeItem (createRootValueTree(), undoManager));
        tree.setColour (TreeView::backgroundColourId, Colours::white);

        addAndMakeVisible (undoButton);
        addAndMakeVisible (redoButton);
        undoButton.addListener (this);
        redoButton.addListener (this);

        startTimer (500);
    }

    ~ValueTreesDemo()
    {
        tree.setRootItem (nullptr);
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (8));

        Rectangle<int> buttons (r.removeFromBottom (22));
        undoButton.setBounds (buttons.removeFromLeft (100));
        buttons.removeFromLeft (6);
        redoButton.setBounds (buttons.removeFromLeft (100));

        r.removeFromBottom (4);
        tree.setBounds (r);
    }

    static ValueTree createTree (const String& desc)
    {
        ValueTree t ("Item");
        t.setProperty ("name", desc, nullptr);
        return t;
    }

    static ValueTree createRootValueTree()
    {
        ValueTree vt = createTree ("This demo displays a ValueTree as a treeview.");
        vt.addChild (createTree ("You can drag around the nodes to rearrange them"), -1, nullptr);
        vt.addChild (createTree ("..and press 'delete' to delete them"), -1, nullptr);
        vt.addChild (createTree ("Then, you can use the undo/redo buttons to undo these changes"), -1, nullptr);

        int n = 1;
        vt.addChild (createRandomTree (n, 0), -1, nullptr);

        return vt;
    }

    static ValueTree createRandomTree (int& counter, int depth)
    {
        ValueTree t = createTree ("Item " + String (counter++));

        if (depth < 3)
            for (int i = 1 + Random::getSystemRandom().nextInt (7); --i >= 0;)
                t.addChild (createRandomTree (counter, depth + 1), -1, nullptr);

        return t;
    }

    void deleteSelectedItems()
    {
        OwnedArray<ValueTree> selectedItems;
        ValueTreeItem::getSelectedTreeViewItems (tree, selectedItems);

        for (int i = selectedItems.size(); --i >= 0;)
        {
            ValueTree& v = *selectedItems.getUnchecked(i);

            if (v.getParent().isValid())
                v.getParent().removeChild (v, &undoManager);
        }
    }

    bool keyPressed (const KeyPress& key) override
    {
        if (key == KeyPress::deleteKey)
        {
            deleteSelectedItems();
            return true;
        }

        if (key == KeyPress ('z', ModifierKeys::commandModifier, 0))
        {
            undoManager.undo();
            return true;
        }

        if (key == KeyPress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
        {
            undoManager.redo();
            return true;
        }

        return Component::keyPressed (key);
    }

    void buttonClicked (Button* b) override
    {
        if (b == &undoButton)
            undoManager.undo();
        else if (b == &redoButton)
            undoManager.redo();
    }

private:
    TreeView tree;
    TextButton undoButton, redoButton;
    ScopedPointer<ValueTreeItem> rootItem;
    UndoManager undoManager;

    void timerCallback() override
    {
        undoManager.beginNewTransaction();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreesDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<ValueTreesDemo> demo ("40 ValueTrees");
