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

#include "juce_FileBrowserComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../graphics/drawables/juce_DrawablePath.h"
#include "../../../text/juce_LocalisedStrings.h"
#include "../../../core/juce_SystemStats.h"
#include "juce_FileListComponent.h"
#include "juce_FileTreeComponent.h"


//==============================================================================
class DirectoriesOnlyFilter    : public FileFilter
{
public:
    DirectoriesOnlyFilter() : FileFilter (String::empty) {}

    bool isFileSuitable (const File&) const         { return false; }
    bool isDirectorySuitable (const File&) const    { return true; }
};


//==============================================================================
FileBrowserComponent::FileBrowserComponent (FileChooserMode mode_,
                                            const File& initialFileOrDirectory,
                                            const FileFilter* fileFilter,
                                            FilePreviewComponent* previewComp_,
                                            const bool useTreeView,
                                            const bool filenameTextBoxIsReadOnly)
   : directoriesOnlyFilter (0),
     mode (mode_),
     listeners (2),
     previewComp (previewComp_),
     thread ("Juce FileBrowser")
{
    String filename;

    if (initialFileOrDirectory == File::nonexistent)
    {
        currentRoot = File::getCurrentWorkingDirectory();
    }
    else if (initialFileOrDirectory.isDirectory())
    {
        currentRoot = initialFileOrDirectory;
    }
    else
    {
        currentRoot = initialFileOrDirectory.getParentDirectory();
        filename = initialFileOrDirectory.getFileName();
    }

    if (mode_ == chooseDirectoryMode)
        fileFilter = directoriesOnlyFilter = new DirectoriesOnlyFilter();

    fileList = new DirectoryContentsList (fileFilter, thread);

    if (useTreeView)
    {
        FileTreeComponent* const tree = new FileTreeComponent (*fileList);
        addAndMakeVisible (tree);
        fileListComponent = tree;
    }
    else
    {
        FileListComponent* const list = new FileListComponent (*fileList);
        list->setOutlineThickness (1);
        addAndMakeVisible (list);
        fileListComponent = list;
    }

    fileListComponent->addListener (this);

    addAndMakeVisible (currentPathBox = new ComboBox ("path"));
    currentPathBox->setEditableText (true);

    StringArray rootNames, rootPaths;
    const BitArray separators (getRoots (rootNames, rootPaths));

    for (int i = 0; i < rootNames.size(); ++i)
    {
        if (separators [i])
            currentPathBox->addSeparator();

        currentPathBox->addItem (rootNames[i], i + 1);
    }

    currentPathBox->addSeparator();
    currentPathBox->addListener (this);

    addAndMakeVisible (filenameBox = new TextEditor());
    filenameBox->setMultiLine (false);
    filenameBox->setSelectAllWhenFocused (true);
    filenameBox->setText (filename, false);
    filenameBox->addListener (this);
    filenameBox->setReadOnly (filenameTextBoxIsReadOnly);

    Label* label = new Label ("f", (mode == chooseDirectoryMode) ? TRANS("folder:")
                                                                 : TRANS("file:"));
    addAndMakeVisible (label);
    label->attachToComponent (filenameBox, true);

    addAndMakeVisible (goUpButton = getLookAndFeel().createFileBrowserGoUpButton());

    goUpButton->addButtonListener (this);
    goUpButton->setTooltip (TRANS ("go up to parent directory"));

    if (previewComp != 0)
        addAndMakeVisible (previewComp);

    setRoot (currentRoot);

    thread.startThread (4);
}

FileBrowserComponent::~FileBrowserComponent()
{
    if (previewComp != 0)
        removeChildComponent (previewComp);

    deleteAllChildren();

    deleteAndZero (fileList);
    delete directoriesOnlyFilter;

    thread.stopThread (10000);
}

//==============================================================================
void FileBrowserComponent::addListener (FileBrowserListener* const newListener) throw()
{
    jassert (newListener != 0)

    if (newListener != 0)
        listeners.add (newListener);
}

void FileBrowserComponent::removeListener (FileBrowserListener* const listener) throw()
{
    listeners.removeValue (listener);
}

//==============================================================================
const File FileBrowserComponent::getCurrentFile() const throw()
{
    return currentRoot.getChildFile (filenameBox->getText());
}

