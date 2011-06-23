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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

namespace ActiveXHelpers
{
    //==============================================================================
    class JuceIStorage   : public ComBaseClassHelper <IStorage>
    {
    public:
        JuceIStorage() {}

        HRESULT __stdcall CreateStream (const WCHAR*, DWORD, DWORD, DWORD, IStream**)           { return E_NOTIMPL; }
        HRESULT __stdcall OpenStream (const WCHAR*, void*, DWORD, DWORD, IStream**)             { return E_NOTIMPL; }
        HRESULT __stdcall CreateStorage (const WCHAR*, DWORD, DWORD, DWORD, IStorage**)         { return E_NOTIMPL; }
        HRESULT __stdcall OpenStorage (const WCHAR*, IStorage*, DWORD, SNB, DWORD, IStorage**)  { return E_NOTIMPL; }
        HRESULT __stdcall CopyTo (DWORD, IID const*, SNB, IStorage*)                            { return E_NOTIMPL; }
        HRESULT __stdcall MoveElementTo (const OLECHAR*,IStorage*, const OLECHAR*, DWORD)       { return E_NOTIMPL; }
        HRESULT __stdcall Commit (DWORD)                                                        { return E_NOTIMPL; }
        HRESULT __stdcall Revert()                                                              { return E_NOTIMPL; }
        HRESULT __stdcall EnumElements (DWORD, void*, DWORD, IEnumSTATSTG**)                    { return E_NOTIMPL; }
        HRESULT __stdcall DestroyElement (const OLECHAR*)                                       { return E_NOTIMPL; }
        HRESULT __stdcall RenameElement (const WCHAR*, const WCHAR*)                            { return E_NOTIMPL; }
        HRESULT __stdcall SetElementTimes (const WCHAR*, FILETIME const*, FILETIME const*, FILETIME const*)    { return E_NOTIMPL; }
        HRESULT __stdcall SetClass (REFCLSID)                                                   { return S_OK; }
        HRESULT __stdcall SetStateBits (DWORD, DWORD)                                           { return E_NOTIMPL; }
        HRESULT __stdcall Stat (STATSTG*, DWORD)                                                { return E_NOTIMPL; }
    };

    //==============================================================================
    class JuceOleInPlaceFrame   : public ComBaseClassHelper <IOleInPlaceFrame>
    {
    public:
        JuceOleInPlaceFrame (HWND window_)   : window (window_) {}

        HRESULT __stdcall GetWindow (HWND* lphwnd)                      { *lphwnd = window; return S_OK; }
        HRESULT __stdcall ContextSensitiveHelp (BOOL)                   { return E_NOTIMPL; }
        HRESULT __stdcall GetBorder (LPRECT)                            { return E_NOTIMPL; }
        HRESULT __stdcall RequestBorderSpace (LPCBORDERWIDTHS)          { return E_NOTIMPL; }
        HRESULT __stdcall SetBorderSpace (LPCBORDERWIDTHS)              { return E_NOTIMPL; }
        HRESULT __stdcall SetActiveObject (IOleInPlaceActiveObject*, LPCOLESTR)     { return S_OK; }
        HRESULT __stdcall InsertMenus (HMENU, LPOLEMENUGROUPWIDTHS)     { return E_NOTIMPL; }
        HRESULT __stdcall SetMenu (HMENU, HOLEMENU, HWND)               { return S_OK; }
        HRESULT __stdcall RemoveMenus (HMENU)                           { return E_NOTIMPL; }
        HRESULT __stdcall SetStatusText (LPCOLESTR)                     { return S_OK; }
        HRESULT __stdcall EnableModeless (BOOL)                         { return S_OK; }
        HRESULT __stdcall TranslateAccelerator (LPMSG, WORD)            { return E_NOTIMPL; }

    private:
        HWND window;
    };

    //==============================================================================
    class JuceIOleInPlaceSite   : public ComBaseClassHelper <IOleInPlaceSite>
    {
    public:
        JuceIOleInPlaceSite (HWND window_)
            : window (window_),
              frame (new JuceOleInPlaceFrame (window))
        {}

        ~JuceIOleInPlaceSite()
        {
            frame->Release();
        }

        HRESULT __stdcall GetWindow (HWND* lphwnd)      { *lphwnd = window; return S_OK; }
        HRESULT __stdcall ContextSensitiveHelp (BOOL)   { return E_NOTIMPL; }
        HRESULT __stdcall CanInPlaceActivate()          { return S_OK; }
        HRESULT __stdcall OnInPlaceActivate()           { return S_OK; }
        HRESULT __stdcall OnUIActivate()                { return S_OK; }

