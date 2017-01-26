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

namespace FileChooserHelpers
{
    struct FileChooserCallbackInfo
    {
        String initialPath;
        String returnedString; // need this to get non-existent pathnames from the directory chooser
        ScopedPointer<Component> customComponent;
    };

    static int CALLBACK browseCallbackProc (HWND hWnd, UINT msg, LPARAM lParam, LPARAM lpData)
    {
        FileChooserCallbackInfo* info = (FileChooserCallbackInfo*) lpData;

        if (msg == BFFM_INITIALIZED)
            SendMessage (hWnd, BFFM_SETSELECTIONW, TRUE, (LPARAM) info->initialPath.toWideCharPointer());
        else if (msg == BFFM_VALIDATEFAILEDW)
            info->returnedString = (LPCWSTR) lParam;
        else if (msg == BFFM_VALIDATEFAILEDA)
            info->returnedString = (const char*) lParam;

        return 0;
    }

    static UINT_PTR CALLBACK openCallback (HWND hdlg, UINT uiMsg, WPARAM /*wParam*/, LPARAM lParam)
    {
        if (uiMsg == WM_INITDIALOG)
        {
            Component* customComp = ((FileChooserCallbackInfo*) (((OPENFILENAMEW*) lParam)->lCustData))->customComponent;

            HWND dialogH = GetParent (hdlg);
            jassert (dialogH != 0);
            if (dialogH == 0)
                dialogH = hdlg;

            RECT r, cr;
            GetWindowRect (dialogH, &r);
            GetClientRect (dialogH, &cr);

            SetWindowPos (dialogH, 0,
                          r.left, r.top,
                          customComp->getWidth() + jmax (150, (int) (r.right - r.left)),
                          jmax (150, (int) (r.bottom - r.top)),
                          SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);

            customComp->setBounds (cr.right, cr.top, customComp->getWidth(), cr.bottom - cr.top);
            customComp->addToDesktop (0, dialogH);
        }
        else if (uiMsg == WM_NOTIFY)
        {
            LPOFNOTIFY ofn = (LPOFNOTIFY) lParam;

            if (ofn->hdr.code == CDN_SELCHANGE)
            {
                FileChooserCallbackInfo* info = (FileChooserCallbackInfo*) ofn->lpOFN->lCustData;

                if (FilePreviewComponent* comp = dynamic_cast<FilePreviewComponent*> (info->customComponent->getChildComponent(0)))
                {
                    WCHAR path [MAX_PATH * 2] = { 0 };
                    CommDlg_OpenSave_GetFilePath (GetParent (hdlg), (LPARAM) &path, MAX_PATH);

                    comp->selectedFileChanged (File (path));
                }
            }
        }

        return 0;
    }

    class CustomComponentHolder  : public Component
    {
    public:
        CustomComponentHolder (Component* const customComp)
        {
            setVisible (true);
            setOpaque (true);
            addAndMakeVisible (customComp);
            setSize (jlimit (20, 800, customComp->getWidth()), customComp->getHeight());
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::lightgrey);
        }

        void resized() override
        {
            if (Component* const c = getChildComponent(0))
                c->setBounds (getLocalBounds());
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomComponentHolder)
    };
}

//==============================================================================
bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    return true;
   #endif
}

