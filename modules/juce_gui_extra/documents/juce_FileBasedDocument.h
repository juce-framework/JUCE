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

#ifndef JUCE_FILEBASEDDOCUMENT_H_INCLUDED
#define JUCE_FILEBASEDDOCUMENT_H_INCLUDED


//==============================================================================
/**
    A class to take care of the logic involved with the loading/saving of some kind
    of document.

    There's quite a lot of tedious logic involved in writing all the load/save/save-as
    functions you need for documents that get saved to a file, so this class attempts
    to abstract most of the boring stuff.

    Your subclass should just implement all the pure virtual methods, and you can
    then use the higher-level public methods to do the load/save dialogs, to warn the user
    about overwriting files, etc.

    The document object keeps track of whether it has changed since it was last saved or
    loaded, so when you change something, call its changed() method. This will set a
    flag so it knows it needs saving, and will also broadcast a change message using the
    ChangeBroadcaster base class.

    @see ChangeBroadcaster
*/
class JUCE_API FileBasedDocument  : public ChangeBroadcaster
{
public:
    /** Creates a FileBasedDocument.

        @param fileExtension            the extension to use when loading/saving files, e.g. ".doc"
        @param fileWildCard             the wildcard to use in file dialogs, e.g. "*.doc"
        @param openFileDialogTitle      the title to show on an open-file dialog, e.g. "Choose a file to open.."
        @param saveFileDialogTitle      the title to show on an save-file dialog, e.g. "Choose a file to save as.."
    */
    FileBasedDocument (const String& fileExtension,
                       const String& fileWildCard,
                       const String& openFileDialogTitle,
                       const String& saveFileDialogTitle);

    /** Destructor. */
    virtual ~FileBasedDocument();

    //==============================================================================
    /** Returns true if the changed() method has been called since the file was
        last saved or loaded.

        @see setChangedFlag, changed
    */
    bool hasChangedSinceSaved() const                           { return changedSinceSave; }

    /** Called to indicate that the document has changed and needs saving.

        This method will also trigger a change message to be sent out using the
        ChangeBroadcaster base class.

        After calling the method, the hasChangedSinceSaved() method will return true, until
        it is reset either by saving to a file or using the setChangedFlag() method.

        @see hasChangedSinceSaved, setChangedFlag
    */
    virtual void changed();

    /** Sets the state of the 'changed' flag.

        The 'changed' flag is set to true when the changed() method is called - use this method
        to reset it or to set it without also broadcasting a change message.

        @see changed, hasChangedSinceSaved
    */
    void setChangedFlag (bool hasChanged);

    //==============================================================================
    /** Tries to open a file.

        If the file opens correctly, the document's file (see the getFile() method) is set
        to this new one; if it fails, the document's file is left unchanged, and optionally
        a message box is shown telling the user there was an error.

        @returns A result indicating whether the new file loaded successfully, or the error
                 message if it failed.
        @see loadDocument, loadFromUserSpecifiedFile
    */
    Result loadFrom (const File& fileToLoadFrom,
                     bool showMessageOnFailure);

    /** Asks the user for a file and tries to load it.

        This will pop up a dialog box using the title, file extension and
        wildcard specified in the document's constructor, and asks the user
        for a file. If they pick one, the loadFrom() method is used to
        try to load it, optionally showing a message if it fails.

        @returns    a result indicating success; This will be a failure message if the user
                    cancelled or if they picked a file which failed to load correctly
        @see loadFrom
    */
    Result loadFromUserSpecifiedFile (bool showMessageOnFailure);

    //==============================================================================
    /** A set of possible outcomes of one of the save() methods
    */
    enum SaveResult
    {
        savedOk = 0,            /**< indicates that a file was saved successfully. */
        userCancelledSave,      /**< indicates that the user aborted the save operation. */
        failedToWriteToFile     /**< indicates that it tried to write to a file but this failed. */
    };

    /** Tries to save the document to the last file it was saved or loaded from.

        This will always try to write to the file, even if the document isn't flagged as
        having changed.

        @param askUserForFileIfNotSpecified     if there's no file currently specified and this is
                                                true, it will prompt the user to pick a file, as if
                                                saveAsInteractive() was called.
        @param showMessageOnFailure             if true it will show a warning message when if the
                                                save operation fails
        @see saveIfNeededAndUserAgrees, saveAs, saveAsInteractive
    */
    SaveResult save (bool askUserForFileIfNotSpecified,
                     bool showMessageOnFailure);

