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

// Implemented in juce_win32_Messaging.cpp
bool dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages);

class Win32NativeFileChooser  : private Thread
{
public:
    enum { charsAvailableForResult = 32768 };

    Win32NativeFileChooser (Component* parent, int flags, FilePreviewComponent* previewComp,
                            const File& startingFile, const String& titleToUse,
                            const String& filtersToUse)
        : Thread ("Native Win32 FileChooser"),
          owner (parent),
          title (titleToUse),
          filtersString (filtersToUse.replaceCharacter (',', ';')),
          selectsDirectories ((flags & FileBrowserComponent::canSelectDirectories)   != 0),
          isSave             ((flags & FileBrowserComponent::saveMode)               != 0),
          warnAboutOverwrite ((flags & FileBrowserComponent::warnAboutOverwriting)   != 0),
          selectMultiple     ((flags & FileBrowserComponent::canSelectMultipleItems) != 0)
    {
        auto parentDirectory = startingFile.getParentDirectory();

        // Handle nonexistent root directories in the same way as existing ones
        files.calloc (static_cast<size_t> (charsAvailableForResult) + 1);

        if (startingFile.isDirectory() || startingFile.isRoot())
        {
            initialPath = startingFile.getFullPathName();
        }
        else
        {
            startingFile.getFileName().copyToUTF16 (files,
                                                    static_cast<size_t> (charsAvailableForResult) * sizeof (WCHAR));
            initialPath = parentDirectory.getFullPathName();
        }

        if (! selectsDirectories)
        {
            if (previewComp != nullptr)
                customComponent.reset (new CustomComponentHolder (previewComp));

            setupFilters();
        }
    }

    ~Win32NativeFileChooser() override
    {
        signalThreadShouldExit();

        while (isThreadRunning())
        {
            if (! dispatchNextMessageOnSystemQueue (true))
                Thread::sleep (1);
        }
    }

    void open (bool async)
    {
        results.clear();

        // the thread should not be running
        nativeDialogRef.set (nullptr);

        if (async)
        {
            jassert (! isThreadRunning());

            startThread();
        }
        else
        {
            results = openDialog (false);
            owner->exitModalState (results.size() > 0 ? 1 : 0);
        }
    }

    void cancel()
    {
        ScopedLock lock (deletingDialog);

        customComponent = nullptr;
        shouldCancel = true;

        if (auto hwnd = nativeDialogRef.get())
            PostMessage (hwnd, WM_CLOSE, 0, 0);
    }

    Component* getCustomComponent()    { return customComponent.get(); }

    Array<URL> results;

private:
    //==============================================================================
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

    //==============================================================================
    const Component::SafePointer<Component> owner;
    String title, filtersString;
    std::unique_ptr<CustomComponentHolder> customComponent;
    String initialPath, returnedString;

    CriticalSection deletingDialog;

    bool selectsDirectories, isSave, warnAboutOverwrite, selectMultiple;

    HeapBlock<WCHAR> files;
    HeapBlock<WCHAR> filters;

    Atomic<HWND> nativeDialogRef { nullptr };
    bool shouldCancel = false;

    struct FreeLPWSTR
    {
        void operator() (LPWSTR ptr) const noexcept { CoTaskMemFree (ptr); }
    };

