/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

extern int64 getMouseEventTime();

JUCE_DECLARE_UUID_GETTER (IOleObject,       "00000112-0000-0000-C000-000000000046")
JUCE_DECLARE_UUID_GETTER (IOleWindow,       "00000114-0000-0000-C000-000000000046")
JUCE_DECLARE_UUID_GETTER (IOleInPlaceSite,  "00000119-0000-0000-C000-000000000046")

namespace ActiveXHelpers
{
    //==============================================================================
    struct JuceIStorage final : public ComBaseClassHelper<IStorage>
    {
        JuceIStorage() = default;

        JUCE_COMRESULT CreateStream (const WCHAR*, DWORD, DWORD, DWORD, IStream**)                        override { return E_NOTIMPL; }
        JUCE_COMRESULT OpenStream (const WCHAR*, void*, DWORD, DWORD, IStream**)                          override { return E_NOTIMPL; }
        JUCE_COMRESULT CreateStorage (const WCHAR*, DWORD, DWORD, DWORD, IStorage**)                      override { return E_NOTIMPL; }
        JUCE_COMRESULT OpenStorage (const WCHAR*, IStorage*, DWORD, SNB, DWORD, IStorage**)               override { return E_NOTIMPL; }
        JUCE_COMRESULT CopyTo (DWORD, IID const*, SNB, IStorage*)                                         override { return E_NOTIMPL; }
        JUCE_COMRESULT MoveElementTo (const OLECHAR*,IStorage*, const OLECHAR*, DWORD)                    override { return E_NOTIMPL; }
        JUCE_COMRESULT Commit (DWORD)                                                                     override { return E_NOTIMPL; }
        JUCE_COMRESULT Revert()                                                                           override { return E_NOTIMPL; }
        JUCE_COMRESULT EnumElements (DWORD, void*, DWORD, IEnumSTATSTG**)                                 override { return E_NOTIMPL; }
        JUCE_COMRESULT DestroyElement (const OLECHAR*)                                                    override { return E_NOTIMPL; }
        JUCE_COMRESULT RenameElement (const WCHAR*, const WCHAR*)                                         override { return E_NOTIMPL; }
        JUCE_COMRESULT SetElementTimes (const WCHAR*, FILETIME const*, FILETIME const*, FILETIME const*)  override { return E_NOTIMPL; }
        JUCE_COMRESULT SetClass (REFCLSID)                                                                override { return S_OK; }
        JUCE_COMRESULT SetStateBits (DWORD, DWORD)                                                        override { return E_NOTIMPL; }
        JUCE_COMRESULT Stat (STATSTG*, DWORD)                                                             override { return E_NOTIMPL; }
    };

    //==============================================================================
    struct JuceOleInPlaceFrame final : public ComBaseClassHelper<IOleInPlaceFrame>
    {
        JuceOleInPlaceFrame (HWND hwnd)   : window (hwnd) {}

        JUCE_COMRESULT GetWindow (HWND* lphwnd)                                 override { *lphwnd = window; return S_OK; }
        JUCE_COMRESULT ContextSensitiveHelp (BOOL)                              override { return E_NOTIMPL; }
        JUCE_COMRESULT GetBorder (LPRECT)                                       override { return E_NOTIMPL; }
        JUCE_COMRESULT RequestBorderSpace (LPCBORDERWIDTHS)                     override { return E_NOTIMPL; }
        JUCE_COMRESULT SetBorderSpace (LPCBORDERWIDTHS)                         override { return E_NOTIMPL; }
        JUCE_COMRESULT SetActiveObject (IOleInPlaceActiveObject* a, LPCOLESTR)  override { activeObject = addComSmartPtrOwner (a); return S_OK; }
        JUCE_COMRESULT InsertMenus (HMENU, LPOLEMENUGROUPWIDTHS)                override { return E_NOTIMPL; }
        JUCE_COMRESULT SetMenu (HMENU, HOLEMENU, HWND)                          override { return S_OK; }
        JUCE_COMRESULT RemoveMenus (HMENU)                                      override { return E_NOTIMPL; }
        JUCE_COMRESULT SetStatusText (LPCOLESTR)                                override { return S_OK; }
        JUCE_COMRESULT EnableModeless (BOOL)                                    override { return S_OK; }
        JUCE_COMRESULT TranslateAccelerator (LPMSG, WORD)                       override { return E_NOTIMPL; }

