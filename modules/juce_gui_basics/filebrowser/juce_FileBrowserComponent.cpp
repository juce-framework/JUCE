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

FileBrowserComponent::FileBrowserComponent (int flags_,
                                            const File& initialFileOrDirectory,
                                            const FileFilter* fileFilter_,
                                            FilePreviewComponent* previewComp_)
   : FileFilter ({}),
     fileFilter (fileFilter_),
     flags (flags_),
     previewComp (previewComp_),
     currentPathBox ("path"),
     fileLabel ("f", TRANS ("file:")),
     thread ("JUCE FileBrowser"),
     wasProcessActive (true)
{
    // You need to specify one or other of the open/save flags..
    jassert ((flags & (saveMode | openMode)) != 0);
    jassert ((flags & (saveMode | openMode)) != (saveMode | openMode));

    // You need to specify at least one of these flags..
    jassert ((flags & (canSelectFiles | canSelectDirectories)) != 0);

    String filename;

    if (initialFileOrDirectory == File())
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

    fileList.reset (new DirectoryContentsList (this, thread));

    if ((flags & useTreeView) != 0)
    {
        auto tree = new FileTreeComponent (*fileList);
        fileListComponent.reset (tree);

        if ((flags & canSelectMultipleItems) != 0)
            tree->setMultiSelectEnabled (true);

        addAndMakeVisible (tree);
    }
    else
    {
        auto list = new FileListComponent (*fileList);
        fileListComponent.reset (list);
        list->setOutlineThickness (1);

        if ((flags & canSelectMultipleItems) != 0)
            list->setMultipleSelectionEnabled (true);

        addAndMakeVisible (list);
    }

    fileListComponent->addListener (this);

    addAndMakeVisible (currentPathBox);
    currentPathBox.setEditableText (true);
    resetRecentPaths();
    currentPathBox.onChange = [this] { updateSelectedPath(); };

    addAndMakeVisible (filenameBox);
    filenameBox.setMultiLine (false);
    filenameBox.setSelectAllWhenFocused (true);
    filenameBox.setText (filename, false);
    filenameBox.onTextChange = [this] { sendListenerChangeMessage(); };
    filenameBox.onReturnKey  = [this] { changeFilename(); };
    filenameBox.onFocusLost  = [this]
    {
        if (! isSaveMode())
            selectionChanged();
    };

    filenameBox.setReadOnly ((flags & (filenameBoxIsReadOnly | canSelectMultipleItems)) != 0);

    addAndMakeVisible (fileLabel);
    fileLabel.attachToComponent (&filenameBox, true);

    goUpButton.reset (getLookAndFeel().createFileBrowserGoUpButton());
    addAndMakeVisible (goUpButton.get());
    goUpButton->onClick = [this] { goUp(); };
    goUpButton->setTooltip (TRANS ("Go up to parent directory"));

    if (previewComp != nullptr)
        addAndMakeVisible (previewComp);

    lookAndFeelChanged();

    setRoot (currentRoot);

    thread.startThread (4);

    startTimer (2000);
}

FileBrowserComponent::~FileBrowserComponent()
{
    fileListComponent.reset();
    fileList.reset();
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
bool FileBrowserComponent::isSaveMode() const noexcept
{
    return (flags & saveMode) != 0;
}

int FileBrowserComponent::getNumSelectedFiles() const noexcept
{
    if (chosenFiles.isEmpty() && currentFileIsValid())
        return 1;

    return chosenFiles.size();
}

File FileBrowserComponent::getSelectedFile (int index) const noexcept
{
    if ((flags & canSelectDirectories) != 0 && filenameBox.getText().isEmpty())
        return currentRoot;

    if (! filenameBox.isReadOnly())
        return currentRoot.getChildFile (filenameBox.getText());

    return chosenFiles[index];
}

bool FileBrowserComponent::currentFileIsValid() const
{
    auto f = getSelectedFile (0);

    if (isSaveMode())
        return (flags & canSelectDirectories) != 0 || ! f.isDirectory();

    return f.exists();
}

File FileBrowserComponent::getHighlightedFile() const noexcept
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
    return (flags & canSelectFiles) != 0
             && (fileFilter == nullptr || fileFilter->isFileSuitable (file));
}

bool FileBrowserComponent::isDirectorySuitable (const File&) const
{
    return true;
}

