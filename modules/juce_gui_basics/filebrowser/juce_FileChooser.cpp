/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

FileChooser::FileChooser (const String& chooserBoxTitle,
                          const File& currentFileOrDirectory,
                          const String& fileFilters,
                          const bool useNativeDialogBox_)
    : title (chooserBoxTitle),
      filters (fileFilters),
      startingFile (currentFileOrDirectory),
      useNativeDialogBox (useNativeDialogBox_)
{
    if (useNativeDialogBox)
    {
        static bool canUseNativeBox = isPlatformDialogAvailable();
        if (! canUseNativeBox)
            useNativeDialogBox = false;
    }

    if (! fileFilters.containsNonWhitespaceChars())
        filters = "*";
}

FileChooser::~FileChooser()
{
}

#if JUCE_MODAL_LOOPS_PERMITTED
bool FileChooser::browseForFileToOpen (FilePreviewComponent* previewComponent)
{
    return showDialog (false, true, false, false, false, previewComponent);
}

bool FileChooser::browseForMultipleFilesToOpen (FilePreviewComponent* previewComponent)
{
    return showDialog (false, true, false, false, true, previewComponent);
}

bool FileChooser::browseForMultipleFilesOrDirectories (FilePreviewComponent* previewComponent)
{
    return showDialog (true, true, false, false, true, previewComponent);
}

bool FileChooser::browseForFileToSave (const bool warnAboutOverwritingExistingFiles)
{
    return showDialog (false, true, true, warnAboutOverwritingExistingFiles, false, nullptr);
}

bool FileChooser::browseForDirectory()
{
    return showDialog (true, false, false, false, false, nullptr);
}

bool FileChooser::showDialog (const bool selectsDirectories,
                              const bool selectsFiles,
                              const bool isSave,
                              const bool warnAboutOverwritingExistingFiles,
                              const bool selectMultipleFiles,
                              FilePreviewComponent* const previewComponent)
{
    WeakReference<Component> previouslyFocused (Component::getCurrentlyFocusedComponent());

    results.clear();

    // the preview component needs to be the right size before you pass it in here..
    jassert (previewComponent == nullptr || (previewComponent->getWidth() > 10
                                               && previewComponent->getHeight() > 10));

   #if JUCE_WINDOWS
    if (useNativeDialogBox && ! (selectsFiles && selectsDirectories))
   #elif JUCE_MAC || JUCE_LINUX
    if (useNativeDialogBox && (previewComponent == nullptr))
   #else
    if (false)
   #endif
    {
        showPlatformDialog (results, title, startingFile, filters,
                            selectsDirectories, selectsFiles, isSave,
                            warnAboutOverwritingExistingFiles,
                            selectMultipleFiles,
                            previewComponent);
    }
    else
    {
        WildcardFileFilter wildcard (selectsFiles ? filters : String::empty,
                                     selectsDirectories ? "*" : String::empty,
                                     String::empty);

        int flags = isSave ? FileBrowserComponent::saveMode
                           : FileBrowserComponent::openMode;

        if (selectsFiles)
            flags |= FileBrowserComponent::canSelectFiles;

        if (selectsDirectories)
        {
            flags |= FileBrowserComponent::canSelectDirectories;

            if (! isSave)
                flags |= FileBrowserComponent::filenameBoxIsReadOnly;
        }

        if (selectMultipleFiles)
            flags |= FileBrowserComponent::canSelectMultipleItems;

        FileBrowserComponent browserComponent (flags, startingFile, &wildcard, previewComponent);

        FileChooserDialogBox box (title, String::empty,
                                  browserComponent,
                                  warnAboutOverwritingExistingFiles,
                                  browserComponent.findColour (AlertWindow::backgroundColourId));

        if (box.show())
        {
            for (int i = 0; i < browserComponent.getNumSelectedFiles(); ++i)
                results.add (browserComponent.getSelectedFile (i));
        }
    }

    if (previouslyFocused != nullptr)
        previouslyFocused->grabKeyboardFocus();

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

const Array<File>& FileChooser::getResults() const
{
    return results;
}

//==============================================================================
FilePreviewComponent::FilePreviewComponent()
{
}

FilePreviewComponent::~FilePreviewComponent()
{
}