        HRESULT OfferKeyTranslation (LPMSG lpmsg)
        {
            if (activeObject != nullptr)
                return activeObject->TranslateAcceleratorW (lpmsg);

            return S_FALSE;
        }

        HWND window;
        ComSmartPtr<IOleInPlaceActiveObject> activeObject;
    };

    //==============================================================================
    struct JuceIOleInPlaceSite final : public ComBaseClassHelper<IOleInPlaceSite>
    {
        JuceIOleInPlaceSite (HWND hwnd)
            : window (hwnd),
              frame (new JuceOleInPlaceFrame (window))
        {}

        ~JuceIOleInPlaceSite() override
        {
            frame->Release();
        }

        JUCE_COMRESULT GetWindow (HWND* lphwnd)      override { *lphwnd = window; return S_OK; }
        JUCE_COMRESULT ContextSensitiveHelp (BOOL)   override { return E_NOTIMPL; }
        JUCE_COMRESULT CanInPlaceActivate()          override { return S_OK; }
        JUCE_COMRESULT OnInPlaceActivate()           override { return S_OK; }
        JUCE_COMRESULT OnUIActivate()                override { return S_OK; }

        JUCE_COMRESULT GetWindowContext (LPOLEINPLACEFRAME* lplpFrame, LPOLEINPLACEUIWINDOW* lplpDoc, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO lpFrameInfo) override
        {
            /* Note: If you call AddRef on the frame here, then some types of object (e.g. web browser control) cause leaks..
               If you don't call AddRef then others crash (e.g. QuickTime).. Bit of a catch-22, so letting it leak is probably preferable.
            */
            if (lplpFrame != nullptr) { frame->AddRef(); *lplpFrame = frame; }
            if (lplpDoc != nullptr)   *lplpDoc = nullptr;
            lpFrameInfo->fMDIApp = FALSE;
            lpFrameInfo->hwndFrame = window;
            lpFrameInfo->haccel = nullptr;
            lpFrameInfo->cAccelEntries = 0;
            return S_OK;
        }

        JUCE_COMRESULT Scroll (SIZE)                 override { return E_NOTIMPL; }
        JUCE_COMRESULT OnUIDeactivate (BOOL)         override { return S_OK; }
        JUCE_COMRESULT OnInPlaceDeactivate()         override { return S_OK; }
        JUCE_COMRESULT DiscardUndoState()            override { return E_NOTIMPL; }
        JUCE_COMRESULT DeactivateAndUndo()           override { return E_NOTIMPL; }
        JUCE_COMRESULT OnPosRectChange (LPCRECT)     override { return S_OK; }

        LRESULT offerEventToActiveXControl (::MSG& msg)
        {
            if (frame != nullptr)
                return frame->OfferKeyTranslation (&msg);

            return S_FALSE;
        }

        HWND window;
        JuceOleInPlaceFrame* frame;
    };

    //==============================================================================
    struct JuceIOleClientSite final : public ComBaseClassHelper<IOleClientSite>
    {
        JuceIOleClientSite (HWND window)  : inplaceSite (new JuceIOleInPlaceSite (window))
        {}

        ~JuceIOleClientSite() override
        {
            inplaceSite->Release();

            if (dispatchEventHandler != nullptr)
            {
                dispatchEventHandler->Release();
                dispatchEventHandler = nullptr;
            }
        }

        JUCE_COMRESULT QueryInterface (REFIID type, void** result) override
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

            if (type == __uuidof (IOleInPlaceSite))
            {
                inplaceSite->AddRef();
                *result = static_cast<IOleInPlaceSite*> (inplaceSite);
                return S_OK;
            }
            else if (type == __uuidof (IDispatch) && dispatchEventHandler != nullptr)
            {
                dispatchEventHandler->AddRef();
                *result = dispatchEventHandler;
                return S_OK;
            }

            return ComBaseClassHelper <IOleClientSite>::QueryInterface (type, result);

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }

