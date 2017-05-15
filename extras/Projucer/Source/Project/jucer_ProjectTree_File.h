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

        return {};
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

        m.addItem (1, "Open in external editor");
        m.addItem (2,
                     #if JUCE_MAC
                      "Reveal in Finder");
                     #else
                      "Reveal in Explorer");
                     #endif

        m.addItem (4, "Rename File...");
        m.addSeparator();

        if (auto* group = dynamic_cast<GroupItem*> (getParentItem()))
        {
            if (group->isRoot())
            {
                m.addItem (5, "Binary Resource", true, item.shouldBeAddedToBinaryResources());
                m.addItem (6, "Xcode Resource",  true, item.shouldBeAddedToXcodeResources());
                m.addItem (7, "Compile",         true, item.shouldBeCompiled());
                m.addSeparator();
            }
        }

        m.addItem (3, "Delete");

        launchPopupMenu (m);
    }

    void showPlusMenu() override
    {
        if (auto* group = dynamic_cast<GroupItem*> (getParentItem()))
            group->showPlusMenu();
    }

    void handlePopupMenuResult (int resultCode) override
    {
        switch (resultCode)
        {
            case 1:     getFile().startAsProcess(); break;
            case 2:     revealInFinder(); break;
            case 3:     deleteAllSelectedItems(); break;
            case 4:     triggerAsyncRename (item); break;
            case 5:     item.getShouldAddToBinaryResourcesValue().setValue (! item.shouldBeAddedToBinaryResources()); break;
            case 6:     item.getShouldAddToXcodeResourcesValue().setValue (! item.shouldBeAddedToXcodeResources()); break;
            case 7:     item.getShouldCompileValue().setValue (! item.shouldBeCompiled()); break;

            default:
                if (GroupItem* parentGroup = dynamic_cast<GroupItem*> (getParentProjectItem()))
                    parentGroup->processCreateFileMenuItem (resultCode);

                break;
        }
    }
};
