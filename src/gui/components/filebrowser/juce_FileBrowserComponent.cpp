/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
FileBrowserComponent::FileBrowserComponent (int flags_,
                                            const File& initialFileOrDirectory,
                                            const FileFilter* fileFilter_,
                                            FilePreviewComponent* previewComp_)
   : FileFilter (String::empty),
     fileFilter (fileFilter_),
     flags (flags_),
     previewComp (previewComp_),
     currentPathBox ("path"),
     fileLabel ("f", TRANS ("file:")),
     thread ("Juce FileBrowser")
{
    // You need to specify one or other of the open/save flags..
    jassert ((flags & (saveMode | openMode)) != 0);
    jassert ((flags & (saveMode | openMode)) != (saveMode | openMode));

    // You need to specify at least one of these flags..
    jassert ((flags & (canSelectFiles | canSelectDirectories)) != 0);

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
        chosenFiles.add (initialFileOrDirectory);
        currentRoot = initialFileOrDirectory.getParentDirectory();
        filename = initialFileOrDirectory.getFileName();
    }

    fileList = new DirectoryContentsList (this, thread);

    if ((flags & useTreeView) != 0)
    {
        FileTreeComponent* const tree = new FileTreeComponent (*fileList);
        fileListComponent = tree;

        if ((flags & canSelectMultipleItems) != 0)
            tree->setMultiSelectEnabled (true);

        addAndMakeVisible (tree);
    }
    else
    {
        FileListComponent* const list = new FileListComponent (*fileList);
        fileListComponent = list;
        list->setOutlineThickness (1);

        if ((flags & canSelectMultipleItems) != 0)
            list->setMultipleSelectionEnabled (true);

        addAndMakeVisible (list);
    }

    fileListComponent->addListener (this);

    addAndMakeVisible (&currentPathBox);
    currentPathBox.setEditableText (true);

    StringArray rootNames, rootPaths;
    const BigInteger separators (getRoots (rootNames, rootPaths));

    for (int i = 0; i < rootNames.size(); ++i)
    {
        if (separators [i])
            currentPathBox.addSeparator();

        currentPathBox.addItem (rootNames[i], i + 1);
    }

    currentPathBox.addSeparator();
    currentPathBox.addListener (this);

    addAndMakeVisible (&filenameBox);
    filenameBox.setMultiLine (false);
    filenameBox.setSelectAllWhenFocused (true);
    filenameBox.setText (filename, false);
    filenameBox.addListener (this);
    filenameBox.setReadOnly ((flags & (filenameBoxIsReadOnly | canSelectMultipleItems)) != 0);

    addAndMakeVisible (&fileLabel);
    fileLabel.attachToComponent (&filenameBox, true);

    addAndMakeVisible (goUpButton = getLookAndFeel().createFileBrowserGoUpButton());
    goUpButton->addListener (this);
    goUpButton->setTooltip (TRANS ("go up to parent directory"));

    if (previewComp != 0)
        addAndMakeVisible (previewComp);

    setRoot (currentRoot);

    thread.startThread (4);
}

FileBrowserComponent::~FileBrowserComponent()
{
    fileListComponent = 0;
    fileList = 0;
    thread.stopThread (10000);
}

//==============================================================================
void FileBrowserComponent::addListener (FileBrowserListener* const newListener)
{
    listeners.add (newListener);
}

void FileBrowserComponent::removeListener (FileBrowserListener* const listener)
{
    listeners.remove (listener);
}

//==============================================================================
bool FileBrowserComponent::isSaveMode() const throw()
{
    return (flags & saveMode) != 0;
}

int FileBrowserComponent::getNumSelectedFiles() const throw()
{
    if (chosenFiles.size() == 0 && currentFileIsValid())
        return 1;

    return chosenFiles.size();
}

const File FileBrowserComponent::getSelectedFile (int index) const throw()
{
    if ((flags & canSelectDirectories) != 0 && filenameBox.getText().isEmpty())
        return currentRoot;

    if (! filenameBox.isReadOnly())
        return currentRoot.getChildFile (filenameBox.getText());

    return chosenFiles[index];
}

bool FileBrowserComponent::currentFileIsValid() const
{
    if (isSaveMode())
        return ! getSelectedFile (0).isDirectory();
    else
        return getSelectedFile (0).exists();
}

const File FileBrowserComponent::getHighlightedFile() const throw()
{
    return fileListComponent->getSelectedFile (0);
}

void FileBrowserComponent::deselectAllFiles()
{
    fileListComponent->deselectAllFiles();
}

//==============================================================================
bool FileBrowserComponent::isFileSuitable (const File& file) const
{
    return (flags & canSelectFiles) != 0 && (fileFilter == 0 || fileFilter->isFileSuitable (file));
}

bool FileBrowserComponent::isDirectorySuitable (const File&) const
{
    return true;
}

