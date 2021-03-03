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

//==============================================================================
class FileChooser::NonNative    : public FileChooser::Pimpl
{
public:
    NonNative (FileChooser& fileChooser, int flags, FilePreviewComponent* preview)
        : owner (fileChooser),
          selectsDirectories ((flags & FileBrowserComponent::canSelectDirectories)   != 0),
          selectsFiles       ((flags & FileBrowserComponent::canSelectFiles)         != 0),
          warnAboutOverwrite ((flags & FileBrowserComponent::warnAboutOverwriting)   != 0),

          filter (selectsFiles ? owner.filters : String(), selectsDirectories ? "*" : String(), {}),
          browserComponent (flags, owner.startingFile, &filter, preview),
          dialogBox (owner.title, {}, browserComponent, warnAboutOverwrite,
                     browserComponent.findColour (AlertWindow::backgroundColourId), owner.parent)
    {}

    ~NonNative() override
    {
        dialogBox.exitModalState (0);
    }

    void launch() override
    {
        dialogBox.centreWithDefaultSize (nullptr);
        dialogBox.enterModalState (true, ModalCallbackFunction::create ([this] (int r) { modalStateFinished (r); }), true);
    }

    void runModally() override
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        modalStateFinished (dialogBox.show() ? 1 : 0);
       #else
        jassertfalse;
       #endif
    }

private:
    void modalStateFinished (int returnValue)
    {
        Array<URL> result;

        if (returnValue != 0)
        {
            for (int i = 0; i < browserComponent.getNumSelectedFiles(); ++i)
                result.add (URL (browserComponent.getSelectedFile (i)));
        }

        owner.finished (result);
    }

    //==============================================================================
    FileChooser& owner;
    bool selectsDirectories, selectsFiles, warnAboutOverwrite;

    WildcardFileFilter filter;
    FileBrowserComponent browserComponent;
    FileChooserDialogBox dialogBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonNative)
};

//==============================================================================
FileChooser::FileChooser (const String& chooserBoxTitle,
                          const File& currentFileOrDirectory,
                          const String& fileFilters,
                          const bool useNativeBox,
                          const bool treatFilePackagesAsDirectories,
                          Component* parentComponentToUse)
    : title (chooserBoxTitle),
      filters (fileFilters),
      startingFile (currentFileOrDirectory),
      parent (parentComponentToUse),
      useNativeDialogBox (useNativeBox && isPlatformDialogAvailable()),
      treatFilePackagesAsDirs (treatFilePackagesAsDirectories)
{
   #ifndef JUCE_MAC
    ignoreUnused (treatFilePackagesAsDirs);
   #endif

    if (! fileFilters.containsNonWhitespaceChars())
        filters = "*";
}

FileChooser::~FileChooser()
{
    asyncCallback = nullptr;
}

#if JUCE_MODAL_LOOPS_PERMITTED
bool FileChooser::browseForFileToOpen (FilePreviewComponent* previewComp)
{
    return showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectFiles,
                       previewComp);
}

bool FileChooser::browseForMultipleFilesToOpen (FilePreviewComponent* previewComp)
{
    return showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectFiles
                        | FileBrowserComponent::canSelectMultipleItems,
                       previewComp);
}

bool FileChooser::browseForMultipleFilesOrDirectories (FilePreviewComponent* previewComp)
{
    return showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectFiles
                        | FileBrowserComponent::canSelectDirectories
                        | FileBrowserComponent::canSelectMultipleItems,
                       previewComp);
}

bool FileChooser::browseForFileToSave (const bool warnAboutOverwrite)
{
    return showDialog (FileBrowserComponent::saveMode
                        | FileBrowserComponent::canSelectFiles
                        | (warnAboutOverwrite ? FileBrowserComponent::warnAboutOverwriting : 0),
                       nullptr);
}

bool FileChooser::browseForDirectory()
{
    return showDialog (FileBrowserComponent::openMode
                        | FileBrowserComponent::canSelectDirectories,
                       nullptr);
}

bool FileChooser::showDialog (const int flags, FilePreviewComponent* const previewComp)
{
    FocusRestorer focusRestorer;

    pimpl = createPimpl (flags, previewComp);
    pimpl->runModally();

    // ensure that the finished function was invoked
    jassert (pimpl == nullptr);

    return (results.size() > 0);
}
#endif

void FileChooser::launchAsync (int flags, std::function<void (const FileChooser&)> callback,
                               FilePreviewComponent* previewComp)
{
    // You must specify a callback when using launchAsync
    jassert (callback);

    // you cannot run two file chooser dialog boxes at the same time
    jassert (asyncCallback == nullptr);

    asyncCallback = std::move (callback);

    pimpl = createPimpl (flags, previewComp);
    pimpl->launch();
}


std::shared_ptr<FileChooser::Pimpl> FileChooser::createPimpl (int flags, FilePreviewComponent* previewComp)
{
    results.clear();

    // the preview component needs to be the right size before you pass it in here..
    jassert (previewComp == nullptr || (previewComp->getWidth() > 10
                                         && previewComp->getHeight() > 10));

    if (pimpl != nullptr)
    {
        // you cannot run two file chooser dialog boxes at the same time
        jassertfalse;
        pimpl.reset();
    }

    // You've set the flags for both saveMode and openMode!
    jassert (! (((flags & FileBrowserComponent::saveMode) != 0)
                && ((flags & FileBrowserComponent::openMode) != 0)));

   #if JUCE_WINDOWS
    const bool selectsFiles       = (flags & FileBrowserComponent::canSelectFiles) != 0;
    const bool selectsDirectories = (flags & FileBrowserComponent::canSelectDirectories) != 0;

    if (useNativeDialogBox && ! (selectsFiles && selectsDirectories))
   #else
    if (useNativeDialogBox)
   #endif
    {
        return showPlatformDialog (*this, flags, previewComp);
    }

    return std::make_unique<NonNative> (*this, flags, previewComp);
}

Array<File> FileChooser::getResults() const noexcept
{
    Array<File> files;

    for (auto url : getURLResults())
        if (url.isLocalFile())
            files.add (url.getLocalFile());

    return files;
}

File FileChooser::getResult() const
{
    auto fileResults = getResults();

    // if you've used a multiple-file select, you should use the getResults() method
    // to retrieve all the files that were chosen.
    jassert (fileResults.size() <= 1);

    return fileResults.getFirst();
}

URL FileChooser::getURLResult() const
{
    // if you've used a multiple-file select, you should use the getResults() method
    // to retrieve all the files that were chosen.
    jassert (results.size() <= 1);

    return results.getFirst();
}

void FileChooser::finished (const Array<URL>& asyncResults)
{
     std::function<void (const FileChooser&)> callback;
     std::swap (callback, asyncCallback);

     results = asyncResults;

     pimpl.reset();

     if (callback)
         callback (*this);
}

//==============================================================================
FilePreviewComponent::FilePreviewComponent() {}
FilePreviewComponent::~FilePreviewComponent() {}

} // namespace juce