bool FileBrowserComponent::currentFileIsValid() const
{
    if (mode == saveFileMode)
        return ! getCurrentFile().isDirectory();
    else if (mode == loadFileMode)
        return getCurrentFile().existsAsFile();
    else if (mode == chooseDirectoryMode)
        return getCurrentFile().isDirectory();

    jassertfalse
    return false;
}

//==============================================================================
const File FileBrowserComponent::getRoot() const
{
    return currentRoot;
}

void FileBrowserComponent::setRoot (const File& newRootDirectory)
{
    if (currentRoot != newRootDirectory)
    {
        fileListComponent->scrollToTop();

        if (mode == chooseDirectoryMode)
            filenameBox->setText (String::empty, false);

        String path (newRootDirectory.getFullPathName());

        if (path.isEmpty())
            path += File::separator;

        StringArray rootNames, rootPaths;
        getRoots (rootNames, rootPaths);

        if (! rootPaths.contains (path, true))
        {
            bool alreadyListed = false;

            for (int i = currentPathBox->getNumItems(); --i >= 0;)
            {
                if (currentPathBox->getItemText (i).equalsIgnoreCase (path))
                {
                    alreadyListed = true;
                    break;
                }
            }

            if (! alreadyListed)
                currentPathBox->addItem (path, currentPathBox->getNumItems() + 2);
        }
    }

    currentRoot = newRootDirectory;
    fileList->setDirectory (currentRoot, true, true);

    String currentRootName (currentRoot.getFullPathName());
    if (currentRootName.isEmpty())
        currentRootName += File::separator;

    currentPathBox->setText (currentRootName, true);

    goUpButton->setEnabled (currentRoot.getParentDirectory().isDirectory()
                             && currentRoot.getParentDirectory() != currentRoot);
}

void FileBrowserComponent::goUp()
{
    setRoot (getRoot().getParentDirectory());
}

void FileBrowserComponent::refresh()
{
    fileList->refresh();
}

const String FileBrowserComponent::getActionVerb() const
{
    return (mode == chooseDirectoryMode) ? TRANS("Choose")
                                         : ((mode == saveFileMode) ? TRANS("Save") : TRANS("Open"));
}

FilePreviewComponent* FileBrowserComponent::getPreviewComponent() const throw()
{
    return previewComp;
}

//==============================================================================
void FileBrowserComponent::resized()
{
    getLookAndFeel()
        .layoutFileBrowserComponent (*this, fileListComponent,
                                     previewComp, currentPathBox,
                                     filenameBox, goUpButton);
}

//==============================================================================
void FileBrowserComponent::sendListenerChangeMessage()
{
    ComponentDeletionWatcher deletionWatcher (this);

    if (previewComp != 0)
        previewComp->selectedFileChanged (getCurrentFile());

    jassert (! deletionWatcher.hasBeenDeleted());

    for (int i = listeners.size(); --i >= 0;)
    {
        ((FileBrowserListener*) listeners.getUnchecked (i))->selectionChanged();

        if (deletionWatcher.hasBeenDeleted())
            return;

        i = jmin (i, listeners.size() - 1);
    }
}

void FileBrowserComponent::selectionChanged()
{
    const File selected (fileListComponent->getSelectedFile());

    if ((mode == chooseDirectoryMode && selected.isDirectory())
         || selected.existsAsFile())
    {
        filenameBox->setText (selected.getRelativePathFrom (getRoot()), false);
    }

    sendListenerChangeMessage();
}

void FileBrowserComponent::fileClicked (const File& f, const MouseEvent& e)
{
    ComponentDeletionWatcher deletionWatcher (this);

    for (int i = listeners.size(); --i >= 0;)
    {
        ((FileBrowserListener*) listeners.getUnchecked (i))->fileClicked (f, e);

        if (deletionWatcher.hasBeenDeleted())
            return;

        i = jmin (i, listeners.size() - 1);
    }
}

void FileBrowserComponent::fileDoubleClicked (const File& f)
{
    if (f.isDirectory())
    {
        setRoot (f);
    }
    else
    {
        ComponentDeletionWatcher deletionWatcher (this);

        for (int i = listeners.size(); --i >= 0;)
        {
            ((FileBrowserListener*) listeners.getUnchecked (i))->fileDoubleClicked (f);

            if (deletionWatcher.hasBeenDeleted())
                return;

            i = jmin (i, listeners.size() - 1);
        }
    }
}

