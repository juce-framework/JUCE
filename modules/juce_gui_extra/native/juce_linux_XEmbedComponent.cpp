/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

//==============================================================================
bool juce_handleXEmbedEvent (ComponentPeer*, void*);
Window juce_getCurrentFocusWindow (ComponentPeer*);

//==============================================================================
unsigned long juce_createKeyProxyWindow (ComponentPeer*);
void juce_deleteKeyProxyWindow (ComponentPeer*);

//==============================================================================
class XEmbedComponent::Pimpl : private ComponentListener
{
public:
    //==============================================================================
    enum
    {
        maxXEmbedVersionToSupport = 0
    };

    enum Flags
    {
        XEMBED_MAPPED  = (1<<0)
    };

    enum
    {
        XEMBED_EMBEDDED_NOTIFY        = 0,
        XEMBED_WINDOW_ACTIVATE        = 1,
        XEMBED_WINDOW_DEACTIVATE      = 2,
        XEMBED_REQUEST_FOCUS          = 3,
        XEMBED_FOCUS_IN               = 4,
        XEMBED_FOCUS_OUT              = 5,
        XEMBED_FOCUS_NEXT             = 6,
        XEMBED_FOCUS_PREV             = 7,
        XEMBED_MODALITY_ON            = 10,
        XEMBED_MODALITY_OFF           = 11,
        XEMBED_REGISTER_ACCELERATOR   = 12,
        XEMBED_UNREGISTER_ACCELERATOR = 13,
        XEMBED_ACTIVATE_ACCELERATOR   = 14
    };

    enum
    {
        XEMBED_FOCUS_CURRENT = 0,
        XEMBED_FOCUS_FIRST   = 1,
        XEMBED_FOCUS_LAST    = 2
    };

    //==============================================================================
    class SharedKeyWindow
    {
    public:
        //==============================================================================
        class Ref
        {
        public:
            Ref() : keyWindow (nullptr) {}
            Ref (Pimpl& p) { keyWindow = getKeyWindowForPeer (p.owner.getPeer()); }
            ~Ref() { free(); }

            //==============================================================================
            Ref (const Ref& o)   : keyWindow (o.keyWindow) { if (keyWindow != nullptr) keyWindow->numRefs++; }
            Ref (Ref && o)       : keyWindow (o.keyWindow) { o.keyWindow = nullptr; }
            Ref (std::nullptr_t) : keyWindow (nullptr) {}

            //==============================================================================
            Ref& operator= (std::nullptr_t) { free(); return *this; }
            Ref& operator= (const Ref& o)
            {
                free();
                keyWindow = o.keyWindow;
                if (keyWindow != nullptr)
                    keyWindow->numRefs++;

                return *this;
            }

            Ref& operator= (Ref && o)
            {
                if (keyWindow != o.keyWindow)
                {
                    free();
                    keyWindow = o.keyWindow;
                }

                o.keyWindow = nullptr;
                return *this;
            }

            //==============================================================================
            SharedKeyWindow& operator*()  noexcept  { return *keyWindow; }
            SharedKeyWindow* operator->() noexcept  { return  keyWindow; }

            //==============================================================================
            bool operator== (std::nullptr_t) const noexcept  { return (keyWindow == nullptr); }
            bool operator!= (std::nullptr_t) const noexcept  { return (keyWindow != nullptr); }
        private:
            //==============================================================================
            void free()
            {
                if (keyWindow != nullptr)
                {
                    if (--keyWindow->numRefs == 0)
                        delete keyWindow;

                    keyWindow = nullptr;
                }
            }

            SharedKeyWindow* keyWindow;
        };

    public:
        //==============================================================================
        Window getHandle()    { return keyProxy; }

        static Window getCurrentFocusWindow (ComponentPeer* peerToLookFor)
        {
            if (keyWindows != nullptr && peerToLookFor != nullptr)
            {
                SharedKeyWindow* foundKeyWindow = (*keyWindows)[peerToLookFor];

                if (foundKeyWindow != nullptr)
                    return foundKeyWindow->keyProxy;
            }

            return (Window)0;
        }

    private:
        //==============================================================================
        friend class Ref;

        SharedKeyWindow (ComponentPeer* peerToUse)
            : keyPeer (peerToUse),
              keyProxy (juce_createKeyProxyWindow (keyPeer)),
              numRefs (1)
        {}

        ~SharedKeyWindow()
        {
            juce_deleteKeyProxyWindow (keyPeer);

            if (keyWindows != nullptr)
            {
                keyWindows->remove (keyPeer);
                if (keyWindows->size() == 0)
                {
                    delete keyWindows;
                    keyWindows = nullptr;
                }
            }
        }

