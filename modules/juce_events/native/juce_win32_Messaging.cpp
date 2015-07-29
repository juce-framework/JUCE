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

extern HWND juce_messageWindowHandle;

typedef bool (*CheckEventBlockedByModalComps) (const MSG&);
CheckEventBlockedByModalComps isEventBlockedByModalComps = nullptr;

//==============================================================================
namespace WindowsMessageHelpers
{
    const unsigned int specialId   = WM_APP + 0x4400;
    const unsigned int broadcastId = WM_APP + 0x4403;

    const TCHAR messageWindowName[] = _T("JUCEWindow");
    ScopedPointer<HiddenMessageWindow> messageWindow;

    void dispatchMessageFromLParam (LPARAM lParam)
    {
        MessageManager::MessageBase* const message = reinterpret_cast<MessageManager::MessageBase*> (lParam);

        JUCE_TRY
        {
            message->messageCallback();
        }
        JUCE_CATCH_EXCEPTION

        message->decReferenceCount();
    }

    //==============================================================================
    LRESULT CALLBACK messageWndProc (HWND h, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept
    {
        if (h == juce_messageWindowHandle)
        {
            if (message == specialId)
            {
                // (These are trapped early in our dispatch loop, but must also be checked
                // here in case some 3rd-party code is running the dispatch loop).
                dispatchMessageFromLParam (lParam);
                return 0;
            }

            if (message == broadcastId)
            {
                const ScopedPointer<String> messageString ((String*) lParam);
                MessageManager::getInstance()->deliverBroadcastMessage (*messageString);
                return 0;
            }

            if (message == WM_COPYDATA)
            {
                const COPYDATASTRUCT* const data = reinterpret_cast<const COPYDATASTRUCT*> (lParam);

                if (data->dwData == broadcastId)
                {
                    const String messageString (CharPointer_UTF32 ((const CharPointer_UTF32::CharType*) data->lpData),
                                                data->cbData / sizeof (CharPointer_UTF32::CharType));

                    PostMessage (juce_messageWindowHandle, broadcastId, 0, (LPARAM) new String (messageString));
                    return 0;
                }
            }
        }

        return DefWindowProc (h, message, wParam, lParam);
    }

    BOOL CALLBACK broadcastEnumWindowProc (HWND hwnd, LPARAM lParam)
    {
        if (hwnd != juce_messageWindowHandle)
            reinterpret_cast<Array<HWND>*> (lParam)->add (hwnd);

        return TRUE;
    }
}

//==============================================================================
bool MessageManager::dispatchNextMessageOnSystemQueue (const bool returnIfNoPendingMessages)
{
    using namespace WindowsMessageHelpers;
    MSG m;

    if (returnIfNoPendingMessages && ! PeekMessage (&m, (HWND) 0, 0, 0, PM_NOREMOVE))
        return false;

    if (GetMessage (&m, (HWND) 0, 0, 0) >= 0)
    {
        if (m.message == specialId && m.hwnd == juce_messageWindowHandle)
        {
            dispatchMessageFromLParam (m.lParam);
        }
        else if (m.message == WM_QUIT)
        {
            if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
                app->systemRequestedQuit();
        }
        else if (isEventBlockedByModalComps == nullptr || ! isEventBlockedByModalComps (m))
        {
            if ((m.message == WM_LBUTTONDOWN || m.message == WM_RBUTTONDOWN)
                  && ! JuceWindowIdentifier::isJUCEWindow (m.hwnd))
            {
                // if it's someone else's window being clicked on, and the focus is
                // currently on a juce window, pass the kb focus over..
                HWND currentFocus = GetFocus();

                if (currentFocus == 0 || JuceWindowIdentifier::isJUCEWindow (currentFocus))
                    SetFocus (m.hwnd);
            }

            TranslateMessage (&m);
            DispatchMessage (&m);
        }
    }

    return true;
}

bool MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    message->incReferenceCount();
    return PostMessage (juce_messageWindowHandle, WindowsMessageHelpers::specialId, 0, (LPARAM) message) != 0;
}

void MessageManager::broadcastMessage (const String& value)
{
    Array<HWND> windows;
    EnumWindows (&WindowsMessageHelpers::broadcastEnumWindowProc, (LPARAM) &windows);

    const String localCopy (value);

    COPYDATASTRUCT data;
    data.dwData = WindowsMessageHelpers::broadcastId;
    data.cbData = (localCopy.length() + 1) * sizeof (CharPointer_UTF32::CharType);
    data.lpData = (void*) localCopy.toUTF32().getAddress();

    for (int i = windows.size(); --i >= 0;)
    {
        HWND hwnd = windows.getUnchecked(i);

        TCHAR windowName[64] = { 0 }; // no need to read longer strings than this
        GetWindowText (hwnd, windowName, 63);

        if (String (windowName) == WindowsMessageHelpers::messageWindowName)
        {
            DWORD_PTR result;
            SendMessageTimeout (hwnd, WM_COPYDATA,
                                (WPARAM) juce_messageWindowHandle,
                                (LPARAM) &data,
                                SMTO_BLOCK | SMTO_ABORTIFHUNG, 8000, &result);
        }
    }
}

//==============================================================================
void MessageManager::doPlatformSpecificInitialisation()
{
    OleInitialize (0);

    using namespace WindowsMessageHelpers;
    messageWindow = new HiddenMessageWindow (messageWindowName, (WNDPROC) messageWndProc);
    juce_messageWindowHandle = messageWindow->getHWND();
}

void MessageManager::doPlatformSpecificShutdown()
{
    WindowsMessageHelpers::messageWindow = nullptr;

    OleUninitialize();
}

//==============================================================================
struct MountedVolumeListChangeDetector::Pimpl   : private DeviceChangeDetector
{
    Pimpl (MountedVolumeListChangeDetector& d) : DeviceChangeDetector (L"MountedVolumeList"), owner (d)
    {
        File::findFileSystemRoots (lastVolumeList);
    }

    void systemDeviceChanged() override
    {
        Array<File> newList;
        File::findFileSystemRoots (newList);

        if (lastVolumeList != newList)
        {
            lastVolumeList = newList;
            owner.mountedVolumeListChanged();
        }
    }

    MountedVolumeListChangeDetector& owner;
    Array<File> lastVolumeList;
};

MountedVolumeListChangeDetector::MountedVolumeListChangeDetector()  { pimpl = new Pimpl (*this); }
MountedVolumeListChangeDetector::~MountedVolumeListChangeDetector() {}
