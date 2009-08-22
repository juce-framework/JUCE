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

#include "juce_FilenameComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../filebrowser/juce_FileChooser.h"
#include "../../../text/juce_LocalisedStrings.h"


//==============================================================================
FilenameComponent::FilenameComponent (const String& name,
                                      const File& currentFile,
                                      const bool canEditFilename,
                                      const bool isDirectory,
                                      const bool isForSaving,
                                      const String& fileBrowserWildcard,
                                      const String& enforcedSuffix_,
                                      const String& textWhenNothingSelected)
    : Component (name),
      maxRecentFiles (30),
      isDir (isDirectory),
      isSaving (isForSaving),
      isFileDragOver (false),
      wildcard (fileBrowserWildcard),
      enforcedSuffix (enforcedSuffix_)
{
    addAndMakeVisible (filenameBox = new ComboBox (T("fn")));
    filenameBox->setEditableText (canEditFilename);
    filenameBox->addListener (this);
    filenameBox->setTextWhenNothingSelected (textWhenNothingSelected);
    filenameBox->setTextWhenNoChoicesAvailable (TRANS("(no recently seleced files)"));

    browseButton = 0;
    setBrowseButtonText (T("..."));

    setCurrentFile (currentFile, true);
}

FilenameComponent::~FilenameComponent()
{
    deleteAllChildren();
}

//==============================================================================
void FilenameComponent::paintOverChildren (Graphics& g)
{
    if (isFileDragOver)
    {
        g.setColour (Colours::red.withAlpha (0.2f));
        g.drawRect (0, 0, getWidth(), getHeight(), 3);
    }
}

void FilenameComponent::resized()
{
    getLookAndFeel().layoutFilenameComponent (*this, filenameBox, browseButton);
}

void FilenameComponent::setBrowseButtonText (const String& newBrowseButtonText)
{
    browseButtonText = newBrowseButtonText;
    lookAndFeelChanged();
}

void FilenameComponent::lookAndFeelChanged()
{
    deleteAndZero (browseButton);

    addAndMakeVisible (browseButton = getLookAndFeel().createFilenameComponentBrowseButton (browseButtonText));
    browseButton->setConnectedEdges (Button::ConnectedOnLeft);
    resized();

    browseButton->addButtonListener (this);
}

void FilenameComponent::setTooltip (const String& newTooltip)
{
    SettableTooltipClient::setTooltip (newTooltip);
    filenameBox->setTooltip (newTooltip);
}

void FilenameComponent::setDefaultBrowseTarget (const File& newDefaultDirectory) throw()
{
    defaultBrowseFile = newDefaultDirectory;
}

void FilenameComponent::buttonClicked (Button*)
{
    FileChooser fc (TRANS("Choose a new file"),
                    getCurrentFile() == File::nonexistent ? defaultBrowseFile
                                                          : getCurrentFile(),
                    wildcard);

    if (isDir ? fc.browseForDirectory()
              : (isSaving ? fc.browseForFileToSave (false)
                          : fc.browseForFileToOpen()))
    {
        setCurrentFile (fc.getResult(), true);
    }
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
const File FilenameComponent::getCurrentFile() const
{
    File f (filenameBox->getText());

    if (enforcedSuffix.isNotEmpty())
        f = f.withFileExtension (enforcedSuffix);

    return f;
}

void FilenameComponent::setCurrentFile (File newFile,
                                        const bool addToRecentlyUsedList,
                                        const bool sendChangeNotification)
{
    if (enforcedSuffix.isNotEmpty())
        newFile = newFile.withFileExtension (enforcedSuffix);

    if (newFile.getFullPathName() != lastFilename)
    {
        lastFilename = newFile.getFullPathName();

        if (addToRecentlyUsedList)
            addRecentlyUsedFile (newFile);

        filenameBox->setText (lastFilename, true);

        if (sendChangeNotification)
            triggerAsyncUpdate();
    }
}

void FilenameComponent::setFilenameIsEditable (const bool shouldBeEditable)
{
    filenameBox->setEditableText (shouldBeEditable);
}

const StringArray FilenameComponent::getRecentlyUsedFilenames() const
{
    StringArray names;

    for (int i = 0; i < filenameBox->getNumItems(); ++i)
        names.add (filenameBox->getItemText (i));

    return names;
}

void FilenameComponent::setRecentlyUsedFilenames (const StringArray& filenames)
{
    if (filenames != getRecentlyUsedFilenames())
    {
        filenameBox->clear();

        for (int i = 0; i < jmin (filenames.size(), maxRecentFiles); ++i)
            filenameBox->addItem (filenames[i], i + 1);
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
void FilenameComponent::addListener (FilenameComponentListener* const listener) throw()
{
    jassert (listener != 0);

    if (listener != 0)
        listeners.add (listener);
}

void FilenameComponent::removeListener (FilenameComponentListener* const listener) throw()
{
    listeners.removeValue (listener);
}

void FilenameComponent::handleAsyncUpdate()
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ((FilenameComponentListener*) listeners.getUnchecked (i))->filenameComponentChanged (this);
        i = jmin (i, listeners.size());
    }
}

END_JUCE_NAMESPACE