        ComponentPeer* keyPeer;
        Window keyProxy;
        int numRefs;

        static SharedKeyWindow* getKeyWindowForPeer (ComponentPeer* peerToLookFor)
        {
            jassert (peerToLookFor != nullptr);

            if (keyWindows == nullptr)
                keyWindows = new HashMap<ComponentPeer*,SharedKeyWindow*>;

            SharedKeyWindow* foundKeyWindow = (*keyWindows)[peerToLookFor];
            if (foundKeyWindow == nullptr)
            {
                foundKeyWindow = new SharedKeyWindow (peerToLookFor);
                keyWindows->set (peerToLookFor, foundKeyWindow);
            }

            return foundKeyWindow;
        }

        //==============================================================================
        friend class Ref;
        static HashMap<ComponentPeer*,SharedKeyWindow*>* keyWindows;
    };

public:
    //==============================================================================
    Pimpl (XEmbedComponent& parent, Window x11Window,
           bool wantsKeyboardFocus, bool isClientInitiated, bool shouldAllowResize)
        : owner (parent), atoms (x11display.get()), clientInitiated (isClientInitiated),
          wantsFocus (wantsKeyboardFocus), allowResize (shouldAllowResize)
    {
        if (widgets == nullptr)
            widgets = new Array<Pimpl*>;

        widgets->add (this);

        createHostWindow();

        if (clientInitiated)
            setClient (x11Window, true);

        owner.setWantsKeyboardFocus (wantsFocus);
        owner.addComponentListener (this);
    }

    ~Pimpl()
    {
        owner.removeComponentListener (this);
        setClient (0, true);

        if (host != 0)
        {
            Display* dpy = getDisplay();
            XDestroyWindow (dpy, host);
            XSync (dpy, false);

            const long mask = NoEventMask | KeyPressMask | KeyReleaseMask
                            | EnterWindowMask | LeaveWindowMask | PointerMotionMask
                            | KeymapStateMask | ExposureMask | StructureNotifyMask
                            | FocusChangeMask;

            XEvent event;
            while (XCheckWindowEvent (dpy, host, mask, &event) == True)
            {}

            host = 0;
        }

        if (widgets != nullptr)
        {
            widgets->removeAllInstancesOf (this);

            if (widgets->size() == 0)
            {
                delete widgets;
                widgets = nullptr;
            }
        }
    }
    //==============================================================================
    void setClient (Window xembedClient, bool shouldReparent)
    {
        removeClient();

        if (xembedClient != 0)
        {
            Display* dpy = getDisplay();

            client = xembedClient;

            // if the client has initiated the component then keep the clients size
            // otherwise the client should use the host's window' size
            if (clientInitiated)
            {
                configureNotify();
            }
            else
            {
                Rectangle<int> newBounds = getX11BoundsFromJuce();
                XResizeWindow (dpy, client, static_cast<unsigned int> (newBounds.getWidth()),
                                            static_cast<unsigned int> (newBounds.getHeight()));
            }

            XSelectInput (dpy, client, StructureNotifyMask | PropertyChangeMask | FocusChangeMask);
            getXEmbedMappedFlag();

            if (shouldReparent)
                XReparentWindow (dpy, client, host, 0, 0);

            if (supportsXembed)
                sendXEmbedEvent (CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, (long) host, xembedVersion);

            updateMapping();
        }
    }

    void focusGained (FocusChangeType changeType)
    {
        if (client != 0 && supportsXembed && wantsFocus)
        {
            updateKeyFocus();
            sendXEmbedEvent (CurrentTime, XEMBED_FOCUS_IN,
                             (changeType == focusChangedByTabKey ? XEMBED_FOCUS_FIRST : XEMBED_FOCUS_CURRENT));
        }
    }

    void focusLost (FocusChangeType)
    {
        if (client != 0 && supportsXembed && wantsFocus)
        {
            sendXEmbedEvent (CurrentTime, XEMBED_FOCUS_OUT);
            updateKeyFocus();
        }
    }

    void broughtToFront()
    {
        if (client != 0 && supportsXembed)
            sendXEmbedEvent (CurrentTime, XEMBED_WINDOW_ACTIVATE);
    }

    unsigned long getHostWindowID()
    {
        // You are using the client initiated version of the protocol. You cannot
        // retrieve the window id of the host. Please read the documentation for
        // the XEmebedComponent class.
        jassert (! clientInitiated);

        return host;
    }

private:
    //==============================================================================
    XEmbedComponent& owner;
    Window client = 0, host = 0;

