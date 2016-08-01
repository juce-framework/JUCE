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

extern int64 getMouseEventTime();

#if JUCE_MINGW
 #define JUCE_COMCLASS(name, guid) \
    template<> struct UUIDGetter<::name>   { static CLSID get() { return uuidFromString (guid); } };

 #ifdef __uuidof
  #undef __uuidof
 #endif

 #define __uuidof(cls) UUIDGetter<::cls>::get()

#else
 #define JUCE_COMCLASS(name, guid)
#endif

JUCE_COMCLASS (IOleObject,                "00000112-0000-0000-C000-000000000046")
JUCE_COMCLASS (IOleWindow,                "00000114-0000-0000-C000-000000000046")
JUCE_COMCLASS (IOleInPlaceSite,           "00000119-0000-0000-C000-000000000046")

namespace ActiveXHelpers
{
    //==============================================================================
    class JuceIStorage   : public ComBaseClassHelper <IStorage>
    {
    public:
        JuceIStorage() {}

        JUCE_COMRESULT CreateStream (const WCHAR*, DWORD, DWORD, DWORD, IStream**)           { return E_NOTIMPL; }
        JUCE_COMRESULT OpenStream (const WCHAR*, void*, DWORD, DWORD, IStream**)             { return E_NOTIMPL; }
        JUCE_COMRESULT CreateStorage (const WCHAR*, DWORD, DWORD, DWORD, IStorage**)         { return E_NOTIMPL; }
        JUCE_COMRESULT OpenStorage (const WCHAR*, IStorage*, DWORD, SNB, DWORD, IStorage**)  { return E_NOTIMPL; }
        JUCE_COMRESULT CopyTo (DWORD, IID const*, SNB, IStorage*)                            { return E_NOTIMPL; }
        JUCE_COMRESULT MoveElementTo (const OLECHAR*,IStorage*, const OLECHAR*, DWORD)       { return E_NOTIMPL; }
        JUCE_COMRESULT Commit (DWORD)                                                        { return E_NOTIMPL; }
        JUCE_COMRESULT Revert()                                                              { return E_NOTIMPL; }
        JUCE_COMRESULT EnumElements (DWORD, void*, DWORD, IEnumSTATSTG**)                    { return E_NOTIMPL; }
        JUCE_COMRESULT DestroyElement (const OLECHAR*)                                       { return E_NOTIMPL; }
        JUCE_COMRESULT RenameElement (const WCHAR*, const WCHAR*)                            { return E_NOTIMPL; }
        JUCE_COMRESULT SetElementTimes (const WCHAR*, FILETIME const*, FILETIME const*, FILETIME const*)    { return E_NOTIMPL; }
        JUCE_COMRESULT SetClass (REFCLSID)                                                   { return S_OK; }
        JUCE_COMRESULT SetStateBits (DWORD, DWORD)                                           { return E_NOTIMPL; }
        JUCE_COMRESULT Stat (STATSTG*, DWORD)                                                { return E_NOTIMPL; }
    };

    //==============================================================================
    class JuceOleInPlaceFrame   : public ComBaseClassHelper <IOleInPlaceFrame>
    {
    public:
        JuceOleInPlaceFrame (HWND hwnd)   : window (hwnd) {}

        JUCE_COMRESULT GetWindow (HWND* lphwnd)                      { *lphwnd = window; return S_OK; }
        JUCE_COMRESULT ContextSensitiveHelp (BOOL)                   { return E_NOTIMPL; }
        JUCE_COMRESULT GetBorder (LPRECT)                            { return E_NOTIMPL; }
        JUCE_COMRESULT RequestBorderSpace (LPCBORDERWIDTHS)          { return E_NOTIMPL; }
        JUCE_COMRESULT SetBorderSpace (LPCBORDERWIDTHS)              { return E_NOTIMPL; }
        JUCE_COMRESULT SetActiveObject (IOleInPlaceActiveObject*, LPCOLESTR)     { return S_OK; }
        JUCE_COMRESULT InsertMenus (HMENU, LPOLEMENUGROUPWIDTHS)     { return E_NOTIMPL; }
        JUCE_COMRESULT SetMenu (HMENU, HOLEMENU, HWND)               { return S_OK; }
        JUCE_COMRESULT RemoveMenus (HMENU)                           { return E_NOTIMPL; }
        JUCE_COMRESULT SetStatusText (LPCOLESTR)                     { return S_OK; }
        JUCE_COMRESULT EnableModeless (BOOL)                         { return S_OK; }
        JUCE_COMRESULT TranslateAccelerator (LPMSG, WORD)            { return E_NOTIMPL; }

