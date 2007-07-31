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

#include <Carbon/Carbon.h>
#include <fnmatch.h>

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "../../../src/juce_appframework/gui/components/filebrowser/juce_FileChooser.h"
#include "../../../src/juce_appframework/gui/components/juce_Desktop.h"
#include "../../../src/juce_appframework/application/juce_Application.h"
#include "../../../src/juce_appframework/events/juce_MessageManager.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/text/juce_LocalisedStrings.h"


//==============================================================================
struct JuceNavInfo
{
    StringArray filters;
    AEDesc defaultLocation;
    bool defaultLocationValid;
};

static void pascal juceNavEventProc (NavEventCallbackMessage callbackSelector,
                                     NavCBRecPtr callbackParms,
                                     void *callBackUD)
{
    if (callbackSelector == kNavCBStart)
    {
        if (((JuceNavInfo*) callBackUD)->defaultLocationValid)
        {
            NavCustomControl (callbackParms->context,
                              kNavCtlSetLocation,
                              (void*) &((JuceNavInfo*) callBackUD)->defaultLocation);
        }

        for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            Component* const c = Desktop::getInstance().getComponent (i);

            if (c != 0 && c->isAlwaysOnTop() && c->isVisible())
            {
                SetWindowGroup (callbackParms->window,
                                GetWindowGroup ((WindowRef) c->getWindowHandle()));

                break;
            }
        }

        BringToFront (callbackParms->window);
        SelectWindow (callbackParms->window);
        SetUserFocusWindow (callbackParms->window);
    }
}

