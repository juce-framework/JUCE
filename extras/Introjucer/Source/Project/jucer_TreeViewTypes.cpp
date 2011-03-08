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

#include "jucer_TreeViewTypes.h"
#include "jucer_ProjectInformationComponent.h"
#include "jucer_GroupInformationComponent.h"
#include "../Application/jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "jucer_NewFileWizard.h"


//==============================================================================
GroupTreeViewItem::GroupTreeViewItem (const Project::Item& item_)
    : ProjectTreeViewBase (item_)
{
}

GroupTreeViewItem::~GroupTreeViewItem()
{
}

void GroupTreeViewItem::addNewGroup()
{
    Project::Item group (item.getProject().createNewGroup());
    item.addChild (group, 0);
    triggerAsyncRename (group);
}

bool GroupTreeViewItem::acceptsDragItems (const OwnedArray <Project::Item>& selectedNodes)
{
    for (int i = selectedNodes.size(); --i >= 0;)
        if (item.canContain (*selectedNodes.getUnchecked(i)))
            return true;

    return false;
}

void GroupTreeViewItem::addFiles (const StringArray& files, int insertIndex)
{
    for (int i = 0; i < files.size(); ++i)
    {
        const File file (files[i]);

        if (item.addFile (file, insertIndex))
            ++insertIndex;
    }
}

void GroupTreeViewItem::moveSelectedItemsTo (OwnedArray <Project::Item>& selectedNodes, int insertIndex)
{
    moveItems (selectedNodes, item, insertIndex);
}

void GroupTreeViewItem::checkFileStatus()
{
    for (int i = 0; i < getNumSubItems(); ++i)
    {
        ProjectTreeViewBase* p = dynamic_cast <ProjectTreeViewBase*> (getSubItem(i));

        if (p != 0)
            p->checkFileStatus();
    }
}

ProjectTreeViewBase* GroupTreeViewItem::createSubItem (const Project::Item& child)
{
    if (child.isGroup())
        return new GroupTreeViewItem (child);

    if (child.isFile())
        return new SourceFileTreeViewItem (child);

    jassertfalse
    return 0;
}

void GroupTreeViewItem::showDocument()
{
    ProjectContentComponent* pcc = getProjectContentComponent();

    if (pcc != 0)
    {
        if (isRoot())
            pcc->setEditorComponent (new ProjectInformationComponent (item.getProject()), 0);
        else
            pcc->setEditorComponent (new GroupInformationComponent (item), 0);
    }
}

void GroupTreeViewItem::showPopupMenu()
{
    PopupMenu m;
    addCreateFileMenuItems (m);
    m.addSeparator();
    m.addItem (3, "Sort Contents Alphabetically");
    m.addSeparator();
    m.addItem (1, "Rename...");

    if (! isRoot())
        m.addItem (2, "Delete");

    const int res = m.show();
    switch (res)
    {
        case 1:     triggerAsyncRename (item); break;
        case 2:     deleteAllSelectedItems(); break;
        case 3:     item.sortAlphabetically(); break;
        default:    processCreateFileMenuItem (res); break;
    }
}

void GroupTreeViewItem::addCreateFileMenuItems (PopupMenu& m)
{
    m.addItem (1001, "Add New Group");
    m.addItem (1002, "Add Existing Files...");

    m.addSeparator();
    NewFileWizard::getInstance()->addWizardsToMenu (m);
}

void GroupTreeViewItem::processCreateFileMenuItem (int menuID)
{
    switch (menuID)
    {
        case 1001:  addNewGroup(); break;
        case 1002:  browseToAddExistingFiles(); break;

        default:
            NewFileWizard::getInstance()->runWizardFromMenu (menuID, item);
            break;
    }
}

//==============================================================================
//==============================================================================
SourceFileTreeViewItem::SourceFileTreeViewItem (const Project::Item& item_)
    : ProjectTreeViewBase (item_)
{
}

SourceFileTreeViewItem::~SourceFileTreeViewItem()
{
}

const String SourceFileTreeViewItem::getDisplayName() const
{
    return getFile().getFileName();
}

static const File findCorrespondingHeaderOrCpp (const File& f)
{
    if (f.hasFileExtension (sourceFileExtensions))
        return f.withFileExtension (".h");
    else if (f.hasFileExtension (headerFileExtensions))
        return f.withFileExtension (".cpp");

    return File::nonexistent;
}

void SourceFileTreeViewItem::setName (const String& newName)
{
    if (newName != File::createLegalFileName (newName))
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "File Rename",
                                     "That filename contained some illegal characters!");
        triggerAsyncRename (item);
        return;
    }

    File oldFile (getFile());
    File newFile (oldFile.getSiblingFile (newName));
    File correspondingFile (findCorrespondingHeaderOrCpp (oldFile));

    if (correspondingFile.exists() && newFile.hasFileExtension (oldFile.getFileExtension()))
    {
        Project::Item correspondingItem (item.getProject().getMainGroup().findItemForFile (correspondingFile));

        if (correspondingItem.isValid())
        {
            if (AlertWindow::showOkCancelBox (AlertWindow::NoIcon, "File Rename",
                                              "Do you also want to rename the corresponding file \"" + correspondingFile.getFileName()
                                                + "\" to match?"))
            {
                if (! item.renameFile (newFile))
                {
                    AlertWindow::showMessageBox (AlertWindow::WarningIcon, "File Rename",
                                                 "Failed to rename \"" + oldFile.getFullPathName() + "\"!\n\nCheck your file permissions!");
                    return;
                }

                if (! correspondingItem.renameFile (newFile.withFileExtension (correspondingFile.getFileExtension())))
                {
                    AlertWindow::showMessageBox (AlertWindow::WarningIcon, "File Rename",
                                                 "Failed to rename \"" + correspondingFile.getFullPathName() + "\"!\n\nCheck your file permissions!");
                }
            }
        }
    }

    if (! item.renameFile (newFile))
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "File Rename",
                                     "Failed to rename the file!\n\nCheck your file permissions!");
    }
}

ProjectTreeViewBase* SourceFileTreeViewItem::createSubItem (const Project::Item& child)
{
    jassertfalse
    return 0;
}

void SourceFileTreeViewItem::showDocument()
{
    ProjectContentComponent* pcc = getProjectContentComponent();
    const File f (getFile());

    if (pcc != 0 && f.exists())
        pcc->showEditorForFile (f);
}

void SourceFileTreeViewItem::showPopupMenu()
{
    GroupTreeViewItem* parentGroup = dynamic_cast <GroupTreeViewItem*> (getParentProjectItem());

    PopupMenu m;

    if (parentGroup != 0)
    {
        parentGroup->addCreateFileMenuItems (m);
        m.addSeparator();
    }

    m.addItem (1, "Open in external editor");
    m.addItem (2,
#if JUCE_MAC
                  "Reveal in Finder");
#else
                  "Reveal in Explorer");
#endif

    m.addItem (4, "Rename File...");
    m.addSeparator();
    m.addItem (3, "Delete");

    const int res = m.show();
    switch (res)
    {
        case 1:     getFile().startAsProcess(); break;
        case 2:     revealInFinder(); break;
        case 3:     deleteAllSelectedItems(); break;
        case 4:     triggerAsyncRename (item); break;

        default:
            if (parentGroup != 0)
                parentGroup->processCreateFileMenuItem (res);

            break;
    }
}
