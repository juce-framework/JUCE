/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class FileListTreeItem   : public TreeViewItem,
                           private TimeSliceClient,
                           private AsyncUpdater
{
public:
    FileListTreeItem (FileTreeComponent& treeComp,
                      const File& f,
                      TimeSliceThread& t)
        : file (f),
          owner (treeComp),
          thread (t)
    {
    }

    void update (const DirectoryContentsList::FileInfo& fileInfo)
    {
        fileSize = File::descriptionOfSizeInBytes (fileInfo.fileSize);
        modTime = fileInfo.modificationTime.formatted ("%d %b '%y %H:%M");
        isDirectory = fileInfo.isDirectory;
        repaintItem();
    }

    ~FileListTreeItem() override
    {
        thread.removeTimeSliceClient (this);
        clearSubItems();
    }

    //==============================================================================
    bool mightContainSubItems() override                 { return isDirectory; }
    String getUniqueName() const override                { return file.getFullPathName(); }
    int getItemHeight() const override                   { return owner.getItemHeight(); }

    var getDragSourceDescription() override              { return owner.getDragAndDropDescription(); }

    void itemOpennessChanged (bool isNowOpen) override
    {
        NullCheckedInvocation::invoke (onOpennessChanged, file, isNowOpen);
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        ScopedLock lock (iconUpdate);

        if (file != File())
        {
            updateIcon (true);

            if (icon.isNull())
                thread.addTimeSliceClient (this);
        }

        owner.getLookAndFeel().drawFileBrowserRow (g, width, height,
                                                   file, file.getFileName(),
                                                   &icon, fileSize, modTime,
                                                   isDirectory, isSelected(),
                                                   getIndexInParent(), owner);
    }

    String getAccessibilityName() override
    {
        return file.getFileName();
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
    std::function<void (const File&, bool)> onOpennessChanged;

private:
    FileTreeComponent& owner;
    bool isDirectory = false;
    TimeSliceThread& thread;
    CriticalSection iconUpdate;
    Image icon;
    String fileSize, modTime;

    void updateIcon (const bool onlyUpdateIfCached)
    {
        if (icon.isNull())
        {
            auto hashCode = (file.getFullPathName() + "_iconCacheSalt").hashCode();
            auto im = ImageCache::getFromHashCode (hashCode);

            if (im.isNull() && ! onlyUpdateIfCached)
            {
                im = detail::WindowingHelpers::createIconForFile (file);

                if (im.isValid())
                    ImageCache::addImageToCache (im, hashCode);
            }

            if (im.isValid())
            {
                {
                    ScopedLock lock (iconUpdate);
                    icon = im;
                }

                triggerAsyncUpdate();
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListTreeItem)
};

class DirectoryScanner  : private ChangeListener
{
public:
    struct Listener
    {
        virtual ~Listener() = default;

        virtual void rootChanged() = 0;
        virtual void directoryChanged (const DirectoryContentsList&) = 0;
    };

    DirectoryScanner (DirectoryContentsList& rootIn, Listener& listenerIn)
        : root (rootIn), listener (listenerIn)
    {
        root.addChangeListener (this);
    }

    ~DirectoryScanner() override
    {
        root.removeChangeListener (this);
    }

    void refresh()
    {
        root.refresh();
    }

    void open (const File& f)
    {
        auto& contentsList = [&]() -> auto&
        {
            if (auto it = contentsLists.find (f); it != contentsLists.end())
                return it->second;

            auto insertion = contentsLists.emplace (std::piecewise_construct,
                                                   std::forward_as_tuple (f),
                                                   std::forward_as_tuple (nullptr, root.getTimeSliceThread()));
            return insertion.first->second;
        }();

        contentsList.addChangeListener (this);
        contentsList.setDirectory (f, true, true);
        contentsList.refresh();
    }

    void close (const File& f)
    {
        if (auto it = contentsLists.find (f); it != contentsLists.end())
            contentsLists.erase (it);
    }

    File getRootDirectory() const
    {
        return root.getDirectory();
    }

    bool isStillLoading() const
    {
        return std::any_of (contentsLists.begin(),
                            contentsLists.end(),
                            [] (const auto& it)
                            {
                                return it.second.isStillLoading();
                            });
    }

private:
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        auto* sourceList = static_cast<DirectoryContentsList*> (source);

        if (sourceList == &root)
        {
            if (std::exchange (lastDirectory, root.getDirectory()) != root.getDirectory())
            {
                contentsLists.clear();
                listener.rootChanged();
            }
            else
            {
                for (auto& contentsList : contentsLists)
                    contentsList.second.refresh();
            }
        }

        listener.directoryChanged (*sourceList);
    }

    DirectoryContentsList& root;
    Listener& listener;
    File lastDirectory;
    std::map<File, DirectoryContentsList> contentsLists;
};

class FileTreeComponent::Controller  : private DirectoryScanner::Listener
{
public:
    explicit Controller (FileTreeComponent& ownerIn)
        : owner (ownerIn),
          scanner (owner.directoryContentsList, *this)
    {
        refresh();
    }

    ~Controller() override
    {
        owner.deleteRootItem();
    }

    void refresh()
    {
        scanner.refresh();
    }

    void selectFile (const File& target)
    {
        pendingFileSelection.emplace (target);
        tryResolvePendingFileSelection();
    }

private:
    template <typename ItemCallback>
    static void forEachItemRecursive (TreeViewItem* item, ItemCallback&& cb)
    {
        if (item == nullptr)
            return;

        if (auto* fileListItem = dynamic_cast<FileListTreeItem*> (item))
            cb (fileListItem);

        for (int i = 0; i < item->getNumSubItems(); ++i)
            forEachItemRecursive (item->getSubItem (i), cb);
    }

    //==============================================================================
    void rootChanged() override
    {
        owner.deleteRootItem();
        treeItemForFile.clear();
        owner.setRootItem (createNewItem (scanner.getRootDirectory()).release());
    }

    void directoryChanged (const DirectoryContentsList& contentsList) override
    {
        auto* parentItem = [&]() -> FileListTreeItem*
        {
            if (auto it = treeItemForFile.find (contentsList.getDirectory()); it != treeItemForFile.end())
                return it->second;

            return nullptr;
        }();

        if (parentItem == nullptr)
        {
            jassertfalse;
            return;
        }

        for (int i = 0; i < contentsList.getNumFiles(); ++i)
        {
            auto file = contentsList.getFile (i);

            DirectoryContentsList::FileInfo fileInfo;
            contentsList.getFileInfo (i, fileInfo);

            auto* item = [&]
            {
                if (auto it = treeItemForFile.find (file); it != treeItemForFile.end())
                    return it->second;

                auto* newItem = createNewItem (file).release();
                parentItem->addSubItem (newItem);
                return newItem;
            }();

            if (item->isOpen() && fileInfo.isDirectory)
                scanner.open (item->file);

            item->update (fileInfo);
        }

        if (contentsList.isStillLoading())
            return;

        std::set<File> allFiles;

        for (int i = 0; i < contentsList.getNumFiles(); ++i)
            allFiles.insert (contentsList.getFile (i));

        for (int i = 0; i < parentItem->getNumSubItems();)
        {
            auto* fileItem = dynamic_cast<FileListTreeItem*> (parentItem->getSubItem (i));

            if (fileItem != nullptr && allFiles.count (fileItem->file) == 0)
            {
                forEachItemRecursive (parentItem->getSubItem (i),
                                      [this] (auto* item)
                                      {
                                          scanner.close (item->file);
                                          treeItemForFile.erase (item->file);
                                      });

                parentItem->removeSubItem (i);
            }
            else
            {
                ++i;
            }
        }

        struct Comparator
        {
            static int compareElements (TreeViewItem* first, TreeViewItem* second)
            {
                auto* item1 = dynamic_cast<FileListTreeItem*> (first);
                auto* item2 = dynamic_cast<FileListTreeItem*> (second);

                if (item1 == nullptr || item2 == nullptr)
                    return 0;

                if (item1->file < item2->file)
                    return -1;

                if (item1->file > item2->file)
                    return 1;

                return 0;
            }
        };

        static Comparator comparator;
        parentItem->sortSubItems (comparator);
        tryResolvePendingFileSelection();
    }

    std::unique_ptr<FileListTreeItem> createNewItem (const File& file)
    {
        auto newItem = std::make_unique<FileListTreeItem> (owner,
                                                           file,
                                                           owner.directoryContentsList.getTimeSliceThread());

        newItem->onOpennessChanged = [this, itemPtr = newItem.get()] (const auto& f, auto isOpen)
        {
            if (isOpen)
            {
                scanner.open (f);
            }
            else
            {
                forEachItemRecursive (itemPtr,
                                      [this] (auto* item)
                                      {
                                          scanner.close (item->file);
                                      });
            }
        };

        treeItemForFile[file] = newItem.get();
        return newItem;
    }

    void tryResolvePendingFileSelection()
    {
        if (! pendingFileSelection.has_value())
            return;

        if (auto item = treeItemForFile.find (*pendingFileSelection); item != treeItemForFile.end())
        {
            item->second->setSelected (true, true);
            pendingFileSelection.reset();
            return;
        }

        if (owner.directoryContentsList.isStillLoading() || scanner.isStillLoading())
            return;

        owner.clearSelectedItems();
    }

    FileTreeComponent& owner;
    std::map<File, FileListTreeItem*> treeItemForFile;
    DirectoryScanner scanner;
    std::optional<File> pendingFileSelection;
};

//==============================================================================
FileTreeComponent::FileTreeComponent (DirectoryContentsList& listToShow)
    : DirectoryContentsDisplayComponent (listToShow),
      itemHeight (22)
{
    controller = std::make_unique<Controller> (*this);
    setRootItemVisible (false);
    refresh();
}

FileTreeComponent::~FileTreeComponent()
{
    deleteRootItem();
}

void FileTreeComponent::refresh()
{
    controller->refresh();
}

//==============================================================================
File FileTreeComponent::getSelectedFile (const int index) const
{
    if (auto* item = dynamic_cast<const FileListTreeItem*> (getSelectedItem (index)))
        return item->file;

    return {};
}

void FileTreeComponent::deselectAllFiles()
{
    clearSelectedItems();
}

void FileTreeComponent::scrollToTop()
{
    getViewport()->getVerticalScrollBar().setCurrentRangeStart (0);
}

void FileTreeComponent::setDragAndDropDescription (const String& description)
{
    dragAndDropDescription = description;
}

void FileTreeComponent::setSelectedFile (const File& target)
{
    controller->selectFile (target);
}

void FileTreeComponent::setItemHeight (int newHeight)
{
    if (itemHeight != newHeight)
    {
        itemHeight = newHeight;

        if (auto* root = getRootItem())
            root->treeHasChanged();
    }
}

} // namespace juce