static Boolean pascal juceNavFilterProc (AEDesc* theItem,
                                         void*,
                                         void* callBackUD,
                                         NavFilterModes filterMode)
{
    // must return true if we don't understand the object
    bool result = true;

    if (filterMode == kNavFilteringBrowserList)
    {
        AEDesc desc;
        if (AECoerceDesc (theItem, typeFSRef, &desc) == noErr)
        {
            Size size = AEGetDescDataSize (&desc);

            if (size > 0)
            {
                void* data = juce_calloc (size);

                if (AEGetDescData (&desc, data, size) == noErr)
                {
                    const String path (PlatformUtilities::makePathFromFSRef ((FSRef*) data));

                    if (path.isNotEmpty())
                    {
                        const File file (path);

                        if ((! file.isDirectory()) || PlatformUtilities::isBundle (path))
                        {
                            const String filename (file.getFileName().toLowerCase());
                            const char* const filenameUTF8 = filename.toUTF8();

                            const JuceNavInfo* const info = (const JuceNavInfo*) callBackUD;

                            if (info != 0)
                            {
                                result = false;

                                for (int i = info->filters.size(); --i >= 0;)
                                {
                                    const String wildcard (info->filters[i].toLowerCase());

                                    if (fnmatch (wildcard.toUTF8(), filenameUTF8, 0) == 0)
                                    {
                                        result = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                juce_free (data);
            }

            AEDisposeDesc (&desc);
        }
    }

    return result;
}

void FileChooser::showPlatformDialog (OwnedArray<File>& results,
                                      const String& title,
                                      const File& currentFileOrDirectory,
                                      const String& filter,
                                      bool selectsDirectory,
                                      bool isSaveDialogue,
                                      bool warnAboutOverwritingExistingFiles,
                                      bool selectMultipleFiles,
                                      FilePreviewComponent* extraInfoComponent)
{
    JuceNavInfo userInfo;
    userInfo.filters.addTokens (filter.replaceCharacters (T(",:"), T(";;")), T(";"), 0);
    userInfo.filters.trim();
    userInfo.filters.removeEmptyStrings();
    userInfo.defaultLocationValid = false;
    void* const userInfoPtr = (void*) &userInfo;

    const int oldTimeBeforeWaitCursor = MessageManager::getInstance()->getTimeBeforeShowingWaitCursor();
    MessageManager::getInstance()->setTimeBeforeShowingWaitCursor (0);

    NavEventUPP eventProc = NewNavEventUPP (juceNavEventProc);
    NavObjectFilterUPP filterProc = NewNavObjectFilterUPP (juceNavFilterProc);

    FSRef defaultRef;

    if ((currentFileOrDirectory.isOnHardDisk()
           && PlatformUtilities::makeFSRefFromPath (&defaultRef,
                                                    currentFileOrDirectory.getFullPathName()))
        || (currentFileOrDirectory.getParentDirectory().isOnHardDisk()
             && PlatformUtilities::makeFSRefFromPath (&defaultRef,
                                                      currentFileOrDirectory.getParentDirectory().getFullPathName())))
    {
        if (AECreateDesc (typeFSRef, &defaultRef, sizeof (defaultRef), &userInfo.defaultLocation) == noErr)
        {
            userInfo.defaultLocationValid = true;
        }
    }

    WindowRef lastFocused = GetUserFocusWindow();
    NavDialogCreationOptions options;

    if (NavGetDefaultDialogCreationOptions (&options) == noErr)
    {
        options.optionFlags |= kNavSelectDefaultLocation
                                | kNavSupportPackages
                                | kNavAllowPreviews;

        if (! warnAboutOverwritingExistingFiles)
            options.optionFlags |= kNavDontConfirmReplacement;

        if (selectMultipleFiles)
            options.optionFlags |= kNavAllowMultipleFiles;

        const String name (selectsDirectory ? TRANS("Choose folder")
                                            : TRANS("Choose file"));

        options.clientName = PlatformUtilities::juceStringToCFString (name);
        CFStringRef message = PlatformUtilities::juceStringToCFString (title);

        // nasty layout bug if the message text is set for a directory browser..
        if (selectsDirectory)
            options.windowTitle = message;
        else
            options.message = message;

        NavDialogRef dialog = 0;
        bool ok = false;

        if (selectsDirectory)
        {
            ok = (NavCreateChooseFolderDialog (&options, eventProc, 0, userInfoPtr, &dialog) == noErr);
        }
        else if (isSaveDialogue)
        {
            ok = (NavCreatePutFileDialog (&options, 0, 0, eventProc, userInfoPtr, &dialog) == noErr);
        }
        else
        {
            ok = (NavCreateGetFileDialog (&options, 0, eventProc, 0, filterProc, userInfoPtr, &dialog) == noErr);
        }

        if (ok && (NavDialogRun (dialog) == noErr))
        {
            NavReplyRecord reply;
            if (NavDialogGetReply (dialog, &reply) == noErr)
            {
                if (reply.validRecord)
                {
                    long count;
                    if (AECountItems (&(reply.selection), &count) == noErr
                        && count > 0)
                    {
                        AEKeyword theKeyword;
                        DescType actualType;
                        Size actualSize;
                        FSRef file;

                        for (int i = 1; i <= count; ++i)
                        {
                            // Get a pointer to selected file
                            if (AEGetNthPtr (&(reply.selection),
                                             i,
                                             typeFSRef,
                                             &theKeyword,
                                             &actualType,
                                             &file,
                                             sizeof (file),
                                             &actualSize) == noErr)
                            {
                                String result (PlatformUtilities::makePathFromFSRef (&file));

                                if (result.isNotEmpty() && isSaveDialogue && ! selectsDirectory)
                                {
                                    CFStringRef saveName = NavDialogGetSaveFileName (dialog);

                                    result = File (result)
                                                .getChildFile (PlatformUtilities::convertToPrecomposedUnicode (PlatformUtilities::cfStringToJuceString (saveName)))
                                                .getFullPathName();
                                }

                                results.add (new File (result));
                            }
                        }
                    }
                }

                NavDisposeReply (&reply);
            }
        }

        if (dialog != 0)
            NavDialogDispose (dialog);

        CFRelease (message);
        CFRelease (options.clientName);
    }

    if (userInfo.defaultLocationValid)
        AEDisposeDesc (&userInfo.defaultLocation);

    DisposeNavEventUPP (eventProc);
    DisposeNavObjectFilterUPP (filterProc);

    MessageManager::getInstance()->setTimeBeforeShowingWaitCursor (oldTimeBeforeWaitCursor);

    SetUserFocusWindow (lastFocused);
}

END_JUCE_NAMESPACE
