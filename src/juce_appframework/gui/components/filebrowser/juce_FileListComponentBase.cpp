/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_FileListComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../graphics/imaging/juce_ImageCache.h"
#include "../../../events/juce_AsyncUpdater.h"

Image* juce_createIconForFile (const File& file);

//==============================================================================
FileListComponentBase::FileListComponentBase (DirectoryContentsList& listToShow)
    : fileList (listToShow),
      listeners (2)
{
}

FileListComponentBase::~FileListComponentBase()
{
}

//==============================================================================
FileBrowserListener::~FileBrowserListener()
{
}

void FileListComponentBase::addListener (FileBrowserListener* const listener) throw()
{
    jassert (listener != 0);
    if (listener != 0)
        listeners.add (listener);
}

void FileListComponentBase::removeListener (FileBrowserListener* const listener) throw()
{
    listeners.removeValue (listener);
}

void FileListComponentBase::sendSelectionChangeMessage()
{
    const ComponentDeletionWatcher deletionWatcher (dynamic_cast <Component*> (this));

    for (int i = listeners.size(); --i >= 0;)
    {
        ((FileBrowserListener*) listeners.getUnchecked (i))->selectionChanged();

        if (deletionWatcher.hasBeenDeleted())
            return;

        i = jmin (i, listeners.size() - 1);
    }
}

void FileListComponentBase::sendMouseClickMessage (const File& file, const MouseEvent& e)
{
    if (fileList.getDirectory().exists())
    {
        const ComponentDeletionWatcher deletionWatcher (dynamic_cast <Component*> (this));

        for (int i = listeners.size(); --i >= 0;)
        {
            ((FileBrowserListener*) listeners.getUnchecked (i))->fileClicked (file, e);

            if (deletionWatcher.hasBeenDeleted())
                return;

            i = jmin (i, listeners.size() - 1);
        }
    }
}

void FileListComponentBase::sendDoubleClickMessage (const File& file)
{
    if (fileList.getDirectory().exists())
    {
        const ComponentDeletionWatcher deletionWatcher (dynamic_cast <Component*> (this));

        for (int i = listeners.size(); --i >= 0;)
        {
            ((FileBrowserListener*) listeners.getUnchecked (i))->fileDoubleClicked (file);

            if (deletionWatcher.hasBeenDeleted())
                return;

            i = jmin (i, listeners.size() - 1);
        }
    }
}

//==============================================================================
FileListComponent::FileListComponent (DirectoryContentsList& listToShow)
    : FileListComponentBase (listToShow),
      ListBox (String::empty, 0)
{
    setModel (this);
    fileList.addChangeListener (this);
}

FileListComponent::~FileListComponent()
{
    fileList.removeChangeListener (this);
    deleteAllChildren();
}

const File FileListComponent::getSelectedFile() const
{
    return fileList.getFile (getSelectedRow());
}

void FileListComponent::scrollToTop()
{
    getVerticalScrollBar()->setCurrentRangeStart (0);
}

//==============================================================================
void FileListComponent::changeListenerCallback (void*)
{
    updateContent();
}