    private:
        HWND window;
    };

    //==============================================================================
    class JuceIOleInPlaceSite   : public ComBaseClassHelper <IOleInPlaceSite>
    {
    public:
        JuceIOleInPlaceSite (HWND hwnd)
            : window (hwnd),
              frame (new JuceOleInPlaceFrame (window))
        {}

        ~JuceIOleInPlaceSite()
        {
            frame->Release();
        }

        JUCE_COMRESULT GetWindow (HWND* lphwnd)      { *lphwnd = window; return S_OK; }
        JUCE_COMRESULT ContextSensitiveHelp (BOOL)   { return E_NOTIMPL; }
        JUCE_COMRESULT CanInPlaceActivate()          { return S_OK; }
        JUCE_COMRESULT OnInPlaceActivate()           { return S_OK; }
        JUCE_COMRESULT OnUIActivate()                { return S_OK; }

        JUCE_COMRESULT GetWindowContext (LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO lpFrameInfo)
        {
            /* Note: if you call AddRef on the frame here, then some types of object (e.g. web browser control) cause leaks..
               If you don't call AddRef then others crash (e.g. QuickTime).. Bit of a catch-22, so letting it leak is probably preferable.
            */
            if (lplpFrame != nullptr) { frame->AddRef(); *lplpFrame = frame; }
            if (lplpDoc != nullptr)   *lplpDoc = nullptr;
            lpFrameInfo->fMDIApp = FALSE;
            lpFrameInfo->hwndFrame = window;
            lpFrameInfo->haccel = 0;
            lpFrameInfo->cAccelEntries = 0;
            return S_OK;
        }

        JUCE_COMRESULT Scroll (SIZE)                 { return E_NOTIMPL; }
        JUCE_COMRESULT OnUIDeactivate (BOOL)         { return S_OK; }
        JUCE_COMRESULT OnInPlaceDeactivate()         { return S_OK; }
        JUCE_COMRESULT DiscardUndoState()            { return E_NOTIMPL; }
        JUCE_COMRESULT DeactivateAndUndo()           { return E_NOTIMPL; }
        JUCE_COMRESULT OnPosRectChange (LPCRECT)     { return S_OK; }

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

        JUCE_COMRESULT QueryInterface (REFIID type, void** result)
        {
            if (type == __uuidof (IOleInPlaceSite))
            {
                inplaceSite->AddRef();
                *result = static_cast<IOleInPlaceSite*> (inplaceSite);
                return S_OK;
            }

            return ComBaseClassHelper <IOleClientSite>::QueryInterface (type, result);
        }

        JUCE_COMRESULT SaveObject()                                  { return E_NOTIMPL; }
        JUCE_COMRESULT GetMoniker (DWORD, DWORD, IMoniker**)         { return E_NOTIMPL; }
        JUCE_COMRESULT GetContainer (LPOLECONTAINER* ppContainer)    { *ppContainer = nullptr; return E_NOINTERFACE; }
        JUCE_COMRESULT ShowObject()                                  { return S_OK; }
        JUCE_COMRESULT OnShowWindow (BOOL)                           { return E_NOTIMPL; }
        JUCE_COMRESULT RequestNewObjectLayout()                      { return E_NOTIMPL; }

    private:
        JuceIOleInPlaceSite* inplaceSite;
    };

    //==============================================================================
    static Array<ActiveXControlComponent*> activeXComps;