    ScopedXDisplay x11display;
    Atoms atoms;

    bool clientInitiated;
    bool wantsFocus        = false;
    bool allowResize       = false;
    bool supportsXembed    = false;
    bool hasBeenMapped     = false;
    int xembedVersion      = maxXEmbedVersionToSupport;

    ComponentPeer* lastPeer = nullptr;
    SharedKeyWindow::Ref keyWindow;

    //==============================================================================
    void componentParentHierarchyChanged (Component&) override   { peerChanged (owner.getPeer()); }
    void componentMovedOrResized (Component&, bool, bool) override
    {
        if (host != 0 && lastPeer != nullptr)
        {
            Display* dpy = getDisplay();
            Rectangle<int> newBounds = getX11BoundsFromJuce();
            XWindowAttributes attr;

            if (XGetWindowAttributes (dpy, host, &attr))
            {
                Rectangle<int> currentBounds (attr.x, attr.y, attr.width, attr.height);
                if (currentBounds != newBounds)
                {
                    XMoveResizeWindow (dpy, host, newBounds.getX(), newBounds.getY(),
                                       static_cast<unsigned int> (newBounds.getWidth()),
                                       static_cast<unsigned int> (newBounds.getHeight()));

                    if (client != 0 && (currentBounds.getWidth() != newBounds.getWidth()
                                        || currentBounds.getHeight() != newBounds.getHeight()))
                        XResizeWindow (dpy, client,
                                       static_cast<unsigned int> (newBounds.getWidth()),
                                       static_cast<unsigned int> (newBounds.getHeight()));
                }
            }
        }
    }

    //==============================================================================
    void createHostWindow()
    {
        Display* dpy = getDisplay();
        int defaultScreen = XDefaultScreen (dpy);
        Window root = RootWindow (dpy, defaultScreen);

        XSetWindowAttributes swa;
        swa.border_pixel = 0;
        swa.background_pixmap = None;
        swa.override_redirect = True;
        swa.event_mask = SubstructureNotifyMask | StructureNotifyMask | FocusChangeMask;

        host = XCreateWindow (dpy, root, 0, 0, 1, 1, 0, CopyFromParent,
                              InputOutput, CopyFromParent,
                              CWEventMask | CWBorderPixel | CWBackPixmap | CWOverrideRedirect,
                              &swa);
    }

    void removeClient()
    {
        if (client != 0)
        {
            Display* dpy = getDisplay();
            XSelectInput (dpy, client, 0);

            keyWindow = nullptr;

            int defaultScreen = XDefaultScreen (dpy);
            Window root = RootWindow (dpy, defaultScreen);

            if (hasBeenMapped)
            {
                XUnmapWindow (dpy, client);
                hasBeenMapped = false;
            }

            XReparentWindow (dpy, client, root, 0, 0);
            client = 0;
        }
    }

    void updateMapping()
    {
        if (client != 0)
        {
            const bool shouldBeMapped = getXEmbedMappedFlag();
            if (shouldBeMapped != hasBeenMapped)
            {
                hasBeenMapped = shouldBeMapped;

                if (shouldBeMapped)
                    XMapWindow (getDisplay(), client);
                else
                    XUnmapWindow (getDisplay(), client);
            }
        }
    }

    Window getParentX11Window()
    {
        if (ComponentPeer* peer = owner.getPeer())
            return reinterpret_cast<Window> (peer->getNativeHandle());

        return 0;
    }

    Display* getDisplay()   { return reinterpret_cast<Display*> (x11display.get()); }

    //==============================================================================
    bool getXEmbedMappedFlag()
    {
        GetXProperty embedInfo (x11display.get(), client, atoms.XembedInfo, 0, 2, false, atoms.XembedInfo);
        if (embedInfo.success && embedInfo.actualFormat == 32
            && embedInfo.numItems >= 2 && embedInfo.data != nullptr)
        {
            long* buffer = (long*) embedInfo.data;

            supportsXembed = true;
            xembedVersion = jmin ((int) maxXEmbedVersionToSupport, (int) buffer[0]);

            return ((buffer[1] & XEMBED_MAPPED) != 0);
        }
        else
        {
            supportsXembed = false;
            xembedVersion = maxXEmbedVersionToSupport;
        }

        return true;
    }

    //==============================================================================
    void propertyChanged (const Atom& a)
    {
        if (a == atoms.XembedInfo)
            updateMapping();
    }