bool FileBrowserComponent::keyPressed (const KeyPress& key)
{
#if JUCE_LINUX || JUCE_WINDOWS
    if (key.getModifiers().isCommandDown()
         && (key.getKeyCode() == 'H' || key.getKeyCode() == 'h'))
    {
        fileList->setIgnoresHiddenFiles (! fileList->ignoresHiddenFiles());
        fileList->refresh();
        return true;
    }
#endif

    return false;
}

//==============================================================================
void FileBrowserComponent::textEditorTextChanged (TextEditor&)
{
    sendListenerChangeMessage();
}

void FileBrowserComponent::textEditorReturnKeyPressed (TextEditor&)
{
    if (filenameBox->getText().containsChar (File::separator))
    {
        const File f (currentRoot.getChildFile (filenameBox->getText()));

        if (f.isDirectory())
        {
            setRoot (f);
            filenameBox->setText (String::empty);
        }
        else
        {
            setRoot (f.getParentDirectory());
            filenameBox->setText (f.getFileName());
        }
    }
    else
    {
        fileDoubleClicked (getCurrentFile());
    }
}

void FileBrowserComponent::textEditorEscapeKeyPressed (TextEditor&)
{
}

void FileBrowserComponent::textEditorFocusLost (TextEditor&)
{
    if (mode != saveFileMode)
        selectionChanged();
}

//==============================================================================
void FileBrowserComponent::buttonClicked (Button*)
{
    goUp();
}


void FileBrowserComponent::comboBoxChanged (ComboBox*)
{
    const String newText (currentPathBox->getText().trim().unquoted());

    if (newText.isNotEmpty())
    {
        const int index = currentPathBox->getSelectedId() - 1;

        StringArray rootNames, rootPaths;
        getRoots (rootNames, rootPaths);

        if (rootPaths [index].isNotEmpty())
        {
            setRoot (File (rootPaths [index]));
        }
        else
        {
            File f (newText);

            for (;;)
            {
                if (f.isDirectory())
                {
                    setRoot (f);
                    break;
                }

                if (f.getParentDirectory() == f)
                    break;

                f = f.getParentDirectory();
            }
        }
    }
}

const BitArray FileBrowserComponent::getRoots (StringArray& rootNames, StringArray& rootPaths)
{
    BitArray separators;

#if JUCE_WINDOWS
    OwnedArray<File> roots;
    File::findFileSystemRoots (roots);
    rootPaths.clear();

    for (int i = 0; i < roots.size(); ++i)
    {
        const File* const drive = roots.getUnchecked(i);

        String name (drive->getFullPathName());
        rootPaths.add (name);

        if (drive->isOnHardDisk())
        {
            String volume (drive->getVolumeLabel());

            if (volume.isEmpty())
                volume = TRANS("Hard Drive");

            name << " [" << drive->getVolumeLabel() << ']';
        }
        else if (drive->isOnCDRomDrive())
        {
            name << TRANS(" [CD/DVD drive]");
        }

        rootNames.add (name);
    }

    separators.setBit (rootPaths.size());

    rootPaths.add (File::getSpecialLocation (File::userDocumentsDirectory).getFullPathName());
    rootNames.add ("Documents");
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add ("Desktop");
#endif

#if JUCE_MAC
    rootPaths.add (File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    rootNames.add ("Home folder");
    rootPaths.add (File::getSpecialLocation (File::userDocumentsDirectory).getFullPathName());
    rootNames.add ("Documents");
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add ("Desktop");

    separators.setBit (rootPaths.size());

    OwnedArray <File> volumes;
    File vol ("/Volumes");
    vol.findChildFiles (volumes, File::findDirectories, false);

    for (int i = 0; i < volumes.size(); ++i)
    {
        const File* const volume = volumes.getUnchecked(i);

        if (volume->isDirectory() && ! volume->getFileName().startsWithChar (T('.')))
        {
            rootPaths.add (volume->getFullPathName());
            rootNames.add (volume->getFileName());
        }
    }
#endif

#if JUCE_LINUX
    rootPaths.add ("/");
    rootNames.add ("/");
    rootPaths.add (File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    rootNames.add ("Home folder");
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add ("Desktop");
#endif

    return separators;
}


END_JUCE_NAMESPACE
