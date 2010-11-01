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

#include "jucer_ProjectTreeViewBase.h"
#include "../Application/jucer_OpenDocumentManager.h"


//==============================================================================
ProjectTreeViewBase::ProjectTreeViewBase (const Project::Item& item_)
    : item (item_), isFileMissing (false)
{
    item.getNode().addListener (this);
}

ProjectTreeViewBase::~ProjectTreeViewBase()
{
    item.getNode().removeListener (this);
}

//==============================================================================
const String ProjectTreeViewBase::getDisplayName() const
{
    return item.getName().toString();
}

void ProjectTreeViewBase::setName (const String& newName)
{
    if (item.isMainGroup())
        item.getProject().setTitle (newName);
    else
        item.getName() = newName;
}

//==============================================================================
const File ProjectTreeViewBase::getFile() const
{
    return item.getFile();
}

void ProjectTreeViewBase::browseToAddExistingFiles()
{
    const File location (item.isGroup() ? item.determineGroupFolder() : getFile());
    FileChooser fc ("Add Files to Jucer Project", location, String::empty, false);

    if (fc.browseForMultipleFilesOrDirectories())
    {
        StringArray files;

        for (int i = 0; i < fc.getResults().size(); ++i)
            files.add (fc.getResults().getReference(i).getFullPathName());

        addFiles (files, 0);
    }
}

void ProjectTreeViewBase::addFiles (const StringArray& files, int insertIndex)
{
    ProjectTreeViewBase* p = dynamic_cast <ProjectTreeViewBase*> (getParentItem());

    if (p != 0)
        p->addFiles (files, insertIndex);
}

void ProjectTreeViewBase::moveSelectedItemsTo (OwnedArray <Project::Item>& selectedNodes, int insertIndex)
{
    jassertfalse;
}

//==============================================================================
ProjectTreeViewBase* ProjectTreeViewBase::findTreeViewItem (const Project::Item& itemToFind)
{
    if (item == itemToFind)
    {
        return this;
    }
    else
    {
        const bool wasOpen = isOpen();
        setOpen (true);

        for (int i = getNumSubItems(); --i >= 0;)
        {
            ProjectTreeViewBase* pg = dynamic_cast <ProjectTreeViewBase*> (getSubItem(i));

            if (pg != 0)
            {
                pg = pg->findTreeViewItem (itemToFind);

                if (pg != 0)
                    return pg;
            }
        }

        setOpen (wasOpen);
    }

    return 0;
}

//==============================================================================
void ProjectTreeViewBase::triggerAsyncRename (const Project::Item& itemToRename)
{
    class RenameMessage  : public CallbackMessage
    {
    public:
        RenameMessage (TreeView* const tree_, const Project::Item& itemToRename_)
            : tree (tree_), itemToRename (itemToRename_)  {}

        void messageCallback()
        {
            if (tree != 0)
            {
                ProjectTreeViewBase* pg = dynamic_cast <ProjectTreeViewBase*> (tree->getRootItem());

                if (pg != 0)
                {
                    pg = pg->findTreeViewItem (itemToRename);

                    if (pg != 0)
                        pg->showRenameBox();
                }
            }
        }

    private:
        Component::SafePointer<TreeView> tree;
        Project::Item itemToRename;
    };

    (new RenameMessage (getOwnerView(), itemToRename))->post();
}

//==============================================================================
void ProjectTreeViewBase::checkFileStatus()
{
    const File file (getFile());
    const bool nowMissing = file != File::nonexistent && ! file.exists();

    if (nowMissing != isFileMissing)
    {
        isFileMissing = nowMissing;
        repaintItem();
    }
}

void ProjectTreeViewBase::revealInFinder() const
{
    getFile().revealToUser();
}

void ProjectTreeViewBase::deleteItem()
{
    item.removeItemFromProject();
}

