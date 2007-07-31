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

#include "../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_FileBasedDocument.h"
#include "../gui/components/mouse/juce_MouseCursor.h"
#include "../gui/components/windows/juce_AlertWindow.h"
#include "../gui/components/filebrowser/juce_FileChooser.h"
#include "../../juce_core/text/juce_LocalisedStrings.h"


//==============================================================================
FileBasedDocument::FileBasedDocument (const String& fileExtension_,
                                      const String& fileWildcard_,
                                      const String& openFileDialogTitle_,
                                      const String& saveFileDialogTitle_)
    : changedSinceSave (false),
      fileExtension (fileExtension_),
      fileWildcard (fileWildcard_),
      openFileDialogTitle (openFileDialogTitle_),
      saveFileDialogTitle (saveFileDialogTitle_)
{
}

FileBasedDocument::~FileBasedDocument()
{
}

//==============================================================================
void FileBasedDocument::setChangedFlag (const bool hasChanged)
{
    changedSinceSave = hasChanged;
}

void FileBasedDocument::changed()
{
    changedSinceSave = true;
    sendChangeMessage (this);
}

//==============================================================================
void FileBasedDocument::setFile (const File& newFile)
{
    if (documentFile != newFile)
    {
        documentFile = newFile;
        changedSinceSave = true;
    }
}

//==============================================================================
bool FileBasedDocument::loadFrom (const File& newFile,
                                  const bool showMessageOnFailure)
{
    MouseCursor::showWaitCursor();

    const File oldFile (documentFile);
    documentFile = newFile;

    String error;

    if (newFile.existsAsFile())
    {
        error = loadDocument (newFile);

        if (error.isEmpty())
        {
            setChangedFlag (false);
            MouseCursor::hideWaitCursor();

            setLastDocumentOpened (newFile);
            return true;
        }
    }
    else
    {
        error = "The file doesn't exist";
    }

    documentFile = oldFile;
    MouseCursor::hideWaitCursor();

    if (showMessageOnFailure)
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                     TRANS("Failed to open file..."),
                                     TRANS("There was an error while trying to load the file:\n\n")
                                       + newFile.getFullPathName()
                                       + T("\n\n")
                                       + error);
    }

    return false;
}

bool FileBasedDocument::loadFromUserSpecifiedFile (const bool showMessageOnFailure)
{
    FileChooser fc (openFileDialogTitle,
                    getLastDocumentOpened(),
                    fileWildcard);

    if (fc.browseForFileToOpen())
        return loadFrom (fc.getResult(), showMessageOnFailure);

    return false;
}

//==============================================================================
FileBasedDocument::SaveResult FileBasedDocument::save (const bool askUserForFileIfNotSpecified,
                                                       const bool showMessageOnFailure)
{
    return saveAs (documentFile,
                   false,
                   askUserForFileIfNotSpecified,
                   showMessageOnFailure);
}

FileBasedDocument::SaveResult FileBasedDocument::saveAs (const File& newFile,
                                                         const bool warnAboutOverwritingExistingFiles,
                                                         const bool askUserForFileIfNotSpecified,
                                                         const bool showMessageOnFailure)
{
    if (newFile == File::nonexistent)
    {
        if (askUserForFileIfNotSpecified)
        {
            return saveAsInteractive (true);
        }
        else
        {
            // can't save to an unspecified file
            jassertfalse
            return failedToWriteToFile;
        }
    }

    if (warnAboutOverwritingExistingFiles && newFile.exists())
    {
        if (! AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                            TRANS("File already exists"),
                                            TRANS("There's already a file called:\n\n")
                                              + newFile.getFullPathName()
                                              + TRANS("\n\nAre you sure you want to overwrite it?"),
                                            TRANS("overwrite"),
                                            TRANS("cancel")))
        {
            return userCancelledSave;
        }
    }

    MouseCursor::showWaitCursor();

    const File oldFile (documentFile);
    documentFile = newFile;

    String error (saveDocument (newFile));

    if (error.isEmpty())
    {
        setChangedFlag (false);
        MouseCursor::hideWaitCursor();

        return savedOk;
    }

    documentFile = oldFile;
    MouseCursor::hideWaitCursor();

    if (showMessageOnFailure)
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                     TRANS("Error writing to file..."),
                                     TRANS("An error occurred while trying to save \"")
                                        + getDocumentTitle()
                                        + TRANS("\" to the file:\n\n")
                                        + newFile.getFullPathName()
                                        + T("\n\n")
                                        + error);
    }

    return failedToWriteToFile;
}

FileBasedDocument::SaveResult FileBasedDocument::saveIfNeededAndUserAgrees()
{
    if (! hasChangedSinceSaved())
        return savedOk;

    const int r = AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon,
                                                   TRANS("Closing document..."),
                                                   TRANS("Do you want to save the changes to \"")
                                                       + getDocumentTitle() + T("\"?"),
                                                   TRANS("save"),
                                                   TRANS("discard changes"),
                                                   TRANS("cancel"));

    if (r == 1)
    {
        // save changes
        return save (true, true);
    }
    else if (r == 2)
    {
        // discard changes
        return savedOk;
    }

    return userCancelledSave;
}

FileBasedDocument::SaveResult FileBasedDocument::saveAsInteractive (const bool warnAboutOverwritingExistingFiles)
{
    File f;

    if (documentFile.existsAsFile())
        f = documentFile;
    else
        f = getLastDocumentOpened();

    String legalFilename (File::createLegalFileName (getDocumentTitle()));

    if (legalFilename.isEmpty())
        legalFilename = "unnamed";

    if (f.existsAsFile() || f.getParentDirectory().isDirectory())
        f = f.getSiblingFile (legalFilename);
    else
        f = File::getCurrentWorkingDirectory().getChildFile (legalFilename);

    f = f.withFileExtension (fileExtension)
         .getNonexistentSibling (true);

    FileChooser fc (saveFileDialogTitle, f, fileWildcard);

    if (fc.browseForFileToSave (warnAboutOverwritingExistingFiles))
    {
        setLastDocumentOpened (fc.getResult());

        return saveAs (fc.getResult(),
                       false, false, true);
    }

    return userCancelledSave;
}

END_JUCE_NAMESPACE
