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

Image juce_createIconForFile (const File&);

//==============================================================================
class FileListTreeItem   : public TreeViewItem,
                           private TimeSliceClient,
                           private AsyncUpdater,
                           private ChangeListener
{
public:
    FileListTreeItem (FileTreeComponent& treeComp,
                      DirectoryContentsList* const parentContents,
                      const int indexInContents,
                      const File& f,
                      TimeSliceThread& t)
        : file (f),
          owner (treeComp),
          parentContentsList (parentContents),
          indexInContentsList (indexInContents),
          subContentsList (nullptr, false),
          thread (t)
    {
        DirectoryContentsList::FileInfo fileInfo;

        if (parentContents != nullptr
             && parentContents->getFileInfo (indexInContents, fileInfo))
        {
            fileSize = File::descriptionOfSizeInBytes (fileInfo.fileSize);
            modTime = fileInfo.modificationTime.formatted ("%d %b '%y %H:%M");
            isDirectory = fileInfo.isDirectory;
        }
        else
        {
            isDirectory = true;
        }
    }

    ~FileListTreeItem()
    {
        thread.removeTimeSliceClient (this);
        clearSubItems();
        removeSubContentsList();
    }

    //==============================================================================
    bool mightContainSubItems() override                 { return isDirectory; }
    String getUniqueName() const override                { return file.getFullPathName(); }
    int getItemHeight() const override                   { return owner.getItemHeight(); }

    var getDragSourceDescription() override              { return owner.getDragAndDropDescription(); }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
        {
            clearSubItems();

            isDirectory = file.isDirectory();

            if (isDirectory)
            {
                if (subContentsList == nullptr)
                {
                    jassert (parentContentsList != nullptr);

                    DirectoryContentsList* const l = new DirectoryContentsList (parentContentsList->getFilter(), thread);

                    l->setDirectory (file,
                                     parentContentsList->isFindingDirectories(),
                                     parentContentsList->isFindingFiles());

                    setSubContentsList (l, true);
                }

                changeListenerCallback (nullptr);
            }
        }
    }

    void removeSubContentsList()
    {
        if (subContentsList != nullptr)
        {
            subContentsList->removeChangeListener (this);
            subContentsList.clear();
        }
    }

    void setSubContentsList (DirectoryContentsList* newList, const bool canDeleteList)
    {
        removeSubContentsList();

        OptionalScopedPointer<DirectoryContentsList> newPointer (newList, canDeleteList);
        subContentsList = newPointer;
        newList->addChangeListener (this);
    }

    bool selectFile (const File& target)
    {
        if (file == target)
        {
            setSelected (true, true);
            return true;
        }

        if (target.isAChildOf (file))
        {
            setOpen (true);

            for (int maxRetries = 500; --maxRetries > 0;)
            {
                for (int i = 0; i < getNumSubItems(); ++i)
                    if (FileListTreeItem* f = dynamic_cast <FileListTreeItem*> (getSubItem (i)))
                        if (f->selectFile (target))
                            return true;

                // if we've just opened and the contents are still loading, wait for it..
                if (subContentsList != nullptr && subContentsList->isStillLoading())
                {
                    Thread::sleep (10);
                    rebuildItemsFromContentList();
                }
                else
                {
                    break;
                }
            }
        }

        return false;
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        rebuildItemsFromContentList();
    }

    void rebuildItemsFromContentList()
    {
        clearSubItems();

        if (isOpen() && subContentsList != nullptr)
        {
            for (int i = 0; i < subContentsList->getNumFiles(); ++i)
                addSubItem (new FileListTreeItem (owner, subContentsList, i,
                                                  subContentsList->getFile(i), thread));
        }
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        if (file != File::nonexistent)
        {
            updateIcon (true);

            if (icon.isNull())
                thread.addTimeSliceClient (this);
        }

        owner.getLookAndFeel().drawFileBrowserRow (g, width, height,
                                                   file.getFileName(),
                                                   &icon, fileSize, modTime,
                                                   isDirectory, isSelected(),
                                                   indexInContentsList, owner);
    }

    void itemClicked (const MouseEvent& e) override
    {
        owner.sendMouseClickMessage (file, e);
    }

    void itemDoubleClicked (const MouseEvent& e) override
    {
        TreeViewItem::itemDoubleClicked (e);

        owner.sendDoubleClickMessage (file);
    }

    void itemSelectionChanged (bool) override
    {
        owner.sendSelectionChangeMessage();
    }

    int useTimeSlice() override
    {
        updateIcon (false);
        return -1;
    }

    void handleAsyncUpdate() override
    {
        owner.repaint();
    }

    const File file;

private:
    FileTreeComponent& owner;
    DirectoryContentsList* parentContentsList;
    int indexInContentsList;
    OptionalScopedPointer<DirectoryContentsList> subContentsList;
    bool isDirectory;
    TimeSliceThread& thread;
    Image icon;
    String fileSize, modTime;

    void updateIcon (const bool onlyUpdateIfCached)
    {
        if (icon.isNull())
        {
            const int hashCode = (file.getFullPathName() + "_iconCacheSalt").hashCode();
            Image im (ImageCache::getFromHashCode (hashCode));

            if (im.isNull() && ! onlyUpdateIfCached)
            {
                im = juce_createIconForFile (file);

                if (im.isValid())
                    ImageCache::addImageToCache (im, hashCode);
            }

            if (im.isValid())
            {
                icon = im;
                triggerAsyncUpdate();
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListTreeItem)
};

//==============================================================================
FileTreeComponent::FileTreeComponent (DirectoryContentsList& listToShow)
    : DirectoryContentsDisplayComponent (listToShow),
      itemHeight (22)
{
    setRootItemVisible (false);
    refresh();
}

FileTreeComponent::~FileTreeComponent()
{
    deleteRootItem();
}

void FileTreeComponent::refresh()
{
    deleteRootItem();

    FileListTreeItem* const root
        = new FileListTreeItem (*this, nullptr, 0, fileList.getDirectory(),
                                fileList.getTimeSliceThread());

    root->setSubContentsList (&fileList, false);
    setRootItem (root);
}

//==============================================================================
File FileTreeComponent::getSelectedFile (const int index) const
{
    if (const FileListTreeItem* const item = dynamic_cast <const FileListTreeItem*> (getSelectedItem (index)))
        return item->file;

    return File::nonexistent;
}

void FileTreeComponent::deselectAllFiles()
{
    clearSelectedItems();
}

void FileTreeComponent::scrollToTop()
{
    getViewport()->getVerticalScrollBar()->setCurrentRangeStart (0);
}

void FileTreeComponent::setDragAndDropDescription (const String& description)
{
    dragAndDropDescription = description;
}

void FileTreeComponent::setSelectedFile (const File& target)
{
    if (FileListTreeItem* t = dynamic_cast <FileListTreeItem*> (getRootItem()))
        if (! t->selectFile (target))
            clearSelectedItems();
}

void FileTreeComponent::setItemHeight (int newHeight)
{
    if (itemHeight != newHeight)
    {
        itemHeight = newHeight;

        if (TreeViewItem* root = getRootItem())
            root->treeHasChanged();
    }
}
