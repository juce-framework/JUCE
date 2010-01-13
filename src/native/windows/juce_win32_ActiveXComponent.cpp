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
class JuceIStorage   : public IStorage
{
    int refCount;

public:
    JuceIStorage() : refCount (1) {}

    virtual ~JuceIStorage()
    {
        jassert (refCount == 0);
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown || id == IID_IStorage)
        {
            AddRef();
            *result = this;
            return S_OK;
        }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

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

    juce_UseDebuggingNewOperator
};


class JuceOleInPlaceFrame   : public IOleInPlaceFrame
{
    int refCount;
    HWND window;

public:
    JuceOleInPlaceFrame (HWND window_)
        : refCount (1),
          window (window_)
    {
    }

    virtual ~JuceOleInPlaceFrame()
    {
        jassert (refCount == 0);
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown || id == IID_IOleInPlaceFrame)
        {
            AddRef();
            *result = this;
            return S_OK;
        }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

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
    HRESULT __stdcall TranslateAccelerator(LPMSG, WORD)             { return E_NOTIMPL; }

    juce_UseDebuggingNewOperator
};


class JuceIOleInPlaceSite   : public IOleInPlaceSite
{
    int refCount;
    HWND window;
    JuceOleInPlaceFrame* frame;

public:
    JuceIOleInPlaceSite (HWND window_)
        : refCount (1),
          window (window_)
    {
        frame = new JuceOleInPlaceFrame (window);
    }

    virtual ~JuceIOleInPlaceSite()
    {
        jassert (refCount == 0);
        frame->Release();
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown || id == IID_IOleInPlaceSite)
        {
            AddRef();
            *result = this;
            return S_OK;
        }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

    HRESULT __stdcall GetWindow (HWND* lphwnd)      { *lphwnd = window; return S_OK; }
    HRESULT __stdcall ContextSensitiveHelp (BOOL)   { return E_NOTIMPL; }
    HRESULT __stdcall CanInPlaceActivate()          { return S_OK; }
    HRESULT __stdcall OnInPlaceActivate()           { return S_OK; }
    HRESULT __stdcall OnUIActivate()                { return S_OK; }

    HRESULT __stdcall GetWindowContext (LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO lpFrameInfo)
    {
        // frame->AddRef();   // MS docs are unclear about whether this is needed, but it seems to lead to a memory leak..
        *lplpFrame = frame;
        *lplpDoc = 0;
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

    juce_UseDebuggingNewOperator
};


class JuceIOleClientSite  : public IOleClientSite
{
    int refCount;
    JuceIOleInPlaceSite* inplaceSite;

public:
    JuceIOleClientSite (HWND window)
        : refCount (1)
    {
        inplaceSite = new JuceIOleInPlaceSite (window);
    }

    virtual ~JuceIOleClientSite()
    {
        jassert (refCount == 0);
        inplaceSite->Release();
    }

    HRESULT __stdcall QueryInterface (REFIID id, void __RPC_FAR* __RPC_FAR* result)
    {
        if (id == IID_IUnknown || id == IID_IOleClientSite)
        {
            AddRef();
            *result = this;
            return S_OK;
        }
        else if (id == IID_IOleInPlaceSite)
        {
            inplaceSite->AddRef();
            *result = inplaceSite;
            return S_OK;
        }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

    HRESULT __stdcall SaveObject()                                  { return E_NOTIMPL; }
    HRESULT __stdcall GetMoniker (DWORD, DWORD, IMoniker**)         { return E_NOTIMPL; }
    HRESULT __stdcall GetContainer (LPOLECONTAINER* ppContainer)    { *ppContainer = 0; return E_NOINTERFACE; }
    HRESULT __stdcall ShowObject()                                  { return S_OK; }
    HRESULT __stdcall OnShowWindow (BOOL)                           { return E_NOTIMPL; }
    HRESULT __stdcall RequestNewObjectLayout()                      { return E_NOTIMPL; }

    juce_UseDebuggingNewOperator
};

//==============================================================================
class ActiveXControlData  : public ComponentMovementWatcher
{
    ActiveXControlComponent* const owner;
    bool wasShowing;

public:
    HWND controlHWND;
    IStorage* storage;
    IOleClientSite* clientSite;
    IOleObject* control;