    HWND getHWND (const ActiveXControlComponent* const component)
    {
        HWND hwnd = 0;
        const IID iid = __uuidof(IOleWindow);

        if (IOleWindow* const window = (IOleWindow*) component->queryInterface (&iid))
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

        switch (message)
        {
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
                peer->handleMouseEvent (0, Point<int> (GET_X_LPARAM (lParam) + activeXRect.left - peerRect.left,
                                                       GET_Y_LPARAM (lParam) + activeXRect.top  - peerRect.top).toFloat(),
                                        ModifierKeys::getCurrentModifiersRealtime(),
                                        MouseInputSource::invalidPressure,
                                        getMouseEventTime());
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
    Pimpl (HWND hwnd, ActiveXControlComponent& activeXComp)
        : ComponentMovementWatcher (&activeXComp),
          owner (activeXComp),
          controlHWND (0),
          storage (new ActiveXHelpers::JuceIStorage()),
          clientSite (new ActiveXHelpers::JuceIOleClientSite (hwnd)),
          control (nullptr),
          originalWndProc (0)
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

    void setControlBounds (const Rectangle<int>& newBounds) const
    {
        if (controlHWND != 0)
            MoveWindow (controlHWND, newBounds.getX(), newBounds.getY(), newBounds.getWidth(), newBounds.getHeight(), TRUE);
    }

    void setControlVisible (bool shouldBeVisible) const
    {
        if (controlHWND != 0)
            ShowWindow (controlHWND, shouldBeVisible ? SW_SHOWNA : SW_HIDE);
    }

    //==============================================================================
    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (ComponentPeer* const peer = owner.getTopLevelComponent()->getPeer())
            setControlBounds (peer->getAreaCoveredBy (owner));
    }

    void componentPeerChanged() override
    {
        componentMovedOrResized (true, true);
    }

    void componentVisibilityChanged() override
    {
        setControlVisible (owner.isShowing());
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
                        if (ComponentPeer* const peer = ax->getPeer())
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

                return CallWindowProc (ax->control->originalWndProc, hwnd, message, wParam, lParam);
            }
        }

        return DefWindowProc (hwnd, message, wParam, lParam);
    }

    ActiveXControlComponent& owner;
    HWND controlHWND;
    IStorage* storage;
    IOleClientSite* clientSite;
    IOleObject* control;
    WNDPROC originalWndProc;
};

//==============================================================================
ActiveXControlComponent::ActiveXControlComponent()
    : mouseEventsAllowed (true)
{
    ActiveXHelpers::activeXComps.add (this);
}

ActiveXControlComponent::~ActiveXControlComponent()
{
    deleteControl();
    ActiveXHelpers::activeXComps.removeFirstMatchingValue (this);
}

void ActiveXControlComponent::paint (Graphics& g)
{
    if (control == nullptr)
        g.fillAll (Colours::lightgrey);
}

bool ActiveXControlComponent::createControl (const void* controlIID)
{
    deleteControl();

    if (ComponentPeer* const peer = getPeer())
    {
        const Rectangle<int> controlBounds (peer->getAreaCoveredBy (*this));

        HWND hwnd = (HWND) peer->getNativeHandle();

        ScopedPointer<Pimpl> newControl (new Pimpl (hwnd, *this));

        HRESULT hr;
        if ((hr = OleCreate (*(const IID*) controlIID, __uuidof (IOleObject), 1 /*OLERENDER_DRAW*/, 0,
                             newControl->clientSite, newControl->storage,
                             (void**) &(newControl->control))) == S_OK)
        {
            newControl->control->SetHostNames (L"JUCE", 0);

            if (OleSetContainedObject (newControl->control, TRUE) == S_OK)
            {
                RECT rect;
                rect.left   = controlBounds.getX();
                rect.top    = controlBounds.getY();
                rect.right  = controlBounds.getRight();
                rect.bottom = controlBounds.getBottom();

                if (newControl->control->DoVerb (OLEIVERB_SHOW, 0, newControl->clientSite, 0, hwnd, &rect) == S_OK)
                {
                    control = newControl;
                    control->controlHWND = ActiveXHelpers::getHWND (this);

                    if (control->controlHWND != 0)
                    {
                        control->setControlBounds (controlBounds);

                        control->originalWndProc = (WNDPROC) GetWindowLongPtr ((HWND) control->controlHWND, GWLP_WNDPROC);
                        SetWindowLongPtr ((HWND) control->controlHWND, GWLP_WNDPROC, (LONG_PTR) Pimpl::activeXHookWndProc);
                    }

                    return true;
                }
            }
        }
    }
    else
    {
        // the component must have already been added to a real window when you call this!
        jassertfalse;
    }

    return false;
}

void ActiveXControlComponent::deleteControl()
{
    control = nullptr;
}

void* ActiveXControlComponent::queryInterface (const void* iid) const
{
    void* result = nullptr;

    if (control != nullptr && control->control != nullptr
         && SUCCEEDED (control->control->QueryInterface (*(const IID*) iid, &result)))
        return result;

    return nullptr;
}

void ActiveXControlComponent::setMouseEventsAllowed (const bool eventsCanReachControl)
{
    mouseEventsAllowed = eventsCanReachControl;
}
