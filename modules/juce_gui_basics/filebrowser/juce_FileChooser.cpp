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

FileChooser::FileChooser (const String& chooserBoxTitle,
                          const File& currentFileOrDirectory,
                          const String& fileFilters,
                          const bool useNativeBox,
                          const bool treatFilePackagesAsDirectories)
    : title (chooserBoxTitle),
      filters (fileFilters),
      startingFile (currentFileOrDirectory),
      useNativeDialogBox (useNativeBox && isPlatformDialogAvailable()),
      treatFilePackagesAsDirs (treatFilePackagesAsDirectories)
{
    if (! fileFilters.containsNonWhitespaceChars())
        filters = "*";
}

FileChooser::~FileChooser() {}

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

    results.clear();

    // the preview component needs to be the right size before you pass it in here..
    jassert (previewComp == nullptr || (previewComp->getWidth() > 10
                                         && previewComp->getHeight() > 10));

    const bool selectsDirectories = (flags & FileBrowserComponent::canSelectDirectories) != 0;
    const bool selectsFiles       = (flags & FileBrowserComponent::canSelectFiles) != 0;
    const bool isSave             = (flags & FileBrowserComponent::saveMode) != 0;
    const bool warnAboutOverwrite = (flags & FileBrowserComponent::warnAboutOverwriting) != 0;
    const bool selectMultiple     = (flags & FileBrowserComponent::canSelectMultipleItems) != 0;

    // You've set the flags for both saveMode and openMode!
    jassert (! (isSave && (flags & FileBrowserComponent::openMode) != 0));

   #if JUCE_WINDOWS
    if (useNativeDialogBox && ! (selectsFiles && selectsDirectories))
   #elif JUCE_MAC || JUCE_LINUX
    if (useNativeDialogBox)
   #else
    if (false)
   #endif
    {
        showPlatformDialog (results, title, startingFile, filters,
                            selectsDirectories, selectsFiles, isSave,
                            warnAboutOverwrite, selectMultiple, treatFilePackagesAsDirs,
                            previewComp);
    }
    else
    {
        ignoreUnused (selectMultiple);

        WildcardFileFilter wildcard (selectsFiles ? filters : String(),
                                     selectsDirectories ? "*" : String(),
                                     String());

        FileBrowserComponent browserComponent (flags, startingFile, &wildcard, previewComp);

        FileChooserDialogBox box (title, String(),
                                  browserComponent, warnAboutOverwrite,
                                  browserComponent.findColour (AlertWindow::backgroundColourId));

        if (box.show())
        {
            for (int i = 0; i < browserComponent.getNumSelectedFiles(); ++i)
                results.add (browserComponent.getSelectedFile (i));
        }
    }

    return results.size() > 0;
}
#endif

File FileChooser::getResult() const
{
    // if you've used a multiple-file select, you should use the getResults() method
    // to retrieve all the files that were chosen.
    jassert (results.size() <= 1);

    return results.getFirst();
}

//==============================================================================
FilePreviewComponent::FilePreviewComponent() {}
FilePreviewComponent::~FilePreviewComponent() {}

} // namespace juce