void ProjectTreeViewBase::deleteAllSelectedItems()
{
    TreeView* tree = getOwnerView();
    const int numSelected = tree->getNumSelectedItems();
    OwnedArray <File> filesToTrash;
    OwnedArray <Project::Item> itemsToRemove;

    int i;
    for (i = 0; i < numSelected; ++i)
    {
        const ProjectTreeViewBase* const p = dynamic_cast <ProjectTreeViewBase*> (tree->getSelectedItem (i));

        if (p != 0)
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
        for (i = jmin (maxFilesToList, filesToTrash.size()); --i >= 0;)
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

    ProjectTreeViewBase* treeRootItem = dynamic_cast <ProjectTreeViewBase*> (tree->getRootItem());
    jassert (treeRootItem != 0);

    if (treeRootItem != 0)
    {
        for (i = filesToTrash.size(); --i >= 0;)
        {
            const File f (*filesToTrash.getUnchecked(i));

            OpenDocumentManager::getInstance()->closeFile (f, false);

            if (! f.moveToTrash())
            {
                // xxx
            }
        }

        for (i = itemsToRemove.size(); --i >= 0;)
        {
            ProjectTreeViewBase* itemToRemove = treeRootItem->findTreeViewItem (*itemsToRemove.getUnchecked(i));

            if (itemToRemove != 0)
            {
                OpenDocumentManager::getInstance()->closeFile (itemToRemove->getFile(), false);
                itemToRemove->deleteItem();
            }
        }
    }
}

static int indexOfNode (const ValueTree& parent, const ValueTree& child)
{
    for (int i = parent.getNumChildren(); --i >= 0;)
        if (parent.getChild (i) == child)
            return i;

    return -1;
}

void ProjectTreeViewBase::moveItems (OwnedArray <Project::Item>& selectedNodes,
                                     Project::Item destNode, int insertIndex)
{
    int i;
    for (i = selectedNodes.size(); --i >= 0;)
    {
        Project::Item* const n = selectedNodes.getUnchecked(i);

        if (destNode == *n || destNode.getNode().isAChildOf (n->getNode())) // Check for recursion.
            return;

        if (! destNode.canContain (*n))
            selectedNodes.remove (i);
    }

    // Don't include any nodes that are children of other selected nodes..
    for (i = selectedNodes.size(); --i >= 0;)
    {
        Project::Item* const n = selectedNodes.getUnchecked(i);

        for (int j = selectedNodes.size(); --j >= 0;)
        {
            if (j != i && n->getNode().isAChildOf (selectedNodes.getUnchecked(j)->getNode()))
            {
                selectedNodes.remove (i);
                break;
            }
        }
    }

    // Remove and re-insert them one at a time..
    for (i = 0; i < selectedNodes.size(); ++i)
    {
        Project::Item* selectedNode = selectedNodes.getUnchecked(i);

        if (selectedNode->getNode().getParent() == destNode.getNode()
              && indexOfNode (destNode.getNode(), selectedNode->getNode()) < insertIndex)
            --insertIndex;

        selectedNode->removeItemFromProject();
        destNode.addChild (*selectedNode, insertIndex++);
    }
}

//==============================================================================
bool ProjectTreeViewBase::isInterestedInFileDrag (const StringArray& files)
{
    return acceptsFileDrop (files);
}

void ProjectTreeViewBase::filesDropped (const StringArray& files, int insertIndex)
{
    addFiles (files, insertIndex);
}

static void getAllSelectedNodesInTree (Component* componentInTree, OwnedArray <Project::Item>& selectedNodes)
{
    TreeView* tree = dynamic_cast <TreeView*> (componentInTree);

    if (tree == 0)
        tree = componentInTree->findParentComponentOfClass ((TreeView*) 0);

    if (tree != 0)
    {
        const int numSelected = tree->getNumSelectedItems();

        for (int i = 0; i < numSelected; ++i)
        {
            const ProjectTreeViewBase* const p = dynamic_cast <ProjectTreeViewBase*> (tree->getSelectedItem (i));

            if (p != 0)
                selectedNodes.add (new Project::Item (p->item));
        }
    }
}

bool ProjectTreeViewBase::isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent)
{
    if (sourceDescription != projectItemDragType)
        return false;

    OwnedArray <Project::Item> selectedNodes;
    getAllSelectedNodesInTree (sourceComponent, selectedNodes);

    return selectedNodes.size() > 0 && acceptsDragItems (selectedNodes);
}

void ProjectTreeViewBase::itemDropped (const String& sourceDescription, Component* sourceComponent, int insertIndex)
{
    OwnedArray <Project::Item> selectedNodes;
    getAllSelectedNodesInTree (sourceComponent, selectedNodes);

    if (selectedNodes.size() > 0)
    {
        TreeView* tree = getOwnerView();
        ScopedPointer <XmlElement> oldOpenness (tree->getOpennessState (false));

        moveSelectedItemsTo (selectedNodes, insertIndex);

        if (oldOpenness != 0)
            tree->restoreOpennessState (*oldOpenness);
    }
}

//==============================================================================
void ProjectTreeViewBase::valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
{
    if (tree == item.getNode())
        repaintItem();
}

void ProjectTreeViewBase::valueTreeChildrenChanged (ValueTree& tree)
{
    if (tree == item.getNode())
    {
        refreshSubItems();
        treeHasChanged();
        setOpen (true);
    }
}

void ProjectTreeViewBase::valueTreeParentChanged (ValueTree& tree)
{
}

//==============================================================================
bool ProjectTreeViewBase::mightContainSubItems()
{
    return item.getNumChildren() > 0;
}

const String ProjectTreeViewBase::getUniqueName() const
{
    jassert (item.getID().isNotEmpty());
    return item.getID();
}

void ProjectTreeViewBase::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen)
        refreshSubItems();
}

void ProjectTreeViewBase::addSubItems()
{
    for (int i = 0; i < item.getNumChildren(); ++i)
    {
        ProjectTreeViewBase* p = createSubItem (item.getChild(i));

        if (p != 0)
            addSubItem (p);
    }
}

void ProjectTreeViewBase::refreshSubItems()
{
    ScopedPointer <XmlElement> oldOpenness (getOpennessState());

    clearSubItems();
    addSubItems();

    if (oldOpenness != 0)
        restoreOpennessState (*oldOpenness);
}

void ProjectTreeViewBase::showMultiSelectionPopupMenu()
{
    PopupMenu m;
    m.addItem (6, "Delete");

    switch (m.show())
    {
        case 6:     deleteAllSelectedItems(); break;
        default:    break;
    }
}

void ProjectTreeViewBase::itemDoubleClicked (const MouseEvent& e)
{
    showDocument();
}

void ProjectTreeViewBase::itemSelectionChanged (bool isNowSelected)
{
    if (isNowSelected)
        showDocument();
}

const String ProjectTreeViewBase::getTooltip()
{
    return String::empty;
}

const String ProjectTreeViewBase::getDragSourceDescription()
{
    return projectItemDragType;
}

//==============================================================================
ProjectTreeViewBase* ProjectTreeViewBase::getParentProjectItem() const
{
    return dynamic_cast <ProjectTreeViewBase*> (getParentItem());
}

ProjectContentComponent* ProjectTreeViewBase::getProjectContentComponent() const
{
    Component* c = getOwnerView();

    while (c != 0)
    {
        ProjectContentComponent* pcc = dynamic_cast <ProjectContentComponent*> (c);

        if (pcc != 0)
            return pcc;

        c = c->getParentComponent();
    }

    return 0;
}