    //==============================================================================
    ActiveXControlData (HWND hwnd,
                        ActiveXControlComponent* const owner_)
        : ComponentMovementWatcher (owner_),
          owner (owner_),
          wasShowing (owner_ != 0 && owner_->isShowing()),
          controlHWND (0),
          storage (new JuceIStorage()),
          clientSite (new JuceIOleClientSite (hwnd)),
          control (0)
    {
    }

    ~ActiveXControlData()
    {
        if (control != 0)
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
        Component* const topComp = owner->getTopLevelComponent();

        if (topComp->getPeer() != 0)
        {
            int x = 0, y = 0;
            owner->relativePositionToOtherComponent (topComp, x, y);

            owner->setControlBounds (Rectangle (x, y, owner->getWidth(), owner->getHeight()));
        }
    }

    void componentPeerChanged()
    {
        const bool isShowingNow = owner->isShowing();

        if (wasShowing != isShowingNow)
        {
            wasShowing = isShowingNow;
            owner->setControlVisible (isShowingNow);
        }

        componentMovedOrResized (true, true);
    }

    void componentVisibilityChanged (Component&)
    {
        componentPeerChanged();
    }

    static bool doesWindowMatch (const ActiveXControlComponent* const ax, HWND hwnd)
    {
        return ((ActiveXControlData*) ax->control) != 0
                 && ((ActiveXControlData*) ax->control)->controlHWND == hwnd;
    }
};

//==============================================================================
static VoidArray activeXComps;

static HWND getHWND (const ActiveXControlComponent* const component)
{
    HWND hwnd = 0;

    const IID iid = IID_IOleWindow;
    IOleWindow* const window = (IOleWindow*) component->queryInterface (&iid);

    if (window != 0)
    {
        window->GetWindow (&hwnd);
        window->Release();
    }

    return hwnd;
}

static void offerActiveXMouseEventToPeer (ComponentPeer* const peer, HWND hwnd, UINT message, LPARAM lParam)
{
    RECT activeXRect, peerRect;
    GetWindowRect (hwnd, &activeXRect);
    GetWindowRect ((HWND) peer->getNativeHandle(), &peerRect);

    const int mx = GET_X_LPARAM (lParam) + activeXRect.left - peerRect.left;
    const int my = GET_Y_LPARAM (lParam) + activeXRect.top - peerRect.top;
    const int64 mouseEventTime = getMouseEventTime();

    const int oldModifiers = currentModifiers;
    ModifierKeys::getCurrentModifiersRealtime(); // to update the mouse button flags

    switch (message)
    {
    case WM_MOUSEMOVE:
        if (ModifierKeys (currentModifiers).isAnyMouseButtonDown())
            peer->handleMouseDrag (mx, my, mouseEventTime);
        else
            peer->handleMouseMove (mx, my, mouseEventTime);
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        peer->handleMouseDown (mx, my, mouseEventTime);
        break;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        peer->handleMouseUp (oldModifiers, mx, my, mouseEventTime);
        break;

    default:
        break;
    }
}

// intercepts events going to an activeX control, so we can sneakily use the mouse events
static LRESULT CALLBACK activeXHookWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    for (int i = activeXComps.size(); --i >= 0;)
    {
        const ActiveXControlComponent* const ax = (const ActiveXControlComponent*) activeXComps.getUnchecked(i);

        if (ActiveXControlData::doesWindowMatch (ax, hwnd))
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

                    if (peer != 0)
                    {
                        offerActiveXMouseEventToPeer (peer, hwnd, message, lParam);

                        if (! ax->areMouseEventsAllowed())
                            return 0;
                    }
                }
                break;