//==============================================================================
class FileListItemComponent  : public Component,
                               public TimeSliceClient,
                               public AsyncUpdater
{
public:
    JUCE_CALLTYPE FileListItemComponent (FileListComponent& owner_,
                                         TimeSliceThread& thread_) throw()
        : owner (owner_),
          thread (thread_),
          icon (0),
          defaultFileIcon (0),
          defaultFolderIcon (0)
    {
    }

    JUCE_CALLTYPE ~FileListItemComponent() throw()
    {
        thread.removeTimeSliceClient (this);

        clearIcon();
        ImageCache::release (defaultFileIcon);
        ImageCache::release (defaultFolderIcon);
    }

    void paint (Graphics& g)
    {
        if (highlighted)
            g.fillAll (owner.findColour (FileListComponent::highlightColourId));

        g.setColour (owner.findColour (FileListComponent::textColourId));
        g.setFont (getHeight() * 0.7f);

        const int x = 32;

        Image* im = icon;

        if (im == 0)
        {
            if (defaultFileIcon == 0)
                lookAndFeelChanged();

            im = isDirectory ? defaultFolderIcon : defaultFileIcon;
        }

        if (im != 0)
        {
            g.drawImageWithin (im, 2, 2, x - 4, getHeight() - 4,
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                               false);
        }

        if (getWidth() > 450 && ! isDirectory)
        {
            const int sizeX = proportionOfWidth (0.7f);
            const int dateX = proportionOfWidth (0.8f);

            g.drawFittedText (file.getFileName(),
                              x, 0, sizeX - x, getHeight(),
                              Justification::centredLeft, 1);

            g.setFont (getHeight() * 0.5f);
            g.setColour (Colours::darkgrey);

            if (! isDirectory)
            {
                g.drawFittedText (fileSize,
                                  sizeX, 0, dateX - sizeX - 8, getHeight(),
                                  Justification::centredRight, 1);

                g.drawFittedText (modTime,
                                  dateX, 0, getWidth() - 8 - dateX, getHeight(),
                                  Justification::centredRight, 1);
            }
        }
        else
        {
            g.drawFittedText (file.getFileName(),
                              x, 0, getWidth() - x, getHeight(),
                              Justification::centredLeft, 1);

        }
    }

    void resized()
    {
    }

    void lookAndFeelChanged()
    {
        ImageCache::release (defaultFileIcon);
        ImageCache::release (defaultFolderIcon);

        defaultFileIcon = getLookAndFeel().getDefaultDocumentFileImage();
        defaultFolderIcon = getLookAndFeel().getDefaultFolderImage();
    }

    void mouseDown (const MouseEvent& e)
    {
        owner.selectRowsBasedOnModifierKeys (index, e.mods);
        owner.sendMouseClickMessage (file, e);
    }

    void mouseDoubleClick (const MouseEvent&)
    {
        owner.sendDoubleClickMessage (file);
    }

    void update (const File& root,
                 const DirectoryContentsList::FileInfo* fileInfo,
                 const int index_,
                 const bool highlighted_)
    {
        thread.removeTimeSliceClient (this);

        if (highlighted_ != highlighted
             || index_ != index)
        {
            index = index_;
            highlighted = highlighted_;
            repaint();
        }

        File newFile;
        String newFileSize;
        String newModTime;

        if (fileInfo != 0)
        {
            newFile = root.getChildFile (fileInfo->filename);
            newFileSize = File::descriptionOfSizeInBytes (fileInfo->fileSize);
            newModTime = fileInfo->modificationTime.formatted (T("%d %b '%y %H:%M"));
        }

        if (newFile != file
             || fileSize != newFileSize
             || modTime != newModTime)
        {
            file = newFile;
            fileSize = newFileSize;
            modTime = newModTime;

            isDirectory = fileInfo != 0 && fileInfo->isDirectory;
            repaint();

            clearIcon();
        }

        if (file != File::nonexistent
            && icon == 0 && ! isDirectory)
        {
            updateIcon (true);

            if (icon == 0)
                thread.addTimeSliceClient (this);
        }
    }

    bool useTimeSlice()
    {
        updateIcon (false);
        return false;
    }

    void handleAsyncUpdate()
    {
        repaint();
    }

private:
    FileListComponent& owner;
    TimeSliceThread& thread;
    bool highlighted;
    int index;
    File file;
    String fileSize;
    String modTime;
    Image* icon;
    bool isDirectory;
    Image* defaultFileIcon;
    Image* defaultFolderIcon;

    void JUCE_CALLTYPE clearIcon() throw()
    {
        ImageCache::release (icon);
        icon = 0;
    }

    void JUCE_CALLTYPE updateIcon (const bool onlyUpdateIfCached)
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

int FileListComponent::getNumRows()
{
    return fileList.getNumFiles();
}

void FileListComponent::paintListBoxItem (int, Graphics&, int, int, bool)
{
}

Component* FileListComponent::refreshComponentForRow (int row, bool isSelected, Component* existingComponentToUpdate)
{
    FileListItemComponent* comp = dynamic_cast <FileListItemComponent*> (existingComponentToUpdate);

    if (comp == 0)
    {
        delete existingComponentToUpdate;
        existingComponentToUpdate = comp = new FileListItemComponent (*this, fileList.getTimeSliceThread());
    }

    DirectoryContentsList::FileInfo fileInfo;

    if (fileList.getFileInfo (row, fileInfo))
        comp->update (fileList.getDirectory(), &fileInfo, row, isSelected);
    else
        comp->update (fileList.getDirectory(), 0, row, isSelected);

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

//==============================================================================
class FileListTreeItem   : public TreeViewItem,
                           public TimeSliceClient,
                           public AsyncUpdater,
                           public ChangeListener
{
public:
    JUCE_CALLTYPE FileListTreeItem (FileTreeComponent& owner_,
                                    DirectoryContentsList* const parentContentsList_,
                                    const int indexInContentsList_,
                                    const File& file_,
                                    TimeSliceThread& thread_) throw()
        : owner (owner_),
          parentContentsList (parentContentsList_),
          indexInContentsList (indexInContentsList_),
          subContentsList (0),
          canDeleteSubContentsList (false),
          file (file_),
          thread (thread_),
          icon (0),
          defaultFileIcon (0),
          defaultFolderIcon (0)
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

        if (file != File::nonexistent && ! isDirectory)
        {
            updateIcon (true);

            if (icon == 0)
                thread.addTimeSliceClient (this);
        }
    }

    JUCE_CALLTYPE ~FileListTreeItem() throw()
    {
        thread.removeTimeSliceClient (this);

        clearIcon();
        ImageCache::release (defaultFileIcon);
        ImageCache::release (defaultFolderIcon);

        delete subContentsList;
    }

    bool mightContainSubItems()             { return isDirectory; }
    const String getUniqueName() const      { return file.getFullPathName(); }

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

    void JUCE_CALLTYPE setSubContentsList (DirectoryContentsList* newList) throw()
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
        if (isSelected())
            g.fillAll (owner.findColour (FileTreeComponent::highlightColourId));

        g.setColour (owner.findColour (FileTreeComponent::textColourId));
        g.setFont (height * 0.7f);

        const int x = 32;

        Image* im = icon;

        if (im == 0)
        {
            if (defaultFileIcon == 0)
                reloadIcons();

            im = isDirectory ? defaultFolderIcon : defaultFileIcon;
        }

        if (im != 0)
        {
            g.drawImageWithin (im, 2, 2, x - 4, height - 4,
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                               false);
        }

        if (width > 450 && ! isDirectory)
        {
            const int sizeX = roundFloatToInt (width * 0.7f);
            const int dateX = roundFloatToInt (width * 0.8f);

            g.drawFittedText (file.getFileName(),
                              x, 0, sizeX - x, height,
                              Justification::centredLeft, 1);

            g.setFont (height * 0.5f);
            g.setColour (Colours::darkgrey);

            if (! isDirectory)
            {
                g.drawFittedText (fileSize,
                                  sizeX, 0, dateX - sizeX - 8, height,
                                  Justification::centredRight, 1);

                g.drawFittedText (modTime,
                                  dateX, 0, width - 8 - dateX, height,
                                  Justification::centredRight, 1);
            }
        }
        else
        {
            g.drawFittedText (file.getFileName(),
                              x, 0, width - x, height,
                              Justification::centredLeft, 1);

        }
    }

    void itemClicked (const MouseEvent& e)
    {
        owner.sendMouseClickMessage (file, e);
    }

    void itemDoubleClicked (const MouseEvent&)
    {
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

    const File& getFile() const throw()                 { return file; }

private:
    FileTreeComponent& owner;
    DirectoryContentsList* parentContentsList;
    int indexInContentsList;
    DirectoryContentsList* subContentsList;
    bool isDirectory, canDeleteSubContentsList;
    TimeSliceThread& thread;
    const File file;
    String fileSize;
    String modTime;
    Image* icon;
    Image* defaultFileIcon;
    Image* defaultFolderIcon;

    void clearIcon() throw()
    {
        ImageCache::release (icon);
        icon = 0;
    }

    void reloadIcons() throw()
    {
        ImageCache::release (defaultFileIcon);
        ImageCache::release (defaultFolderIcon);

        defaultFileIcon = owner.getLookAndFeel().getDefaultDocumentFileImage();
        defaultFolderIcon = owner.getLookAndFeel().getDefaultFolderImage();
    }

    void updateIcon (const bool onlyUpdateIfCached)
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
    : FileListComponentBase (listToShow)
{
    FileListTreeItem* const root 
        = new FileListTreeItem (*this, 0, 0, File::nonexistent, listToShow.getTimeSliceThread());

    root->setSubContentsList (&listToShow);
    setRootItemVisible (false);
    setRootItem (root);
}

FileTreeComponent::~FileTreeComponent()
{
    setRootItem (0);
}

//==============================================================================
const File FileTreeComponent::getSelectedFile() const
{
    FileListTreeItem* item = dynamic_cast <FileListTreeItem*> (getSelectedItem (0));

    if (item != 0)
        return item->getFile();

    return File::nonexistent;
}

void FileTreeComponent::scrollToTop()
{
    getViewport()->getVerticalScrollBar()->setCurrentRangeStart (0);
}

END_JUCE_NAMESPACE
