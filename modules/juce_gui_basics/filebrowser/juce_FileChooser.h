/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Creates a dialog box to choose a file or directory to load or save.

    @code
    std::unique_ptr<FileChooser> myChooser;

    void loadMooseFile()
    {
        myChooser = std::make_unique<FileChooser> ("Please select the moose you want to load...",
                                                   File::getSpecialLocation (File::userHomeDirectory),
                                                   "*.moose");

        auto folderChooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

        myChooser->launchAsync (folderChooserFlags, [this] (const FileChooser& chooser)
        {
            File mooseFile (chooser.getResult());

            loadMoose (mooseFile);
        });
    }
    @endcode

    @tags{GUI}
*/
class JUCE_API  FileChooser
{
public:
    //==============================================================================
    /** Creates a FileChooser.

        After creating one of these, use one of the browseFor... methods to display it.

        @param dialogBoxTitle                 a text string to display in the dialog box to
                                              tell the user what's going on
        @param initialFileOrDirectory         the file or directory that should be selected
                                              when the dialog box opens. If this parameter is
                                              set to File(), a sensible default directory will
                                              be used instead. When using native dialogs, not
                                              all platforms will actually select the file. For
                                              example, on macOS, when initialFileOrDirectory is
                                              a file, only the parent directory of
                                              initialFileOrDirectory will be used as the initial
                                              directory of the native file chooser.

                                              Note: On iOS when saving a file, a user will not
                                              be able to change a file name, so it may be a good
                                              idea to include at least a valid file name in
                                              initialFileOrDirectory. When no filename is found,
                                              "Untitled" will be used.

                                              Also, if you pass an already existing file on iOS,
                                              that file will be automatically copied to the
                                              destination chosen by user and if it can be previewed,
                                              its preview will be presented in the dialog too. You
                                              will still be able to write into this file copy, since
                                              its URL will be returned by getURLResult(). This can be
                                              useful when you want to save e.g. an image, so that
                                              you can pass a (temporary) file with low quality
                                              preview and after the user picks the destination,
                                              you can write a high quality image into the copied
                                              file. If you create such a temporary file, you need
                                              to delete it yourself, once it is not needed anymore.

        @param filePatternsAllowed            a set of file patterns to specify which files can be
                                              selected - each pattern should be separated by a comma or
                                              semi-colon, e.g. "*" or "*.jpg;*.gif". The native MacOS
                                              file browser only supports wildcard that specify
                                              extensions, so "*.jpg" is OK but "myfilename*" will not
                                              work. An empty string means that all files are allowed
        @param useOSNativeDialogBox           if true, then a native dialog box will be used
                                              if possible; if false, then a Juce-based
                                              browser dialog box will always be used
        @param treatFilePackagesAsDirectories if true, then the file chooser will allow the
                                              selection of files inside packages when
                                              invoked on OS X and when using native dialog
                                              boxes.
        @param parentComponent                An optional component which should be the parent
                                              for the file chooser. If this is a nullptr then the
                                              FileChooser will be a top-level window. AUv3s on iOS
                                              must specify this parameter as opening a top-level window
                                              in an AUv3 is forbidden due to sandbox restrictions.

        @see browseForFileToOpen, browseForFileToSave, browseForDirectory
    */
    FileChooser (const String& dialogBoxTitle,
                 const File& initialFileOrDirectory = File(),
                 const String& filePatternsAllowed = String(),
                 bool useOSNativeDialogBox = true,
                 bool treatFilePackagesAsDirectories = false,
                 Component* parentComponent = nullptr);

    /** Destructor. */
    ~FileChooser();

    //==============================================================================
   #if JUCE_MODAL_LOOPS_PERMITTED
    /** Shows a dialog box to choose a file to open.

        This will display the dialog box modally, using an "open file" mode, so that
        it won't allow non-existent files or directories to be chosen.

        @param previewComponent   an optional component to display inside the dialog
                                  box to show special info about the files that the user
                                  is browsing. The component will not be deleted by this
                                  object, so the caller must take care of it.
        @returns    true if the user selected a file, in which case, use the getResult()
                    method to find out what it was. Returns false if they cancelled instead.
        @see browseForFileToSave, browseForDirectory
    */
    bool browseForFileToOpen (FilePreviewComponent* previewComponent = nullptr);

    /** Same as browseForFileToOpen, but allows the user to select multiple files.

        The files that are returned can be obtained by calling getResults(). See
        browseForFileToOpen() for more info about the behaviour of this method.
    */
    bool browseForMultipleFilesToOpen (FilePreviewComponent* previewComponent = nullptr);

    /** Shows a dialog box to choose a file to save.

        This will display the dialog box modally, using an "save file" mode, so it
        will allow non-existent files to be chosen, but not directories.

        @param warnAboutOverwritingExistingFiles     if true, the dialog box will ask
                    the user if they're sure they want to overwrite a file that already
                    exists
        @returns    true if the user chose a file and pressed 'ok', in which case, use
                    the getResult() method to find out what the file was. Returns false
                    if they cancelled instead.
        @see browseForFileToOpen, browseForDirectory
    */
    bool browseForFileToSave (bool warnAboutOverwritingExistingFiles);

    /** Shows a dialog box to choose a directory.

        This will display the dialog box modally, using an "open directory" mode, so it
        will only allow directories to be returned, not files.

        @returns    true if the user chose a directory and pressed 'ok', in which case, use
                    the getResult() method to find out what they chose. Returns false
                    if they cancelled instead.
        @see browseForFileToOpen, browseForFileToSave
    */
    bool browseForDirectory();

    /** Same as browseForFileToOpen, but allows the user to select multiple files and directories.

        The files that are returned can be obtained by calling getResults(). See
        browseForFileToOpen() for more info about the behaviour of this method.
    */
    bool browseForMultipleFilesOrDirectories (FilePreviewComponent* previewComponent = nullptr);

