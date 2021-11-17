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

#pragma once


//==============================================================================
class FileTreeItemBase   : public JucerTreeViewBase,
                           private ValueTree::Listener
{
public:
    FileTreeItemBase (const Project::Item& projectItem)
        : item (projectItem), isFileMissing (false)
    {
        item.state.addListener (this);
    }

    ~FileTreeItemBase() override
    {
        item.state.removeListener (this);
    }

    //==============================================================================
    virtual bool acceptsFileDrop (const StringArray& files) const = 0;
    virtual bool acceptsDragItems (const OwnedArray<Project::Item>& selectedNodes) = 0;

    //==============================================================================
    String getDisplayName() const override              { return item.getName(); }
    String getRenamingName() const override             { return getDisplayName(); }
    void setName (const String& newName) override       { item.getNameValue() = newName; }
    bool isMissing() const override                     { return isFileMissing; }
    virtual File getFile() const                        { return item.getFile(); }

    void deleteItem() override                          { item.removeItemFromProject(); }

    void deleteAllSelectedItems() override
    {
        auto* tree = getOwnerView();
        Array<File> filesToTrash;
        Array<Project::Item> itemsToRemove;

        for (int i = 0; i < tree->getNumSelectedItems(); ++i)
        {
            if (auto* p = dynamic_cast<FileTreeItemBase*> (tree->getSelectedItem (i)))
            {
                itemsToRemove.add (p->item);

                if (p->item.isGroup())
                {
                    for (int j = 0; j < p->item.getNumChildren(); ++j)
                    {
                        auto associatedFile = p->item.getChild (j).getFile();

                        if (associatedFile.existsAsFile())
                            filesToTrash.addIfNotAlreadyThere (associatedFile);
                    }
                }
                else if (p->getFile().existsAsFile())
                {
                    filesToTrash.addIfNotAlreadyThere (p->getFile());
                }
            }
        }

        WeakReference<FileTreeItemBase> treeRootItem { dynamic_cast<FileTreeItemBase*> (tree->getRootItem()) };

        if (treeRootItem == nullptr)
        {
            jassertfalse;
            return;
        }

        auto doDelete = [treeRootItem, itemsToRemove] (const Array<File>& fsToTrash)
        {
            if (treeRootItem == nullptr)
                return;

            auto& om = ProjucerApplication::getApp().openDocumentManager;

            for (auto i = fsToTrash.size(); --i >= 0;)
            {
                auto f = fsToTrash.getUnchecked(i);

                om.closeFileWithoutSaving (f);

                if (! f.moveToTrash())
                {
                    // xxx
                }
            }

            for (auto i = itemsToRemove.size(); --i >= 0;)
            {
                if (auto itemToRemove = treeRootItem->findTreeViewItem (itemsToRemove.getUnchecked (i)))
                {
                    if (auto* pcc = treeRootItem->getProjectContentComponent())
                    {
                        if (auto* fileInfoComp = dynamic_cast<FileGroupInformationComponent*> (pcc->getEditorComponent()))
                            if (fileInfoComp->getGroupPath() == itemToRemove->getFile().getFullPathName())
                                pcc->hideEditor();
                    }

                    om.closeFileWithoutSaving (itemToRemove->getFile());
                    itemToRemove->deleteItem();
                }
            }
        };

        if (! filesToTrash.isEmpty())
        {
            String fileList;
            auto maxFilesToList = 10;
            for (auto i = jmin (maxFilesToList, filesToTrash.size()); --i >= 0;)
                fileList << filesToTrash.getUnchecked(i).getFullPathName() << "\n";

            if (filesToTrash.size() > maxFilesToList)
                fileList << "\n...plus " << (filesToTrash.size() - maxFilesToList) << " more files...";

            AlertWindow::showYesNoCancelBox (MessageBoxIconType::NoIcon,
                                             "Delete Project Items",
                                             "As well as removing the selected item(s) from the project, do you also want to move their files to the trash:\n\n"
                                                  + fileList,
                                             "Just remove references",
                                             "Also move files to Trash",
                                             "Cancel",
                                             tree->getTopLevelComponent(),
                                             ModalCallbackFunction::create ([treeRootItem, filesToTrash, doDelete] (int r) mutable
            {
                if (treeRootItem == nullptr)
                    return;

                if (r == 0)
                    return;

                if (r != 2)
                    filesToTrash.clear();

                doDelete (filesToTrash);
            }));

            return;
        }

        doDelete (filesToTrash);
    }

    virtual void revealInFinder() const
    {
        getFile().revealToUser();
    }

    virtual void browseToAddExistingFiles()
    {
        auto location = item.isGroup() ? item.determineGroupFolder() : getFile();
        chooser = std::make_unique<FileChooser> ("Add Files to Jucer Project", location, "");
        auto flags = FileBrowserComponent::openMode
                   | FileBrowserComponent::canSelectFiles
                   | FileBrowserComponent::canSelectDirectories
                   | FileBrowserComponent::canSelectMultipleItems;

        chooser->launchAsync (flags, [this] (const FileChooser& fc)
        {
            if (fc.getResults().isEmpty())
                return;

            StringArray files;

            for (int i = 0; i < fc.getResults().size(); ++i)
                files.add (fc.getResults().getReference(i).getFullPathName());

            addFilesRetainingSortOrder (files);
        });
    }

    virtual void checkFileStatus()  // (recursive)
    {
        auto file = getFile();
        auto nowMissing = (file != File() && ! file.exists());

        if (nowMissing != isFileMissing)
        {
            isFileMissing = nowMissing;
            repaintItem();
        }
    }

    virtual void addFilesAtIndex (const StringArray& files, int insertIndex)
    {
        if (auto* p = getParentProjectItem())
            p->addFilesAtIndex (files, insertIndex);
    }

    virtual void addFilesRetainingSortOrder (const StringArray& files)
    {
        if (auto* p = getParentProjectItem())
            p->addFilesRetainingSortOrder (files);
    }

    virtual void moveSelectedItemsTo (OwnedArray<Project::Item>&, int /*insertIndex*/)
    {
        jassertfalse;
    }

    void showMultiSelectionPopupMenu (Point<int> p) override
    {
        PopupMenu m;
        m.addItem (1, "Delete");

        m.showMenuAsync (PopupMenu::Options().withTargetScreenArea ({ p.x, p.y, 1, 1 }),
                         ModalCallbackFunction::create (treeViewMultiSelectItemChosen, this));
    }

    static void treeViewMultiSelectItemChosen (int resultCode, FileTreeItemBase* item)
    {
        switch (resultCode)
        {
            case 1:     item->deleteAllSelectedItems(); break;
            default:    break;
        }
    }

    virtual FileTreeItemBase* findTreeViewItem (const Project::Item& itemToFind)
    {
        if (item == itemToFind)
            return this;

        auto wasOpen = isOpen();
        setOpen (true);

        for (auto i = getNumSubItems(); --i >= 0;)
        {
            if (auto* pg = dynamic_cast<FileTreeItemBase*> (getSubItem(i)))
                if (auto* found = pg->findTreeViewItem (itemToFind))
                    return found;
        }

        setOpen (wasOpen);
        return nullptr;
    }

    //==============================================================================
    bool mightContainSubItems() override                { return item.getNumChildren() > 0; }
    String getUniqueName() const override               { jassert (item.getID().isNotEmpty()); return item.getID(); }
    bool canBeSelected() const override                 { return true; }
    String getTooltip() override                        { return {}; }
    File getDraggableFile() const override              { return getFile(); }

    var getDragSourceDescription() override
    {
        cancelDelayedSelectionTimer();
        return projectItemDragType;
    }

    void addSubItems() override
    {
        for (int i = 0; i < item.getNumChildren(); ++i)
            if (auto* p = createSubItem (item.getChild(i)))
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
            ProjucerApplication::getApp().openFile (files[0], [] (bool) {});
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
            auto* tree = getOwnerView();
            std::unique_ptr<XmlElement> oldOpenness (tree->getOpennessState (false));

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
            auto* tree = dynamic_cast<TreeView*> (dragSourceDetails.sourceComponent.get());

            if (tree == nullptr)
                tree = dragSourceDetails.sourceComponent->findParentComponentOfClass<TreeView>();

            if (tree != nullptr)
            {
                auto numSelected = tree->getNumSelectedItems();

                for (int i = 0; i < numSelected; ++i)
                    if (auto* p = dynamic_cast<FileTreeItemBase*> (tree->getSelectedItem (i)))
                        selectedNodes.add (new Project::Item (p->item));
            }
        }
    }

    FileTreeItemBase* getParentProjectItem() const
    {
        return dynamic_cast<FileTreeItemBase*> (getParentItem());
    }

    //==============================================================================
    Project::Item item;