    bool showDialog (IFileDialog& dialog, bool async)
    {
        FILEOPENDIALOGOPTIONS flags = {};

        if (FAILED (dialog.GetOptions (&flags)))
            return false;

        const auto setBit = [] (FILEOPENDIALOGOPTIONS& field, bool value, FILEOPENDIALOGOPTIONS option)
        {
            if (value)
                field |= option;
            else
                field &= ~option;
        };

        setBit (flags, selectsDirectories,         FOS_PICKFOLDERS);
        setBit (flags, warnAboutOverwrite,         FOS_OVERWRITEPROMPT);
        setBit (flags, selectMultiple,             FOS_ALLOWMULTISELECT);
        setBit (flags, customComponent != nullptr, FOS_FORCEPREVIEWPANEON);

        if (FAILED (dialog.SetOptions (flags)) || FAILED (dialog.SetTitle (title.toUTF16())))
            return false;

        PIDLIST_ABSOLUTE pidl = {};

        if (FAILED (SHParseDisplayName (initialPath.toWideCharPointer(), nullptr, &pidl, SFGAO_FOLDER, nullptr)))
        {
            LPWSTR ptr = nullptr;
            auto result = SHGetKnownFolderPath (FOLDERID_Desktop, 0, nullptr, &ptr);
            std::unique_ptr<WCHAR, FreeLPWSTR> desktopPath (ptr);

            if (FAILED (result))
                return false;

            if (FAILED (SHParseDisplayName (desktopPath.get(), nullptr, &pidl, SFGAO_FOLDER, nullptr)))
                return false;
        }

        const auto item = [&]
        {
            ComSmartPtr<IShellItem> ptr;
            SHCreateShellItem (nullptr, nullptr, pidl, ptr.resetAndGetPointerAddress());
            return ptr;
        }();

        if (item != nullptr)
        {
            dialog.SetDefaultFolder (item);

            if (! initialPath.isEmpty())
                dialog.SetFolder (item);
        }

        String filename (files.getData());

        if (FAILED (dialog.SetFileName (filename.toWideCharPointer())))
            return false;

        auto extension = getDefaultFileExtension (filename);

        if (extension.isNotEmpty() && FAILED (dialog.SetDefaultExtension (extension.toWideCharPointer())))
            return false;

        const COMDLG_FILTERSPEC spec[] { { filtersString.toWideCharPointer(), filtersString.toWideCharPointer() } };

        if (! selectsDirectories && FAILED (dialog.SetFileTypes (numElementsInArray (spec), spec)))
            return false;

        struct Events  : public ComBaseClassHelper<IFileDialogEvents>
        {
            explicit Events (Win32NativeFileChooser& o) : owner (o) {}

            JUCE_COMRESULT OnTypeChange (IFileDialog* d) override                                                 { return updateHwnd (d); }
            JUCE_COMRESULT OnFolderChanging (IFileDialog* d, IShellItem*) override                                { return updateHwnd (d); }
            JUCE_COMRESULT OnFileOk (IFileDialog* d) override                                                     { return updateHwnd (d); }
            JUCE_COMRESULT OnFolderChange (IFileDialog* d) override                                               { return updateHwnd (d); }
            JUCE_COMRESULT OnSelectionChange (IFileDialog* d) override                                            { return updateHwnd (d); }
            JUCE_COMRESULT OnShareViolation (IFileDialog* d, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*) override  { return updateHwnd (d); }
            JUCE_COMRESULT OnOverwrite (IFileDialog* d, IShellItem*, FDE_OVERWRITE_RESPONSE*) override            { return updateHwnd (d); }

            JUCE_COMRESULT updateHwnd (IFileDialog* d)
            {
                HWND hwnd = nullptr;

                if (auto window = ComSmartPtr<IFileDialog> { d }.getInterface<IOleWindow>())
                    window->GetWindow (&hwnd);

                ScopedLock lock (owner.deletingDialog);

                if (owner.shouldCancel)
                    d->Close (S_FALSE);
                else if (hwnd != nullptr)
                    owner.nativeDialogRef = hwnd;

                return S_OK;
            }

            Win32NativeFileChooser& owner;
        };

        {
            ScopedLock lock (deletingDialog);

            if (shouldCancel)
                return false;
        }

        const auto result = [&]
        {
            struct ScopedAdvise
            {
                ScopedAdvise (IFileDialog& d, Events& events) : dialog (d) { dialog.Advise (&events, &cookie); }
                ~ScopedAdvise() { dialog.Unadvise (cookie); }
                IFileDialog& dialog;
                DWORD cookie = 0;
            };

            Events events { *this };
            ScopedAdvise scope { dialog, events };
            return dialog.Show (async ? nullptr : static_cast<HWND> (owner->getWindowHandle())) == S_OK;
        }();

        ScopedLock lock (deletingDialog);
        nativeDialogRef = nullptr;

        return result;
    }