    //==============================================================================
    /** Runs a dialog box for the given set of option flags.
        The flag values used are those in FileBrowserComponent::FileChooserFlags.

        @returns    true if the user chose a directory and pressed 'ok', in which case, use
                    the getResult() method to find out what they chose. Returns false
                    if they cancelled instead.
        @see FileBrowserComponent::FileChooserFlags
    */
    bool showDialog (int flags, FilePreviewComponent* previewComponent);
   #endif

    /** Use this method to launch the file browser window asynchronously.

        This will create a file browser dialog based on the settings in this
        structure and will launch it modally, returning immediately.

        You must specify a callback which is called when the file browser is
        cancelled or a file is selected. To abort the file selection, simply
        delete the FileChooser object.

        You must ensure that the lifetime of the callback object is longer than
        the lifetime of the file-chooser.
    */
    void launchAsync (int flags,
                      std::function<void (const FileChooser&)>,
                      FilePreviewComponent* previewComponent = nullptr);

    //==============================================================================
    /** Returns the last file that was chosen by one of the browseFor methods.

        After calling the appropriate browseFor... method, this method lets you
        find out what file or directory they chose.

        Note that the file returned is only valid if the browse method returned true (i.e.
        if the user pressed 'ok' rather than cancelling).

        On mobile platforms, the file browser may return a URL instead of a local file.
        Therefore, on mobile platforms, you should call getURLResult() instead.

        If you're using a multiple-file select, then use the getResults() method instead,
        to obtain the list of all files chosen.

        @see getURLResult, getResults
    */
    File getResult() const;

    /** Returns a list of all the files that were chosen during the last call to a
        browse method.

        On mobile platforms, the file browser may return a URL instead of a local file.
        Therefore, on mobile platforms, you should call getURLResults() instead.

        This array may be empty if no files were chosen, or can contain multiple entries
        if multiple files were chosen.

        @see getURLResults, getResult
    */
    Array<File> getResults() const noexcept;

    //==============================================================================
    /** Returns the last document that was chosen by one of the browseFor methods.

        Use this method if you are using the FileChooser on a mobile platform which
        may return a URL to a remote document. If a local file is chosen then you can
        convert this file to a JUCE File class via the URL::getLocalFile method.

        Note: On iOS you must use the returned URL object directly (you are also
        allowed to copy- or move-construct another URL from the returned URL), rather
        than just storing the path as a String and then creating a new URL from that
        String. This is because the returned URL contains internally a security
        bookmark that is required to access the files pointed by it. Then, once you stop
        dealing with the file pointed by the URL, you should dispose that URL object,
        so that the security bookmark can be released by the system (only a limited
        number of such URLs is allowed).

        @see getResult, URL::getLocalFile
    */
    URL getURLResult() const;

    /** Returns a list of all the files that were chosen during the last call to a
        browse method.

        Use this method if you are using the FileChooser on a mobile platform which
        may return a URL to a remote document. If a local file is chosen then you can
        convert this file to a JUCE File class via the URL::getLocalFile method.

        This array may be empty if no files were chosen, or can contain multiple entries
        if multiple files were chosen.

        Note: On iOS you must use the returned URL object directly (you are also
        allowed to copy- or move-construct another URL from the returned URL), rather
        than just storing the path as a String and then creating a new URL from that
        String. This is because the returned URL contains internally a security
        bookmark that is required to access the files pointed by it. Then, once you stop
        dealing with the file pointed by the URL, you should dispose that URL object,
        so that the security bookmark can be released by the system (only a limited
        number of such URLs is allowed).

        @see getResults, URL::getLocalFile
    */
    const Array<URL>& getURLResults() const noexcept      { return results; }

    //==============================================================================
    /** Returns if a native filechooser is currently available on this platform.

        Note: On iOS this will only return true if you have iCloud permissions
        and code-signing enabled in the Projucer and have added iCloud containers
        to your app in Apple's online developer portal. Additionally, the user must
        have installed the iCloud app on their device and used the app at least once.
    */
    static bool isPlatformDialogAvailable();

    /** Associate a particular file-extension to a mime-type

        On Android, JUCE needs to convert common file extensions to mime-types when using
        wildcard filters in native file chooser dialog boxes. JUCE has an extensive conversion
        table to convert between the most common file-types and mime-types transparently, but
        some more obscure file-types may be missing. Use this method to register your own
        mime-type to file extension conversions. Please contact the JUCE team if you think
        that a common mime-type/file-extension entry is missing in JUCE's internal tables.
        Does nothing on other platforms.
    */
    static void registerCustomMimeTypeForFileExtension (const String& mimeType,
                                                        const String& fileExtension);

    //==============================================================================
    /** @cond */
    class Native;
    /** @endcond */

private:
    //==============================================================================
    String title, filters;
    File startingFile;
    Component* parent;
    Array<URL> results;
    const bool useNativeDialogBox;
    const bool treatFilePackagesAsDirs;
    std::function<void (const FileChooser&)> asyncCallback;

    //==============================================================================
    void finished (const Array<URL>&);

    //==============================================================================
    struct Pimpl
    {
        virtual ~Pimpl() = default;

        virtual void launch()     = 0;
        virtual void runModally() = 0;
    };

    std::shared_ptr<Pimpl> pimpl;

    //==============================================================================
    std::shared_ptr<Pimpl> createPimpl (int, FilePreviewComponent*);
    static std::shared_ptr<Pimpl> showPlatformDialog (FileChooser&, int, FilePreviewComponent*);

    class NonNative;
    friend class NonNative;
    friend class Native;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileChooser)
};

} // namespace juce