void FileChooser::showPlatformDialog (Array<File>& results, const String& title_, const File& currentFileOrDirectory,
                                      const String& filter, bool selectsDirectory, bool /*selectsFiles*/,
                                      bool isSaveDialogue, bool warnAboutOverwritingExistingFiles,
                                      bool selectMultipleFiles, bool /*treatFilePackagesAsDirs*/,
                                      FilePreviewComponent* extraInfoComponent)
{
    using namespace FileChooserHelpers;

    const String title (title_);
    String defaultExtension; // scope of these strings must extend beyond dialog's lifetime.

    HeapBlock<WCHAR> files;
    const size_t charsAvailableForResult = 32768;
    files.calloc (charsAvailableForResult + 1);
    int filenameOffset = 0;

    FileChooserCallbackInfo info;

    // use a modal window as the parent for this dialog box
    // to block input from other app windows
    Component parentWindow;
    const Rectangle<int> mainMon (Desktop::getInstance().getDisplays().getMainDisplay().userArea);
    parentWindow.setBounds (mainMon.getX() + mainMon.getWidth() / 4,
                            mainMon.getY() + mainMon.getHeight() / 4,
                            0, 0);
    parentWindow.setOpaque (true);
    parentWindow.setAlwaysOnTop (juce_areThereAnyAlwaysOnTopWindows());
    parentWindow.addToDesktop (0);

    if (extraInfoComponent == nullptr)
        parentWindow.enterModalState();

    if (currentFileOrDirectory.isDirectory())
    {
        info.initialPath = currentFileOrDirectory.getFullPathName();
    }
    else
    {
        currentFileOrDirectory.getFileName().copyToUTF16 (files, charsAvailableForResult * sizeof (WCHAR));
        info.initialPath = currentFileOrDirectory.getParentDirectory().getFullPathName();
    }

    if (selectsDirectory)
    {
        BROWSEINFO bi = { 0 };
        bi.hwndOwner = (HWND) parentWindow.getWindowHandle();
        bi.pszDisplayName = files;
        bi.lpszTitle = title.toWideCharPointer();
        bi.lParam = (LPARAM) &info;
        bi.lpfn = browseCallbackProc;
       #ifdef BIF_USENEWUI
        bi.ulFlags = BIF_USENEWUI | BIF_VALIDATE;
       #else
        bi.ulFlags = 0x50;
       #endif

        LPITEMIDLIST list = SHBrowseForFolder (&bi);

        if (! SHGetPathFromIDListW (list, files))
        {
            files[0] = 0;
            info.returnedString.clear();
        }

        LPMALLOC al;
        if (list != nullptr && SUCCEEDED (SHGetMalloc (&al)))
            al->Free (list);

        if (info.returnedString.isNotEmpty())
        {
            results.add (File (String (files)).getSiblingFile (info.returnedString));
            return;
        }
    }
    else
    {
        DWORD flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_ENABLESIZING;

        if (warnAboutOverwritingExistingFiles)
            flags |= OFN_OVERWRITEPROMPT;

        if (selectMultipleFiles)
            flags |= OFN_ALLOWMULTISELECT;

        if (extraInfoComponent != nullptr)
        {
            flags |= OFN_ENABLEHOOK;

            info.customComponent = new CustomComponentHolder (extraInfoComponent);
            info.customComponent->enterModalState();
        }

        const size_t filterSpaceNumChars = 2048;
        HeapBlock<WCHAR> filters;
        filters.calloc (filterSpaceNumChars);
        const size_t bytesWritten = filter.copyToUTF16 (filters.getData(), filterSpaceNumChars * sizeof (WCHAR));
        filter.copyToUTF16 (filters + (bytesWritten / sizeof (WCHAR)),
                            ((filterSpaceNumChars - 1) * sizeof (WCHAR) - bytesWritten));

        for (size_t i = 0; i < filterSpaceNumChars; ++i)
            if (filters[i] == '|')
                filters[i] = 0;

        OPENFILENAMEW of = { 0 };
        String localPath (info.initialPath);

       #ifdef OPENFILENAME_SIZE_VERSION_400W
        of.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
       #else
        of.lStructSize = sizeof (of);
       #endif
        of.hwndOwner = (HWND) parentWindow.getWindowHandle();
        of.lpstrFilter = filters.getData();
        of.nFilterIndex = 1;
        of.lpstrFile = files;
        of.nMaxFile = (DWORD) charsAvailableForResult;
        of.lpstrInitialDir = localPath.toWideCharPointer();
        of.lpstrTitle = title.toWideCharPointer();
        of.Flags = flags;
        of.lCustData = (LPARAM) &info;

        if (extraInfoComponent != nullptr)
            of.lpfnHook = &openCallback;

        if (isSaveDialogue)
        {
            StringArray tokens;
            tokens.addTokens (filter, ";,", "\"'");
            tokens.trim();
            tokens.removeEmptyStrings();

            if (tokens.size() == 1 && tokens[0].removeCharacters ("*.").isNotEmpty())
            {
                defaultExtension = tokens[0].fromFirstOccurrenceOf (".", false, false);
                of.lpstrDefExt = defaultExtension.toWideCharPointer();
            }

            if (! GetSaveFileName (&of))
                return;
        }
        else
        {
            if (! GetOpenFileName (&of))
                return;
        }

        filenameOffset = of.nFileOffset;
    }

    if (selectMultipleFiles && filenameOffset > 0 && files [filenameOffset - 1] == 0)
    {
        const WCHAR* filename = files + filenameOffset;

        while (*filename != 0)
        {
            results.add (File (String (files)).getChildFile (String (filename)));
            filename += wcslen (filename) + 1;
        }
    }
    else if (files[0] != 0)
    {
        results.add (File (String (files)));
    }
}