protected:
    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier&) override
    {
        if (tree == item.state)
            repaintItem();
    }

    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { treeChildrenChanged (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override  { treeChildrenChanged (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override    { treeChildrenChanged (parentTree); }

    bool isFileMissing;

    virtual FileTreeItemBase* createSubItem (const Project::Item& node) = 0;

    Icon getIcon() const override
    {
        auto colour = getOwnerView()->findColour (isSelected() ? defaultHighlightedTextColourId
                                                               : treeIconColourId);

        return item.getIcon (isOpen()).withColour (colour);
    }

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
                    if (auto* root = dynamic_cast<FileTreeItemBase*> (tree->getRootItem()))
                        if (auto* found = root->findTreeViewItem (itemToRename))
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
        for (auto i = selectedNodes.size(); --i >= 0;)
        {
            auto* n = selectedNodes.getUnchecked(i);

            if (destNode == *n || destNode.state.isAChildOf (n->state)) // Check for recursion.
                return;

            if (! destNode.canContain (*n))
                selectedNodes.remove (i);
        }

        // Don't include any nodes that are children of other selected nodes..
        for (auto i = selectedNodes.size(); --i >= 0;)
        {
            auto* n = selectedNodes.getUnchecked(i);

            for (auto j = selectedNodes.size(); --j >= 0;)
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
            auto* selectedNode = selectedNodes.getUnchecked(i);

            if (selectedNode->state.getParent() == destNode.state
                  && indexOfNode (destNode.state, selectedNode->state) < insertIndex)
                --insertIndex;

            selectedNode->removeItemFromProject();
            destNode.addChild (*selectedNode, insertIndex++);
        }
    }

    static int indexOfNode (const ValueTree& parent, const ValueTree& child)
    {
        for (auto i = parent.getNumChildren(); --i >= 0;)
            if (parent.getChild (i) == child)
                return i;

        return -1;
    }

private:
    std::unique_ptr<FileChooser> chooser;

    JUCE_DECLARE_WEAK_REFERENCEABLE (FileTreeItemBase)
};

//==============================================================================
class SourceFileItem   : public FileTreeItemBase
{
public:
    SourceFileItem (const Project::Item& projectItem)
        : FileTreeItemBase (projectItem)
    {
    }

    bool acceptsFileDrop (const StringArray&) const override             { return false; }
    bool acceptsDragItems (const OwnedArray<Project::Item>&) override    { return false; }

    String getDisplayName() const override
    {
        return getFile().getFileName();
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        JucerTreeViewBase::paintItem (g, width, height);

        if (item.needsSaving())
        {
            auto bounds = g.getClipBounds().withY (0).withHeight (height);

            g.setFont (getFont());
            g.setColour (getContentColour (false));

            g.drawFittedText ("*", bounds.removeFromLeft (height), Justification::centred, 1);
        }
    }

    static File findCorrespondingHeaderOrCpp (const File& f)
    {
        if (f.hasFileExtension (sourceFileExtensions))  return f.withFileExtension (".h");
        if (f.hasFileExtension (headerFileExtensions))  return f.withFileExtension (".cpp");

        return {};
    }

    void setName (const String& newName) override
    {
        if (newName != File::createLegalFileName (newName))
        {
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                              "File Rename",
                                              "That filename contained some illegal characters!");
            triggerAsyncRename (item);
            return;
        }

        auto oldFile = getFile();
        auto newFile = oldFile.getSiblingFile (newName);
        auto correspondingFile = findCorrespondingHeaderOrCpp (oldFile);

        if (correspondingFile.exists() && newFile.hasFileExtension (oldFile.getFileExtension()))
        {
            auto correspondingItem = item.project.getMainGroup().findItemForFile (correspondingFile);

            if (correspondingItem.isValid())
            {
                AlertWindow::showOkCancelBox (MessageBoxIconType::NoIcon,
                                              "File Rename",
                                              "Do you also want to rename the corresponding file \"" + correspondingFile.getFileName() + "\" to match?",
                                              {},
                                              {},
                                              nullptr,
                                              ModalCallbackFunction::create ([parent = WeakReference<SourceFileItem> { this },
                                                                              oldFile, newFile, correspondingFile, correspondingItem] (int result) mutable
                {
                    if (parent == nullptr || result == 0)
                        return;

                    if (! parent->item.renameFile (newFile))
                    {
                        AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                                          "File Rename",
                                                          "Failed to rename \"" + oldFile.getFullPathName() + "\"!\n\nCheck your file permissions!");
                        return;
                    }

                    if (! correspondingItem.renameFile (newFile.withFileExtension (correspondingFile.getFileExtension())))
                    {
                        AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                                          "File Rename",
                                                          "Failed to rename \"" + correspondingFile.getFullPathName() + "\"!\n\nCheck your file permissions!");
                    }

                }));
            }
        }

        if (! item.renameFile (newFile))
        {
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                              "File Rename",
                                              "Failed to rename the file!\n\nCheck your file permissions!");
        }
    }

    FileTreeItemBase* createSubItem (const Project::Item&) override
    {
        jassertfalse;
        return nullptr;
    }

    void showDocument() override
    {
        auto f = getFile();

        if (f.exists())
            if (auto* pcc = getProjectContentComponent())
                pcc->showEditorForFile (f, false);
    }

    void showPopupMenu (Point<int> p) override
    {
        PopupMenu m;

        m.addItem (1, "Open in external editor");
        m.addItem (2,
                     #if JUCE_MAC
                      "Reveal in Finder");
                     #else
                      "Reveal in Explorer");
                     #endif

        m.addItem (4, "Rename File...");
        m.addSeparator();

        m.addItem (5, "Binary Resource", true, item.shouldBeAddedToBinaryResources());
        m.addItem (6, "Xcode Resource", true, item.shouldBeAddedToXcodeResources());
        m.addItem (7, "Compile", item.isSourceFile(), item.shouldBeCompiled());
        m.addItem (8, "Skip PCH", item.shouldBeCompiled(), item.shouldSkipPCH());
        m.addSeparator();

        m.addItem (3, "Delete");

        launchPopupMenu (m, p);
    }

    void showAddMenu (Point<int> p) override
    {
        if (auto* group = dynamic_cast<GroupItem*> (getParentItem()))
            group->showAddMenu (p);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        switch (resultCode)
        {
            case 1:  getFile().startAsProcess(); break;
            case 2:  revealInFinder(); break;
            case 3:  deleteAllSelectedItems(); break;
            case 4:  triggerAsyncRename (item); break;
            case 5:  item.getShouldAddToBinaryResourcesValue().setValue (! item.shouldBeAddedToBinaryResources()); break;
            case 6:  item.getShouldAddToXcodeResourcesValue().setValue (! item.shouldBeAddedToXcodeResources()); break;
            case 7:  item.getShouldCompileValue().setValue (! item.shouldBeCompiled()); break;
            case 8:  item.getShouldSkipPCHValue().setValue (! item.shouldSkipPCH()); break;

            default:
                if (auto* parentGroup = dynamic_cast<GroupItem*> (getParentProjectItem()))
                    parentGroup->processCreateFileMenuItem (resultCode);

                break;
        }
    }

    JUCE_DECLARE_WEAK_REFERENCEABLE (SourceFileItem)
};

