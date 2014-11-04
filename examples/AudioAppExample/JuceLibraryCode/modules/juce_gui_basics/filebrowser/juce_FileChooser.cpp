/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

FileChooser::FileChooser (const String& chooserBoxTitle,
                          const File& currentFileOrDirectory,
                          const String& fileFilters,
                          const bool useNativeBox)
    : title (chooserBoxTitle),
      filters (fileFilters),
      startingFile (currentFileOrDirectory),
      useNativeDialogBox (useNativeBox && isPlatformDialogAvailable())
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
    if (useNativeDialogBox && (previewComp == nullptr))
   #else
    if (false)
   #endif
    {
        showPlatformDialog (results, title, startingFile, filters,
                            selectsDirectories, selectsFiles, isSave,
                            warnAboutOverwrite, selectMultiple, previewComp);
    }
    else
    {
        WildcardFileFilter wildcard (selectsFiles ? filters : String::empty,
                                     selectsDirectories ? "*" : String::empty,
                                     String::empty);

        FileBrowserComponent browserComponent (flags, startingFile, &wildcard, previewComp);

        FileChooserDialogBox box (title, String::empty,
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
