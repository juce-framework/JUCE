/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

FilenameComponent::FilenameComponent (const String& name,
                                      const File& currentFile,
                                      const bool canEditFilename,
                                      const bool isDirectory,
                                      const bool isForSaving,
                                      const String& fileBrowserWildcard,
                                      const String& suffix,
                                      const String& textWhenNothingSelected)
    : Component (name),
      maxRecentFiles (30),
      isDir (isDirectory),
      isSaving (isForSaving),
      isFileDragOver (false),
      wildcard (fileBrowserWildcard),
      enforcedSuffix (suffix)
{
    addAndMakeVisible (filenameBox);
    filenameBox.setEditableText (canEditFilename);
    filenameBox.addListener (this);
    filenameBox.setTextWhenNothingSelected (textWhenNothingSelected);
    filenameBox.setTextWhenNoChoicesAvailable (TRANS ("(no recently selected files)"));

    setBrowseButtonText ("...");

    setCurrentFile (currentFile, true, dontSendNotification);
}

FilenameComponent::~FilenameComponent()
{
}

//==============================================================================
void FilenameComponent::paintOverChildren (Graphics& g)
{
    if (isFileDragOver)
    {
        g.setColour (Colours::red.withAlpha (0.2f));
        g.drawRect (getLocalBounds(), 3);
    }
}

void FilenameComponent::resized()
{
    getLookAndFeel().layoutFilenameComponent (*this, &filenameBox, browseButton);
}

KeyboardFocusTraverser* FilenameComponent::createFocusTraverser()
{
    // This prevents the sub-components from grabbing focus if the
    // FilenameComponent has been set to refuse focus.
    return getWantsKeyboardFocus() ? Component::createFocusTraverser() : nullptr;
}

void FilenameComponent::setBrowseButtonText (const String& newBrowseButtonText)
{
    browseButtonText = newBrowseButtonText;
    lookAndFeelChanged();
}

void FilenameComponent::lookAndFeelChanged()
{
    browseButton = nullptr;

    addAndMakeVisible (browseButton = getLookAndFeel().createFilenameComponentBrowseButton (browseButtonText));
    browseButton->setConnectedEdges (Button::ConnectedOnLeft);
    resized();

    browseButton->addListener (this);
}

void FilenameComponent::setTooltip (const String& newTooltip)
{
    SettableTooltipClient::setTooltip (newTooltip);
    filenameBox.setTooltip (newTooltip);
}

void FilenameComponent::setDefaultBrowseTarget (const File& newDefaultDirectory)
{
    defaultBrowseFile = newDefaultDirectory;
}

File FilenameComponent::getLocationToBrowse()
{
    return getCurrentFile() == File::nonexistent ? defaultBrowseFile
                                                 : getCurrentFile();
}

void FilenameComponent::buttonClicked (Button*)
{
   #if JUCE_MODAL_LOOPS_PERMITTED
    FileChooser fc (isDir ? TRANS ("Choose a new directory")
                          : TRANS ("Choose a new file"),
                    getLocationToBrowse(),
                    wildcard);

    if (isDir ? fc.browseForDirectory()
              : (isSaving ? fc.browseForFileToSave (false)
                          : fc.browseForFileToOpen()))
    {
        setCurrentFile (fc.getResult(), true);
    }
   #else
    jassertfalse; // needs rewriting to deal with non-modal environments
   #endif
}

void FilenameComponent::comboBoxChanged (ComboBox*)
{
    setCurrentFile (getCurrentFile(), true);
}

bool FilenameComponent::isInterestedInFileDrag (const StringArray&)
{
    return true;
}

void FilenameComponent::filesDropped (const StringArray& filenames, int, int)
{
    isFileDragOver = false;
    repaint();

    const File f (filenames[0]);

    if (f.exists() && (f.isDirectory() == isDir))
        setCurrentFile (f, true);
}

void FilenameComponent::fileDragEnter (const StringArray&, int, int)
{
    isFileDragOver = true;
    repaint();
}

void FilenameComponent::fileDragExit (const StringArray&)
{
    isFileDragOver = false;
    repaint();
}

//==============================================================================
String FilenameComponent::getCurrentFileText() const
{
    return filenameBox.getText();
}

File FilenameComponent::getCurrentFile() const
{
    File f (File::getCurrentWorkingDirectory().getChildFile (getCurrentFileText()));

    if (enforcedSuffix.isNotEmpty())
        f = f.withFileExtension (enforcedSuffix);

    return f;
}

void FilenameComponent::setCurrentFile (File newFile,
                                        const bool addToRecentlyUsedList,
                                        NotificationType notification)
{
    if (enforcedSuffix.isNotEmpty())
        newFile = newFile.withFileExtension (enforcedSuffix);

    if (newFile.getFullPathName() != lastFilename)
    {
        lastFilename = newFile.getFullPathName();

        if (addToRecentlyUsedList)
            addRecentlyUsedFile (newFile);

        filenameBox.setText (lastFilename, dontSendNotification);

        if (notification != dontSendNotification)
        {
            triggerAsyncUpdate();

            if (notification == sendNotificationSync)
                handleUpdateNowIfNeeded();
        }
    }
}

void FilenameComponent::setFilenameIsEditable (const bool shouldBeEditable)
{
    filenameBox.setEditableText (shouldBeEditable);
}

StringArray FilenameComponent::getRecentlyUsedFilenames() const
{
    StringArray names;

    for (int i = 0; i < filenameBox.getNumItems(); ++i)
        names.add (filenameBox.getItemText (i));

    return names;
}

void FilenameComponent::setRecentlyUsedFilenames (const StringArray& filenames)
{
    if (filenames != getRecentlyUsedFilenames())
    {
        filenameBox.clear();

        for (int i = 0; i < jmin (filenames.size(), maxRecentFiles); ++i)
            filenameBox.addItem (filenames[i], i + 1);
    }
}

void FilenameComponent::setMaxNumberOfRecentFiles (const int newMaximum)
{
    maxRecentFiles = jmax (1, newMaximum);

    setRecentlyUsedFilenames (getRecentlyUsedFilenames());
}

void FilenameComponent::addRecentlyUsedFile (const File& file)
{
    StringArray files (getRecentlyUsedFilenames());

    if (file.getFullPathName().isNotEmpty())
    {
        files.removeString (file.getFullPathName(), true);
        files.insert (0, file.getFullPathName());

        setRecentlyUsedFilenames (files);
    }
}

//==============================================================================
void FilenameComponent::addListener (FilenameComponentListener* const listener)
{
    listeners.add (listener);
}

void FilenameComponent::removeListener (FilenameComponentListener* const listener)
{
    listeners.remove (listener);
}

void FilenameComponent::handleAsyncUpdate()
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, &FilenameComponentListener::filenameComponentChanged, this);
}