    /** If the file needs saving, it'll ask the user if that's what they want to do, and save
        it if they say yes.

        If you've got a document open and want to close it (e.g. to quit the app), this is the
        method to call.

        If the document doesn't need saving it'll return the value savedOk so
        you can go ahead and delete the document.

        If it does need saving it'll prompt the user, and if they say "discard changes" it'll
        return savedOk, so again, you can safely delete the document.

        If the user clicks "cancel", it'll return userCancelledSave, so if you can abort the
        close-document operation.

        And if they click "save changes", it'll try to save and either return savedOk, or
        failedToWriteToFile if there was a problem.

        @see save, saveAs, saveAsInteractive
    */
    SaveResult saveIfNeededAndUserAgrees();

    /** Tries to save the document to a specified file.

        If this succeeds, it'll also change the document's internal file (as returned by
        the getFile() method). If it fails, the file will be left unchanged.

        @param newFile                      the file to try to write to
        @param warnAboutOverwritingExistingFiles    if true and the file exists, it'll ask
                                            the user first if they want to overwrite it
        @param askUserForFileIfNotSpecified if the file is non-existent and this is true, it'll
                                            use the saveAsInteractive() method to ask the user for a
                                            filename
        @param showMessageOnFailure         if true and the write operation fails, it'll show
                                            a message box to warn the user
        @see saveIfNeededAndUserAgrees, save, saveAsInteractive
    */
    SaveResult saveAs (const File& newFile,
                       bool warnAboutOverwritingExistingFiles,
                       bool askUserForFileIfNotSpecified,
                       bool showMessageOnFailure);

    /** Prompts the user for a filename and tries to save to it.

        This will pop up a dialog box using the title, file extension and
        wildcard specified in the document's constructor, and asks the user
        for a file. If they pick one, the saveAs() method is used to try to save
        to this file.

        @param warnAboutOverwritingExistingFiles    if true and the file exists, it'll ask
                                            the user first if they want to overwrite it
        @see saveIfNeededAndUserAgrees, save, saveAs
    */
    SaveResult saveAsInteractive (bool warnAboutOverwritingExistingFiles);

    //==============================================================================
    /** Returns the file that this document was last successfully saved or loaded from.

        When the document object is created, this will be set to File::nonexistent.

        It is changed when one of the load or save methods is used, or when setFile()
        is used to explicitly set it.
    */
    const File& getFile() const                             { return documentFile; }

    /** Sets the file that this document thinks it was loaded from.

        This won't actually load anything - it just changes the file stored internally.

        @see getFile
    */
    void setFile (const File& newFile);


protected:
    //==============================================================================
    /** Overload this to return the title of the document.

        This is used in message boxes, filenames and file choosers, so it should be
        something sensible.
    */
    virtual String getDocumentTitle() = 0;

    /** This method should try to load your document from the given file.
        @returns a Result object to indicate the whether there was an error.
    */
    virtual Result loadDocument (const File& file) = 0;

    /** This method should try to write your document to the given file.
        @returns a Result object to indicate the whether there was an error.
    */
    virtual Result saveDocument (const File& file) = 0;

    /** This is used for dialog boxes to make them open at the last folder you
        were using.

        getLastDocumentOpened() and setLastDocumentOpened() are used to store
        the last document that was used - you might want to store this value
        in a static variable, or even in your application's properties. It should
        be a global setting rather than a property of this object.

        This method works very well in conjunction with a RecentlyOpenedFilesList
        object to manage your recent-files list.

        As a default value, it's ok to return File::nonexistent, and the document
        object will use a sensible one instead.

        @see RecentlyOpenedFilesList
    */
    virtual File getLastDocumentOpened() = 0;

    /** This is used for dialog boxes to make them open at the last folder you
        were using.

        getLastDocumentOpened() and setLastDocumentOpened() are used to store
        the last document that was used - you might want to store this value
        in a static variable, or even in your application's properties. It should
        be a global setting rather than a property of this object.

        This method works very well in conjunction with a RecentlyOpenedFilesList
        object to manage your recent-files list.

        @see RecentlyOpenedFilesList
    */
    virtual void setLastDocumentOpened (const File& file) = 0;

   #if JUCE_MODAL_LOOPS_PERMITTED
    /** This is called by saveAsInteractive() to allow you to optionally customise the
        filename that the user is presented with in the save dialog.
        The defaultFile parameter is an initial suggestion based on what the class knows
        about the current document - you can return a variation on this file with a different
        extension, etc, or just return something completely different.
    */
    virtual File getSuggestedSaveAsFile (const File& defaultFile);
   #endif

private:
    //==============================================================================
    File documentFile;
    bool changedSinceSave;
    String fileExtension, fileWildcard, openFileDialogTitle, saveFileDialogTitle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileBasedDocument)
};


#endif   // JUCE_FILEBASEDDOCUMENT_H_INCLUDED
