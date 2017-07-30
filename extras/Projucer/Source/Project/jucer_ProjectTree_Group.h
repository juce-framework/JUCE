/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

class GroupItem   : public ProjectTreeItemBase
{
public:
    GroupItem (const Project::Item& projectItem, const String& filter = String())
        : ProjectTreeItemBase (projectItem),
          searchFilter (filter)
    {
    }

    bool isRoot() const override                                 { return item.isMainGroup(); }
    bool acceptsFileDrop (const StringArray&) const override     { return true; }

    void addNewGroup()
    {
        Project::Item newGroup (item.addNewSubGroup ("New Group", 0));
        triggerAsyncRename (newGroup);
    }

    bool acceptsDragItems (const OwnedArray<Project::Item>& selectedNodes) override
    {
        for (int i = selectedNodes.size(); --i >= 0;)
            if (item.canContain (*selectedNodes.getUnchecked(i)))
                return true;

        return false;
    }

    void addFilesAtIndex (const StringArray& files, int insertIndex) override
    {
        for (int i = 0; i < files.size(); ++i)
        {
            const File file (files[i]);

            if (item.addFileAtIndex (file, insertIndex, true))
                ++insertIndex;
        }
    }

    void addFilesRetainingSortOrder (const StringArray& files) override
    {
        for (int i = files.size(); --i >= 0;)
            item.addFileRetainingSortOrder (files[i], true);
    }

    void moveSelectedItemsTo (OwnedArray<Project::Item>& selectedNodes, int insertIndex) override
    {
        moveItems (selectedNodes, item, insertIndex);
    }

    void checkFileStatus() override
    {
        for (int i = 0; i < getNumSubItems(); ++i)
            if (ProjectTreeItemBase* p = dynamic_cast<ProjectTreeItemBase*> (getSubItem(i)))
                p->checkFileStatus();
    }

    bool isGroupEmpty (const Project::Item& group) // recursive
    {
        for (auto i = 0; i < group.getNumChildren(); ++i)
        {
            auto child = group.getChild (i);

            if ((child.isGroup() && ! isGroupEmpty (child))
                   || (child.isFile() && child.getName().containsIgnoreCase (searchFilter)))
                return false;
        }

        return true;
    }

    ProjectTreeItemBase* createSubItem (const Project::Item& child) override
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
        if (ProjectContentComponent* pcc = getProjectContentComponent())
            pcc->setEditorComponent (new GroupInformationComponent (item), nullptr);
    }

    static void openAllGroups (TreeViewItem* root)
    {
        for (auto i = 0; i < root->getNumSubItems(); ++i)
            if (auto* sub = root->getSubItem (i))
                openOrCloseAllSubGroups (*sub, true);
    }

    static void closeAllGroups (TreeViewItem* root)
    {
        for (auto i = 0; i < root->getNumSubItems(); ++i)
            if (auto* sub = root->getSubItem (i))
                openOrCloseAllSubGroups (*sub, false);
    }

    static void openOrCloseAllSubGroups (TreeViewItem& item, bool shouldOpen)
    {
        item.setOpen (shouldOpen);

        for (int i = item.getNumSubItems(); --i >= 0;)
            if (auto* sub = item.getSubItem (i))
                openOrCloseAllSubGroups (*sub, shouldOpen);
    }

    static void setFilesToCompile (Project::Item item, const bool shouldCompile)
    {
        if (item.isFile())
            item.getShouldCompileValue() = shouldCompile;

        for (int i = item.getNumChildren(); --i >= 0;)
            setFilesToCompile (item.getChild (i), shouldCompile);
    }

    void showPopupMenu() override
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

        launchPopupMenu (m);
    }

    void showPlusMenu() override
    {
        PopupMenu m;
        addCreateFileMenuItems (m);

        launchPopupMenu (m);
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
        NewFileWizard().addWizardsToMenu (m);
    }

    void processCreateFileMenuItem (int menuID)
    {
        switch (menuID)
        {
            case 1001:  addNewGroup(); break;
            case 1002:  browseToAddExistingFiles(); break;

            default:
                jassert (getProject() != nullptr);
                NewFileWizard().runWizardFromMenu (menuID, *getProject(), item);
                break;
        }
    }

    Project* getProject()
    {
        if (TreeView* tv = getOwnerView())
            if (ProjectContentComponent* pcc = tv->findParentComponentOfClass<ProjectContentComponent>())
                return pcc->getProject();

        return nullptr;
    }

    void setSearchFilter (const String& filter)
    {
        searchFilter = filter;
        refreshSubItems();
    }

    String searchFilter;
};
