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

class SourceFileItem   : public ProjectTreeItemBase
{
public:
    SourceFileItem (const Project::Item& projectItem)
        : ProjectTreeItemBase (projectItem)
    {
    }

    bool acceptsFileDrop (const StringArray&) const override             { return false; }
    bool acceptsDragItems (const OwnedArray <Project::Item>&) override   { return false; }

    String getDisplayName() const override
    {
        return getFile().getFileName();
    }

    static File findCorrespondingHeaderOrCpp (const File& f)
    {
        if (f.hasFileExtension (sourceFileExtensions))  return f.withFileExtension (".h");
        if (f.hasFileExtension (headerFileExtensions))  return f.withFileExtension (".cpp");

        return File();
    }

    void setName (const String& newName) override
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
            Project::Item correspondingItem (item.project.getMainGroup().findItemForFile (correspondingFile));

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

    ProjectTreeItemBase* createSubItem (const Project::Item&) override
    {
        jassertfalse;
        return nullptr;
    }

    void showDocument() override
    {
        const File f (getFile());

        if (f.exists())
            if (ProjectContentComponent* pcc = getProjectContentComponent())
                pcc->showEditorForFile (f, false);
    }

    void showPopupMenu() override
    {
        PopupMenu m;

        if (GroupItem* parentGroup = dynamic_cast<GroupItem*> (getParentProjectItem()))
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

        launchPopupMenu (m);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        switch (resultCode)
        {
            case 1:     getFile().startAsProcess(); break;
            case 2:     revealInFinder(); break;
            case 3:     deleteAllSelectedItems(); break;
            case 4:     triggerAsyncRename (item); break;

            default:
                if (GroupItem* parentGroup = dynamic_cast<GroupItem*> (getParentProjectItem()))
                    parentGroup->processCreateFileMenuItem (resultCode);

                break;
        }
    }
};
