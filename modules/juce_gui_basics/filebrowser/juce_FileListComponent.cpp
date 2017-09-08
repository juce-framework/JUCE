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

namespace juce
{

Image juce_createIconForFile (const File& file);


//==============================================================================
FileListComponent::FileListComponent (DirectoryContentsList& listToShow)
    : ListBox ({}, nullptr),
      DirectoryContentsDisplayComponent (listToShow)
{
    setModel (this);
    directoryContentsList.addChangeListener (this);
}

FileListComponent::~FileListComponent()
{
    directoryContentsList.removeChangeListener (this);
}

int FileListComponent::getNumSelectedFiles() const
{
    return getNumSelectedRows();
}

File FileListComponent::getSelectedFile (int index) const
{
    return directoryContentsList.getFile (getSelectedRow (index));
}

void FileListComponent::deselectAllFiles()
{
    deselectAllRows();
}

void FileListComponent::scrollToTop()
{
    getVerticalScrollBar()->setCurrentRangeStart (0);
}

void FileListComponent::setSelectedFile (const File& f)
{
    for (int i = directoryContentsList.getNumFiles(); --i >= 0;)
    {
        if (directoryContentsList.getFile(i) == f)
        {
            selectRow (i);
            return;
        }
    }

    deselectAllRows();
}

//==============================================================================
void FileListComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateContent();

    if (lastDirectory != directoryContentsList.getDirectory())
    {
        lastDirectory = directoryContentsList.getDirectory();
        deselectAllRows();
    }
}

//==============================================================================
class FileListComponent::ItemComponent  : public Component,
                                          private TimeSliceClient,
                                          private AsyncUpdater
{
public:
    ItemComponent (FileListComponent& fc, TimeSliceThread& t)
        : owner (fc), thread (t)
    {
    }

    ~ItemComponent()
    {
        thread.removeTimeSliceClient (this);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        getLookAndFeel().drawFileBrowserRow (g, getWidth(), getHeight(),
                                             file, file.getFileName(),
                                             &icon, fileSize, modTime,
                                             isDirectory, highlighted,
                                             index, owner);
    }

    void mouseDown (const MouseEvent& e) override
    {
        owner.selectRowsBasedOnModifierKeys (index, e.mods, true);
        owner.sendMouseClickMessage (file, e);
    }

    void mouseDoubleClick (const MouseEvent&) override
    {
        owner.sendDoubleClickMessage (file);
    }

    void update (const File& root, const DirectoryContentsList::FileInfo* fileInfo,
                 int newIndex, bool nowHighlighted)
    {
        thread.removeTimeSliceClient (this);

        if (nowHighlighted != highlighted || newIndex != index)
        {
            index = newIndex;
            highlighted = nowHighlighted;
            repaint();
        }

        File newFile;
        String newFileSize, newModTime;

        if (fileInfo != nullptr)
        {
            newFile = root.getChildFile (fileInfo->filename);
            newFileSize = File::descriptionOfSizeInBytes (fileInfo->fileSize);
            newModTime = fileInfo->modificationTime.formatted ("%d %b '%y %H:%M");
        }

        if (newFile != file
             || fileSize != newFileSize
             || modTime != newModTime)
        {
            file = newFile;
            fileSize = newFileSize;
            modTime = newModTime;
            icon = Image();
            isDirectory = fileInfo != nullptr && fileInfo->isDirectory;

            repaint();
        }

        if (file != File() && icon.isNull() && ! isDirectory)
        {
            updateIcon (true);

            if (! icon.isValid())
                thread.addTimeSliceClient (this);
        }
    }

    int useTimeSlice() override
    {
        updateIcon (false);
        return -1;
    }

    void handleAsyncUpdate() override
    {
        repaint();
    }

private:
    //==============================================================================
    FileListComponent& owner;
    TimeSliceThread& thread;
    File file;
    String fileSize, modTime;
    Image icon;
    int index = 0;
    bool highlighted = false, isDirectory = false;

    void updateIcon (const bool onlyUpdateIfCached)
    {
        if (icon.isNull())
        {
            auto hashCode = (file.getFullPathName() + "_iconCacheSalt").hashCode();
            auto im = ImageCache::getFromHashCode (hashCode);

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};

//==============================================================================
int FileListComponent::getNumRows()
{
    return directoryContentsList.getNumFiles();
}

void FileListComponent::paintListBoxItem (int, Graphics&, int, int, bool)
{
}

Component* FileListComponent::refreshComponentForRow (int row, bool isSelected, Component* existingComponentToUpdate)
{
    jassert (existingComponentToUpdate == nullptr || dynamic_cast<ItemComponent*> (existingComponentToUpdate) != nullptr);

    auto comp = static_cast<ItemComponent*> (existingComponentToUpdate);

    if (comp == nullptr)
        comp = new ItemComponent (*this, directoryContentsList.getTimeSliceThread());

    DirectoryContentsList::FileInfo fileInfo;
    comp->update (directoryContentsList.getDirectory(),
                  directoryContentsList.getFileInfo (row, fileInfo) ? &fileInfo : nullptr,
                  row, isSelected);

    return comp;
}

void FileListComponent::selectedRowsChanged (int /*lastRowSelected*/)
{
    sendSelectionChangeMessage();
}

void FileListComponent::deleteKeyPressed (int /*currentSelectedRow*/)
{
}

void FileListComponent::returnKeyPressed (int currentSelectedRow)
{
    sendDoubleClickMessage (directoryContentsList.getFile (currentSelectedRow));
}

} // namespace juce