        HRESULT __stdcall GetWindowContext (LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO lpFrameInfo)
        {
            /* Note: if you call AddRef on the frame here, then some types of object (e.g. web browser control) cause leaks..
               If you don't call AddRef then others crash (e.g. QuickTime).. Bit of a catch-22, so letting it leak is probably preferable.
            */
            if (lplpFrame != nullptr) { frame->AddRef(); *lplpFrame = frame; }
            if (lplpDoc != nullptr)   *lplpDoc = 0;
            lpFrameInfo->fMDIApp = FALSE;
            lpFrameInfo->hwndFrame = window;
            lpFrameInfo->haccel = 0;
            lpFrameInfo->cAccelEntries = 0;
            return S_OK;
        }

        HRESULT __stdcall Scroll (SIZE)                 { return E_NOTIMPL; }
        HRESULT __stdcall OnUIDeactivate (BOOL)         { return S_OK; }
        HRESULT __stdcall OnInPlaceDeactivate()         { return S_OK; }
        HRESULT __stdcall DiscardUndoState()            { return E_NOTIMPL; }
        HRESULT __stdcall DeactivateAndUndo()           { return E_NOTIMPL; }
        HRESULT __stdcall OnPosRectChange (LPCRECT)     { return S_OK; }

    private:
        HWND window;
        JuceOleInPlaceFrame* frame;
    };

    //==============================================================================
    class JuceIOleClientSite  : public ComBaseClassHelper <IOleClientSite>
    {
    public:
        JuceIOleClientSite (HWND window)
            : inplaceSite (new JuceIOleInPlaceSite (window))
        {}

        ~JuceIOleClientSite()
        {
            inplaceSite->Release();
        }

        HRESULT __stdcall QueryInterface (REFIID type, void** result)
        {
            if (type == IID_IOleInPlaceSite)
            {
                inplaceSite->AddRef();
                *result = static_cast <IOleInPlaceSite*> (inplaceSite);
                return S_OK;
            }

            return ComBaseClassHelper <IOleClientSite>::QueryInterface (type, result);
        }

        HRESULT __stdcall SaveObject()                                  { return E_NOTIMPL; }
        HRESULT __stdcall GetMoniker (DWORD, DWORD, IMoniker**)         { return E_NOTIMPL; }
        HRESULT __stdcall GetContainer (LPOLECONTAINER* ppContainer)    { *ppContainer = 0; return E_NOINTERFACE; }
        HRESULT __stdcall ShowObject()                                  { return S_OK; }
        HRESULT __stdcall OnShowWindow (BOOL)                           { return E_NOTIMPL; }
        HRESULT __stdcall RequestNewObjectLayout()                      { return E_NOTIMPL; }

    private:
        JuceIOleInPlaceSite* inplaceSite;
    };

    //==============================================================================
    static Array<ActiveXControlComponent*> activeXComps;

    HWND getHWND (const ActiveXControlComponent* const component)
    {
        HWND hwnd = 0;

        const IID iid = IID_IOleWindow;
        IOleWindow* const window = (IOleWindow*) component->queryInterface (&iid);

        if (window != nullptr)
        {
            window->GetWindow (&hwnd);
            window->Release();
        }

        return hwnd;
    }

    void offerActiveXMouseEventToPeer (ComponentPeer* const peer, HWND hwnd, UINT message, LPARAM lParam)
    {
        RECT activeXRect, peerRect;
        GetWindowRect (hwnd, &activeXRect);
        GetWindowRect ((HWND) peer->getNativeHandle(), &peerRect);

        const Point<int> mousePos (GET_X_LPARAM (lParam) + activeXRect.left - peerRect.left,
                                   GET_Y_LPARAM (lParam) + activeXRect.top - peerRect.top);
        const int64 mouseEventTime = Win32ComponentPeer::getMouseEventTime();

        ModifierKeys::getCurrentModifiersRealtime(); // to update the mouse button flags

        switch (message)
        {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            peer->handleMouseEvent (0, mousePos, Win32ComponentPeer::currentModifiers, mouseEventTime);
            break;

        default:
            break;
        }
    }
}