    void configureNotify()
    {
        XWindowAttributes attr;
        Display* dpy = getDisplay();

        if (XGetWindowAttributes (dpy, client, &attr))
        {
            XWindowAttributes hostAttr;

            if (XGetWindowAttributes (dpy, host, &hostAttr))
                if (attr.width != hostAttr.width || attr.height != hostAttr.height)
                    XResizeWindow (dpy, host, (unsigned int) attr.width, (unsigned int) attr.height);

            // as the client window is not on any screen yet, we need to guess
            // on which screen it might appear to get a scaling factor :-(
            const Desktop::Displays& displays = Desktop::getInstance().getDisplays();
            ComponentPeer* peer = owner.getPeer();
            const double scale = (peer != nullptr ? displays.getDisplayContaining (peer->getBounds().getCentre())
                                  : displays.getMainDisplay()).scale;

            Point<int> topLeftInPeer
                = (peer != nullptr ? peer->getComponent().getLocalPoint (&owner, Point<int> (0, 0))
                   : owner.getBounds().getTopLeft());

            Rectangle<int> newBounds (topLeftInPeer.getX(), topLeftInPeer.getY(),
                                      static_cast<int> (static_cast<double> (attr.width)  / scale),
                                      static_cast<int> (static_cast<double> (attr.height) / scale));


            if (peer != nullptr)
                newBounds = owner.getLocalArea (&peer->getComponent(), newBounds);

            jassert (newBounds.getX() == 0 && newBounds.getY() == 0);

            if (newBounds != owner.getLocalBounds())
                owner.setSize (newBounds.getWidth(), newBounds.getHeight());
        }
    }

    void peerChanged (ComponentPeer* newPeer)
    {
        if (newPeer != lastPeer)
        {
            if (lastPeer != nullptr)
                keyWindow = nullptr;

            Display* dpy = getDisplay();
            Window rootWindow = RootWindow (dpy, DefaultScreen (dpy));
            Rectangle<int> newBounds = getX11BoundsFromJuce();

            if (newPeer == nullptr)
                XUnmapWindow (dpy, host);

            Window newParent = (newPeer != nullptr ? getParentX11Window() : rootWindow);
            XReparentWindow (dpy, host, newParent, newBounds.getX(), newBounds.getY());

            lastPeer = newPeer;

            if (newPeer != nullptr)
            {
                if (wantsFocus)
                {
                    keyWindow = SharedKeyWindow::Ref (*this);
                    updateKeyFocus();
                }

                componentMovedOrResized (owner, true, true);
                XMapWindow (dpy, host);

                broughtToFront();
            }
        }
    }

    void updateKeyFocus()
    {
        if (lastPeer != nullptr && lastPeer->isFocused())
            XSetInputFocus (getDisplay(), getCurrentFocusWindow (lastPeer), RevertToParent, CurrentTime);
    }

    //==============================================================================
    void handleXembedCmd (const ::Time& /*xTime*/, long opcode, long /*detail*/, long /*data1*/, long /*data2*/)
    {
        switch (opcode)
        {
        case XEMBED_REQUEST_FOCUS:
            if (wantsFocus)
                owner.grabKeyboardFocus();
            break;
        case XEMBED_FOCUS_NEXT:
            if (wantsFocus)
                owner.moveKeyboardFocusToSibling (true);
            break;
        case XEMBED_FOCUS_PREV:
            if (wantsFocus)
                owner.moveKeyboardFocusToSibling (false);
            break;
        }
    }

    bool handleX11Event (const XEvent& e)
    {
        if (e.xany.window == client && client != 0)
        {
            switch (e.type)
            {
            case PropertyNotify:
                propertyChanged (e.xproperty.atom);
                return true;
            case ConfigureNotify:
                if (allowResize)
                    configureNotify();
                else
                    MessageManager::callAsync([this] () {componentMovedOrResized (owner, true, true);});

                return true;
            }
        }
        else if (e.xany.window == host && host != 0)
        {
            switch (e.type)
            {
            case ReparentNotify:
                if (e.xreparent.parent == host && e.xreparent.window != client)
                {
                    setClient (e.xreparent.window, false);
                    return true;
                }
                break;
            case CreateNotify:
                if (e.xcreatewindow.parent != e.xcreatewindow.window && e.xcreatewindow.parent == host && e.xcreatewindow.window != client)
                {
                    setClient (e.xcreatewindow.window, false);
                    return true;
                }
                break;
            case GravityNotify:
                componentMovedOrResized (owner, true, true);
                return true;
            case ClientMessage:
                if (e.xclient.message_type == atoms.XembedMsgType && e.xclient.format == 32)
                {
                    handleXembedCmd ((::Time) e.xclient.data.l[0], e.xclient.data.l[1],
                                     e.xclient.data.l[2], e.xclient.data.l[3],
                                     e.xclient.data.l[4]);

                    return true;
                }
                break;

            }
        }

        return false;
    }

