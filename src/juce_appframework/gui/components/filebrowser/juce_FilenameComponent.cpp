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

#include "juce_FilenameComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../filebrowser/juce_FileChooser.h"
#include "../../../../juce_core/text/juce_LocalisedStrings.h"


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

bool FilenameComponent::filesDropped (const StringArray& filenames, int, int)
{
    const File f (filenames[0]);

    if (f.exists() && (f.isDirectory() == isDir))
        setCurrentFile (f, true);

    return true;
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
