/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
static const void* defaultDirPath = 0;
static String returnedString; // need this to get non-existent pathnames from the directory chooser
static Component* currentExtraFileWin = 0;

static bool areThereAnyAlwaysOnTopWindows()
{
    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        Component* c = Desktop::getInstance().getComponent (i);

        if (c != 0 && c->isAlwaysOnTop() && c->isShowing())
            return true;
    }

    return false;
}

static int CALLBACK browseCallbackProc (HWND hWnd, UINT msg, LPARAM lParam, LPARAM /*lpData*/)
{
    if (msg == BFFM_INITIALIZED)
    {
        SendMessage (hWnd, BFFM_SETSELECTIONW, TRUE, (LPARAM) defaultDirPath);
    }
    else if (msg == BFFM_VALIDATEFAILEDW)
    {
        returnedString = (LPCWSTR) lParam;
    }
    else if (msg == BFFM_VALIDATEFAILEDA)
    {
        returnedString = (const char*) lParam;
    }

    return 0;
}

void juce_setWindowStyleBit (HWND h, const int styleType, const int feature, const bool bitIsSet) throw();

static UINT_PTR CALLBACK openCallback (HWND hdlg, UINT uiMsg, WPARAM /*wParam*/, LPARAM lParam)
{
    if (currentExtraFileWin != 0)
    {
        if (uiMsg == WM_INITDIALOG)
        {
            HWND dialogH = GetParent (hdlg);
            jassert (dialogH != 0);
            if (dialogH == 0)
                dialogH = hdlg;

            RECT r, cr;
            GetWindowRect (dialogH, &r);
            GetClientRect (dialogH, &cr);

            SetWindowPos (dialogH, 0,
                          r.left, r.top,
                          currentExtraFileWin->getWidth() + jmax (150, (int) (r.right - r.left)),
                          jmax (150, (int) (r.bottom - r.top)),
                          SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);

            currentExtraFileWin->setBounds (cr.right, cr.top, currentExtraFileWin->getWidth(), cr.bottom - cr.top);
            currentExtraFileWin->getChildComponent(0)->setBounds (0, 0, currentExtraFileWin->getWidth(), currentExtraFileWin->getHeight());

            SetParent ((HWND) currentExtraFileWin->getWindowHandle(), (HWND) dialogH);
            juce_setWindowStyleBit ((HWND)currentExtraFileWin->getWindowHandle(), GWL_STYLE, WS_CHILD, (dialogH != 0));
            juce_setWindowStyleBit ((HWND)currentExtraFileWin->getWindowHandle(), GWL_STYLE, WS_POPUP, (dialogH == 0));
        }
        else if (uiMsg == WM_NOTIFY)
        {
            LPOFNOTIFY ofn = (LPOFNOTIFY) lParam;

            if (ofn->hdr.code == CDN_SELCHANGE)
            {
                FilePreviewComponent* comp = (FilePreviewComponent*) currentExtraFileWin->getChildComponent(0);

                if (comp != 0)
                {
                    TCHAR path [MAX_PATH * 2];
                    path[0] = 0;
                    CommDlg_OpenSave_GetFilePath (GetParent (hdlg), (LPARAM) &path, MAX_PATH);

                    const String fn ((const WCHAR*) path);

                    comp->selectedFileChanged (File (fn));
                }
            }
        }
    }

    return 0;
}

class FPComponentHolder  : public Component
{
public:
    FPComponentHolder()
    {
        setVisible (true);
        setOpaque (true);
    }

    ~FPComponentHolder()
    {
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::lightgrey);
    }

private:
    FPComponentHolder (const FPComponentHolder&);
    const FPComponentHolder& operator= (const FPComponentHolder&);
};

