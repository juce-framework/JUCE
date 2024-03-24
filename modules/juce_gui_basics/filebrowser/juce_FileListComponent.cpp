/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
FileListComponent::FileListComponent (DirectoryContentsList& listToShow)
    : ListBox ({}, this),
      DirectoryContentsDisplayComponent (listToShow),
      lastDirectory (listToShow.getDirectory())
{
    setTitle ("Files");
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
    getVerticalScrollBar().setCurrentRangeStart (0);
}

void FileListComponent::setSelectedFile (const File& f)
{
    if (! directoryContentsList.isStillLoading())
    {
        for (int i = directoryContentsList.getNumFiles(); --i >= 0;)
        {
            if (directoryContentsList.getFile (i) == f)
            {
                fileWaitingToBeSelected = File();

                updateContent();
                selectRow (i);
                return;
            }
        }
    }

    deselectAllRows();
    fileWaitingToBeSelected = f;
}

//==============================================================================
void FileListComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateContent();

    if (lastDirectory != directoryContentsList.getDirectory())
    {
        fileWaitingToBeSelected = File();
        lastDirectory = directoryContentsList.getDirectory();
        deselectAllRows();
    }

    if (fileWaitingToBeSelected != File())
        setSelectedFile (fileWaitingToBeSelected);
}

//==============================================================================
class FileListComponent::ItemComponent final : public Component,
                                               public TooltipClient,
                                               private TimeSliceClient,
                                               private AsyncUpdater
{
public:
    ItemComponent (FileListComponent& fc, TimeSliceThread& t)
        : owner (fc), thread (t)
    {
    }

    ~ItemComponent() override
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

    String getTooltip() override
    {
        return owner.getTooltipForRow (index);
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

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

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

String FileListComponent::getNameForRow (int rowNumber)
{
    return directoryContentsList.getFile (rowNumber).getFileName();
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