bool FileBrowserComponent::isFileOrDirSuitable (const File& f) const
{
    if (f.isDirectory())
        return (flags & canSelectDirectories) != 0
                 && (fileFilter == nullptr || fileFilter->isDirectorySuitable (f));

    return (flags & canSelectFiles) != 0 && f.exists()
             && (fileFilter == nullptr || fileFilter->isFileSuitable (f));
}

//==============================================================================
const File& FileBrowserComponent::getRoot() const
{
    return currentRoot;
}

void FileBrowserComponent::setRoot (const File& newRootDirectory)
{
    bool callListeners = false;

    if (currentRoot != newRootDirectory)
    {
        callListeners = true;
        fileListComponent->scrollToTop();

        String path (newRootDirectory.getFullPathName());

        if (path.isEmpty())
            path = File::getSeparatorString();

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

    if (auto* tree = dynamic_cast<FileTreeComponent*> (fileListComponent.get()))
        tree->refresh();

    auto currentRootName = currentRoot.getFullPathName();

    if (currentRootName.isEmpty())
        currentRootName = File::getSeparatorString();

    currentPathBox.setText (currentRootName, dontSendNotification);

    goUpButton->setEnabled (currentRoot.getParentDirectory().isDirectory()
                             && currentRoot.getParentDirectory() != currentRoot);

    if (callListeners)
    {
        Component::BailOutChecker checker (this);
        listeners.callChecked (checker, [&] (FileBrowserListener& l) { l.browserRootChanged (currentRoot); });
    }
}

void FileBrowserComponent::setFileName (const String& newName)
{
    filenameBox.setText (newName, true);

    fileListComponent->setSelectedFile (currentRoot.getChildFile (newName));
}

void FileBrowserComponent::resetRecentPaths()
{
    currentPathBox.clear();

    StringArray rootNames, rootPaths;
    getRoots (rootNames, rootPaths);

    for (int i = 0; i < rootNames.size(); ++i)
    {
        if (rootNames[i].isEmpty())
            currentPathBox.addSeparator();
        else
            currentPathBox.addItem (rootNames[i], i + 1);
    }

    currentPathBox.addSeparator();
}

void FileBrowserComponent::goUp()
{
    setRoot (getRoot().getParentDirectory());
}

void FileBrowserComponent::refresh()
{
    fileList->refresh();
}

void FileBrowserComponent::setFileFilter (const FileFilter* const newFileFilter)
{
    if (fileFilter != newFileFilter)
    {
        fileFilter = newFileFilter;
        refresh();
    }
}

String FileBrowserComponent::getActionVerb() const
{
    return isSaveMode() ? ((flags & canSelectDirectories) != 0 ? TRANS("Choose")
                                                               : TRANS("Save"))
                        : TRANS("Open");
}

void FileBrowserComponent::setFilenameBoxLabel (const String& name)
{
    fileLabel.setText (name, dontSendNotification);
}

FilePreviewComponent* FileBrowserComponent::getPreviewComponent() const noexcept
{
    return previewComp;
}

DirectoryContentsDisplayComponent* FileBrowserComponent::getDisplayComponent() const noexcept
{
    return fileListComponent.get();
}

//==============================================================================
void FileBrowserComponent::resized()
{
    getLookAndFeel()
        .layoutFileBrowserComponent (*this, fileListComponent.get(), previewComp,
                                     &currentPathBox, &filenameBox, goUpButton.get());
}

//==============================================================================
void FileBrowserComponent::lookAndFeelChanged()
{
    currentPathBox.setColour (ComboBox::backgroundColourId,    findColour (currentPathBoxBackgroundColourId));
    currentPathBox.setColour (ComboBox::textColourId,          findColour (currentPathBoxTextColourId));
    currentPathBox.setColour (ComboBox::arrowColourId,         findColour (currentPathBoxArrowColourId));

    filenameBox.setColour (TextEditor::backgroundColourId,     findColour (filenameBoxBackgroundColourId));
    filenameBox.setColour (TextEditor::textColourId,           findColour (filenameBoxTextColourId));
}

//==============================================================================
void FileBrowserComponent::sendListenerChangeMessage()
{
    Component::BailOutChecker checker (this);

    if (previewComp != nullptr)
        previewComp->selectedFileChanged (getSelectedFile (0));

    // You shouldn't delete the browser when the file gets changed!
    jassert (! checker.shouldBailOut());

    listeners.callChecked (checker, [] (FileBrowserListener& l) { l.selectionChanged(); });
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
    listeners.callChecked (checker, [&] (FileBrowserListener& l) { l.fileClicked (f, e); });
}

void FileBrowserComponent::fileDoubleClicked (const File& f)
{
    if (f.isDirectory())
    {
        setRoot (f);

        if ((flags & canSelectDirectories) != 0 && (flags & doNotClearFileNameOnRootChange) == 0)
            filenameBox.setText ({});
    }
    else
    {
        Component::BailOutChecker checker (this);
        listeners.callChecked (checker, [&] (FileBrowserListener& l) { l.fileDoubleClicked (f); });
    }
}

void FileBrowserComponent::browserRootChanged (const File&) {}

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

    ignoreUnused (key);
    return false;
}