    //==============================================================================
    Array<URL> openDialogVistaAndUp (bool async)
    {
        const auto getUrl = [] (IShellItem& item)
        {
            LPWSTR ptr = nullptr;

            if (item.GetDisplayName (SIGDN_FILESYSPATH, &ptr) != S_OK)
                return URL();

            const auto path = std::unique_ptr<WCHAR, FreeLPWSTR> { ptr };
            return URL (File (String (path.get())));
        };

        if (isSave)
        {
            const auto dialog = [&]
            {
                ComSmartPtr<IFileDialog> ptr;
                ptr.CoCreateInstance (CLSID_FileSaveDialog, CLSCTX_INPROC_SERVER);
                return ptr;
            }();

            if (dialog == nullptr)
                return {};

            showDialog (*dialog, async);

            const auto item = [&]
            {
                ComSmartPtr<IShellItem> ptr;
                dialog->GetResult (ptr.resetAndGetPointerAddress());
                return ptr;
            }();

            if (item == nullptr)
                return {};

            const auto url = getUrl (*item);

            if (url.isEmpty())
                return {};

            return { url };
        }

        const auto dialog = [&]
        {
            ComSmartPtr<IFileOpenDialog> ptr;
            ptr.CoCreateInstance (CLSID_FileOpenDialog, CLSCTX_INPROC_SERVER);
            return ptr;
        }();

        if (dialog == nullptr)
            return {};

        showDialog (*dialog, async);

        const auto items = [&]
        {
            ComSmartPtr<IShellItemArray> ptr;
            dialog->GetResults (ptr.resetAndGetPointerAddress());
            return ptr;
        }();

        if (items == nullptr)
            return {};

        Array<URL> result;

        DWORD numItems = 0;
        items->GetCount (&numItems);

        for (DWORD i = 0; i < numItems; ++i)
        {
            ComSmartPtr<IShellItem> scope;
            items->GetItemAt (i, scope.resetAndGetPointerAddress());

            if (scope != nullptr)
            {
                const auto url = getUrl (*scope);

                if (! url.isEmpty())
                    result.add (url);
            }
        }

        return result;
    }

    Array<URL> openDialogPreVista (bool async)
    {
        Array<URL> selections;

        if (selectsDirectories)
        {
            BROWSEINFO bi = {};
            bi.hwndOwner = (HWND) (async ? nullptr : owner->getWindowHandle());
            bi.pszDisplayName = files;
            bi.lpszTitle = title.toWideCharPointer();
            bi.lParam = (LPARAM) this;
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
                returnedString.clear();
            }

            LPMALLOC al;

            if (list != nullptr && SUCCEEDED (SHGetMalloc (&al)))
                al->Free (list);

            if (files[0] != 0)
            {
                File result (String (files.get()));

                if (returnedString.isNotEmpty())
                    result = result.getSiblingFile (returnedString);

                selections.add (URL (result));
            }
        }
        else
        {
            OPENFILENAMEW of = {};

           #ifdef OPENFILENAME_SIZE_VERSION_400W
            of.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
           #else
            of.lStructSize = sizeof (of);
           #endif

            if (files[0] != 0)
            {
                auto startingFile = File (initialPath).getChildFile (String (files.get()));
                startingFile.getFullPathName().copyToUTF16 (files, charsAvailableForResult * sizeof (WCHAR));
            }

            of.hwndOwner = (HWND) (async ? nullptr : owner->getWindowHandle());
            of.lpstrFilter = filters.getData();
            of.nFilterIndex = 1;
            of.lpstrFile = files;
            of.nMaxFile = (DWORD) charsAvailableForResult;
            of.lpstrInitialDir = initialPath.toWideCharPointer();
            of.lpstrTitle = title.toWideCharPointer();
            of.Flags = getOpenFilenameFlags (async);
            of.lCustData = (LPARAM) this;
            of.lpfnHook = &openCallback;

            if (isSave)
            {
                auto extension = getDefaultFileExtension (files.getData());

                if (extension.isNotEmpty())
                    of.lpstrDefExt = extension.toWideCharPointer();

                if (! GetSaveFileName (&of))
                    return {};
            }
            else
            {
                if (! GetOpenFileName (&of))
                    return {};
            }

            if (selectMultiple && of.nFileOffset > 0 && files[of.nFileOffset - 1] == 0)
            {
                const WCHAR* filename = files + of.nFileOffset;

                while (*filename != 0)
                {
                    selections.add (URL (File (String (files.get())).getChildFile (String (filename))));
                    filename += wcslen (filename) + 1;
                }
            }
            else if (files[0] != 0)
            {
                selections.add (URL (File (String (files.get()))));
            }
        }