        JUCE_COMRESULT SaveObject()                                  override { return E_NOTIMPL; }
        JUCE_COMRESULT GetMoniker (DWORD, DWORD, IMoniker**)         override { return E_NOTIMPL; }
        JUCE_COMRESULT GetContainer (LPOLECONTAINER* ppContainer)    override { *ppContainer = nullptr; return E_NOINTERFACE; }
        JUCE_COMRESULT ShowObject()                                  override { return S_OK; }
        JUCE_COMRESULT OnShowWindow (BOOL)                           override { return E_NOTIMPL; }
        JUCE_COMRESULT RequestNewObjectLayout()                      override { return E_NOTIMPL; }

        LRESULT offerEventToActiveXControl (::MSG& msg)
        {
            if (inplaceSite != nullptr)
                return inplaceSite->offerEventToActiveXControl (msg);

            return S_FALSE;
        }

        void setEventHandler (void* eventHandler)
        {
            IDispatch* newEventHandler = nullptr;

            if (eventHandler != nullptr)
            {
                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

                auto iidIDispatch = __uuidof (IDispatch);

                if (static_cast<IUnknown*> (eventHandler)->QueryInterface (iidIDispatch, (void**) &newEventHandler) != S_OK
                    || newEventHandler == nullptr)
                    return;

               JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            }

            if (dispatchEventHandler != nullptr)
                dispatchEventHandler->Release();

            dispatchEventHandler = newEventHandler;
        }

        JuceIOleInPlaceSite* inplaceSite;
        IDispatch* dispatchEventHandler = nullptr;
    };

    //==============================================================================
    static Array<ActiveXControlComponent*> activeXComps;

    static HWND getHWND (const ActiveXControlComponent* const component)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

        HWND hwnd = {};
        const IID iid = __uuidof (IOleWindow);

        if (auto* window = (IOleWindow*) component->queryInterface (&iid))
        {
            window->GetWindow (&hwnd);
            window->Release();
        }

        return hwnd;

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    static void offerActiveXMouseEventToPeer (ComponentPeer* peer, HWND hwnd, UINT message, LPARAM lParam)
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
            {
                RECT activeXRect, peerRect;
                GetWindowRect (hwnd, &activeXRect);
                GetWindowRect ((HWND) peer->getNativeHandle(), &peerRect);

                peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse,
                                        { (float) (GET_X_LPARAM (lParam) + activeXRect.left - peerRect.left),
                                          (float) (GET_Y_LPARAM (lParam) + activeXRect.top  - peerRect.top) },
                                        ComponentPeer::getCurrentModifiersRealtime(),
                                        MouseInputSource::defaultPressure,
                                        MouseInputSource::defaultOrientation,
                                        getMouseEventTime());
                break;
            }

            default:
                break;
        }
    }
}

