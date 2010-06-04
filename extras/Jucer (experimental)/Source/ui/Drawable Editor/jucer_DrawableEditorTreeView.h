/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__
#define __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__


//==============================================================================
/**
*/
class DrawableTreeViewItem  : public JucerTreeViewBase,
                              public ValueTree::Listener,
                              public ChangeListener,
                              public AsyncUpdater
{
public:
    DrawableTreeViewItem (DrawableEditor& editor_, const ValueTree& drawableRoot)
        : editor (editor_), node (drawableRoot), typeName (drawableRoot.getType().toString())
    {
        node.getState().addListener (this);
        editor.getSelection().addChangeListener (this);
    }

    ~DrawableTreeViewItem()
    {
        editor.getSelection().removeChangeListener (this);
        node.getState().removeListener (this);
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
    {
        if (property == Drawable::ValueTreeWrapperBase::idProperty)
            repaintItem();
    }

    void valueTreeChildrenChanged (ValueTree& tree)
    {
        if (tree == node.getState() || tree.isAChildOf (node.getState()))
            triggerAsyncUpdate();
    }

    void valueTreeParentChanged (ValueTree& tree)
    {
    }

    void handleAsyncUpdate()
    {
        refreshSubItems();
    }

    //==============================================================================
    // TreeViewItem stuff..
    bool mightContainSubItems()
    {
        return node.getState().getType() == DrawableComposite::valueTreeType;
    }

    const String getUniqueName() const
    {
        jassert (node.getID().isNotEmpty());
        return node.getID();
    }

    void itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
            refreshSubItems();
    }

    void refreshSubItems()
    {
        if (node.getState().getType() == DrawableComposite::valueTreeType)
        {
            ScopedPointer <XmlElement> oldOpenness (getOpennessState());

            clearSubItems();

            DrawableComposite::ValueTreeWrapper composite (node.getState());

            for (int i = 0; i < composite.getNumDrawables(); ++i)
            {
                ValueTree subNode (composite.getDrawableState (i));
                DrawableTreeViewItem* const item = new DrawableTreeViewItem (editor, subNode);
                addSubItem (item);
            }

            if (oldOpenness != 0)
                restoreOpennessState (*oldOpenness);

            editor.getSelection().changed();
        }
    }

    const String getDisplayName() const
    {
        const String name (getRenamingName());
        return typeName + (name.isEmpty() ? String::empty
                                          : (" \"" + name + "\""));
    }

    const String getRenamingName() const
    {
        return node.getID();
    }

    void setName (const String& newName)
    {
    }

    bool isMissing()            { return false; }

    const Image getIcon() const
    {
        return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage();
    }

    void itemClicked (const MouseEvent& e)
    {
    }

    void itemDoubleClicked (const MouseEvent& e)
    {
    }

    void itemSelectionChanged (bool isNowSelected)
    {
        const String objectId (node.getID());

        if (isNowSelected)
            editor.getSelection().addToSelection (objectId);
        else
            editor.getSelection().deselect (objectId);
    }

    void changeListenerCallback (void*)
    {
        setSelected (editor.getSelection().isSelected (node.getID()), false);
    }

    const String getTooltip()
    {
        return String::empty;
    }

    static const String getDragIdFor (DrawableEditor& editor)
    {
        return drawableItemDragType + editor.getDocument().getUniqueId();
    }

    const String getDragSourceDescription()
    {
        return getDragIdFor (editor);
    }

    //==============================================================================
    // Drag-and-drop stuff..
    bool isInterestedInFileDrag (const StringArray& files)
    {
        return false;
    }

    void filesDropped (const StringArray& files, int insertIndex)
    {
    }

    bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent)
    {
        return node.getState().getType() == DrawableComposite::valueTreeType
                && sourceDescription == getDragIdFor (editor)
                    && editor.getSelection().getNumSelected() > 0;
    }

    void itemDropped (const String& sourceDescription, Component* sourceComponent, int insertIndex)
    {
        if (editor.getSelection().getNumSelected() > 0)
        {
            TreeView* tree = getOwnerView();
            const ScopedPointer <XmlElement> oldOpenness (tree->getOpennessState (false));

            Array <ValueTree> selectedComps;
            // scan the source tree rather than look at the selection manager, because it might
            // be from a different editor, and the order needs to be correct.
            getAllSelectedNodesInTree (sourceComponent, selectedComps);
            insertItems (selectedComps, insertIndex);

            if (oldOpenness != 0)
                tree->restoreOpennessState (*oldOpenness);
        }
    }

    static void getAllSelectedNodesInTree (Component* componentInTree, Array<ValueTree>& selectedItems)
    {
        TreeView* tree = dynamic_cast <TreeView*> (componentInTree);

        if (tree == 0)
            tree = componentInTree->findParentComponentOfClass ((TreeView*) 0);

        if (tree != 0)
        {
            const int numSelected = tree->getNumSelectedItems();

            for (int i = 0; i < numSelected; ++i)
            {
                DrawableTreeViewItem* const item = dynamic_cast <DrawableTreeViewItem*> (tree->getSelectedItem (i));

                if (item != 0)
                    selectedItems.add (item->node.getState());
            }
        }
    }

    void insertItems (Array <ValueTree>& items, int insertIndex)
    {
        DrawableComposite::ValueTreeWrapper composite (node.getState());

        int i;
        for (i = items.size(); --i >= 0;)
            if (node.getState() == items.getReference(i) || composite.getState().isAChildOf (items.getReference(i))) // Check for recursion.
                return;

        // Don't include any nodes that are children of other selected nodes..
        for (i = items.size(); --i >= 0;)
        {
            const ValueTree& n = items.getReference(i);

            for (int j = items.size(); --j >= 0;)
            {
                if (j != i && n.isAChildOf (items.getReference(j)))
                {
                    items.remove (i);
                    break;
                }
            }
        }

        // Remove and re-insert them one at a time..
        for (i = 0; i < items.size(); ++i)
        {
            ValueTree& n = items.getReference(i);

            int index = composite.indexOfDrawable (n);

            if (index >= 0 && index < insertIndex)
                --insertIndex;

            if (index >= 0)
            {
                composite.moveDrawableOrder (index, insertIndex++, editor.getDocument().getUndoManager());
            }
            else
            {
                n.getParent().removeChild (n, editor.getDocument().getUndoManager());
                composite.addDrawable (n, insertIndex++, editor.getDocument().getUndoManager());
            }
        }
    }

    //==============================================================================
    void showRenameBox()
    {
    }

    // Text editor listener for renaming..
    void textEditorTextChanged (TextEditor& textEditor)         {}
    void textEditorReturnKeyPressed (TextEditor& textEditor)    { textEditor.exitModalState (1); }
    void textEditorEscapeKeyPressed (TextEditor& textEditor)    { textEditor.exitModalState (0); }
    void textEditorFocusLost (TextEditor& textEditor)           { textEditor.exitModalState (0); }

    //==============================================================================
    DrawableEditor& editor;
    Drawable::ValueTreeWrapperBase node;

private:
    String typeName;

    DrawableEditor* getEditor() const
    {
        return getOwnerView()->findParentComponentOfClass ((DrawableEditor*) 0);
    }
};


#endif   // __JUCER_DRAWABLETREEVIEWITEM_JUCEHEADER__