        return selections;
    }

    Array<URL> openDialog (bool async)
    {
        struct Remover
        {
            explicit Remover (Win32NativeFileChooser& chooser) : item (chooser) {}
            ~Remover() { getNativeDialogList().removeValue (&item); }

            Win32NativeFileChooser& item;
        };

        const Remover remover (*this);

        if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista
            && customComponent == nullptr)
        {
            return openDialogVistaAndUp (async);
        }

        return openDialogPreVista (async);
    }

    void run() override
    {
        results = [&]
        {
            struct ScopedCoInitialize
            {
                // IUnknown_GetWindow will only succeed when instantiated in a single-thread apartment
                ScopedCoInitialize() { ignoreUnused (CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)); }
                ~ScopedCoInitialize() { CoUninitialize(); }
            };

            ScopedCoInitialize scope;

            return openDialog (true);
        }();

        auto safeOwner = owner;
        auto resultCode = results.size() > 0 ? 1 : 0;

        MessageManager::callAsync ([resultCode, safeOwner]
        {
            if (safeOwner != nullptr)
                safeOwner->exitModalState (resultCode);
        });
    }

    static HashMap<HWND, Win32NativeFileChooser*>& getNativeDialogList()
    {
        static HashMap<HWND, Win32NativeFileChooser*> dialogs;
        return dialogs;
    }

    static Win32NativeFileChooser* getNativePointerForDialog (HWND hwnd)
    {
        return getNativeDialogList()[hwnd];
    }

    //==============================================================================
    void setupFilters()
    {
        const size_t filterSpaceNumChars = 2048;
        filters.calloc (filterSpaceNumChars);

        const size_t bytesWritten = filtersString.copyToUTF16 (filters.getData(), filterSpaceNumChars * sizeof (WCHAR));
        filtersString.copyToUTF16 (filters + (bytesWritten / sizeof (WCHAR)),
                                   ((filterSpaceNumChars - 1) * sizeof (WCHAR) - bytesWritten));

        for (size_t i = 0; i < filterSpaceNumChars; ++i)
            if (filters[i] == '|')
                filters[i] = 0;
    }

    DWORD getOpenFilenameFlags (bool async)
    {
        DWORD ofFlags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_ENABLESIZING;

        if (warnAboutOverwrite)
            ofFlags |= OFN_OVERWRITEPROMPT;

        if (selectMultiple)
            ofFlags |= OFN_ALLOWMULTISELECT;

        if (async || customComponent != nullptr)
            ofFlags |= OFN_ENABLEHOOK;

        return ofFlags;
    }

    String getDefaultFileExtension (const String& filename) const
    {
        auto extension = filename.fromLastOccurrenceOf (".", false, false);

        if (extension.isEmpty())
        {
            auto tokens = StringArray::fromTokens (filtersString, ";,", "\"'");
            tokens.trim();
            tokens.removeEmptyStrings();

            if (tokens.size() == 1 && tokens[0].removeCharacters ("*.").isNotEmpty())
                extension = tokens[0].fromFirstOccurrenceOf (".", false, false);
        }

        return extension;
    }

    //==============================================================================
    void initialised (HWND hWnd)
    {
        SendMessage (hWnd, BFFM_SETSELECTIONW, TRUE, (LPARAM) initialPath.toWideCharPointer());
        initDialog (hWnd);
    }

    void validateFailed (const String& path)
    {
        returnedString = path;
    }

    void initDialog (HWND hdlg)
    {
        ScopedLock lock (deletingDialog);
        getNativeDialogList().set (hdlg, this);

        if (shouldCancel)
        {
            EndDialog (hdlg, 0);
        }
        else
        {
            nativeDialogRef.set (hdlg);

            if (customComponent != nullptr)
            {
                Component::SafePointer<Component> safeCustomComponent (customComponent.get());

                RECT dialogScreenRect, dialogClientRect;
                GetWindowRect (hdlg, &dialogScreenRect);
                GetClientRect (hdlg, &dialogClientRect);

                auto screenRectangle = Rectangle<int>::leftTopRightBottom (dialogScreenRect.left,  dialogScreenRect.top,
                                                                           dialogScreenRect.right, dialogScreenRect.bottom);

                auto scale = Desktop::getInstance().getDisplays().getDisplayForRect (screenRectangle, true)->scale;
                auto physicalComponentWidth = roundToInt (safeCustomComponent->getWidth() * scale);

                SetWindowPos (hdlg, nullptr, screenRectangle.getX(), screenRectangle.getY(),
                              physicalComponentWidth + jmax (150, screenRectangle.getWidth()),
                              jmax (150, screenRectangle.getHeight()),
                              SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);

                auto appendCustomComponent = [safeCustomComponent, dialogClientRect, scale, hdlg]() mutable
                {
                    if (safeCustomComponent != nullptr)
                    {
                        auto scaledClientRectangle = Rectangle<int>::leftTopRightBottom (dialogClientRect.left, dialogClientRect.top,
                                                                                         dialogClientRect.right, dialogClientRect.bottom) / scale;

                        safeCustomComponent->setBounds (scaledClientRectangle.getRight(), scaledClientRectangle.getY(),
                                                        safeCustomComponent->getWidth(), scaledClientRectangle.getHeight());
                        safeCustomComponent->addToDesktop (0, hdlg);
                    }
                };

                if (MessageManager::getInstance()->isThisTheMessageThread())
                    appendCustomComponent();
                else
                    MessageManager::callAsync (appendCustomComponent);
            }
        }
    }

    void destroyDialog (HWND hdlg)
    {
        ScopedLock exiting (deletingDialog);

        getNativeDialogList().remove (hdlg);
        nativeDialogRef.set (nullptr);

        if (MessageManager::getInstance()->isThisTheMessageThread())
            customComponent = nullptr;
        else
            MessageManager::callAsync ([this] { customComponent = nullptr; });
    }

    void selectionChanged (HWND hdlg)
    {
        ScopedLock lock (deletingDialog);

        if (customComponent != nullptr && ! shouldCancel)
        {
            if (FilePreviewComponent* comp = dynamic_cast<FilePreviewComponent*> (customComponent->getChildComponent (0)))
            {
                WCHAR path [MAX_PATH * 2] = { 0 };
                CommDlg_OpenSave_GetFilePath (hdlg, (LPARAM) &path, MAX_PATH);

                if (MessageManager::getInstance()->isThisTheMessageThread())
                {
                    comp->selectedFileChanged (File (path));
                }
                else
                {
                    MessageManager::callAsync ([safeComp = Component::SafePointer<FilePreviewComponent> { comp },
                                                selectedFile = File { path }]() mutable
                                               {
                                                    if (safeComp != nullptr)
                                                        safeComp->selectedFileChanged (selectedFile);
                                               });
                }
            }
        }
    }

    //==============================================================================
    static int CALLBACK browseCallbackProc (HWND hWnd, UINT msg, LPARAM lParam, LPARAM lpData)
    {
        auto* self = reinterpret_cast<Win32NativeFileChooser*> (lpData);

        switch (msg)
        {
            case BFFM_INITIALIZED:       self->initialised (hWnd);                             break;
            case BFFM_VALIDATEFAILEDW:   self->validateFailed (String ((LPCWSTR)     lParam)); break;
            case BFFM_VALIDATEFAILEDA:   self->validateFailed (String ((const char*) lParam)); break;
            default: break;
        }

        return 0;
    }

    static UINT_PTR CALLBACK openCallback (HWND hwnd, UINT uiMsg, WPARAM /*wParam*/, LPARAM lParam)
    {
        auto hdlg = getDialogFromHWND (hwnd);

        switch (uiMsg)
        {
            case WM_INITDIALOG:
            {
                if (auto* self = reinterpret_cast<Win32NativeFileChooser*> (((OPENFILENAMEW*) lParam)->lCustData))
                    self->initDialog (hdlg);

                break;
            }

            case WM_DESTROY:
            {
                if (auto* self = getNativeDialogList()[hdlg])
                    self->destroyDialog (hdlg);

                break;
            }

            case WM_NOTIFY:
            {
                auto ofn = reinterpret_cast<LPOFNOTIFY> (lParam);

                if (ofn->hdr.code == CDN_SELCHANGE)
                    if (auto* self = reinterpret_cast<Win32NativeFileChooser*> (ofn->lpOFN->lCustData))
                        self->selectionChanged (hdlg);

                break;
            }

            default:
                break;
        }

        return 0;
    }

    static HWND getDialogFromHWND (HWND hwnd)
    {
        if (hwnd == nullptr)
            return nullptr;

        HWND dialogH = GetParent (hwnd);

        if (dialogH == nullptr)
            dialogH = hwnd;

        return dialogH;
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Win32NativeFileChooser)
};

