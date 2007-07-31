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

#include "juce_FileChooser.h"
#include "juce_WildcardFileFilter.h"
#include "juce_FileChooserDialogBox.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../windows/juce_AlertWindow.h"


//==============================================================================
FileChooser::FileChooser (const String& chooserBoxTitle,
                          const File& currentFileOrDirectory,
                          const String& fileFilters,
                          const bool useNativeDialogBox_)
    : title (chooserBoxTitle),
      filters (fileFilters),
      startingFile (currentFileOrDirectory),
      useNativeDialogBox (useNativeDialogBox_)
{
#if JUCE_LINUX
    useNativeDialogBox = false;
#endif

    if (fileFilters.trim().isEmpty())
        filters = T("*");
}

FileChooser::~FileChooser()
{
}

bool FileChooser::browseForFileToOpen (FilePreviewComponent* previewComponent)
{
    return showDialog (false, false, false, false, previewComponent);
}

bool FileChooser::browseForMultipleFilesToOpen (FilePreviewComponent* previewComponent)
{
    return showDialog (false, false, false, true, previewComponent);
}

bool FileChooser::browseForFileToSave (const bool warnAboutOverwritingExistingFiles)
{
    return showDialog (false, true, warnAboutOverwritingExistingFiles, false, 0);
}

bool FileChooser::browseForDirectory()
{
    return showDialog (true, false, false, false, 0);
}

const File FileChooser::getResult() const
{
    // if you've used a multiple-file select, you should use the getResults() method
    // to retrieve all the files that were chosen.
    jassert (results.size() <= 1);

    const File* const f = results.getFirst();

    if (f != 0)
        return *f;

    return File::nonexistent;
}

const OwnedArray <File>& FileChooser::getResults() const
{
    return results;
}

bool FileChooser::showDialog (const bool isDirectory,
                              const bool isSave,
                              const bool warnAboutOverwritingExistingFiles,
                              const bool selectMultipleFiles,
                              FilePreviewComponent* const previewComponent)
{
    ComponentDeletionWatcher* currentlyFocusedChecker = 0;
    Component* const currentlyFocused = Component::getCurrentlyFocusedComponent();

    if (currentlyFocused != 0)
        currentlyFocusedChecker = new ComponentDeletionWatcher (currentlyFocused);

    results.clear();

    // the preview component needs to be the right size before you pass it in here..
    jassert (previewComponent == 0 || (previewComponent->getWidth() > 10
                                        && previewComponent->getHeight() > 10));

#if JUCE_WIN32
    if (useNativeDialogBox)
#else
    if (useNativeDialogBox && (previewComponent == 0))
#endif
    {
        showPlatformDialog (results, title, startingFile, filters,
                            isDirectory, isSave,
                            warnAboutOverwritingExistingFiles,
                            selectMultipleFiles,
                            previewComponent);
    }
    else
    {
        jassert (! selectMultipleFiles); // not yet implemented for juce dialogs!

        WildcardFileFilter wildcard (filters, String::empty);

        FileBrowserComponent browserComponent (isDirectory ? FileBrowserComponent::chooseDirectoryMode
                                                           : (isSave ? FileBrowserComponent::saveFileMode
                                                                     : FileBrowserComponent::loadFileMode),
                                               startingFile, &wildcard, previewComponent);

        FileChooserDialogBox box (title, String::empty,
                                  browserComponent,
                                  warnAboutOverwritingExistingFiles,
                                  browserComponent.findColour (AlertWindow::backgroundColourId));

        if (box.show())
            results.add (new File (browserComponent.getCurrentFile()));
    }

    if (currentlyFocused != 0 && ! currentlyFocusedChecker->hasBeenDeleted())
        currentlyFocused->grabKeyboardFocus();

    delete currentlyFocusedChecker;

    return results.size() > 0;
}

//==============================================================================
FilePreviewComponent::FilePreviewComponent()
{
}

FilePreviewComponent::~FilePreviewComponent()
{
}


END_JUCE_NAMESPACE