//==============================================================================
class GroupItem   : public FileTreeItemBase
{
public:
    GroupItem (const Project::Item& projectItem, const String& filter = {})
        : FileTreeItemBase (projectItem),
          searchFilter (filter)
    {
    }

    bool isRoot() const override                                 { return item.isMainGroup(); }
    bool acceptsFileDrop (const StringArray&) const override     { return true; }

    void addNewGroup()
    {
        auto newGroup = item.addNewSubGroup ("New Group", 0);
        triggerAsyncRename (newGroup);
    }

    bool acceptsDragItems (const OwnedArray<Project::Item>& selectedNodes) override
    {
        for (auto i = selectedNodes.size(); --i >= 0;)
            if (item.canContain (*selectedNodes.getUnchecked(i)))
                return true;

        return false;
    }

    void addFilesAtIndex (const StringArray& files, int insertIndex) override
    {
        for (auto f : files)
        {
            if (item.addFileAtIndex (f, insertIndex, true))
                ++insertIndex;
        }
    }

    void addFilesRetainingSortOrder (const StringArray& files) override
    {
        for (auto i = files.size(); --i >= 0;)
            item.addFileRetainingSortOrder (files[i], true);
    }

    void moveSelectedItemsTo (OwnedArray<Project::Item>& selectedNodes, int insertIndex) override
    {
        moveItems (selectedNodes, item, insertIndex);
    }