class FileChooser::Native     : public std::enable_shared_from_this<Native>,
                                public Component,
                                public FileChooser::Pimpl
{
public:
    Native (FileChooser& fileChooser, int flagsIn, FilePreviewComponent* previewComp)
        : owner (fileChooser),
          nativeFileChooser (std::make_unique<Win32NativeFileChooser> (this, flagsIn, previewComp, fileChooser.startingFile,
                                                                       fileChooser.title, fileChooser.filters))
    {
        auto mainMon = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;

        setBounds (mainMon.getX() + mainMon.getWidth() / 4,
                   mainMon.getY() + mainMon.getHeight() / 4,
                   0, 0);

        setOpaque (true);
        setAlwaysOnTop (juce_areThereAnyAlwaysOnTopWindows());
        addToDesktop (0);
    }

    ~Native() override
    {
        exitModalState (0);
        nativeFileChooser->cancel();
    }

    void launch() override
    {
        std::weak_ptr<Native> safeThis = shared_from_this();

        enterModalState (true, ModalCallbackFunction::create ([safeThis] (int)
        {
            if (auto locked = safeThis.lock())
                locked->owner.finished (locked->nativeFileChooser->results);
        }));

        nativeFileChooser->open (true);
    }

    void runModally() override
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        enterModalState (true);
        nativeFileChooser->open (false);
        exitModalState (nativeFileChooser->results.size() > 0 ? 1 : 0);
        nativeFileChooser->cancel();

        owner.finished (nativeFileChooser->results);
       #else
        jassertfalse;
       #endif
    }

    bool canModalEventBeSentToComponent (const Component* targetComponent) override
    {
        if (targetComponent == nullptr)
            return false;

        if (targetComponent == nativeFileChooser->getCustomComponent())
            return true;

        return targetComponent->findParentComponentOfClass<FilePreviewComponent>() != nullptr;
    }

private:
    FileChooser& owner;
    std::shared_ptr<Win32NativeFileChooser> nativeFileChooser;
};

//==============================================================================
bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    return true;
   #endif
}

std::shared_ptr<FileChooser::Pimpl> FileChooser::showPlatformDialog (FileChooser& owner, int flags,
                                                                     FilePreviewComponent* preview)
{
    return std::make_shared<FileChooser::Native> (owner, flags, preview);
}

} // namespace juce