            default:
                break;
            }

            return CallWindowProc ((WNDPROC) (ax->originalWndProc), hwnd, message, wParam, lParam);
        }
    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}

ActiveXControlComponent::ActiveXControlComponent()
    : originalWndProc (0),
      control (0),
      mouseEventsAllowed (true)
{
    activeXComps.add (this);
}

ActiveXControlComponent::~ActiveXControlComponent()
{
    deleteControl();
    activeXComps.removeValue (this);
}

void ActiveXControlComponent::paint (Graphics& g)
{
    if (control == 0)
        g.fillAll (Colours::lightgrey);
}

bool ActiveXControlComponent::createControl (const void* controlIID)
{
    deleteControl();
    ComponentPeer* const peer = getPeer();

    // the component must have already been added to a real window when you call this!
    jassert (dynamic_cast <Win32ComponentPeer*> (peer) != 0);

    if (dynamic_cast <Win32ComponentPeer*> (peer) != 0)
    {
        int x = 0, y = 0;
        relativePositionToOtherComponent (getTopLevelComponent(), x, y);

        HWND hwnd = (HWND) peer->getNativeHandle();

        ScopedPointer <ActiveXControlData> info (new ActiveXControlData (hwnd, this));

        HRESULT hr;
        if ((hr = OleCreate (*(const IID*) controlIID, IID_IOleObject, 1 /*OLERENDER_DRAW*/, 0,
                             info->clientSite, info->storage,
                             (void**) &(info->control))) == S_OK)
        {
            info->control->SetHostNames (L"Juce", 0);

            if (OleSetContainedObject (info->control, TRUE) == S_OK)
            {
                RECT rect;
                rect.left = x;
                rect.top = y;
                rect.right = x + getWidth();
                rect.bottom = y + getHeight();

                if (info->control->DoVerb (OLEIVERB_SHOW, 0, info->clientSite, 0, hwnd, &rect) == S_OK)
                {
                    control = info.release();
                    setControlBounds (Rectangle (x, y, getWidth(), getHeight()));

                    ((ActiveXControlData*) control)->controlHWND = getHWND (this);

                    if (((ActiveXControlData*) control)->controlHWND != 0)
                    {
                        originalWndProc = (void*) (pointer_sized_int) GetWindowLongPtr ((HWND) ((ActiveXControlData*) control)->controlHWND, GWLP_WNDPROC);
                        SetWindowLongPtr ((HWND) ((ActiveXControlData*) control)->controlHWND, GWLP_WNDPROC, (LONG_PTR) activeXHookWndProc);
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
    ActiveXControlData* const info = (ActiveXControlData*) control;

    if (info != 0)
    {
        delete info;
        control = 0;
        originalWndProc = 0;
    }
}

void* ActiveXControlComponent::queryInterface (const void* iid) const
{
    ActiveXControlData* const info = (ActiveXControlData*) control;

    void* result = 0;

    if (info != 0 && info->control != 0
         && info->control->QueryInterface (*(const IID*) iid, &result) == S_OK)
        return result;

    return 0;
}

void ActiveXControlComponent::setControlBounds (const Rectangle& newBounds) const
{
    HWND hwnd = ((ActiveXControlData*) control)->controlHWND;

    if (hwnd != 0)
        MoveWindow (hwnd, newBounds.getX(), newBounds.getY(), newBounds.getWidth(), newBounds.getHeight(), TRUE);
}

void ActiveXControlComponent::setControlVisible (const bool shouldBeVisible) const
{
    HWND hwnd = ((ActiveXControlData*) control)->controlHWND;

    if (hwnd != 0)
        ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);
}

void ActiveXControlComponent::setMouseEventsAllowed (const bool eventsCanReachControl)
{
    mouseEventsAllowed = eventsCanReachControl;
}

#endif
