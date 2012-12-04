/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

Image juce_createIconForFile (const File& file);


//==============================================================================
FileListComponent::FileListComponent (DirectoryContentsList& listToShow)
    : ListBox (String::empty, nullptr),
      DirectoryContentsDisplayComponent (listToShow)
{
    setModel (this);
    fileList.addChangeListener (this);
}

FileListComponent::~FileListComponent()
{
    fileList.removeChangeListener (this);
}

int FileListComponent::getNumSelectedFiles() const
{
    return getNumSelectedRows();
}

File FileListComponent::getSelectedFile (int index) const
{
    return fileList.getFile (getSelectedRow (index));
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
    for (int i = fileList.getNumFiles(); --i >= 0;)
    {
        if (fileList.getFile(i) == f)
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

    if (lastDirectory != fileList.getDirectory())
    {
        lastDirectory = fileList.getDirectory();
        deselectAllRows();
    }
}

//==============================================================================
class FileListComponent::ItemComponent  : public Component,
                                          private TimeSliceClient,
                                          private AsyncUpdater
{
public:
    ItemComponent (FileListComponent& owner_, TimeSliceThread& thread_)
        : owner (owner_), thread (thread_), index (0), highlighted (false)
    {
    }

    ~ItemComponent()
    {
        thread.removeTimeSliceClient (this);
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        getLookAndFeel().drawFileBrowserRow (g, getWidth(), getHeight(),
                                             file.getFileName(),
                                             &icon, fileSize, modTime,
                                             isDirectory, highlighted,
                                             index, owner);
    }

    void mouseDown (const MouseEvent& e)
    {
        owner.selectRowsBasedOnModifierKeys (index, e.mods, false);
        owner.sendMouseClickMessage (file, e);
    }

    void mouseDoubleClick (const MouseEvent&)
    {
        owner.sendDoubleClickMessage (file);
    }

    void update (const File& root,
                 const DirectoryContentsList::FileInfo* const fileInfo,
                 const int index_,
                 const bool highlighted_)
    {
        thread.removeTimeSliceClient (this);

        if (highlighted_ != highlighted || index_ != index)
        {
            index = index_;
            highlighted = highlighted_;
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
            icon = Image::null;
            isDirectory = fileInfo != nullptr && fileInfo->isDirectory;

            repaint();
        }

        if (file != File::nonexistent && icon.isNull() && ! isDirectory)
        {
            updateIcon (true);

            if (! icon.isValid())
                thread.addTimeSliceClient (this);
        }
    }

    int useTimeSlice()
    {
        updateIcon (false);
        return -1;
    }

    void handleAsyncUpdate()
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
    int index;
    bool highlighted, isDirectory;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};

//==============================================================================
int FileListComponent::getNumRows()
{
    return fileList.getNumFiles();
}

void FileListComponent::paintListBoxItem (int, Graphics&, int, int, bool)
{
}

Component* FileListComponent::refreshComponentForRow (int row, bool isSelected, Component* existingComponentToUpdate)
{
    jassert (existingComponentToUpdate == nullptr || dynamic_cast <ItemComponent*> (existingComponentToUpdate) != nullptr);

    ItemComponent* comp = static_cast <ItemComponent*> (existingComponentToUpdate);

    if (comp == nullptr)
        comp = new ItemComponent (*this, fileList.getTimeSliceThread());

    DirectoryContentsList::FileInfo fileInfo;
    comp->update (fileList.getDirectory(),
                  fileList.getFileInfo (row, fileInfo) ? &fileInfo : nullptr,
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
    sendDoubleClickMessage (fileList.getFile (currentSelectedRow));
}
