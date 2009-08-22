/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_FileTreeComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../graphics/imaging/juce_ImageCache.h"
#include "../../../events/juce_AsyncUpdater.h"

Image* juce_createIconForFile (const File& file);


//==============================================================================
class FileListTreeItem   : public TreeViewItem,
                           public TimeSliceClient,
                           public AsyncUpdater,
                           public ChangeListener
{
public:
    //==============================================================================
    FileListTreeItem (FileTreeComponent& owner_,
                      DirectoryContentsList* const parentContentsList_,
                      const int indexInContentsList_,
                      const File& file_,
                      TimeSliceThread& thread_) throw()
        : file (file_),
          owner (owner_),
          parentContentsList (parentContentsList_),
          indexInContentsList (indexInContentsList_),
          subContentsList (0),
          canDeleteSubContentsList (false),
          thread (thread_),
          icon (0)
    {
        DirectoryContentsList::FileInfo fileInfo;

        if (parentContentsList_ != 0
             && parentContentsList_->getFileInfo (indexInContentsList_, fileInfo))
        {
            fileSize = File::descriptionOfSizeInBytes (fileInfo.fileSize);
            modTime = fileInfo.modificationTime.formatted (T("%d %b '%y %H:%M"));
            isDirectory = fileInfo.isDirectory;
        }
        else
        {
            isDirectory = true;
        }
    }

    ~FileListTreeItem() throw()
    {
        thread.removeTimeSliceClient (this);

        clearSubItems();
        ImageCache::release (icon);

        if (canDeleteSubContentsList)
            delete subContentsList;
    }

    //==============================================================================
    bool mightContainSubItems()                 { return isDirectory; }
    const String getUniqueName() const          { return file.getFullPathName(); }
    int getItemHeight() const                   { return 22; }

    const String getDragSourceDescription()     { return owner.getDragAndDropDescription(); }

    void itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
        {
            clearSubItems();

            isDirectory = file.isDirectory();

            if (isDirectory)
            {
                if (subContentsList == 0)
                {
                    jassert (parentContentsList != 0);

                    DirectoryContentsList* const l = new DirectoryContentsList (parentContentsList->getFilter(), thread);
                    l->setDirectory (file, true, true);

                    setSubContentsList (l);
                    canDeleteSubContentsList = true;
                }

                changeListenerCallback (0);
            }
        }
    }

    void setSubContentsList (DirectoryContentsList* newList) throw()
    {
        jassert (subContentsList == 0);
        subContentsList = newList;
        newList->addChangeListener (this);
    }

    void changeListenerCallback (void*)
    {
        clearSubItems();

        if (isOpen() && subContentsList != 0)
        {
            for (int i = 0; i < subContentsList->getNumFiles(); ++i)
            {
                FileListTreeItem* const item
                    = new FileListTreeItem (owner, subContentsList, i, subContentsList->getFile(i), thread);

                addSubItem (item);
            }
        }
    }

    void paintItem (Graphics& g, int width, int height)
    {
        if (file != File::nonexistent && ! isDirectory)
        {
            updateIcon (true);

            if (icon == 0)
                thread.addTimeSliceClient (this);
        }

        owner.getLookAndFeel()
            .drawFileBrowserRow (g, width, height,
                                 file.getFileName(),
                                 icon,
                                 fileSize, modTime,
                                 isDirectory, isSelected(),
                                 indexInContentsList);
    }

    void itemClicked (const MouseEvent& e)
    {
        owner.sendMouseClickMessage (file, e);
    }

    void itemDoubleClicked (const MouseEvent& e)
    {
        TreeViewItem::itemDoubleClicked (e);

        owner.sendDoubleClickMessage (file);
    }

    void itemSelectionChanged (bool)
    {
        owner.sendSelectionChangeMessage();
    }

    bool useTimeSlice()
    {
        updateIcon (false);
        thread.removeTimeSliceClient (this);
        return false;
    }

    void handleAsyncUpdate()
    {
        owner.repaint();
    }

    const File file;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FileTreeComponent& owner;
    DirectoryContentsList* parentContentsList;
    int indexInContentsList;
    DirectoryContentsList* subContentsList;
    bool isDirectory, canDeleteSubContentsList;
    TimeSliceThread& thread;
    Image* icon;
    String fileSize;
    String modTime;

    void updateIcon (const bool onlyUpdateIfCached) throw()
    {
        if (icon == 0)
        {
            const int hashCode = (file.getFullPathName() + T("_iconCacheSalt")).hashCode();
            Image* im = ImageCache::getFromHashCode (hashCode);

            if (im == 0 && ! onlyUpdateIfCached)
            {
                im = juce_createIconForFile (file);

                if (im != 0)
                    ImageCache::addImageToCache (im, hashCode);
            }

            if (im != 0)
            {
                icon = im;
                triggerAsyncUpdate();
            }
        }
    }
};

//==============================================================================
FileTreeComponent::FileTreeComponent (DirectoryContentsList& listToShow)
    : DirectoryContentsDisplayComponent (listToShow)
{
    FileListTreeItem* const root
        = new FileListTreeItem (*this, 0, 0, listToShow.getDirectory(),
                                listToShow.getTimeSliceThread());

    root->setSubContentsList (&listToShow);
    setRootItemVisible (false);
    setRootItem (root);
}

FileTreeComponent::~FileTreeComponent()
{
    TreeViewItem* const root = getRootItem();
    setRootItem (0);
    delete root;
}

//==============================================================================
const File FileTreeComponent::getSelectedFile() const
{
    return getSelectedFile (0);
}

const File FileTreeComponent::getSelectedFile (const int index) const throw()
{
    const FileListTreeItem* const item = dynamic_cast <const FileListTreeItem*> (getSelectedItem (index));

    if (item != 0)
        return item->file;

    return File::nonexistent;
}

void FileTreeComponent::scrollToTop()
{
    getViewport()->getVerticalScrollBar()->setCurrentRangeStart (0);
}

void FileTreeComponent::setDragAndDropDescription (const String& description) throw()
{
    dragAndDropDescription = description;
}

END_JUCE_NAMESPACE
