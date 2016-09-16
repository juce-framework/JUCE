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

class ProjectTreeItemBase   : public JucerTreeViewBase,
                              public ValueTree::Listener
{
public:
    ProjectTreeItemBase (const Project::Item& projectItem)
        : item (projectItem), isFileMissing (false)
    {
        item.state.addListener (this);
    }

    ~ProjectTreeItemBase()
    {
        item.state.removeListener (this);
    }

    //==============================================================================
    virtual bool acceptsFileDrop (const StringArray& files) const = 0;
    virtual bool acceptsDragItems (const OwnedArray<Project::Item>& selectedNodes) = 0;

    //==============================================================================
    String getDisplayName() const override              { return item.getName(); }
    String getRenamingName() const override             { return getDisplayName(); }

    void setName (const String& newName) override
    {
        if (item.isMainGroup())
            item.project.setTitle (newName);
        else
            item.getNameValue() = newName;
    }

    bool isMissing() override                           { return isFileMissing; }
    virtual File getFile() const                        { return item.getFile(); }

    void deleteItem() override                          { item.removeItemFromProject(); }

    virtual void deleteAllSelectedItems() override
    {
        TreeView* tree = getOwnerView();
        const int numSelected = tree->getNumSelectedItems();
        OwnedArray<File> filesToTrash;
        OwnedArray<Project::Item> itemsToRemove;

        for (int i = 0; i < numSelected; ++i)
        {
            if (const ProjectTreeItemBase* const p = dynamic_cast<ProjectTreeItemBase*> (tree->getSelectedItem (i)))
            {
                itemsToRemove.add (new Project::Item (p->item));

                if (p->getFile().existsAsFile())
                    filesToTrash.add (new File (p->getFile()));
            }
        }

        if (filesToTrash.size() > 0)
        {
            String fileList;
            const int maxFilesToList = 10;
            for (int i = jmin (maxFilesToList, filesToTrash.size()); --i >= 0;)
                fileList << filesToTrash.getUnchecked(i)->getFullPathName() << "\n";

            if (filesToTrash.size() > maxFilesToList)
                fileList << "\n...plus " << (filesToTrash.size() - maxFilesToList) << " more files...";

            int r = AlertWindow::showYesNoCancelBox (AlertWindow::NoIcon, "Delete Project Items",
                                                     "As well as removing the selected item(s) from the project, do you also want to move their files to the trash:\n\n"
                                                       + fileList,
                                                     "Just remove references",
                                                     "Also move files to Trash",
                                                     "Cancel",
                                                     tree->getTopLevelComponent());

            if (r == 0)
                return;

            if (r != 2)
                filesToTrash.clear();
        }

        if (ProjectTreeItemBase* treeRootItem = dynamic_cast<ProjectTreeItemBase*> (tree->getRootItem()))
        {
            OpenDocumentManager& om = ProjucerApplication::getApp().openDocumentManager;

            for (int i = filesToTrash.size(); --i >= 0;)
            {
                const File f (*filesToTrash.getUnchecked(i));

                om.closeFile (f, false);

                if (! f.moveToTrash())
                {
                    // xxx
                }
            }

            for (int i = itemsToRemove.size(); --i >= 0;)
            {
                if (ProjectTreeItemBase* itemToRemove = treeRootItem->findTreeViewItem (*itemsToRemove.getUnchecked(i)))
                {
                    om.closeFile (itemToRemove->getFile(), false);
                    itemToRemove->deleteItem();
                }
            }
        }
        else
        {
            jassertfalse;
        }
    }

    virtual void revealInFinder() const
    {
        getFile().revealToUser();
    }

    virtual void browseToAddExistingFiles()
    {
        const File location (item.isGroup() ? item.determineGroupFolder() : getFile());
        FileChooser fc ("Add Files to Jucer Project", location, String(), false);

        if (fc.browseForMultipleFilesOrDirectories())
        {
            StringArray files;

            for (int i = 0; i < fc.getResults().size(); ++i)
                files.add (fc.getResults().getReference(i).getFullPathName());

            addFilesRetainingSortOrder (files);
        }
    }

    virtual void checkFileStatus()  // (recursive)
    {
        const File file (getFile());
        const bool nowMissing = file != File() && ! file.exists();

        if (nowMissing != isFileMissing)
        {
            isFileMissing = nowMissing;
            repaintItem();
        }
    }

    virtual void addFilesAtIndex (const StringArray& files, int insertIndex)
    {
        if (ProjectTreeItemBase* p = getParentProjectItem())
            p->addFilesAtIndex (files, insertIndex);
    }

    virtual void addFilesRetainingSortOrder (const StringArray& files)
    {
        if (ProjectTreeItemBase* p = getParentProjectItem())
            p->addFilesRetainingSortOrder (files);
    }

    virtual void moveSelectedItemsTo (OwnedArray <Project::Item>&, int /*insertIndex*/)
    {
        jassertfalse;
    }

    void showMultiSelectionPopupMenu() override
    {
        PopupMenu m;
        m.addItem (1, "Delete");

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::create (treeViewMultiSelectItemChosen, this));
    }

    static void treeViewMultiSelectItemChosen (int resultCode, ProjectTreeItemBase* item)
    {
        switch (resultCode)
        {
            case 1:     item->deleteAllSelectedItems(); break;
            default:    break;
        }
    }

    virtual ProjectTreeItemBase* findTreeViewItem (const Project::Item& itemToFind)
    {
        if (item == itemToFind)
            return this;

        const bool wasOpen = isOpen();
        setOpen (true);

        for (int i = getNumSubItems(); --i >= 0;)
        {
            if (ProjectTreeItemBase* pg = dynamic_cast<ProjectTreeItemBase*> (getSubItem(i)))
                if (ProjectTreeItemBase* found = pg->findTreeViewItem (itemToFind))
                    return found;
        }

        setOpen (wasOpen);
        return nullptr;
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier&) override
    {
        if (tree == item.state)
            repaintItem();
    }

    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { treeChildrenChanged (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override  { treeChildrenChanged (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override    { treeChildrenChanged (parentTree); }
    void valueTreeParentChanged (ValueTree&) override {}

    //==============================================================================
    bool mightContainSubItems() override                { return item.getNumChildren() > 0; }
    String getUniqueName() const override               { jassert (item.getID().isNotEmpty()); return item.getID(); }
    bool canBeSelected() const override                 { return true; }
    String getTooltip() override                        { return String(); }
    File getDraggableFile() const override              { return getFile(); }

    var getDragSourceDescription() override
    {
        cancelDelayedSelectionTimer();
        return projectItemDragType;
    }

    void addSubItems() override
    {
        for (int i = 0; i < item.getNumChildren(); ++i)
            if (ProjectTreeItemBase* p = createSubItem (item.getChild(i)))
                addSubItem (p);
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
            refreshSubItems();
    }

    //==============================================================================
    bool isInterestedInFileDrag (const StringArray& files) override
    {
        return acceptsFileDrop (files);
    }

    void filesDropped (const StringArray& files, int insertIndex) override
    {
        if (files.size() == 1 && File (files[0]).hasFileExtension (Project::projectFileExtension))
            ProjucerApplication::getApp().openFile (files[0]);
        else
            addFilesAtIndex (files, insertIndex);
    }

    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        OwnedArray<Project::Item> selectedNodes;
        getSelectedProjectItemsBeingDragged (dragSourceDetails, selectedNodes);

        return selectedNodes.size() > 0 && acceptsDragItems (selectedNodes);
    }

    void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex) override
    {
        OwnedArray<Project::Item> selectedNodes;
        getSelectedProjectItemsBeingDragged (dragSourceDetails, selectedNodes);

        if (selectedNodes.size() > 0)
        {
            TreeView* tree = getOwnerView();
            ScopedPointer<XmlElement> oldOpenness (tree->getOpennessState (false));

            moveSelectedItemsTo (selectedNodes, insertIndex);

            if (oldOpenness != nullptr)
                tree->restoreOpennessState (*oldOpenness, false);
        }
    }

    int getMillisecsAllowedForDragGesture() override
    {
        // for images, give the user longer to start dragging before assuming they're
        // clicking to select it for previewing..
        return item.isImageFile() ? 250 : JucerTreeViewBase::getMillisecsAllowedForDragGesture();
    }

    static void getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails& dragSourceDetails,
                                                     OwnedArray<Project::Item>& selectedNodes)
    {
        if (dragSourceDetails.description == projectItemDragType)
        {
            TreeView* tree = dynamic_cast<TreeView*> (dragSourceDetails.sourceComponent.get());

            if (tree == nullptr)
                tree = dragSourceDetails.sourceComponent->findParentComponentOfClass<TreeView>();

            if (tree != nullptr)
            {
                const int numSelected = tree->getNumSelectedItems();

                for (int i = 0; i < numSelected; ++i)
                    if (const ProjectTreeItemBase* const p = dynamic_cast<ProjectTreeItemBase*> (tree->getSelectedItem (i)))
                        selectedNodes.add (new Project::Item (p->item));
            }
        }
    }

    ProjectTreeItemBase* getParentProjectItem() const
    {
        return dynamic_cast<ProjectTreeItemBase*> (getParentItem());
    }

    //==============================================================================
    Project::Item item;

protected:
    bool isFileMissing;

    virtual ProjectTreeItemBase* createSubItem (const Project::Item& node) = 0;

    Icon getIcon() const override           { return item.getIcon().withContrastingColourTo (getBackgroundColour()); }
    bool isIconCrossedOut() const override  { return item.isIconCrossedOut(); }

    void treeChildrenChanged (const ValueTree& parentTree)
    {
        if (parentTree == item.state)
        {
            refreshSubItems();
            treeHasChanged();
            setOpen (true);
        }
    }

    void triggerAsyncRename (const Project::Item& itemToRename)
    {
        struct RenameMessage  : public CallbackMessage
        {
            RenameMessage (TreeView* const t, const Project::Item& i)
                : tree (t), itemToRename (i)  {}

            void messageCallback() override
            {
                if (tree != nullptr)
                    if (ProjectTreeItemBase* root = dynamic_cast<ProjectTreeItemBase*> (tree->getRootItem()))
                        if (ProjectTreeItemBase* found = root->findTreeViewItem (itemToRename))
                            found->showRenameBox();
            }

        private:
            Component::SafePointer<TreeView> tree;
            Project::Item itemToRename;
        };

        (new RenameMessage (getOwnerView(), itemToRename))->post();
    }

    static void moveItems (OwnedArray<Project::Item>& selectedNodes, Project::Item destNode, int insertIndex)
    {
        for (int i = selectedNodes.size(); --i >= 0;)
        {
            Project::Item* const n = selectedNodes.getUnchecked(i);

            if (destNode == *n || destNode.state.isAChildOf (n->state)) // Check for recursion.
                return;

            if (! destNode.canContain (*n))
                selectedNodes.remove (i);
        }

        // Don't include any nodes that are children of other selected nodes..
        for (int i = selectedNodes.size(); --i >= 0;)
        {
            Project::Item* const n = selectedNodes.getUnchecked(i);

            for (int j = selectedNodes.size(); --j >= 0;)
            {
                if (j != i && n->state.isAChildOf (selectedNodes.getUnchecked(j)->state))
                {
                    selectedNodes.remove (i);
                    break;
                }
            }
        }

        // Remove and re-insert them one at a time..
        for (int i = 0; i < selectedNodes.size(); ++i)
        {
            Project::Item* selectedNode = selectedNodes.getUnchecked(i);

            if (selectedNode->state.getParent() == destNode.state
                  && indexOfNode (destNode.state, selectedNode->state) < insertIndex)
                --insertIndex;

            selectedNode->removeItemFromProject();
            destNode.addChild (*selectedNode, insertIndex++);
        }
    }

    static int indexOfNode (const ValueTree& parent, const ValueTree& child)
    {
        for (int i = parent.getNumChildren(); --i >= 0;)
            if (parent.getChild (i) == child)
                return i;

        return -1;
    }
};