bool FileBrowserComponent::isFileOrDirSuitable (const File& f) const
{
    if (f.isDirectory())
        return (flags & canSelectDirectories) != 0 && (fileFilter == 0 || fileFilter->isDirectorySuitable (f));

    return (flags & canSelectFiles) != 0 && f.exists()
            && (fileFilter == 0 || fileFilter->isFileSuitable (f));
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

        String path (newRootDirectory.getFullPathName());

        if (path.isEmpty())
            path = File::separatorString;

        StringArray rootNames, rootPaths;
        getRoots (rootNames, rootPaths);

        if (! rootPaths.contains (path, true))
        {
            bool alreadyListed = false;

            for (int i = currentPathBox.getNumItems(); --i >= 0;)
            {
                if (currentPathBox.getItemText (i).equalsIgnoreCase (path))
                {
                    alreadyListed = true;
                    break;
                }
            }

            if (! alreadyListed)
                currentPathBox.addItem (path, currentPathBox.getNumItems() + 2);
        }
    }

    currentRoot = newRootDirectory;
    fileList->setDirectory (currentRoot, true, true);

    String currentRootName (currentRoot.getFullPathName());
    if (currentRootName.isEmpty())
        currentRootName = File::separatorString;

    currentPathBox.setText (currentRootName, true);

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
    return isSaveMode() ? TRANS("Save") : TRANS("Open");
}

FilePreviewComponent* FileBrowserComponent::getPreviewComponent() const throw()
{
    return previewComp;
}

//==============================================================================
void FileBrowserComponent::resized()
{
    getLookAndFeel()
        .layoutFileBrowserComponent (*this, fileListComponent, previewComp,
                                     &currentPathBox, &filenameBox, goUpButton);
}

//==============================================================================
void FileBrowserComponent::sendListenerChangeMessage()
{
    Component::BailOutChecker checker (this);

    if (previewComp != 0)
        previewComp->selectedFileChanged (getSelectedFile (0));

    // You shouldn't delete the browser when the file gets changed!
    jassert (! checker.shouldBailOut());

    listeners.callChecked (checker, &FileBrowserListener::selectionChanged);
}

void FileBrowserComponent::selectionChanged()
{
    StringArray newFilenames;
    bool resetChosenFiles = true;

    for (int i = 0; i < fileListComponent->getNumSelectedFiles(); ++i)
    {
        const File f (fileListComponent->getSelectedFile (i));

        if (isFileOrDirSuitable (f))
        {
            if (resetChosenFiles)
            {
                chosenFiles.clear();
                resetChosenFiles = false;
            }

            chosenFiles.add (f);
            newFilenames.add (f.getRelativePathFrom (getRoot()));
        }
    }

    if (newFilenames.size() > 0)
        filenameBox.setText (newFilenames.joinIntoString (", "), false);

    sendListenerChangeMessage();
}

void FileBrowserComponent::fileClicked (const File& f, const MouseEvent& e)
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, &FileBrowserListener::fileClicked, f, e);
}

void FileBrowserComponent::fileDoubleClicked (const File& f)
{
    if (f.isDirectory())
    {
        setRoot (f);

        if ((flags & canSelectDirectories) != 0)
            filenameBox.setText (String::empty);
    }
    else
    {
        Component::BailOutChecker checker (this);
        listeners.callChecked (checker, &FileBrowserListener::fileDoubleClicked, f);
    }
}

bool FileBrowserComponent::keyPressed (const KeyPress& key)
{
    (void) key;

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
    if (filenameBox.getText().containsChar (File::separator))
    {
        const File f (currentRoot.getChildFile (filenameBox.getText()));

        if (f.isDirectory())
        {
            setRoot (f);
            chosenFiles.clear();
            filenameBox.setText (String::empty);
        }
        else
        {
            setRoot (f.getParentDirectory());
            chosenFiles.clear();
            chosenFiles.add (f);
            filenameBox.setText (f.getFileName());
        }
    }
    else
    {
        fileDoubleClicked (getSelectedFile (0));
    }
}

void FileBrowserComponent::textEditorEscapeKeyPressed (TextEditor&)
{
}

void FileBrowserComponent::textEditorFocusLost (TextEditor&)
{
    if (! isSaveMode())
        selectionChanged();
}

//==============================================================================
void FileBrowserComponent::buttonClicked (Button*)
{
    goUp();
}


void FileBrowserComponent::comboBoxChanged (ComboBox*)
{
    const String newText (currentPathBox.getText().trim().unquoted());

    if (newText.isNotEmpty())
    {
        const int index = currentPathBox.getSelectedId() - 1;

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

const BigInteger FileBrowserComponent::getRoots (StringArray& rootNames, StringArray& rootPaths)
{
    BigInteger separators;

#if JUCE_WINDOWS
    Array<File> roots;
    File::findFileSystemRoots (roots);
    rootPaths.clear();

    for (int i = 0; i < roots.size(); ++i)
    {
        const File& drive = roots.getReference(i);

        String name (drive.getFullPathName());
        rootPaths.add (name);

        if (drive.isOnHardDisk())
        {
            String volume (drive.getVolumeLabel());

            if (volume.isEmpty())
                volume = TRANS("Hard Drive");

            name << " [" << volume << ']';
        }
        else if (drive.isOnCDRomDrive())
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

    Array <File> volumes;
    File vol ("/Volumes");
    vol.findChildFiles (volumes, File::findDirectories, false);

    for (int i = 0; i < volumes.size(); ++i)
    {
        const File& volume = volumes.getReference(i);

        if (volume.isDirectory() && ! volume.getFileName().startsWithChar ('.'))
        {
            rootPaths.add (volume.getFullPathName());
            rootNames.add (volume.getFileName());
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
