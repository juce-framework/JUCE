/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

FileBasedDocument::FileBasedDocument (const String& fileExtension_,
                                      const String& fileWildcard_,
                                      const String& openFileDialogTitle_,
                                      const String& saveFileDialogTitle_)
    : fileExtension (fileExtension_),
      fileWildcard (fileWildcard_),
      openFileDialogTitle (openFileDialogTitle_),
      saveFileDialogTitle (saveFileDialogTitle_)
{
}

FileBasedDocument::~FileBasedDocument()
{
}

//==============================================================================
void FileBasedDocument::setChangedFlag (bool hasChanged)
{
    if (changedSinceSave != hasChanged)
    {
        changedSinceSave = hasChanged;
        sendChangeMessage();
    }
}

void FileBasedDocument::changed()
{
    changedSinceSave = true;
    sendChangeMessage();
}

//==============================================================================
void FileBasedDocument::setFile (const File& newFile)
{
    if (documentFile != newFile)
    {
        documentFile = newFile;
        changed();
    }
}

//==============================================================================
Result FileBasedDocument::loadFrom (const File& newFile, bool showMessageOnFailure, bool showWaitCursor)
{
    if (showWaitCursor)
        MouseCursor::showWaitCursor();

    auto oldFile = documentFile;
    documentFile = newFile;

    auto result = Result::fail (TRANS("The file doesn't exist"));

    if (newFile.existsAsFile())
    {
        result = loadDocument (newFile);

        if (result.wasOk())
        {
            setChangedFlag (false);

            if (showWaitCursor)
                MouseCursor::hideWaitCursor();

            setLastDocumentOpened (newFile);
            return result;
        }
    }

    documentFile = oldFile;

    if (showWaitCursor)
        MouseCursor::hideWaitCursor();

    if (showMessageOnFailure)
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          TRANS("Failed to open file..."),
                                          TRANS("There was an error while trying to load the file: FLNM")
                                              .replace ("FLNM", "\n" + newFile.getFullPathName())
                                            + "\n\n"
                                            + result.getErrorMessage());

    return result;
}

#if JUCE_MODAL_LOOPS_PERMITTED
Result FileBasedDocument::loadFromUserSpecifiedFile (const bool showMessageOnFailure)
{
    FileChooser fc (openFileDialogTitle,
                    getLastDocumentOpened(),
                    fileWildcard);

    if (fc.browseForFileToOpen())
        return loadFrom (fc.getResult(), showMessageOnFailure);

    return Result::fail (TRANS("User cancelled"));
}

static bool askToOverwriteFile (const File& newFile)
{
    return AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                            TRANS("File already exists"),
                                            TRANS("There's already a file called: FLNM")
                                                .replace ("FLNM", newFile.getFullPathName())
                                             + "\n\n"
                                             + TRANS("Are you sure you want to overwrite it?"),
                                            TRANS("Overwrite"),
                                            TRANS("Cancel"));
}

//==============================================================================
FileBasedDocument::SaveResult FileBasedDocument::save (bool askUserForFileIfNotSpecified,
                                                       bool showMessageOnFailure)
{
    return saveAs (documentFile,
                   false,
                   askUserForFileIfNotSpecified,
                   showMessageOnFailure);
}

FileBasedDocument::SaveResult FileBasedDocument::saveAs (const File& newFile,
                                                         bool warnAboutOverwritingExistingFiles,
                                                         bool askUserForFileIfNotSpecified,
                                                         bool showMessageOnFailure,
                                                         bool showWaitCursor)
{
    if (newFile == File())
    {
        if (askUserForFileIfNotSpecified)
            return saveAsInteractive (true);

        // can't save to an unspecified file
        jassertfalse;
        return failedToWriteToFile;
    }

    if (warnAboutOverwritingExistingFiles
          && newFile.exists()
          && ! askToOverwriteFile (newFile))
        return userCancelledSave;

    if (showWaitCursor)
        MouseCursor::showWaitCursor();

    auto oldFile = documentFile;
    documentFile = newFile;

    auto result = saveDocument (newFile);

    if (result.wasOk())
    {
        setChangedFlag (false);

        if (showWaitCursor)
            MouseCursor::hideWaitCursor();

        sendChangeMessage(); // because the filename may have changed
        return savedOk;
    }

    documentFile = oldFile;

    if (showWaitCursor)
        MouseCursor::hideWaitCursor();

    if (showMessageOnFailure)
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          TRANS("Error writing to file..."),
                                          TRANS("An error occurred while trying to save \"DCNM\" to the file: FLNM")
                                            .replace ("DCNM", getDocumentTitle())
                                            .replace ("FLNM", "\n" + newFile.getFullPathName())
                                           + "\n\n"
                                           + result.getErrorMessage());

    sendChangeMessage(); // because the filename may have changed
    return failedToWriteToFile;
}

FileBasedDocument::SaveResult FileBasedDocument::saveIfNeededAndUserAgrees()
{
    if (! hasChangedSinceSaved())
        return savedOk;

    auto r = AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon,
                                              TRANS("Closing document..."),
                                              TRANS("Do you want to save the changes to \"DCNM\"?")
                                                   .replace ("DCNM", getDocumentTitle()),
                                              TRANS("Save"),
                                              TRANS("Discard changes"),
                                              TRANS("Cancel"));

    if (r == 1)  // save changes
        return save (true, true);

    if (r == 2)  // discard changes
        return savedOk;

    return userCancelledSave;
}

File FileBasedDocument::getSuggestedSaveAsFile (const File& defaultFile)
{
    return defaultFile.withFileExtension (fileExtension).getNonexistentSibling (true);
}

FileBasedDocument::SaveResult FileBasedDocument::saveAsInteractive (bool warnAboutOverwritingExistingFiles)
{
    auto f = documentFile.existsAsFile() ? documentFile : getLastDocumentOpened();

    auto legalFilename = File::createLegalFileName (getDocumentTitle());

    if (legalFilename.isEmpty())
        legalFilename = "unnamed";

    if (f.existsAsFile() || f.getParentDirectory().isDirectory())
        f = f.getSiblingFile (legalFilename);
    else
        f = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile (legalFilename);

    f = getSuggestedSaveAsFile (f);

    FileChooser fc (saveFileDialogTitle, f, fileWildcard);

    if (fc.browseForFileToSave (warnAboutOverwritingExistingFiles))
    {
        auto chosen = fc.getResult();

        if (chosen.getFileExtension().isEmpty())
        {
            chosen = chosen.withFileExtension (fileExtension);

            if (chosen.exists() && ! askToOverwriteFile (chosen))
                return userCancelledSave;
        }

        setLastDocumentOpened (chosen);
        return saveAs (chosen, false, false, true);
    }

    return userCancelledSave;
}
#endif

} // namespace juce