    void checkFileStatus() override
    {
        for (int i = 0; i < getNumSubItems(); ++i)
            if (auto* p = dynamic_cast<FileTreeItemBase*> (getSubItem(i)))
                p->checkFileStatus();
    }

    bool isGroupEmpty (const Project::Item& group) // recursive
    {
        for (int i = 0; i < group.getNumChildren(); ++i)
        {
            auto child = group.getChild (i);

            if ((child.isGroup() && ! isGroupEmpty (child))
                   || (child.isFile() && child.getName().containsIgnoreCase (searchFilter)))
                return false;
        }

        return true;
    }

    FileTreeItemBase* createSubItem (const Project::Item& child) override
    {
        if (child.isGroup())
        {
            if (searchFilter.isNotEmpty() && isGroupEmpty (child))
                return nullptr;

            return new GroupItem (child, searchFilter);
        }

        if (child.isFile())
        {
            if (child.getName().containsIgnoreCase (searchFilter))
                return new SourceFileItem (child);

            return nullptr;
        }

        jassertfalse;
        return nullptr;
    }

    void showDocument() override
    {
        if (auto* pcc = getProjectContentComponent())
            pcc->setScrollableEditorComponent (std::make_unique<FileGroupInformationComponent> (item));
    }

    static void openAllGroups (TreeViewItem* root)
    {
        for (int i = 0; i < root->getNumSubItems(); ++i)
            if (auto* sub = root->getSubItem (i))
                openOrCloseAllSubGroups (*sub, true);
    }

