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
FileListComponent::FileListComponent (DirectoryContentsList& listToShow)
    : ListBox (String::empty, 0),
      fileList (listToShow),
      listeners (2)
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

//==============================================================================
FileBrowserListener::~FileBrowserListener()
{
}

void FileListComponent::addListener (FileBrowserListener* const listener) throw()
{
    jassert (listener != 0);
    if (listener != 0)
        listeners.add (listener);
}

void FileListComponent::removeListener (FileBrowserListener* const listener) throw()
{
    listeners.removeValue (listener);
}

//==============================================================================
void FileListComponent::resized()
{
    ListBox::resized();
}

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
    FileListItemComponent (FileListComponent& owner_,
                           TimeSliceThread& thread_)
        : owner (owner_),
          thread (thread_),
          icon (0),
          defaultFileIcon (0),
          defaultFolderIcon (0)
    {
    }

    ~FileListItemComponent()
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

    void clearIcon()
    {
        ImageCache::release (icon);
        icon = 0;
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
    ComponentDeletionWatcher deletionWatcher (this);

    for (int i = listeners.size(); --i >= 0;)
    {
        ((FileBrowserListener*) listeners.getUnchecked (i))->selectionChanged();

        if (deletionWatcher.hasBeenDeleted())
            return;

        i = jmin (i, listeners.size() - 1);
    }
}

void FileListComponent::deleteKeyPressed (int /*currentSelectedRow*/)
{
}

void FileListComponent::returnKeyPressed (int currentSelectedRow)
{
    sendDoubleClickMessage (fileList.getFile (currentSelectedRow));
}

void FileListComponent::sendMouseClickMessage (const File& file, const MouseEvent& e)
{
    if (file.exists())
    {
        ComponentDeletionWatcher deletionWatcher (this);

        for (int i = listeners.size(); --i >= 0;)
        {
            ((FileBrowserListener*) listeners.getUnchecked (i))->fileClicked (file, e);

            if (deletionWatcher.hasBeenDeleted())
                return;

            i = jmin (i, listeners.size() - 1);
        }
    }
}

void FileListComponent::sendDoubleClickMessage (const File& file)
{
    if (file.exists())
    {
        ComponentDeletionWatcher deletionWatcher (this);

        for (int i = listeners.size(); --i >= 0;)
        {
            ((FileBrowserListener*) listeners.getUnchecked (i))->fileDoubleClicked (file);

            if (deletionWatcher.hasBeenDeleted())
                return;

            i = jmin (i, listeners.size() - 1);
        }
    }
}


END_JUCE_NAMESPACE
