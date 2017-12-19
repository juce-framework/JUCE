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

//==============================================================================
/**
    Creates a dialog box to choose a file or directory to load or save.

    To use a FileChooser:
    - create one (as a local stack variable is the neatest way)
    - call one of its browseFor.. methods
    - if this returns true, the user has selected a file, so you can retrieve it
      with the getResult() method.

    e.g. @code
    void loadMooseFile()
    {
        FileChooser myChooser ("Please select the moose you want to load...",
                               File::getSpecialLocation (File::userHomeDirectory),
                               "*.moose");

        if (myChooser.browseForFileToOpen())
        {
            File mooseFile (myChooser.getResult());

            loadMoose (mooseFile);
        }
    }
    @endcode
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
                                              be used instead.

                                              Note: on iOS when saving a file, a user will not
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

        @see browseForFileToOpen, browseForFileToSave, browseForDirectory
    */
    FileChooser (const String& dialogBoxTitle,
                 const File& initialFileOrDirectory = File(),
                 const String& filePatternsAllowed = String(),
                 bool useOSNativeDialogBox = true,
                 bool treatFilePackagesAsDirectories = false);

    /** Destructor. */
    ~FileChooser();

    //==============================================================================
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

    /** Use this method to launch the file browser window asynchronously.

        This will create a file browser dialog based on the settings in this
        structure and will launch it modally, returning immediately.

        You must specify a callback which is called when the file browser is
        canceled or a file is selected. To abort the file selection, simply
        delete the FileChooser object.

        You can use the ModalCallbackFunction::create method to wrap a lambda
        into a modal Callback object.

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
        Therefore, om mobile platforms, you should call getURLResult() instead.

        If you're using a multiple-file select, then use the getResults() method instead,
        to obtain the list of all files chosen.

        @see getURLResult, getResults
    */
    File getResult() const;

    /** Returns a list of all the files that were chosen during the last call to a
        browse method.

        On mobile platforms, the file browser may return a URL instead of a local file.
        Therefore, om mobile platforms, you should call getURLResults() instead.

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

        Note: on iOS you must use the returned URL object directly (you are also
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

        Note: on iOS you must use the returned URL object directly (you are also
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
        have installed the iCloud app on their device and used the app at leat once.
    */
    static bool isPlatformDialogAvailable();

    //==============================================================================
   #ifndef DOXYGEN
    class Native;
   #endif

private:
    //==============================================================================
    String title, filters;
    File startingFile;
    Array<URL> results;
    const bool useNativeDialogBox;
    const bool treatFilePackagesAsDirs;
    std::function<void (const FileChooser&)> asyncCallback;

    //==============================================================================
    void finished (const Array<URL>&);

    //==============================================================================
    struct Pimpl
    {
        virtual ~Pimpl() {}

        virtual void launch()     = 0;
        virtual void runModally() = 0;
    };

    ScopedPointer<Pimpl> pimpl;

    //==============================================================================
    Pimpl* createPimpl (int, FilePreviewComponent*);
    static Pimpl* showPlatformDialog (FileChooser&, int,
                                      FilePreviewComponent*);

    class NonNative;
    friend class NonNative;
    friend class Native;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileChooser)
};

} // namespace juce