    static void closeAllGroups (TreeViewItem* root)
    {
        for (int i = 0; i < root->getNumSubItems(); ++i)
            if (auto* sub = root->getSubItem (i))
                openOrCloseAllSubGroups (*sub, false);
    }

    static void openOrCloseAllSubGroups (TreeViewItem& treeItem, bool shouldOpen)
    {
        treeItem.setOpen (shouldOpen);

        for (auto i = treeItem.getNumSubItems(); --i >= 0;)
            if (auto* sub = treeItem.getSubItem (i))
                openOrCloseAllSubGroups (*sub, shouldOpen);
    }

    static void setFilesToCompile (Project::Item projectItem, const bool shouldCompile)
    {
        if (projectItem.isFile() && (projectItem.getFile().hasFileExtension (fileTypesToCompileByDefault)))
            projectItem.getShouldCompileValue() = shouldCompile;

        for (auto i = projectItem.getNumChildren(); --i >= 0;)
            setFilesToCompile (projectItem.getChild (i), shouldCompile);
    }

    void showPopupMenu (Point<int> p) override
    {
        PopupMenu m;
        addCreateFileMenuItems (m);

        m.addSeparator();

        m.addItem (1, "Collapse all Groups");
        m.addItem (2, "Expand all Groups");

        if (! isRoot())
        {
            if (isOpen())
                m.addItem (3, "Collapse all Sub-groups");
            else
                m.addItem (4, "Expand all Sub-groups");
        }

        m.addSeparator();
        m.addItem (5, "Enable compiling of all enclosed files");
        m.addItem (6, "Disable compiling of all enclosed files");

        m.addSeparator();
        m.addItem (7, "Sort Items Alphabetically");
        m.addItem (8, "Sort Items Alphabetically (Groups first)");
        m.addSeparator();

        if (! isRoot())
        {
            m.addItem (9, "Rename...");
            m.addItem (10, "Delete");
        }

        launchPopupMenu (m, p);
    }

    void showAddMenu (Point<int> p) override
    {
        PopupMenu m;
        addCreateFileMenuItems (m);

        launchPopupMenu (m, p);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        switch (resultCode)
        {
            case 1:     closeAllGroups (getOwnerView()->getRootItem()); break;
            case 2:     openAllGroups (getOwnerView()->getRootItem()); break;
            case 3:     openOrCloseAllSubGroups (*this, false); break;
            case 4:     openOrCloseAllSubGroups (*this, true); break;
            case 5:     setFilesToCompile (item, true); break;
            case 6:     setFilesToCompile (item, false); break;
            case 7:     item.sortAlphabetically (false, false); break;
            case 8:     item.sortAlphabetically (true, false); break;
            case 9:     triggerAsyncRename (item); break;
            case 10:    deleteAllSelectedItems(); break;
            default:    processCreateFileMenuItem (resultCode); break;
        }
    }

    void addCreateFileMenuItems (PopupMenu& m)
    {
        m.addItem (1001, "Add New Group");
        m.addItem (1002, "Add Existing Files...");

        m.addSeparator();
        wizard.addWizardsToMenu (m);
    }

    void processCreateFileMenuItem (int menuID)
    {
        switch (menuID)
        {
            case 1001:  addNewGroup(); break;
            case 1002:  browseToAddExistingFiles(); break;

            default:
                jassert (getProject() != nullptr);
                wizard.runWizardFromMenu (menuID, *getProject(), item);
                break;
        }
    }

    Project* getProject()
    {
        if (auto* tv = getOwnerView())
            if (auto* pcc = tv->findParentComponentOfClass<ProjectContentComponent>())
                return pcc->getProject();

        return nullptr;
    }

    void setSearchFilter (const String& filter) override
    {
        searchFilter = filter;
        refreshSubItems();
    }

    String searchFilter;
    NewFileWizard wizard;
};