//==============================================================================
void FileBrowserComponent::changeFilename()
{
    if (filenameBox.getText().containsChar (File::getSeparatorChar()))
    {
        auto f = currentRoot.getChildFile (filenameBox.getText());

        if (f.isDirectory())
        {
            setRoot (f);
            chosenFiles.clear();

            if ((flags & doNotClearFileNameOnRootChange) == 0)
                filenameBox.setText ({});
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

//==============================================================================
void FileBrowserComponent::updateSelectedPath()
{
    auto newText = currentPathBox.getText().trim().unquoted();

    if (newText.isNotEmpty())
    {
        auto index = currentPathBox.getSelectedId() - 1;

        StringArray rootNames, rootPaths;
        getRoots (rootNames, rootPaths);

        if (rootPaths[index].isNotEmpty())
        {
            setRoot (File (rootPaths[index]));
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

void FileBrowserComponent::getDefaultRoots (StringArray& rootNames, StringArray& rootPaths)
{
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
            name << " [" << TRANS("CD/DVD drive") << ']';
        }

        rootNames.add (name);
    }

    rootPaths.add ({});
    rootNames.add ({});

    rootPaths.add (File::getSpecialLocation (File::userDocumentsDirectory).getFullPathName());
    rootNames.add (TRANS("Documents"));
    rootPaths.add (File::getSpecialLocation (File::userMusicDirectory).getFullPathName());
    rootNames.add (TRANS("Music"));
    rootPaths.add (File::getSpecialLocation (File::userPicturesDirectory).getFullPathName());
    rootNames.add (TRANS("Pictures"));
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add (TRANS("Desktop"));

   #elif JUCE_MAC
    rootPaths.add (File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    rootNames.add (TRANS("Home folder"));
    rootPaths.add (File::getSpecialLocation (File::userDocumentsDirectory).getFullPathName());
    rootNames.add (TRANS("Documents"));
    rootPaths.add (File::getSpecialLocation (File::userMusicDirectory).getFullPathName());
    rootNames.add (TRANS("Music"));
    rootPaths.add (File::getSpecialLocation (File::userPicturesDirectory).getFullPathName());
    rootNames.add (TRANS("Pictures"));
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add (TRANS("Desktop"));

    rootPaths.add ({});
    rootNames.add ({});

    for (auto& volume : File ("/Volumes").findChildFiles (File::findDirectories, false))
    {
        if (volume.isDirectory() && ! volume.getFileName().startsWithChar ('.'))
        {
            rootPaths.add (volume.getFullPathName());
            rootNames.add (volume.getFileName());
        }
    }

   #else
    rootPaths.add ("/");
    rootNames.add ("/");
    rootPaths.add (File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    rootNames.add (TRANS("Home folder"));
    rootPaths.add (File::getSpecialLocation (File::userDesktopDirectory).getFullPathName());
    rootNames.add (TRANS("Desktop"));
   #endif
}

void FileBrowserComponent::getRoots (StringArray& rootNames, StringArray& rootPaths)
{
    getDefaultRoots (rootNames, rootPaths);
}

void FileBrowserComponent::timerCallback()
{
    const bool isProcessActive = Process::isForegroundProcess();

    if (wasProcessActive != isProcessActive)
    {
        wasProcessActive = isProcessActive;

        if (isProcessActive && fileList != nullptr)
            refresh();
    }
}

} // namespace juce