    void sendXEmbedEvent (const ::Time& xTime, long opcode,
                          long opcodeMinor = 0, long data1 = 0, long data2 = 0)
    {
        XClientMessageEvent msg;
        Display* dpy = getDisplay();

        ::memset (&msg, 0, sizeof (XClientMessageEvent));
        msg.window = client;
        msg.type = ClientMessage;
        msg.message_type = atoms.XembedMsgType;
        msg.format = 32;
        msg.data.l[0] = (long) xTime;
        msg.data.l[1] = opcode;
        msg.data.l[2] = opcodeMinor;
        msg.data.l[3] = data1;
        msg.data.l[4] = data2;

        XSendEvent (dpy, client, False, NoEventMask, (XEvent*) &msg);
        XSync (dpy, False);
    }

    Rectangle<int> getX11BoundsFromJuce()
    {
        if (ComponentPeer* peer = owner.getPeer())
        {
            Rectangle<int> r
                = peer->getComponent().getLocalArea (&owner, owner.getLocalBounds());

            const double scale
                = Desktop::getInstance().getDisplays().getDisplayContaining (peer->localToGlobal (r.getCentre())).scale;

            return r * scale;
        }

        return owner.getLocalBounds();
    }

    //==============================================================================
    friend bool juce::juce_handleXEmbedEvent (ComponentPeer*, void*);
    friend unsigned long juce::juce_getCurrentFocusWindow (ComponentPeer*);

    static Array<Pimpl*>* widgets;

    static bool dispatchX11Event (ComponentPeer* p, const XEvent* eventArg)
    {
        if (widgets != nullptr)
        {
            if (eventArg != nullptr)
            {
                const XEvent& e = *eventArg;
                Window w = e.xany.window;

                if (w == 0) return false;

                for (auto && widget : *widgets)
                    if (w == widget->host || w == widget->client)
                        return widget->handleX11Event (e);
            }
            else
            {
                for (auto && widget : *widgets)
                {
                    if (widget->owner.getPeer() == p)
                        widget->peerChanged (nullptr);
                }
            }
        }

        return false;
    }

    static Window getCurrentFocusWindow (ComponentPeer* p)
    {
        if (widgets != nullptr && p != nullptr)
        {
            for (auto && widget : *widgets)
                if (widget->owner.getPeer() == p && widget->owner.hasKeyboardFocus (false))
                    return widget->client;
        }

        return SharedKeyWindow::getCurrentFocusWindow (p);
    }
};

//==============================================================================
Array<XEmbedComponent::Pimpl*>* XEmbedComponent::Pimpl::widgets = nullptr;
HashMap<ComponentPeer*,XEmbedComponent::Pimpl::SharedKeyWindow*>* XEmbedComponent::Pimpl::SharedKeyWindow::keyWindows = nullptr;

//==============================================================================
XEmbedComponent::XEmbedComponent (bool wantsKeyboardFocus, bool allowForeignWidgetToResizeComponent)
    : pimpl (new Pimpl (*this, 0, wantsKeyboardFocus, false, allowForeignWidgetToResizeComponent))
{
    setOpaque (true);
}

XEmbedComponent::XEmbedComponent (unsigned long wID, bool wantsKeyboardFocus, bool allowForeignWidgetToResizeComponent)
    : pimpl (new Pimpl (*this, wID, wantsKeyboardFocus, true, allowForeignWidgetToResizeComponent))
{
    setOpaque (true);
}

XEmbedComponent::~XEmbedComponent() {}

void XEmbedComponent::paint (Graphics& g)
{
    g.fillAll (Colours::lightgrey);
}

void XEmbedComponent::focusGained (FocusChangeType changeType)     { pimpl->focusGained (changeType); }
void XEmbedComponent::focusLost   (FocusChangeType changeType)     { pimpl->focusLost   (changeType); }
void XEmbedComponent::broughtToFront()                             { pimpl->broughtToFront(); }
unsigned long XEmbedComponent::getHostWindowID()                   { return pimpl->getHostWindowID(); }

//==============================================================================
bool juce_handleXEmbedEvent (ComponentPeer* p, void* e)
{
    return ::XEmbedComponent::Pimpl::dispatchX11Event (p, reinterpret_cast<const XEvent*> (e));
}

unsigned long juce_getCurrentFocusWindow (ComponentPeer* peer)
{
    return (unsigned long) ::XEmbedComponent::Pimpl::getCurrentFocusWindow (peer);
}