//==============================================================================
void FileChooser::showPlatformDialog (OwnedArray<File>& results,
                                      const String& title,
                                      const File& currentFileOrDirectory,
                                      const String& filter,
                                      bool selectsDirectory,
                                      bool /*selectsFiles*/,
                                      bool isSaveDialogue,
                                      bool warnAboutOverwritingExistingFiles,
                                      bool selectMultipleFiles,
                                      FilePreviewComponent* extraInfoComponent)
{
    const int numCharsAvailable = 32768;
    MemoryBlock filenameSpace ((numCharsAvailable + 1) * sizeof (WCHAR), true);
    WCHAR* const fname = (WCHAR*) filenameSpace.getData();
    int fnameIdx = 0;

    JUCE_TRY
    {
        // use a modal window as the parent for this dialog box
        // to block input from other app windows
        const Rectangle mainMon (Desktop::getInstance().getMainMonitorArea());

        Component w (String::empty);
        w.setBounds (mainMon.getX() + mainMon.getWidth() / 4,
                     mainMon.getY() + mainMon.getHeight() / 4,
                     0, 0);
        w.setOpaque (true);
        w.setAlwaysOnTop (areThereAnyAlwaysOnTopWindows());
        w.addToDesktop (0);

        if (extraInfoComponent == 0)
            w.enterModalState();

        String initialDir;

        if (currentFileOrDirectory.isDirectory())
        {
            initialDir = currentFileOrDirectory.getFullPathName();
        }
        else
        {
            currentFileOrDirectory.getFileName().copyToBuffer (fname, numCharsAvailable);

            initialDir = currentFileOrDirectory.getParentDirectory().getFullPathName();
        }

        if (currentExtraFileWin->isValidComponent())
        {
            jassertfalse
            return;
        }

        if (selectsDirectory)
        {
            LPITEMIDLIST list = 0;
            filenameSpace.fillWith (0);

            {
                BROWSEINFO bi;
                zerostruct (bi);

                bi.hwndOwner = (HWND) w.getWindowHandle();
                bi.pszDisplayName = fname;
                bi.lpszTitle = title;
                bi.lpfn = browseCallbackProc;
#ifdef BIF_USENEWUI
                bi.ulFlags = BIF_USENEWUI | BIF_VALIDATE;
#else
                bi.ulFlags = 0x50;
#endif
                defaultDirPath = (const WCHAR*) initialDir;

                list = SHBrowseForFolder (&bi);

                if (! SHGetPathFromIDListW (list, fname))
                {
                    fname[0] = 0;
                    returnedString = String::empty;
                }
            }

            LPMALLOC al;
            if (list != 0 && SUCCEEDED (SHGetMalloc (&al)))
                al->Free (list);

            defaultDirPath = 0;

            if (returnedString.isNotEmpty())
            {
                const String stringFName (fname);

                results.add (new File (File (stringFName).getSiblingFile (returnedString)));
                returnedString = String::empty;

                return;
            }
        }
        else
        {
            DWORD flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

            if (warnAboutOverwritingExistingFiles)
                flags |= OFN_OVERWRITEPROMPT;

            if (selectMultipleFiles)
                flags |= OFN_ALLOWMULTISELECT;

            if (extraInfoComponent != 0)
            {
                flags |= OFN_ENABLEHOOK;

                currentExtraFileWin = new FPComponentHolder();
                currentExtraFileWin->addAndMakeVisible (extraInfoComponent);
                currentExtraFileWin->setSize (jlimit (20, 800, extraInfoComponent->getWidth()),
                                              extraInfoComponent->getHeight());
                currentExtraFileWin->addToDesktop (0);

                currentExtraFileWin->enterModalState();
            }

            {
                WCHAR filters [1024];
                zeromem (filters, sizeof (filters));
                filter.copyToBuffer (filters, 1024);
                filter.copyToBuffer (filters + filter.length() + 1,
                                     1022 - filter.length());

                OPENFILENAMEW of;
                zerostruct (of);

#ifdef OPENFILENAME_SIZE_VERSION_400W
                of.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
#else
                of.lStructSize = sizeof (of);
#endif
                of.hwndOwner = (HWND) w.getWindowHandle();
                of.lpstrFilter = filters;
                of.nFilterIndex = 1;
                of.lpstrFile = fname;
                of.nMaxFile = numCharsAvailable;
                of.lpstrInitialDir = initialDir;
                of.lpstrTitle = title;
                of.Flags = flags;

                if (extraInfoComponent != 0)
                    of.lpfnHook = &openCallback;

                if (isSaveDialogue)
                {
                    if (! GetSaveFileName (&of))
                        fname[0] = 0;
                    else
                        fnameIdx = of.nFileOffset;
                }
                else
                {
                    if (! GetOpenFileName (&of))
                        fname[0] = 0;
                    else
                        fnameIdx = of.nFileOffset;
                }
            }
        }
    }
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS
    catch (...)
    {
        fname[0] = 0;
    }
#endif

    deleteAndZero (currentExtraFileWin);

    const WCHAR* const files = fname;

    if (selectMultipleFiles && fnameIdx > 0 && files [fnameIdx - 1] == 0)
    {
        const WCHAR* filename = files + fnameIdx;

        while (*filename != 0)
        {
            const String filepath (String (files) + T("\\") + String (filename));
            results.add (new File (filepath));
            filename += CharacterFunctions::length (filename) + 1;
        }
    }
    else if (files[0] != 0)
    {
        results.add (new File (files));
    }
}

#endif