//==============================================================================
class ActiveXControlComponent::Pimpl  : public ComponentMovementWatcher,
                                        public ComponentPeer::ScaleFactorListener
{
public:
    Pimpl (HWND hwnd, ActiveXControlComponent& activeXComp)
        : ComponentMovementWatcher (&activeXComp),
          owner (activeXComp),
          storage (new ActiveXHelpers::JuceIStorage()),
          clientSite (new ActiveXHelpers::JuceIOleClientSite (hwnd))
    {
    }

    ~Pimpl() override
    {
        // If the wndproc of the ActiveX HWND isn't set back to it's original
        // wndproc, then clientSite will leak when control is released
        if (controlHWND != nullptr)
            SetWindowLongPtr ((HWND) controlHWND, GWLP_WNDPROC, (LONG_PTR) originalWndProc);

        if (control != nullptr)
        {
            control->Close (OLECLOSE_NOSAVE);
            control->Release();
        }

        clientSite->Release();
        storage->Release();

        if (currentPeer != nullptr)
            currentPeer->removeScaleFactorListener (this);
    }

    void setControlBounds (Rectangle<int> newBounds) const
    {
        if (controlHWND != nullptr)
        {
            if (auto* peer = owner.getTopLevelComponent()->getPeer())
                newBounds = (newBounds.toDouble() * peer->getPlatformScaleFactor()).toNearestInt();

            MoveWindow (controlHWND, newBounds.getX(), newBounds.getY(), newBounds.getWidth(), newBounds.getHeight(), TRUE);
        }
    }

    void setControlVisible (bool shouldBeVisible) const
    {
        if (controlHWND != nullptr)
            ShowWindow (controlHWND, shouldBeVisible ? SW_SHOWNA : SW_HIDE);
    }

    //==============================================================================
    using ComponentMovementWatcher::componentMovedOrResized;

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
            setControlBounds (peer->getAreaCoveredBy (owner));
    }

    void componentPeerChanged() override
    {
        if (currentPeer != nullptr)
            currentPeer->removeScaleFactorListener (this);

        componentMovedOrResized (true, true);

        currentPeer = owner.getTopLevelComponent()->getPeer();

        if (currentPeer != nullptr)
            currentPeer->addScaleFactorListener (this);
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    void componentVisibilityChanged() override
    {
        setControlVisible (owner.isShowing());
        componentPeerChanged();
    }

    void nativeScaleFactorChanged (double /*newScaleFactor*/) override
    {
        componentMovedOrResized (true, true);
    }

    // intercepts events going to an activeX control, so we can sneakily use the mouse events
    static LRESULT CALLBACK activeXHookWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        for (auto* ax : ActiveXHelpers::activeXComps)
        {
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
                            if (auto* peer = ax->getPeer())
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
    ComponentPeer* currentPeer = nullptr;
    HWND controlHWND = {};
    IStorage* storage = nullptr;
    ActiveXHelpers::JuceIOleClientSite* clientSite = nullptr;
    IOleObject* control = nullptr;
    WNDPROC originalWndProc = nullptr;
};

//==============================================================================
ActiveXControlComponent::ActiveXControlComponent()
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

    if (auto* peer = getPeer())
    {
        auto controlBounds = peer->getAreaCoveredBy (*this);
        auto hwnd = (HWND) peer->getNativeHandle();

        std::unique_ptr<Pimpl> newControl (new Pimpl (hwnd, *this));

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

        HRESULT hr = OleCreate (*(const IID*) controlIID, __uuidof (IOleObject), 1 /*OLERENDER_DRAW*/, nullptr,
                                newControl->clientSite, newControl->storage,
                                (void**) &(newControl->control));

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        if (hr == S_OK)
        {
            newControl->control->SetHostNames (L"JUCE", nullptr);

            if (OleSetContainedObject (newControl->control, TRUE) == S_OK)
            {
                RECT rect;
                rect.left   = controlBounds.getX();
                rect.top    = controlBounds.getY();
                rect.right  = controlBounds.getRight();
                rect.bottom = controlBounds.getBottom();

                if (newControl->control->DoVerb (OLEIVERB_SHOW, nullptr, newControl->clientSite, 0, hwnd, &rect) == S_OK)
                {
                    control = std::move (newControl);
                    control->controlHWND = ActiveXHelpers::getHWND (this);

                    if (control->controlHWND != nullptr)
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

intptr_t ActiveXControlComponent::offerEventToActiveXControl (void* ptr)
{
    if (control != nullptr && control->clientSite != nullptr)
        return (intptr_t) control->clientSite->offerEventToActiveXControl (*reinterpret_cast<::MSG*> (ptr));

    return S_FALSE;
}

intptr_t ActiveXControlComponent::offerEventToActiveXControlStatic (void* ptr)
{
    for (auto* ax : ActiveXHelpers::activeXComps)
    {
        auto result = ax->offerEventToActiveXControl (ptr);

        if (result != S_FALSE)
            return result;
    }

    return S_FALSE;
}

void ActiveXControlComponent::setEventHandler (void* eventHandler)
{
    if (control->clientSite != nullptr)
        control->clientSite->setEventHandler (eventHandler);
}

LRESULT juce_offerEventToActiveXControl (::MSG& msg);
LRESULT juce_offerEventToActiveXControl (::MSG& msg)
{
    if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
        return ActiveXControlComponent::offerEventToActiveXControlStatic (&msg);

    return S_FALSE;
}

} // namespace juce