//==============================================================================
class ActiveXControlComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    //==============================================================================
    Pimpl (HWND hwnd, ActiveXControlComponent& owner_)
        : ComponentMovementWatcher (&owner_),
          owner (owner_),
          controlHWND (0),
          storage (new ActiveXHelpers::JuceIStorage()),
          clientSite (new ActiveXHelpers::JuceIOleClientSite (hwnd)),
          control (nullptr)
    {
    }

    ~Pimpl()
    {
        if (control != nullptr)
        {
            control->Close (OLECLOSE_NOSAVE);
            control->Release();
        }

        clientSite->Release();
        storage->Release();
    }

    //==============================================================================
    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        Component* const topComp = owner.getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
            const Point<int> pos (topComp->getLocalPoint (&owner, Point<int>()));
            owner.setControlBounds (Rectangle<int> (pos.getX(), pos.getY(), owner.getWidth(), owner.getHeight()));
        }
    }

    void componentPeerChanged()
    {
        componentMovedOrResized (true, true);
    }

    void componentVisibilityChanged()
    {
        owner.setControlVisible (owner.isShowing());
        componentPeerChanged();
    }

    // intercepts events going to an activeX control, so we can sneakily use the mouse events
    static LRESULT CALLBACK activeXHookWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        for (int i = ActiveXHelpers::activeXComps.size(); --i >= 0;)
        {
            const ActiveXControlComponent* const ax = ActiveXHelpers::activeXComps.getUnchecked(i);

            if (ax->control != nullptr && ax->control->controlHWND == hwnd)
            {
                switch (message)
                {
                case WM_MOUSEMOVE:
                case WM_LBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_MBUTTONUP:
                case WM_RBUTTONUP:
                case WM_LBUTTONDBLCLK:
                case WM_MBUTTONDBLCLK:
                case WM_RBUTTONDBLCLK:
                    if (ax->isShowing())
                    {
                        ComponentPeer* const peer = ax->getPeer();

                        if (peer != nullptr)
                        {
                            ActiveXHelpers::offerActiveXMouseEventToPeer (peer, hwnd, message, lParam);

                            if (! ax->areMouseEventsAllowed())
                                return 0;
                        }
                    }
                    break;

                default:
                    break;
                }

                return CallWindowProc ((WNDPROC) ax->originalWndProc, hwnd, message, wParam, lParam);
            }
        }

        return DefWindowProc (hwnd, message, wParam, lParam);
    }

private:
    ActiveXControlComponent& owner;

public:
    HWND controlHWND;
    IStorage* storage;
    IOleClientSite* clientSite;
    IOleObject* control;
};

//==============================================================================
ActiveXControlComponent::ActiveXControlComponent()
    : originalWndProc (0),
      mouseEventsAllowed (true)
{
    ActiveXHelpers::activeXComps.add (this);
}

ActiveXControlComponent::~ActiveXControlComponent()
{
    deleteControl();
    ActiveXHelpers::activeXComps.removeValue (this);
}

void ActiveXControlComponent::paint (Graphics& g)
{
    if (control == nullptr)
        g.fillAll (Colours::lightgrey);
}

bool ActiveXControlComponent::createControl (const void* controlIID)
{
    deleteControl();
    ComponentPeer* const peer = getPeer();

    // the component must have already been added to a real window when you call this!
    jassert (dynamic_cast <Win32ComponentPeer*> (peer) != nullptr);

    if (dynamic_cast <Win32ComponentPeer*> (peer) != nullptr)
    {
        const Point<int> pos (getTopLevelComponent()->getLocalPoint (this, Point<int>()));
        HWND hwnd = (HWND) peer->getNativeHandle();

        ScopedPointer<Pimpl> newControl (new Pimpl (hwnd, *this));

        HRESULT hr;
        if ((hr = OleCreate (*(const IID*) controlIID, IID_IOleObject, 1 /*OLERENDER_DRAW*/, 0,
                             newControl->clientSite, newControl->storage,
                             (void**) &(newControl->control))) == S_OK)
        {
            newControl->control->SetHostNames (L"Juce", 0);

            if (OleSetContainedObject (newControl->control, TRUE) == S_OK)
            {
                RECT rect;
                rect.left = pos.getX();
                rect.top = pos.getY();
                rect.right = pos.getX() + getWidth();
                rect.bottom = pos.getY() + getHeight();

                if (newControl->control->DoVerb (OLEIVERB_SHOW, 0, newControl->clientSite, 0, hwnd, &rect) == S_OK)
                {
                    control = newControl;
                    setControlBounds (Rectangle<int> (pos.getX(), pos.getY(), getWidth(), getHeight()));

                    control->controlHWND = ActiveXHelpers::getHWND (this);

                    if (control->controlHWND != 0)
                    {
                        originalWndProc = (void*) (pointer_sized_int) GetWindowLongPtr ((HWND) control->controlHWND, GWLP_WNDPROC);
                        SetWindowLongPtr ((HWND) control->controlHWND, GWLP_WNDPROC, (LONG_PTR) Pimpl::activeXHookWndProc);
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

void ActiveXControlComponent::deleteControl()
{
    control = nullptr;
    originalWndProc = 0;
}

void* ActiveXControlComponent::queryInterface (const void* iid) const
{
    void* result = nullptr;

    if (control != nullptr && control->control != nullptr
         && SUCCEEDED (control->control->QueryInterface (*(const IID*) iid, &result)))
        return result;

    return nullptr;
}

void ActiveXControlComponent::setControlBounds (const Rectangle<int>& newBounds) const
{
    if (control->controlHWND != 0)
        MoveWindow (control->controlHWND, newBounds.getX(), newBounds.getY(), newBounds.getWidth(), newBounds.getHeight(), TRUE);
}

void ActiveXControlComponent::setControlVisible (const bool shouldBeVisible) const
{
    if (control->controlHWND != 0)
        ShowWindow (control->controlHWND, shouldBeVisible ? SW_SHOWNA : SW_HIDE);
}

void ActiveXControlComponent::setMouseEventsAllowed (const bool eventsCanReachControl)
{
    mouseEventsAllowed = eventsCanReachControl;
}

#endif
